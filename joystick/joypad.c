/*
	Atari joypad manager, using hardware access

	Copyright (C) 2002	Patrice Mandin

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <stdio.h>
#include <string.h>

#include <mint/osbind.h>
#include <mint/cookie.h>

/*--- Joypad I/O read/write interface ---*/

#define JOYPAD_IO_BASE (0xffff9200)
struct JOYPAD_IO_S {
	unsigned short fires;		/* 00 */
	unsigned short directions;	/* 02 */
	unsigned short dummy1[6];	/* 04,06,08,0a,0c,0e */
	unsigned short paddles[4];	/* 10,12,14,16 */
	unsigned short dummy2[4];	/* 18,1a,1c,1e */
	unsigned short lightpens[2];	/* 20,22 */ 
} __attribute__((packed));
#define JOYPAD_IO ((*(volatile struct JOYPAD_IO_S *)JOYPAD_IO_BASE))

/* Joysticks buttons+directions */
#define PORT_JS_RIGHT	(1<<0)
#define PORT_JS_LEFT	(1<<1)
#define PORT_JS_DOWN	(1<<2)
#define PORT_JS_UP		(1<<3)

/* Joypads buttons+directions */
enum {
	JP_UP=0,	JP_DOWN,	JP_LEFT,	JP_RIGHT,
	JP_KPMULT,	JP_KP7,		JP_KP4,		JP_KP1,
	JP_KP0,		JP_KP8,		JP_KP5,		JP_KP2,
	JP_KPNUM,	JP_KP9,		JP_KP6,		JP_KP3,
	JP_PAUSE,	JP_FIRE0,	JP_UNDEF0,	JP_FIRE1,
	JP_UNDEF1,	JP_FIRE2,	JP_UNDEF2,	JP_OPTION
};

/* Masks to read ports */
static const unsigned short joypad_masks[8*4]={
	0xfffe, 0xfffd, 0xfffb, 0xfff7,
	0xfff0, 0xfff1, 0xfff2, 0xfff3,
	0xfff4, 0xfff5, 0xfff6, 0xfff8,
	0xfff9, 0xfffa, 0xfffc, 0xffff,
	0xffef, 0xffdf, 0xffbf, 0xff7f,
	0xff0f, 0xff1f, 0xff2f, 0xff3f,
	0xff4f, 0xff5f, 0xff6f, 0xff8f,
	0xff9f, 0xffaf, 0xffcf, 0xffff
};

/*--- Defines ---*/

enum {	/*  _MCH cookie */
	MCH_ST=0,
	MCH_STE,
	MCH_TT,
	MCH_F30,
	MCH_ARANYM
};

#define SCANCODE_ESC	0x01

/*--- Variables ---*/

short jp_paddles[4];
short jp_lightpens[2];
unsigned short jp_directions;
unsigned short jp_fires;
unsigned long jp_joypads[8];

short prev_jp_paddles[4];
short prev_jp_lightpens[2];
unsigned short prev_jp_directions;
unsigned short prev_jp_fires;
unsigned long prev_jp_joypads[8];

long prev_raw_lightpens[2], prev_raw_paddles[4], prev_raw_joypads[8*8];
long raw_lightpens[2], raw_paddles[4], raw_joypads[8*8];
char *raw_names[8]={
	"fire, line 0 (pause, fire 0)",
	"direction, line 0 (up, down, left, right)",
	"fire, line 1 (-, fire 1)",
	"direction, line 1 (*, 7, 4, 1)",
	"fire, line 2 (-, fire 2)",
	"direction, line 2 (0, 8, 5, 2)",
	"fire, line 3 (-, option)",
	"direction, line 3 (#, 9, 6, 3)",
};

/*--- Functions prototypes ---*/

void DisplayLogicalValues(void);
void DisplayRawValues(void);
void UpdateJoypads(void);

/*--- Functions ---*/

int main(int argc, char **argv)
{
	void *oldstack;
	unsigned long key_pressed;
	unsigned char scancode;
	int i;

	unsigned long cookie_mch;

	/* Availability: STE, Falcon */
	if (Getcookie(C__MCH, &cookie_mch) != C_FOUND) {
		cookie_mch = MCH_ST << 16;	/* No cookie = ST */
	}

	if ((cookie_mch != MCH_STE<<16) && (cookie_mch != MCH_F30<<16)) {
		printf("No joypad hardware present\n");
	}

	/* Reset variables */
	for (i=0;i<4; i++) {
		jp_paddles[i] = prev_jp_paddles[i] = 0;
	}
	for (i=0;i<2; i++) {
		jp_lightpens[i] = prev_jp_lightpens[i] = 0;
		jp_joypads[i] = prev_jp_joypads[i] = 0;
	}
	jp_directions = prev_jp_directions = 0;
	jp_fires = prev_jp_fires = 0;

	for (i=0; i<2; i++) {
		prev_raw_lightpens[i] = raw_lightpens[i] = 0;
	}
	for (i=0; i<4; i++) {
		prev_raw_paddles[i] = raw_paddles[i] = 0;
	}
	for (i=0; i<8*8; i++) {
		prev_raw_joypads[i] = raw_joypads[i] = 0;
	}

	/* Wait till ESC key pressed */
	key_pressed = scancode = 0;

	printf("Press ESC to quit\n");
	Supexec(UpdateJoypads);

	while (scancode != SCANCODE_ESC) {

		/* Read key pressed, if needed */
		if (Cconis()) {
			key_pressed = Cnecin();
			scancode = (key_pressed >>16) & 0xff;
		}

		Supexec(UpdateJoypads);

		DisplayLogicalValues();
/*		DisplayRawValues();*/
	}
}

/* Interpret values */
void DisplayLogicalValues(void)
{
	int i;

	/* Joysticks (4 joysticks, 2 axis, 1 button) */
	if ((jp_fires != prev_jp_fires) || (jp_directions != prev_jp_directions)) {
		for (i=0;i<4;i++) {
			int fire, directions;

			fire = (jp_fires >> i) & 1;
			directions = (jp_directions >> (i<<2)) & 15;

			printf("Joystick %d: ",i);
			if (directions & PORT_JS_UP) {
				printf("up ");
			}
			if (directions & PORT_JS_DOWN) {
				printf("down ");
			}
			if (directions & PORT_JS_LEFT) {
				printf("left ");
			}
			if (directions & PORT_JS_RIGHT) {
				printf("right ");
			}
			if (fire) {
				printf("fire ");
			}
			printf("\n");
		}

		prev_jp_fires = jp_fires;
		prev_jp_directions = jp_directions;
	}

	/* Lightpens (1 lightpen, 2 axis, 0 buttons) */
	if ((jp_lightpens[0] != prev_jp_lightpens[0]) || (jp_lightpens[1] != prev_jp_lightpens[1])) {
		printf("Lightpen: X=%d, Y=%d\n",jp_lightpens[0],jp_lightpens[1]);

		prev_jp_lightpens[0] = jp_lightpens[0];
		prev_jp_lightpens[1] = jp_lightpens[1];
	}

	/* Analog paddle (2 paddles, 2 axis, 0 buttons) */
	if ((jp_paddles[0] != prev_jp_paddles[0]) || (jp_paddles[1] != prev_jp_paddles[1])) {
		printf("Paddle 0: X=%d, Y=%d\n",jp_paddles[0],jp_paddles[1]);

		prev_jp_paddles[0] = jp_paddles[0];
		prev_jp_paddles[1] = jp_paddles[1];
	}
	if ((jp_paddles[2] != prev_jp_paddles[2]) || (jp_paddles[3] != prev_jp_paddles[3])) {
		printf("Paddle 1: X=%d, Y=%d\n",jp_paddles[2],jp_paddles[3]);

		prev_jp_paddles[2] = jp_paddles[2];
		prev_jp_paddles[3] = jp_paddles[3];
	}

	/* Joypads (8 joypads, 2 axis, 17 buttons) */ 
	for (i=0;i<8;i++) {
		if (jp_joypads[i] != prev_jp_joypads[i]) {
			printf("Joypad %d: ",i);

			if (jp_joypads[i] & (1<<JP_UP)) {
				printf("up ");
			}
			if (jp_joypads[i] & (1<<JP_DOWN)) {
				printf("down ");
			}
			if (jp_joypads[i] & (1<<JP_LEFT)) {
				printf("left ");
			}
			if (jp_joypads[i] & (1<<JP_RIGHT)) {
				printf("right ");
			}

			if (jp_joypads[i] & (1<<JP_PAUSE)) {
				printf("pause ");
			}
			if (jp_joypads[i] & (1<<JP_OPTION)) {
				printf("option ");
			}

			if (jp_joypads[i] & (1<<JP_FIRE0)) {
				printf("a ");
			}
			if (jp_joypads[i] & (1<<JP_FIRE1)) {
				printf("b ");
			}
			if (jp_joypads[i] & (1<<JP_FIRE2)) {
				printf("c ");
			}

			if (jp_joypads[i] & (1<<JP_KPNUM)) {
				printf("# ");
			}
			if (jp_joypads[i] & (1<<JP_KPMULT)) {
				printf("* ");
			}

			if (jp_joypads[i] & (1<<JP_KP0)) {
				printf("0 ");
			}
			if (jp_joypads[i] & (1<<JP_KP1)) {
				printf("1 ");
			}
			if (jp_joypads[i] & (1<<JP_KP2)) {
				printf("2 ");
			}
			if (jp_joypads[i] & (1<<JP_KP3)) {
				printf("3 ");
			}
			if (jp_joypads[i] & (1<<JP_KP4)) {
				printf("4 ");
			}
			if (jp_joypads[i] & (1<<JP_KP5)) {
				printf("5 ");
			}
			if (jp_joypads[i] & (1<<JP_KP6)) {
				printf("6 ");
			}
			if (jp_joypads[i] & (1<<JP_KP7)) {
				printf("7 ");
			}
			if (jp_joypads[i] & (1<<JP_KP8)) {
				printf("8 ");
			}
			if (jp_joypads[i] & (1<<JP_KP9)) {
				printf("9 ");
			}

			printf("\n");

			prev_jp_joypads[i] = jp_joypads[i];
		}
	}
}

/* Display raw values */
void DisplayRawValues(void)
{
	int i;

	for (i=0; i<2; i++) {
		if (prev_raw_lightpens[i] != raw_lightpens[i]) {
			printf("Lightpen: %c=%d\n", (i==0) ? 'X' : 'Y', raw_lightpens[i]);
			prev_raw_lightpens[i] = raw_lightpens[i];
		}
	}

	for (i=0; i<4; i++) {
		if (prev_raw_paddles[i] != raw_paddles[i]) {
			printf("Paddle %d: %c=%d\n", i>>1, ((i&1)==0) ? 'X' : 'Y', raw_paddles[i]);
			prev_raw_paddles[i] = raw_paddles[i];
		}
	}

	for (i=0; i<8*8; i++) {
		if (prev_raw_joypads[i] != raw_joypads[i]) {
			printf("Joypad %d: %s=0x%04x\n", (i>>3), raw_names[i&7], raw_joypads[i]);
			prev_raw_joypads[i] = raw_joypads[i];
		}
	}
}

/* Update joypads values (supervisor mode required) */

void UpdateJoypads(void)
{
	unsigned short tmp;
	int i,j,cur_dir,cur_fire;

	/* Update joysticks */
	jp_fires = (~(JOYPAD_IO.fires)) & 15;
	jp_directions = (~(JOYPAD_IO.directions));
	
	/* Update lightpen */
	tmp = raw_lightpens[0] = JOYPAD_IO.lightpens[0] & 1023;
	jp_lightpens[0] = ((tmp<<6) | (tmp>>4)) ^ 0x8000;
	tmp = raw_lightpens[1] = JOYPAD_IO.lightpens[1] & 1023;
	jp_lightpens[1] = ((tmp<<6) | (tmp>>4)) ^ 0x8000;
	
	/* Update paddles */
	tmp = raw_paddles[0] = JOYPAD_IO.paddles[0] & 255;
	jp_paddles[0] = ((tmp<<8) | tmp) ^ 0x8000;
	tmp = raw_paddles[1] = JOYPAD_IO.paddles[1] & 255;
	jp_paddles[1] = ((tmp<<8) | tmp) ^ 0x8000;
	tmp = raw_paddles[2] = JOYPAD_IO.paddles[2] & 255;
	jp_paddles[2] = ((tmp<<8) | tmp) ^ 0x8000;
	tmp = raw_paddles[3] = JOYPAD_IO.paddles[3] & 255;
	jp_paddles[3] = ((tmp<<8) | tmp) ^ 0x8000;

	/* Update teamtap 0 */	
	for (i=0; i<4; i++) {
		jp_joypads[i] = 0;
		for (j=0; j<4; j++) {
			JOYPAD_IO.directions = joypad_masks[(i*4)+j];

			raw_joypads[(i<<3)+(j<<1)+0] = cur_fire = (~(JOYPAD_IO.fires) & 3)<<16;
			raw_joypads[(i<<3)+(j<<1)+1] = cur_dir = (~(JOYPAD_IO.directions)>>8) & 15;

			jp_joypads[i] |= cur_fire<<(j*2);
			jp_joypads[i] |= cur_dir<<(j*4);
		}
	}

	/* Update teamtap 1 */	
	for (i=4; i<8; i++) {
		jp_joypads[i] = 0;
		for (j=0; j<4; j++) {
			JOYPAD_IO.directions = joypad_masks[(i*4)+j];

			raw_joypads[(i<<3)+(j<<1)+0] = cur_fire = (~(JOYPAD_IO.fires) & 0xc)<<14;
			raw_joypads[(i<<3)+(j<<1)+1] = cur_dir = (~(JOYPAD_IO.directions)>>12) & 15;

			jp_joypads[i] |= cur_fire<<(j*2);
			jp_joypads[i] |= cur_dir<<(j*4);
		}
	}

	JOYPAD_IO.directions=0xffff;
}
