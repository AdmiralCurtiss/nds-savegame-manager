/*
 * savegame_manager: a tool to backup and restore savegames from Nintendo
 *  DS cartridges. Nintendo DS and all derivative names are trademarks
 *  by Nintendo. EZFlash 3-in-1 is a trademark by EZFlash.
 *
 * main.cpp: main file
 *
 * Copyright (C) Pokedoc (2010)
 */
/* 
 * This program is free software; you can redistribute it and/or modify 
 * it under the terms of the GNU General Public License as published by 
 * the Free Software Foundation; either version 2 of the License, or 
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License 
 * for more details.
 * 
 * You should have received a copy of the GNU General Public License along 
 * with this program; if not, write to the Free Software Foundation, Inc., 
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */
 
#include <nds.h>
#include <fat.h>

#include <sys/dir.h>
#include <nds/arm9/console.h>

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <math.h>

#include <dirent.h>

#include <algorithm>

#include "gba.h"
#include "dsi.h"
#include "display.h"
#include "dsCard.h"
#include "hardware.h"
#include "fileselect.h"
#include "strings.h"

#include "libini.h"

u8 *data;
u32 size_buf;

using std::max;


uint32 ezflash = 0;
uint32 dstype = 0;
bool gba = 0;
uint32 mode = 0;
bool slot2 = false;
u8 gbatype = 0;

char ftp_ip[16] = "ftp_ip";
char ftp_user[64] = "ftp_user";
char ftp_pass[64] = "ftp_pass";
int ftp_port = 0;
int ir_delay = 1000;

char bootdir[256] = "/";


// ============================================================================
void mode_dsi()
{
	// TODO: test is SD card is present!

	// use 3in1 to buffer data
	displayPrintState("");
	displayPrintUpper();
	displayPrintLower();
	
	touchPosition touchXY;
	while(1) {
		swiWaitForVBlank();
		touchRead(&touchXY);
		
		// backup
		if ((touchXY.py > 8*0) && (touchXY.py < 8*8)) {
			hwBackupDSi();
		}
		
		// restore
		if ((touchXY.py > 8*8) && (touchXY.py < 8*16)) {
			hwRestoreDSi();
		}
		
		// erase
		if ((touchXY.py > 8*16) && (touchXY.py < 8*24)) {
			swap_cart();
			displayPrintUpper();
			hwErase();
		}
	}

	// DSi mode, does nothing at the moment
	displayPrintState("DSi mode - still unsupported!");
	while (1);
}

void mode_slot2()
{
	// use slot2 DLDI device to store data
	displayPrintState("");
	displayPrintUpper();
	displayPrintLower();
	
	displayMessage("This mode is DISABLED.\nPlease remove Slot-2 module\nand restart this tool.");
	while(1);
	
	touchPosition touchXY;
	while(1) {
		swiWaitForVBlank();
		touchRead(&touchXY);
		
		// backup
		if ((touchXY.py > 8*0) && (touchXY.py < 8*8)) {
			//displayPrintUpper();
			//hwBackupGBA();
		}
		
		// restore
		if ((touchXY.py > 8*8) && (touchXY.py < 8*16)) {
			//displayPrintUpper();
			//hwRestoreGBA();
		}
		
		// erase
		if ((touchXY.py > 8*16) && (touchXY.py < 8*24)) {
			//displayPrintUpper();
			//hwEraseGBA();
		}
	}
}

void mode_3in1()
{
	displayPrintUpper();
	displayPrintLower();
	
	dsCardData data2;
	uint32 ime = hwGrab3in1();
	ReadNorFlash(data, 0, 0x8000);
	hwRelease3in1(ime);
	memcpy(&data2, &data[0x1000], sizeof(data2));
	if ((data2.data[0] == RS_BACKUP) && (data2.data[1] == 0) && (data2.data[3] == 0xffff00ff)) {
		uint32 size = data2.data[2];
		char name[13];
		memcpy(&name[0], &data2.name[0], 12);
		name[12] = 0;
		displayMessageA(STR_HW_3IN1_CLEAR_FLAG); // lower screen is used for file browser
		hwFormatNor(0, 1); // clear reboot flag
		hwDump3in1(size, name);
	}
	
	touchPosition touchXY;
	while(1) {
		swiWaitForVBlank();
		touchRead(&touchXY);
		
		// backup
		if ((touchXY.py > 8*0) && (touchXY.py < 8*8)) {
			hwBackup3in1();
		}
		
		// restore
		if ((touchXY.py > 8*8) && (touchXY.py < 8*16)) {
			hwRestore3in1();
		}
		
		// erase
		if ((touchXY.py > 8*16) && (touchXY.py < 8*24)) {
			swap_cart();
			displayPrintUpper();
			hwErase();
			displayPrintLower();
		}
	}
}

void mode_gba()
{
	// use 3in1 to buffer data
	displayPrintState("");
	gbatype = gbaGetSaveType();
	displayPrintUpper();
	displayPrintLower();

	touchPosition touchXY;
	while(1) {
		swiWaitForVBlank();
		touchRead(&touchXY);
		
		// backup
		if ((touchXY.py > 8*0) && (touchXY.py < 8*8)) {
			displayPrintUpper();
			hwBackupGBA(gbatype);
		}
		
		// restore
		if ((touchXY.py > 8*8) && (touchXY.py < 8*16)) {
			displayPrintUpper();
			hwRestoreGBA();
		}
		
		// erase
		if ((touchXY.py > 8*16) && (touchXY.py < 8*24)) {
			//displayPrintUpper();
			//hwEraseGBA();
		}
	}
}

void mode_wifi()
{
	// use 3in1 to buffer data
	displayPrintState("");
	displayPrintUpper();
	displayPrintLower();
	
	touchPosition touchXY;
	while(1) {
		swiWaitForVBlank();
		touchRead(&touchXY);
		
		// backup
		if ((touchXY.py > 8*0) && (touchXY.py < 8*8)) {
			hwBackupFTP();
			displayPrintLower();
		}
		
		// restore
		if ((touchXY.py > 8*8) && (touchXY.py < 8*16)) {
			hwRestoreFTP();
			displayPrintLower();
		}
		
		// erase
		if ((touchXY.py > 8*16) && (touchXY.py < 8*24)) {
			swap_cart();
			displayPrintUpper();
			hwErase();
			displayPrintLower();
		}
	}
}

int main(int argc, char* argv[])
{
	sysSetBusOwners(true, true);
	
	// Init the screens
	displayInit();
	
	// Init DLDI (file system driver)
	int fat = fatInitDefault();
	if (fat == 0) {
		displayPrintState("DLDI error\n");
		while (1) {};
	}
	
	// test if our flash card supports argv at all. some R4 clones seem to be very picky!
	//  (untested due to lack of an R4 myself, but I hope it works)
	bool has_argv = false;
	if (argc)
		has_argv = true;
	
	// load ini file...
	ini_fd_t ini = 0;
	char inipath[256];
	if (has_argv) {
		if (argv[0]) {
			char *last = strrchr(argv[0], '/');
			int len = (last - argv[0])+1;
			strncpy(bootdir, argv[0], len);
			sprintf(inipath, "%s/savegame_manager.ini", bootdir);
			if (fileExists(inipath))
				ini = ini_open(inipath, "r", "");
		}
	}
	if (!ini) {
		sprintf(inipath, "/savegame_manager.ini");
		if (fileExists(inipath))
			ini = ini_open(inipath, "r", "");
	}
	if (!ini) {
		displayMessage("Unable to open ini file!\nPlease make sure that it is\n1. in this apps folder, or"
		  "\n2. in the root folder\nIf 1. does not work, use 2.");
		while (1);
	}
	
	ini_locateHeading(ini, "");
	ini_locateKey(ini, "ftp_ip");
	ini_readString(ini, ftp_ip, 16);
	ini_locateKey(ini, "ftp_user");
	ini_readString(ini, ftp_user, 64);
	ini_locateKey(ini, "ftp_pass");
	ini_readString(ini, ftp_pass, 64);
	ini_locateKey(ini, "ftp_port");
	ini_readInt(ini, &ftp_port);
	ini_locateKey(ini, "ir_delay");
	ini_readInt(ini, &ir_delay);
	ir_delay = max(ir_delay, 1000);
	ini_close(ini);
	
	// delete temp file (which is a remnant of inilib)
	remove("/tmpfile");
	
	// load strings
	stringsLoadFile(0);
	
	// Identify hardware and branch to corresponding mode
	//displayMessage("Identifying hardware...");

	// 1) Identify DSi (i.e. memory capacity)
	//displayPrintState("ID: DS model");
	if (isDsi()) {
		dstype = 1;
		size_buf = 1 << 23; // 8 MB
	} else {
		dstype = 0;
		size_buf = 1 << 21; // 2 MB
		//size_buf = 1 << 14; // 32 kB - TESTING ONLY!
	}
	data = (u8*)malloc(size_buf);

	// don't try to identify Slot-2 in DSi mode.
	if (dstype == 0) {
		//displayPrintState("ID: Slot 2");
		uint32 ime = enterCriticalSection();
		sysSetBusOwners(true, true);
		OpenNorWrite();
		ezflash = ReadNorFlashID();
		CloseNorWrite();
		leaveCriticalSection(ime);
		chip_reset();

		gba = gbaIsGame();
	}
	
	// Try to identify slot-2 device. Try opening slot-2 root directory
	if (argv[0][3] == '2')
		slot2 = true;
	
	if (dstype == 1) {
		// DSi/DSiXL, branch to SD-card mode when cracked.
		mode = 3;
		mode_dsi();
	} else if (slot2) {
		mode = 4;
		mode_slot2();
	} else if (ezflash != 0) {
		// DS with EZFlash found -> branch to 3in1 mode
		mode = 2;
		mode_3in1();
	} else if (gba) {
		// GBA game in slot 2 -> branch to GBA mode
		mode = 1;
		mode_gba();
	} else {
		// failsafe: enter WiFi mode
		mode = 0;
		mode_wifi();
	}

	return 0;
}
