/*
	Video demo, using Xbios

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
#include <stdlib.h>
#include <string.h>

#include <mint/osbind.h>
#include <mint/cookie.h>
#include <mint/falcon.h>

#include "mvdi.h"
#include "cscreen.h"
#include "overscan.h"
#include "vinside.h"
#include "blowup.h"
#include "nova.h"
#include "sblast3.h"
#include "sblast2.h"
#include "videlity.h"
#include "vscr.h"

#include "rgb.h"
#include "param.h"

/*--- Defines ---*/

#define SCANCODE_ESC	0x01

/* _VDO cookie values */
enum {
	VDO_ST=0,
	VDO_STE,
	VDO_TT,
	VDO_F30
};

/* EgetShift() masks */
#define ES_BANK		0x000f
#define ES_MODE		0x0700
#ifndef ES_GRAY
#define ES_GRAY		0x1000
#define ES_SMEAR	0x8000
#endif

/* ST/TT shifter modes */
#define ST_LOW	0x0000
#define ST_MED	0x0100
#define ST_HIGH	0x0200
#define TT_LOW	0x0700
#define TT_MED	0x0300
#define TT_HIGH	0x0600

enum {
	BPP1=0,
	BPP2,
	BPP4,
	BPP8,
	BPP15,
	BPP16,
	BPP24,
	BPP32
};

/*--- Variables ---*/

unsigned long cookie_vdo;
unsigned long cookie_mil;
unsigned long cookie_vdi;
unsigned long cookie_blow;
unsigned long cookie_cnts;
unsigned long cookie_hade;
unsigned long cookie_nova;
unsigned long cookie_scpn;
unsigned long cookie_vi2;
unsigned long cookie_imne;
unsigned long cookie_osbl;
unsigned long cookie_vscr;
unsigned long cookie_vdly;
#if 0
unsigned long cookie_ostt;
unsigned long cookie_over;
#endif

int plugged_monitor;

/* List of all video modes */
int num_video_modes;
framebuffer_t *video_modes;

/* Video modes per bpp */
int num_bpp_modes[8];

/* For MVDI videomodes enumeration */
int mvdi_nummode;

/*--- Functions prototypes ---*/

void DisplayMonitor(void);
void DisplayVirtualScreen(void);
void DisplayCentScreenMode(centscreen_mode_t *vmode);

framebuffer_t *AddMode(unsigned short width, unsigned short height, unsigned short bpp);

void (*BuildModesList)(void);
void BuildModesListNull(void);
void BuildModesListST(void);
void BuildModesListTT(void);
void BuildModesListF30(void);
void BuildModesListF30ScreenBlaster2(void);
void BuildModesListF30ScreenBlaster3(void);
void BuildModesListF30CentScreen(void);
void BuildModesListF30VidelInside(void);
void BuildModesListF30BlowUp(void);
void BuildModesListF30Videlity(void);
void BuildModesListMilan(void);
void BuildModesListNova(void);
void BuildModesListImagine(void);

void CountBppModes(void);
void DisplayBppModes(int bpp, int spec_bpp);

void DrawDemoLoop(void);
void PrintCookie(char *text, unsigned long cookiename, unsigned long *cookievalue_p);

/*--- Functions ---*/

int main(int argc, char **argv)
{
	PAR_TestRedirect(argc, argv);
	if (output_to_file) {
		PAR_RedirectStdout();
	}

	/* Cookie _VDO present ? if not, assume ST machine */
	fprintf(output_handle, "_VDO cookie (Atari video chip): ");
	if (Getcookie(C__VDO, &cookie_vdo) != C_FOUND) {
		fprintf(output_handle, "no\n");
		cookie_vdo = VDO_ST << 16;
	} else {
		fprintf(output_handle, "yes\n");
	}

	PrintCookie("_MIL cookie (Milan)", C__MIL, &cookie_mil);
	PrintCookie("_VDI cookie (Milan VDI)", C__VDI, &cookie_vdi);
	PrintCookie("BLOW cookie (BlowUp)", C_BLOW, &cookie_blow);
	PrintCookie("CNTS cookie (CentScreen)", C_CNTS, &cookie_cnts);
	PrintCookie("hade cookie (Hades)", C_hade, &cookie_hade);
	PrintCookie("IMNE cookie (Imagine video card)", C_IMNE, &cookie_imne);
	PrintCookie("NOVA cookie (Nova video card)", C_NOVA, &cookie_nova);
	PrintCookie("OSBL cookie (ScreenBlaster II)", C_OSBL, &cookie_osbl);
#if 0
	PrintCookie("OSTT cookie (Overscan TT)", C_OSTT, &cookie_ostt);
	PrintCookie("OVER cookie (Autoswitch Overscan)", C_OVER, &cookie_over);
#endif
	PrintCookie("SCPN cookie (ScreenBlaster III)", C_SCPN, &cookie_scpn);
	PrintCookie("VDLY cookie (Videlity)", C_VDLY, &cookie_vdly);
	PrintCookie("VI-2 cookie (Videl Inside 2)", C_VI_2, &cookie_vi2);
	PrintCookie("VSCR cookie (Virtual screen manager)", C_VSCR, &cookie_vscr);

	/* Read plugged monitor, and corresponding video modes */
	DisplayMonitor();

	num_video_modes = 0;
	video_modes = NULL;
	BuildModesList();

	DisplayVirtualScreen();

	/* Display infos about modes */
	fprintf(output_handle, "\n%d video modes available\n",num_video_modes);

	CountBppModes();

	if (!output_to_file) {
		DrawDemoLoop();
	} else {
		fclose(output_handle);
	}
}

void PrintCookie(char *text, unsigned long cookiename, unsigned long *cookievalue_p)
{
	if (Getcookie(cookiename, cookievalue_p) != C_FOUND) {
		*cookievalue_p = 0;
	}
	fprintf(output_handle, "%s: %s\n", text, ((*cookievalue_p) == 0) ? "no" : "yes" );
}

void DisplayMonitor(void)
{
	/* Check the plugged monitor */
	plugged_monitor=-1;
	BuildModesList = BuildModesListNull;

	/* Atari machines */
	switch( cookie_vdo >>16) {
		case VDO_ST:
		case VDO_STE:
			if ( Getrez() == (ST_HIGH>>8) ) {
				plugged_monitor = STmono;
			} else {
				plugged_monitor = STcolor;
			}
			BuildModesList = BuildModesListST;
			break;
		case VDO_TT:
			if ( (EgetShift() & ES_MODE) == TT_HIGH) {
				plugged_monitor = STmono;
			} else {
				plugged_monitor = VGAcolor;
			}
			BuildModesList = BuildModesListTT;
			break;
		case VDO_F30:
			plugged_monitor = VgetMonitor();
			BuildModesList = BuildModesListF30;
			if (cookie_vi2) {
				BuildModesList = BuildModesListF30VidelInside;
			}
			if (cookie_cnts) {
				BuildModesList = BuildModesListF30CentScreen;
			}
			if (cookie_blow) {
				BuildModesList = BuildModesListF30BlowUp;
			}
			if (cookie_scpn) {
				BuildModesList = BuildModesListF30ScreenBlaster3;
			}
			if (cookie_osbl) {
				BuildModesList = BuildModesListF30ScreenBlaster2;
			}
			if (cookie_vdly) {
				BuildModesList = BuildModesListF30Videlity;
			}
			break;
	}

	/* Hades */
	if (cookie_hade) {
		BuildModesList = BuildModesListNull;
		plugged_monitor = VGAcolor;
	}

	/* Milan */
	if (cookie_mil) {
		BuildModesList = BuildModesListNull;
		/* with MVDI driver ? */
		if (cookie_vdi) {
			BuildModesList = BuildModesListMilan;
		}
		plugged_monitor = VGAcolor;
	}

	/* Imagine */
	if (cookie_imne) {
		BuildModesList = BuildModesListImagine;
		plugged_monitor = VGAcolor;
	}

	/* NOVA */
	if (cookie_nova) {
		BuildModesList = BuildModesListNova;
		plugged_monitor = VGAcolor;
	}

	fprintf(output_handle, "\nPlugged monitor: ");
	switch(plugged_monitor) {
		case STmono:
			fprintf(output_handle, "Monochrome\n");
			break;
		case VGAcolor:
			fprintf(output_handle, "VGA\n");
			break;
		case STcolor:
		case TVcolor:
			fprintf(output_handle, "TV/RGB\n");
			break;
		default:
			fprintf(output_handle, "unknown: _VDO=0x%08x\n",cookie_vdo);
			break;
	}
}

void DisplayVirtualScreen(void)
{
	vscr_cookie_t	*vscr_mgr;

	if (cookie_vscr==0) {
		return;
	}

	vscr_mgr = (vscr_cookie_t *) cookie_vscr;
	fprintf(output_handle, "\nVirtual screen manager:\n");
	fprintf(output_handle, " Provider: %c%c%c%c\n", 
		vscr_mgr->client[0], vscr_mgr->client[1], vscr_mgr->client[2], vscr_mgr->client[3]); 
	fprintf(output_handle, " Version: 0x%04x\n", vscr_mgr->version);
	fprintf(output_handle, " Position: %dx%d\n", vscr_mgr->posx, vscr_mgr->posy);
	fprintf(output_handle, " Window: %dx%d\n", vscr_mgr->width, vscr_mgr->height);
}

/*--- Build list of video modes ---*/

void BuildModesListNull(void)
{
}

void BuildModesListST(void)
{
	fprintf(output_handle, "\nAtari ST/STE shifter\n");

	switch(plugged_monitor) {
		case STmono:
			AddMode(640, 400, 1);
			break;
		case STcolor:
			AddMode(640, 200, 2);
			AddMode(320, 200, 4);
			break;
	}
}

void BuildModesListTT(void)
{
	fprintf(output_handle, "\nAtari TT shifter\n");

	switch(plugged_monitor) {
		case STmono:
			AddMode(1280, 960, 1);
			break;
		case VGAcolor:
			AddMode(640, 480, 4);
			AddMode(640, 400, 1);
			AddMode(640, 200, 2);
			AddMode(320, 480, 8);
			AddMode(320, 200, 4);
			break;
	}
}

void BuildModesListF30(void)
{
	fprintf(output_handle, "\nAtari Falcon 030 VIDEL\n");

	switch(plugged_monitor) {
		case STmono:
			AddMode(640, 400, 1);
			break;
		case VGAcolor:
			AddMode(640, 480, 8);
			AddMode(640, 480, 4);
			AddMode(640, 480, 2);
			AddMode(640, 480, 1);

			AddMode(640, 240, 8);
			AddMode(640, 240, 4);
			AddMode(640, 240, 2);
			AddMode(640, 240, 1);

			AddMode(320, 480, 16);
			AddMode(320, 480, 8);
			AddMode(320, 480, 4);
			AddMode(320, 480, 2);

			AddMode(320, 240, 16);
			AddMode(320, 240, 8);
			AddMode(320, 240, 4);
			AddMode(320, 240, 2);
			break;
		case STcolor:
		case TVcolor:
			AddMode(768, 480, 16);
			AddMode(768, 480, 8);
			AddMode(768, 480, 4);
			AddMode(768, 480, 2);
			AddMode(768, 480, 1);

			AddMode(768, 240, 16);
			AddMode(768, 240, 8);
			AddMode(768, 240, 4);
			AddMode(768, 240, 2);
			AddMode(768, 240, 1);

			AddMode(640, 400, 16);
			AddMode(640, 400, 8);
			AddMode(640, 400, 4);
			AddMode(640, 400, 2);
			AddMode(640, 400, 1);

			AddMode(640, 200, 16);
			AddMode(640, 200, 8);
			AddMode(640, 200, 4);
			AddMode(640, 200, 2);
			AddMode(640, 200, 1);

			AddMode(384, 480, 16);
			AddMode(384, 480, 8);
			AddMode(384, 480, 4);
			AddMode(384, 480, 2);

			AddMode(384, 240, 16);
			AddMode(384, 240, 8);
			AddMode(384, 240, 4);
			AddMode(384, 240, 2);

			AddMode(320, 400, 16);
			AddMode(320, 400, 8);
			AddMode(320, 400, 4);
			AddMode(320, 400, 2);

			AddMode(320, 200, 16);
			AddMode(320, 200, 8);
			AddMode(320, 200, 4);
			AddMode(320, 200, 2);

			break;
	}
}

void BuildModesListF30VidelInside(void)
{
	vi_header_t *vi2dat_p;
	vi_cookie_t *vicookie;
	vi_mode_t *vimode_p;
	vi_limits_t freqlimits;
	unsigned long clocks[3];
	unsigned long freqh, freqv;
	unsigned short modecode, num_modes, num_clock, clock_div, version;
	int i, bpp, width, height;

	vi2dat_p = VI_LoadModes();
	if (vi2dat_p == NULL) {
		return;
	}

	fprintf(output_handle, "\nVidel Inside:\n");

	vicookie = (vi_cookie_t *) cookie_vi2;
	version = vicookie->version;
	fprintf(output_handle, " Version: %d.%02d\n", (version>>8) & 0xff, version & 0xff);

	fprintf(output_handle, " Videl clocks:\n");
	clocks[0] = vi2dat_p->intfreq1 *100000;
	clocks[1] = vi2dat_p->intfreq2 *100000;
	clocks[2] = vi2dat_p->extfreq *100000;
	fprintf(output_handle, "  Internal 1: %.3f MHz\n", ((float) clocks[0])/1000000.0);
	fprintf(output_handle, "  Internal 2: %.3f MHz\n", ((float) clocks[1])/1000000.0);
	fprintf(output_handle, "  External:   %.3f MHz\n", ((float) clocks[2])/1000000.0);

	fprintf(output_handle, " Monitor infos:\n");
	if ((plugged_monitor==VGAcolor) && (vi2dat_p->multisync & 1)) {
		fprintf(output_handle, "  Multisynchrone monitor\n");
		freqlimits.freqh_mini = vi2dat_p->freqh_mini *100;
		freqlimits.freqh_maxi = vi2dat_p->freqh_maxi *100;
		freqlimits.freqv_mini = vi2dat_p->freqv_mini ;
		freqlimits.freqv_maxi = vi2dat_p->freqv_maxi;
	} else {
		fprintf(output_handle, "  Fixed frequency monitor\n");
		freqlimits.freqh_mini = vi_limits[plugged_monitor].freqh_mini;
		freqlimits.freqh_maxi = vi_limits[plugged_monitor].freqh_maxi;
		freqlimits.freqv_mini = vi_limits[plugged_monitor].freqv_mini;
		freqlimits.freqv_maxi = vi_limits[plugged_monitor].freqv_maxi;
	}
	fprintf(output_handle, "  Horizontal frequency range: %.3f - %.3f KHz\n", ((float) freqlimits.freqh_mini)/1000.0, ((float) freqlimits.freqh_maxi)/1000.0);
	fprintf(output_handle, "  Vertical frequency range: %.1f - %.1f Hz\n", ((float) freqlimits.freqv_mini)/10.0, ((float) freqlimits.freqv_maxi)/10.0);

	fprintf(output_handle, " Reading video modes:\n");
	vimode_p = (vi_mode_t *) (((unsigned char *)vi2dat_p)+sizeof(vi_header_t));
	num_modes = vi2dat_p->num_modes;
	for (i=0; i<num_modes; i++,vimode_p++) {
		bpp = 0;
		modecode = vimode_p->modecode;
		switch((modecode>>4) & 7) {
			case 0:	bpp = 1; break;
			case 1:	bpp = 2; break;
			case 2:	bpp = 4; break;
			case 3:	bpp = 8; break;
			case 4:	bpp = 16; break;
		}

		if (bpp<=0) {
			continue;
		}

		num_clock = (modecode>>7) & 3;
		clock_div = 2-((modecode>>2) & 3);

		freqh = clocks[num_clock]/(vimode_p->freqh_div <<clock_div);
		freqv = (freqh*10)/vimode_p->freqv_div;
		width = vimode_p->width;
		height = vimode_p->height;

		fprintf(output_handle, "  Mode %d: %dx%dx%d: HFreq=%.3f KHz, VFreq=%.1f Hz:",i, width, height, bpp, ((float)freqh)/1000.0, ((float)freqv)/10.0);

		if ((freqh<freqlimits.freqh_mini) || (freqh>freqlimits.freqh_maxi)) {
			fprintf(output_handle, " Horizontal frequency out of range\n");
			continue;
		}
		if ((freqv<freqlimits.freqv_mini) || (freqv>freqlimits.freqv_maxi)) {
			fprintf(output_handle, " Vertical frequency out of range\n");
			continue;
		}
		fprintf(output_handle, " Ok\n");

		AddMode(width, height, bpp);
	}

	free(vi2dat_p);
}

void DisplayCentScreenMode(centscreen_mode_t *vmode)
{
	fprintf(output_handle, "  Name: %s\n", vmode->name);
	fprintf(output_handle, "   Resolution: %dx%dx%d\n", vmode->physx, vmode->physy, vmode->plan);
	if (vmode->mode & CSCREEN_VIRTUAL) {
		fprintf(output_handle, "   Virtual screen enabled: %dx%d\n", vmode->logx, vmode->logy);
	}
	if (vmode->mode & CSCREEN_SAVER) {
		fprintf(output_handle, "   Screen saver enabled: delay %d sec.\n", vmode->eco);
	}
	if (vmode->mode & CSCREEN_ENERGYSTAR) {
		fprintf(output_handle, "    Energy star mode enabled: delay %d sec.\n", vmode->eco2);
	}
	if (vmode->mode & CSCREEN_EXTCLOCK_CT2) {
		fprintf(output_handle, "   CT2 external clock enabled\n");
	}
	if (vmode->mode & CSCREEN_EXTCLOCK) {
		fprintf(output_handle, "   External clock enabled\n");
	}
	if (vmode->mode & CSCREEN_STANDARD) {
		fprintf(output_handle, "   Standard resolution\n");
	}
	if (vmode->mode & CSCREEN_DEFAULT) {
		fprintf(output_handle, "   Active video mode\n");
	}
}

void BuildModesListF30CentScreen(void)
{
	centscreen_mode_t currentmode, listedmode;
	unsigned long result;

	Vread(&currentmode);
	Vattrib(&currentmode, &listedmode);
	fprintf(output_handle, "\nCentscreen:\n");
	fprintf(output_handle, " Current mode:\n");
	DisplayCentScreenMode(&currentmode);
	
	currentmode.mode = currentmode.physx = currentmode.physy =
		currentmode.plan = currentmode.logx = currentmode.logy = -1;
	fprintf(output_handle, " List of CentScreen video modes:\n");
	result = Vfirst(&currentmode, &listedmode);
	if (result == 0) {
		while (result == 0) {
			DisplayCentScreenMode(&listedmode);
			AddMode(listedmode.physx, listedmode.physy, listedmode.plan);
			memcpy(&currentmode, &listedmode, sizeof(centscreen_mode_t));
			currentmode.mode = currentmode.physx = currentmode.physy =
				currentmode.plan = currentmode.logx = currentmode.logy = -1;
			result = Vnext(&currentmode, &listedmode);
		}
	} else {
		fprintf(output_handle, "  None\n");
	}
}

void BuildModesListF30BlowUp(void)
{
	blow_mode_t *blowup_mode;
	blow_cookie_t *blowup_infos;
	int i, width, height, bpp, blowup_monitor;

	fprintf(output_handle, "\nBlowUp:\n");

	blowup_infos = (blow_cookie_t *) cookie_blow;

#if 0
	if ( blowup_infos->montype != plugged_monitor) {
		fprintf(output_handle, " Modes created for a different monitor: unusable\n");
		return;
	}
#endif

	for (i=0; i<10; i++) {
		blowup_mode = &(blowup_infos->blowup_modes[i]);

		fprintf(output_handle, " Mode %d ", i);

		if (blowup_mode->enabled != 0) {
			fprintf(output_handle, "disabled\n");
			continue;
		}

		switch(i % 5) {
			case 0:	bpp=1;	break;
			case 1:	bpp=2;	break;
			case 2:	bpp=4;	break;
			case 3:	bpp=8;	break;
			case 4:	bpp=16;	break;
		}
			
		width = (blowup_mode->width)+1,
		height = (blowup_mode->height)+1,
		fprintf(output_handle, "enabled: %dx%dx%d", width, height, bpp);

		blowup_monitor = blowup_mode->monitor;
		switch(blowup_monitor) {
			case STmono:
				fprintf(output_handle, ", monochrome monitor");
				break;
			case VGAcolor:
				fprintf(output_handle, ", VGA monitor");
				break;
			case TVcolor:
			case STcolor:
				fprintf(output_handle, ", TV/RGB monitor");
				break;
		}

		if (blowup_mode->virtual != 0) {
			fprintf(output_handle, ", virtual %dx%dx%d", blowup_mode->virwidth, blowup_mode->virheight, bpp);
		}

		if ((plugged_monitor == blowup_monitor) ||
			((blowup_monitor==TVcolor) && (plugged_monitor==STcolor)) ||
			((blowup_monitor==STcolor) && (plugged_monitor==TVcolor))) {
			AddMode(width, height, bpp);
			fprintf(output_handle, ": Ok\n");
		} else {
			fprintf(output_handle, ": incompatible monitor\n");
		}
	}
}

void BuildModesListF30ScreenBlaster3(void)
{
	scpn_cookie_t	*sbl_cookie;
	scpn_screeninfo_t	*screeninfo;
	int vwidth, vheight, width, height, bpp;

	fprintf(output_handle, "\nScreenBlaster III:\n");

	sbl_cookie = (scpn_cookie_t *) cookie_scpn;
	screeninfo = sbl_cookie->screen_info;
	if (screeninfo == NULL) {
		fprintf(output_handle, " No video mode defined\n");
		return;
	}

	vwidth = screeninfo->virtual_width;
	vheight = screeninfo->virtual_height;
	width = screeninfo->visible_width;
	height = screeninfo->visible_height;
	bpp = 1<<scpn_planes_device[screeninfo->device];

	if ((vwidth>0) && (vheight>0)) {
		if ((vwidth>width) || (vheight>height)) {
			fprintf(output_handle, " Virtual screen: %dx%dx%d\n", vwidth, vheight, bpp);
		}
	}
	fprintf(output_handle, " Screen: %dx%dx%d\n", width, height, bpp);
	AddMode(width, height, bpp);
}

void BuildModesListF30ScreenBlaster2(void)
{
	unsigned long errorcode;
	sblast2_mode_t	*first, *current, *curmode;
	sblast2_header_t *header;
	sblast2_name_t	*mon_names;
	int i, width, height, bpp, monitor;
	unsigned long mon_suitable;

	fprintf(output_handle, "\nScreenBlaster II:\n");

	if (SB2_Init()<0) {
		fprintf(output_handle, "Can not initialize SB2 functions\n");
		return;
	}

	errorcode = SB2_GetStatus();
	fprintf(output_handle, " Enabled: 0x%08x : %s\n", errorcode, errorcode ? "true" : "false");
	if (errorcode == 0) {
		return;
	}

	SB2_GetModes(&first, &current);
	fprintf(output_handle, " Current video mode: ");
	if ((current->virt_width == current->phys_width) && (current->virt_height == current->phys_height)) {
		fprintf(output_handle, "%dx%dx%d\n", current->phys_width +1, current->phys_height +1, current->bpp);
	} else {
		fprintf(output_handle, "virtual: %dx%dx%d, physical: %dx%dx%d\n",
			current->virt_width +1, current->virt_height +1, current->bpp,
			current->phys_width +1, current->phys_height +1, current->bpp
		);
	}

	fprintf(output_handle, " Infos from file:\n");
	header = SB2_GetFile();
/*
	fprintf(output_handle, "  Number of monitors: %d\n", header->mon_count);
	mon_names = (sblast2_name_t *) (((unsigned char *) header) + sizeof(sblast2_header_t));
	for (i=0; i<header->mon_count; i++) {
		fprintf(output_handle, "  Monitor %d: %s\n", i, mon_names[i].name);
	}
*/
	fprintf(output_handle, "  Number of videomodes: %d\n", header->mode_count);
	fprintf(output_handle, "  External clock number: %d\n", header->clock);

	/* Some init to check for suitable monitor */
	monitor = VgetMonitor();
	mon_suitable = (0x10000000<<monitor)|(header->mon_info[monitor]);

	fprintf(output_handle, " Videomodes:\n");
	curmode = first;
	i=0;
	for (;;) {
		if (mon_suitable & curmode->mon_suitable) {

			fprintf(output_handle, "  Mode %d: %s\n   HFreq=%.1f KHz, VFreq=%.1f Hz, ",
				i, curmode->name.pointer,
				((float)curmode->hfreq)/10.0, ((float)curmode->vfreq)/10.0
			);

			width = (curmode->phys_width) +1;
			height = (curmode->phys_height) +1;
			bpp = curmode->bpp;

			if ((((curmode->virt_width)+1) == width) && (((curmode->virt_height)+1) == height)) {
				fprintf(output_handle, "%dx%dx%d\n", width, height, bpp);
			} else {
				fprintf(output_handle, "virtual: %dx%dx%d, physical: %dx%dx%d\n",
					(curmode->virt_width)+1, (curmode->virt_height)+1, bpp,
					width, height, bpp
				);
			}

			AddMode(width, height, bpp);
		} else {
			fprintf(output_handle, "  Mode %d is not usable on this monitor\n", i);
		}

		if (curmode->next.offset == 0xffffffffUL) {
			break;
		} else {
			curmode = (sblast2_mode_t *) curmode->next.pointer;
			++i;
		}
	}
}

void BuildModesListF30Videlity(void)
{
	cookie_vdly_t	*vdl_cookie;
	vdly_modeinfo_t	vdly_modeinfo[20];
	int i,width,height,bpp;

	fprintf(output_handle, "\nVidelity:\n");

	if (VDLY_Init()<0) {
		fprintf(output_handle, "Can not initialize Videlity functions\n");
		return;
	}

	vdl_cookie = (cookie_vdly_t *)cookie_vdly;

	fprintf(output_handle, " Name: %s\n", vdl_cookie->name);

	VDLY_ReadModes(vdly_modeinfo);
	i=0;
	while ((vdly_modeinfo[i].modecode != -1) && (i<20)) {
		width = vdly_modeinfo[i].width;
		height = vdly_modeinfo[i].height;
		bpp = vdly_modeinfo[i].bpp;		

		fprintf(output_handle, " Mode %d: %dx%dx%d\n",i,width,height,bpp);
		AddMode(width,height,bpp);

		i++;
	}
}

unsigned long /*cdecl*/ enumfunc(SCREENINFO *inf, unsigned long flag)
{
	framebuffer_t *framebuffer;

	framebuffer=AddMode(inf->scrWidth, inf->scrHeight, inf->scrPlanes);
	if (framebuffer->bpp==8) {
		framebuffer->format=FBFORMAT_PACKED;
	}

	fprintf(output_handle, "  Mode %d: %dx%dx%d: Ok\n", mvdi_nummode, inf->scrWidth, inf->scrHeight, inf->scrPlanes);
	mvdi_nummode++;

	return ENUMMODE_CONT; 
} 

void BuildModesListMilan(void)
{
	int i;
	unsigned long mvdicurmode;
	framebuffer_t *framebuffer;
	SCREENINFO si;

	fprintf(output_handle, "\nMilan MVDI:\n");

	/* Get infos about current mode */
	fprintf(output_handle, " Current video mode:\n");
	VsetScreen(-1, &mvdicurmode, MI_MAGIC, CMD_GETMODE);
	fprintf(output_handle, "  Video mode code: 0x%08x\n", mvdicurmode);

	si.size = sizeof(SCREENINFO);
	si.devID = mvdicurmode;
	si.scrFlags = 0;
	VsetScreen(-1, &si, MI_MAGIC, CMD_GETINFO) ;
	if (si.scrFlags & SCRINFO_OK) {
		fprintf(output_handle, "  %dx%dx%d mode selected\n", si.scrWidth, si.scrHeight, si.scrPlanes);
	} else {
		fprintf(output_handle, "  Unable to read infos about current mode: 0x%08x\n", si.scrFlags);
	}

	/* Add predefined modes, if valid */
	fprintf(output_handle, " Predefined video modes:\n");
	for (i=0; i<NUM_MVDIMODELIST; i++) {
		fprintf(output_handle, "  Mode %d: %dx%dx%d",i, mvdimodelist[i].width, mvdimodelist[i].height, mvdimodelist[i].bpp);
		if (Validmode(mvdimodelist[i].deviceid)) {
			framebuffer=AddMode(mvdimodelist[i].width, mvdimodelist[i].height, mvdimodelist[i].bpp);
			if (framebuffer->bpp==8) {
				framebuffer->format=FBFORMAT_PACKED;
			}
			fprintf(output_handle, ": Ok\n");
		} else {
			fprintf(output_handle, ": Not a valid mode\n");
		}
	}

	/* Add custom modes */
	fprintf(output_handle, " Custom video modes:\n");
	mvdi_nummode=0;
	VsetScreen(-1,&enumfunc,MI_MAGIC,CMD_ENUMMODES);
}

void BuildModesListNova(void)
{
	nova_xcb_t *cur_xcb;
	int width, height, bpp, pitch;
	int i,num_modes;
	nova_resolution_t *nova_res,*cur_mode;
	framebuffer_t *framebuffer;

	cur_xcb = (nova_xcb_t *)cookie_nova;
	fprintf(output_handle, "\nNOVA video card:\n");

	/* Some infos */
	fprintf(output_handle, " Version: %c%c%c%c\n", cur_xcb->version[0], cur_xcb->version[1], cur_xcb->version[2], cur_xcb->version[3]);
/*	fprintf(output_handle, " Version: 0x%08x\n", cur_xcb->version);*/
	fprintf(output_handle, " Blank time: %d\n", cur_xcb->blnk_time);
	fprintf(output_handle, " Mouse speed: %d\n", cur_xcb->ms_speed);
	fprintf(output_handle, " Video RAM base: 0x%08x\n", cur_xcb->scr_base);
	fprintf(output_handle, " Video RAM size: %d\n", cur_xcb->mem_size);
	fprintf(output_handle, " %d possible screens\n\n", cur_xcb->scrn_cnt);

	/* Display infos about current mode */
	fprintf(output_handle, "\nCurrent video mode:\n");
	fprintf(output_handle, " Name: %s\n", cur_xcb->name);
	width = cur_xcb->max_x +1;
	height = cur_xcb->max_y +1;
	bpp = cur_xcb->planes;
	pitch = cur_xcb->pitch;
	if (bpp<8) {
		pitch *= bpp;
	}
	fprintf(output_handle, " Resolution: %dx%dx%d\n", width, height, bpp);
	fprintf(output_handle, " Pitch: %d\n", pitch);
	fprintf(output_handle, " Virtual screen:\n");
	fprintf(output_handle, "  Real min: %dx%dx%d\n", cur_xcb->rmn_x, cur_xcb->rmn_y, bpp);
	fprintf(output_handle, "  Real max: %dx%dx%d\n\n", cur_xcb->rmx_x +1, cur_xcb->rmx_y +1, bpp);

	/* The other modes are in a .bib file */
	nova_res = nova_LoadModes(&num_modes);
	if (nova_res!=NULL) {
		fprintf(output_handle, "%d defined video modes:\n",num_modes);
		cur_mode=nova_res;
		for (i=0; i<num_modes; i++) {
			fprintf(output_handle, " Mode %d:\n",i);
			fprintf(output_handle, "  Name: %s\n",cur_mode->name);
			width = cur_mode->max_x +1;
			height = cur_mode->max_y +1;
			bpp = cur_mode->planes;
			pitch = cur_mode->pitch;
			if (bpp<8) {
				pitch *= bpp;
			}
			fprintf(output_handle, "  Resolution: %dx%dx%d\n",width,height,bpp);
			fprintf(output_handle, "  Pitch: %d\n", pitch);
			fprintf(output_handle, "  Virtual screen:\n");
			fprintf(output_handle, "   Real x,y: %d,%d\n", cur_mode->real_x +1, cur_mode->real_y +1);
			framebuffer=AddMode(width,height,bpp);
			framebuffer->pitch = pitch;
			switch(framebuffer->bpp) {
				case 8:
					framebuffer->format=FBFORMAT_PACKED;
					break;
				case 15:
					FBMASK(1<<7, 31<<2, (7 << 13) | 3, 31 << 8);
					FBLOSS(7, 3, 6, 3);
					FBSHIFT(7, 2, 0, 8);
					break;
				case 16:
					FBMASK(0, 31<<3, (7<<13)|7, 31<<8);
					FBLOSS(8, 3, 5, 3);
					FBSHIFT(0, 3, 0, 8);
					break;
				case 24:
					FBMASK(0, 255, 255<<8, 255<<16);
					FBSHIFT(0, 0, 8, 16);
					break;
				case 32:
					FBMASK(255, 255<<24, 255<<16, 255<<8);
					FBSHIFT(0, 24, 16, 8);
					break;
			}
			cur_mode++;
		}
	} else {
		/* Add current mode */
		framebuffer=AddMode(width, height, bpp);
		framebuffer->pitch=pitch;
		switch(framebuffer->bpp) {
			case 8:
				framebuffer->format=FBFORMAT_PACKED;
				break;
			case 15:
				FBMASK(1<<7, 31<<2, (7 << 13) | 3, 31 << 8);
				FBLOSS(7, 3, 6, 3);
				FBSHIFT(7, 2, 0, 8);
				break;
			case 16:
				FBMASK(0, 31<<3, (7<<13)|7, 31<<8);
				FBLOSS(8, 3, 5, 3);
				FBSHIFT(0, 3, 0, 8);
				break;
			case 24:
				FBMASK(0, 255, 255<<8, 255<<16);
				FBSHIFT(0, 0, 8, 16);
				break;
			case 32:
				FBMASK(255, 255<<24, 255<<16, 255<<8);
				FBSHIFT(0, 24, 16, 8);
				break;
		}
	}
}

void BuildModesListImagine(void)
{
	nova_icb_t *cur_icb;
	int width, height, bpp, pitch;
	framebuffer_t *framebuffer;

	cur_icb=(nova_icb_t *)cookie_imne;
	fprintf(output_handle, "\nImagine video card:\n");

	/* Some infos */
	fprintf(output_handle, "Version: %c%c%c%c\n", cur_icb->id[0], cur_icb->id[1], cur_icb->id[2], cur_icb->id[3]);
	fprintf(output_handle, "Boot drive: %c\n", 'A'+ cur_icb->boot_drv);
	fprintf(output_handle, "Card type: 0x%02x\n", cur_icb->card);

	/* Display infos about current mode */
	fprintf(output_handle, "Name: %s\n", cur_icb->name);
	width = cur_icb->max_x +1;
	height = cur_icb->max_y +1;
	bpp = cur_icb->planes;
	pitch = cur_icb->pitch;
	if (bpp<8) {
		pitch *= bpp;
	}
	fprintf(output_handle, " Resolution: %dx%dx%d\n", width, height, bpp);
	fprintf(output_handle, " Pitch: %d\n", pitch);
	fprintf(output_handle, " Virtual screen:\n");
	fprintf(output_handle, "  Real min: %dx%dx%d\n", cur_icb->rmn_x, cur_icb->rmn_y, bpp);
	fprintf(output_handle, "  Real max: %dx%dx%d\n", cur_icb->rmx_x +1, cur_icb->rmx_y +1, bpp);
	framebuffer=AddMode(width, height, bpp);
	framebuffer->pitch = pitch;
	switch(framebuffer->bpp) {
		case 8:
			framebuffer->format=FBFORMAT_PACKED;
			break;
		case 15:
			FBMASK(1<<7, 31<<2, (7 << 13) | 3, 31 << 8);
			FBLOSS(7, 3, 6, 3);
			FBSHIFT(7, 2, 0, 8);
			break;
		case 16:
			FBMASK(0, 31<<3, (7<<13)|7, 31<<8);
			FBLOSS(8, 3, 5, 3);
			FBSHIFT(0, 3, 0, 8);
			break;
		case 24:
			FBMASK(0, 255, 255<<8, 255<<16);
			FBSHIFT(0, 0, 8, 16);
			break;
		case 32:
			FBMASK(255, 255<<24, 255<<16, 255<<8);
			FBSHIFT(0, 24, 16, 8);
			break;
	}

	/* The other modes are in a .bib file ? */
}

/*--- Count modes for each bpp ---*/

framebuffer_t *AddMode(unsigned short width, unsigned short height, unsigned short bpp)
{
	framebuffer_t *framebuffer;
	int i, curpos;

	++num_video_modes;
	
	video_modes = (framebuffer_t *) realloc(video_modes, num_video_modes * sizeof(framebuffer_t));

	/* Search the list to know where to put the new mode */
	curpos=0;

	/* 1: bpp (at end of loop: first video mode with equal or lower bpp) */
	for(i=0; i<num_video_modes-1; i++) {
		if (video_modes[i].bpp <= bpp)
			break;
		curpos++;
	}
	/* 2: width (at end of loop: first video mode with equal or lower width) */
	for(i=curpos; i<num_video_modes-1; i++) {
		if (video_modes[i].width <= width)
			break;
		curpos++;
	}
	/* 3: height (at end of loop: first video mode with equal or lower height) */
	for(i=curpos; i<num_video_modes-1; i++) {
		if (video_modes[i].height <= height)
			break;
		curpos++;
	}

	/* Push remaining modes one position further */
	for(i=num_video_modes-1; i>curpos; i--) {
		memcpy(&video_modes[i], &video_modes[i-1], sizeof(framebuffer_t));
	}

	/* Write mode */
	framebuffer = &video_modes[curpos];

	memset(framebuffer, 0, sizeof(framebuffer_t));

	RGB_InitFramebuffer(framebuffer, width, height, bpp);

	return framebuffer;
}

void CountBppModes(void)
{
	framebuffer_t *curmode;	
	int i, spec_bpp;
	
	memset(num_bpp_modes, 0, sizeof(num_bpp_modes));

	curmode = video_modes;
	for (i=0;i<num_video_modes;i++) {
		switch(curmode->bpp) {
			case 1:		spec_bpp = BPP1;	break;
			case 2:		spec_bpp = BPP2;	break;
			case 4:		spec_bpp = BPP4;	break;
			case 8:		spec_bpp = BPP8;	break;
			case 15:	spec_bpp = BPP15;	break;
			case 16:	spec_bpp = BPP16;	break;
			case 24:	spec_bpp = BPP24;	break;
			case 32:	spec_bpp = BPP32;	break;
		}
		num_bpp_modes[spec_bpp]++;
		curmode++;
	}

	DisplayBppModes(1, BPP1);
	DisplayBppModes(2, BPP2);
	DisplayBppModes(4, BPP4);
	DisplayBppModes(8, BPP8);
	DisplayBppModes(15, BPP15);
	DisplayBppModes(16, BPP16);
	DisplayBppModes(24, BPP24);
	DisplayBppModes(32, BPP32);
}

void DisplayBppModes(int bpp, int spec_bpp)
{
	framebuffer_t *curmode;
	int i, j;

	if (num_bpp_modes[spec_bpp]==0)
		return;

	curmode = video_modes;
	fprintf(output_handle, " %d video modes with %d bits:\n ", num_bpp_modes[spec_bpp], bpp);
	j =0;
	for (i=0;i<num_video_modes;i++) {
		if (curmode->bpp == bpp) {
			fprintf(output_handle, " %dx%d",curmode->width, curmode->height);
			j++;
			if ((j & 7)==0) {
				fprintf(output_handle, "\n");
				if (j != num_bpp_modes[spec_bpp]) {
					fprintf(output_handle, " ");
				}
			}
		}
		curmode++;
	}	
	if ((j & 7)!=0) {
		fprintf(output_handle, "\n");
	}
}

/*--- Little demo loop ---*/

void DrawDemoLoop(void)
{
	unsigned long key_pressed;
	unsigned char scancode;

	printf("Press ESC to quit\n");

	key_pressed = scancode = 0;
	while (scancode != SCANCODE_ESC) {

		/* Read key pressed, if needed */
		if (Cconis()) {
			key_pressed = Cnecin();
			scancode = (key_pressed >>16) & 0xff;

			/* Wait key release */
			while (Cconis()!=0) { }
		}
	}
}
