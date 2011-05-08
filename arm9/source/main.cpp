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

#include "globals.h"

using std::max;


char bootdir[256] = "/";


// ============================================================================
void mode_dsi()
{
	// TODO: test is SD card is present!

	// use 3in1 to buffer data
	displayPrintState("");
	displayPrintUpper();
	displayPrintLower();
	
	// DSi mode, does nothing at the moment
	displayMessage2A(STR_BOOT_MODE_UNSUPPORTED,true);
	while(1);

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
}

void mode_slot2()
{
	// use slot2 DLDI device to store data
	displayPrintState("");
	displayPrintUpper();
	displayPrintLower();
	
	displayMessage2A(STR_BOOT_MODE_UNSUPPORTED,true);
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
	displayMessage("");
	displayProgressBar(0,0);
	displayPrintLower();
	
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
	// This function takes some time, since it needs to parse the entire GBA module
	//  for a magic string. It is only called when truly necessary. (i.e. once for now)
	u8 gbatype = gbaGetSaveType();
	
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

void mode_dlp()
{
	// use non-flash card based exploits (download play or Sudoku?). untested, and does not work yet!
	displayPrintState("");
	displayPrintUpper();
	displayPrintLower();
	
	touchPosition touchXY;
	while(1) {
		swiWaitForVBlank();
		touchRead(&touchXY);
		
		// backup
		if ((touchXY.py > 8*0) && (touchXY.py < 8*8)) {
			hwBackupFTP(true);
		}
		
		// restore
		if ((touchXY.py > 8*8) && (touchXY.py < 8*16)) {
			hwRestoreFTP(true);
		}
		
		// erase
		if ((touchXY.py > 8*16) && (touchXY.py < 8*24)) {
			displayPrintUpper();
			hwErase();
			displayMessage2A(STR_HW_DID_DELETE, true);
			while(1);
		}
	}
}

bool loadIniFile(char* path)
{
	// Don't try to load ini file in DLP mode, we need to get our options from a different source.
	//  Still trying to figure something out...
	if (mode == 5)
		return true;

	ini_fd_t ini = 0;
	
	// load ini file...
	char inipath[256];
	if (path) {
		char *last = strrchr(path, '/');
		int len = (last - path)+1;
		strncpy(bootdir, path, len);
		sprintf(inipath, "%s/savegame_manager.ini", bootdir);
		if (fileExists(inipath))
			ini = ini_open(inipath, "r", "");
	}
	if (!ini) {
		sprintf(inipath, "/savegame_manager.ini");
		if (fileExists(inipath))
			ini = ini_open(inipath, "r", "");
	}
	if (!ini) {
		// FIXME
		displayMessage2A(STR_BOOT_NO_INI,true);
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
	
	ini_locateKey(ini, "slot2");
	ini_readInt(ini, &slot2);
	
	ini_close(ini);
	
	// load additional Flash chip signatures (JEDEC IDs)
	ini_locateHeading(ini, "new chips");
	for (int i = 0; i < EXTRA_ARRAY_SIZE; i++) {
		int tmp;
		sprintf(txt, "%i-id", i);
		ini_locateKey(ini, txt);
		ini_readInt(ini, &tmp);
		extra_id[i] = (u32)tmp;
		//
		sprintf(txt, "%i-size", i);
		ini_locateKey(ini, txt);
		ini_readInt(ini, &tmp);
		extra_size[i] = tmp;
	}
	
	// delete temp file (which is a remnant of inilib)
	remove("/tmpfile");
	
	return true;
}

// This is a new attempt to reliably identify if the flash card has argv support or not,
//  including R4 clones which may have *bad* argv support.
bool has_argv(int argc, char* argv[])
{
	// no argv support
	if (argc == 0)
		return false;
	
	// bad argv support: if argc is larger than unity, your flash card can't even do
	//  a proper negative indicator. let us test for 2 argv arguments, just to be safe.
	if (argc > 2)
		return false;
	
	/*
	// test if argv[0] pointer is valid (scan main memory and WRAM)
	if ((argv[0] < 0x02000000) || (argv[0] >= 0x04000000))
		return false;
	*/
	
	// test if argv[0] contains a valid string
	char *arg0 = argv[0];
	// test for possible "/path", "fat*:/path" and "sd:/*".
	if (*arg0 == 'f')
		if (!strncasecmp(argv[0], "fat:/", 5) || !strncasecmp(argv[0], "fat1:/", 6)
			|| !strncasecmp(argv[0], "fat2:/", 6))
			return true;

	if (*arg0 == 's')
		if (!strncasecmp(argv[0], "sd:/", 4))
			return true;
	
	if (*arg0 == '/')
		return true;
	
	return false;
}

int main(int argc, char* argv[])
{
	sysSetBusOwners(true, true);
	
	// Init the screens
	displayInit();

	// detect hardware
	mode = hwDetect();
	
	// Init DLDI (file system driver)
	int fat = fatInitDefault();
	if (fat == 0) {
		displayMessage2A(STR_BOOT_DLDI_ERROR,true);
		while (1);
	}
	
	// Load the ini file with the FTP settings and more options
	for (int i = 0; i < EXTRA_ARRAY_SIZE; i++) {
		extra_id[i] = 0xff000000;
		extra_size[i] = 0;
	}
	if (has_argv(argc, argv))
		loadIniFile(argv[0]);
	else
		loadIniFile(0);

	// load strings
	stringsLoadFile(0);

	// prepare the global data buffer
	data = (u8*)malloc(size_buf);
	// On my Cyclops, no 2MB buffer is available after loading strings! bad memory management?
	//  Anyway, we can't do anything except for the following (which restricts the usefulness
	//  of FTP safe mode)
	//  possible that no continuous 2 MB are available.
	while (data == NULL) {
		size_buf >>= 1;
		data = (u8*)malloc(size_buf);
	}
	
	// okay, we got our HW identified; now branch to the corresponding main function/event handler
	switch (mode) {
		case 1:
			mode_gba();
			break;
		case 2:
			mode_3in1();
			break;
		case 3:
			mode_dsi();
			break;
		case 4:
			mode_slot2();
			break;
		case 5:
			mode_dlp();
			break;
		default:
			mode_wifi();
	}

	return 0;
}
