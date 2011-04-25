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

#include "gba.h"
#include "display.h"
#include "dsCard.h"
#include "hardware.h"
#include "fileselect.h"



uint32 ezflash = 0;
uint32 dstype = 0;
bool gba = 0;
uint32 mode = 0;



// ============================================================================
void mode_dsi()
{
	// DSi mode, does nothing at the moment
	displayPrintState("DSi mode - unsupported!");
	while (1);
}

void mode_3in1()
{
	displayPrintUpper();
	
	dsCardData data2;
	ReadSram(0x0a000000, (u8*)&data2, sizeof(data2));
	if ((data2.data[0] == RS_BACKUP) && (data2.data[1] == 0) && (data2.data[3] == 0xffff00ff)) {
		uint32 size = data2.data[2];
		char name[13];
		memcpy(&name[0], &data2.name[0], 12);
		name[12] = 0;
		memset(&data2, 0, sizeof(data2));
		WriteSram(0x0a000000, (u8*)&data2, sizeof(data2));
		hwDump3in1(size, name);
	} else if ((data2.data[0] == RS_BACKUP) && (data2.data[1] == 0) && (data2.data[3] == 0xbad00bad)) {
		uint32 size = data2.data[2];
		char name[13];
		memcpy(&name[0], &data2.name[0], 12);
		name[12] = 0;
		memset(&data2, 0, sizeof(data2));
		WriteSram(0x0a000000, (u8*)&data2, sizeof(data2));
		hwRestore3in1_b(size);
	}
	
	// use 3in1 to buffer data
	displayPrintState("3in1 mode");
	displayPrintUpper();
	displayPrintLower();
	
	touchPosition touchXY;
	while(1) {
		swiWaitForVBlank();
		touchRead(&touchXY);
		
		// backup
		if ((touchXY.py > 8*0) && (touchXY.py < 8*8)) {
			swap_cart();
			displayPrintUpper();
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
		}
	}
}

void mode_gba()
{
	// use 3in1 to buffer data
	displayPrintState("gba mode");
	displayPrintUpper();
	displayPrintLower();
	
	touchPosition touchXY;
	while(1) {
		swiWaitForVBlank();
		touchRead(&touchXY);
		
		// backup
		if ((touchXY.py > 8*0) && (touchXY.py < 8*8)) {
			displayPrintUpper();
			hwBackupGBA();
			//hwBackup3in1();
		}
		
		// restore
		if ((touchXY.py > 8*8) && (touchXY.py < 8*16)) {
			displayPrintUpper();
			hwRestoreGBA();
		}
		
		// erase
		if ((touchXY.py > 8*16) && (touchXY.py < 8*24)) {
			displayPrintUpper();
			hwEraseGBA();
		}
	}
}

void mode_wifi()
{
	// read/write to ftp server, does nothing at the moment
	displayPrintState("FTP mode - unsupported!");
	while (1);
}

int main(void)
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
	
	// TODO: load ini file...
	
	// Identify hardware and branch to corresponding mode
	displayPrintState("Identifying hardware...");
	OpenNorWrite();
	ezflash = ReadNorFlashID();
	CloseNorWrite();
	dstype = 0; // DS/DSL, no idea how to identify DSi etc.
	gba = gbaIsGame();
	
	if (dstype == 1) {
		// DSi/DSiXL, will branch to SD-card mode when cracked.
		mode = 3;
		mode_dsi();
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
