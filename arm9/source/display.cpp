/*
 * savegame_manager: a tool to backup and restore savegames from Nintendo
 *  DS cartridges. Nintendo DS and all derivative names are trademarks
 *  by Nintendo. EZFlash 3-in-1 is a trademark by EZFlash.
 *
 * display.cpp: A collection of shared functions used to print various
 *    status messages and feedback on the screens.
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

#include <stdio.h>

#include "display.h"
#include "auxspi.h"
#include "hardware.h"
#include "fileselect.h"
#include "gba.h"

#include "auxspi_core.cpp"

#include "globals.h"

PrintConsole upperScreen;
PrintConsole lowerScreen;


extern uint32 mode;
extern u8 gbatype;


//===========================================================
void displayInit()
{
	videoSetMode(MODE_0_2D);
	vramSetBankA(VRAM_A_MAIN_BG);
	consoleInit(&upperScreen, 3,BgType_Text4bpp, BgSize_T_256x256, 31, 0, true, true);

	videoSetModeSub(MODE_0_2D);
	vramSetBankC(VRAM_C_SUB_BG);
	consoleInit(&lowerScreen, 3,BgType_Text4bpp, BgSize_T_256x256, 31, 0, false, true);

	consoleSelect(&upperScreen);
	iprintf("\n\n\n\n\nDS savegame manager\nVersion 0.2.3 Beta\nBy Pokedoc");
	
	displayPrintState("Press (B) to continue");
	while (!(keysCurrent() & KEY_B));
}

void displayPrintUpper()
{
	extern uint32 ezflash;
	extern bool gba;
	extern uint32 dstype;
	extern bool slot2;

	// print upper screen (background)
	consoleSelect(&upperScreen);
	consoleSetWindow(&upperScreen, 0, 0, 32, 24);
	consoleClear();
	iprintf("Mode     :\n");
	iprintf("--- SLOT 1 ---------------------");
	iprintf("Game ID  :\n");
	iprintf("Game name:\n");
	iprintf("Game save:\n");
	iprintf("Special  :\n");
	iprintf("--- SLOT 2 ---------------------");
	if (dstype > 0)
		iprintf("This device has no Slot-2\n");
	else {
		iprintf("Game ID  :\n");
		iprintf("Game name:\n");
		iprintf("Game save:\n");
		iprintf("Special  :\n");
	}
	
	// print upper screen
	consoleSetWindow(&upperScreen, 10, 2, 22, 4);
	consoleClear();
	consoleSetWindow(&upperScreen, 10, 7, 22, 4);
	consoleClear();
	
	// fetch cartridge header (maybe, calling "cardReadHeader" on a FC messes with libfat!)
	bool flash_card = is_flash_card();
	bool ir = auxspi_has_infrared();
	sNDSHeader nds;
	if (!flash_card)
		cardReadHeader((uint8*)&nds);
	// search for the correct header
	/*
	for (int i = 0; i < 0x1000000; i++)
	{
		char *test = (char*)0x02000000;
		if ((test[0]=='C')&&(test[1]=='P')&&(test[2]=='U')&&(test[3]=='D')) {
			iprintf("found!");
			while(1);
		}
		test++;
	}
	*/
	//nds = NDS_HEADER;
	/*
	if (nds.gameTitle[0] == 0xff)
		// DSi
		nds = NDS_HEADER;
		*/
	
	char name[MAXPATHLEN];
	// 0) print the mode
	consoleSetWindow(&upperScreen, 10, 0, 20, 1);
	switch (mode) {
	case 0:
		sprintf(&name[0], "WiFi/FTP");
		break;
	case 1:
		sprintf(&name[0], "GBA");
		break;
	case 2:
		sprintf(&name[0], "3-in-1");
		break;
	case 3:
		sprintf(&name[0], "DSi/SD");
		break;
	case 4:
		sprintf(&name[0], "Slot 2");
		break;
	}
	consoleClear();
	iprintf("%s", name);
	
	// 1) The cart id.
	consoleSetWindow(&upperScreen, 10, 2, 22, 1);
	sprintf(&name[0], "----");
	if (flash_card) {
		sprintf(&name[0], "Flash Card");
	} else /*if (nds.gameCode[0])*/ {
		memcpy(&name[0], &nds.gameCode[0], 4);
		name[4] = 0x00;
	}
	consoleClear();
	iprintf("%s", name);

	// 2) The cart name.
	consoleSetWindow(&upperScreen, 10, 3, 22, 1);
	sprintf(&name[0], "----");
	if (flash_card) {
		sprintf(&name[0], "Flash Card");
	} else /*if (nds.gameTitle[0])*/ {
		memcpy(&name[0], &nds.gameTitle[0], 12);
		name[12] = 0x00;
	}
	consoleClear();
	iprintf("%s", name);

	// 3) The save type
	consoleSetWindow(&upperScreen, 10, 4, 22, 1);
	sprintf(&name[0], "----");
	if (flash_card) {
		sprintf(&name[0], "Flash Card");
	} else {
		uint8 type = auxspi_save_type();
		uint32 size = auxspi_save_size();
		switch (type) {
		case 1:
			sprintf(&name[0], "Eeprom (%i Bytes)", size);
			break;
		case 2:
			sprintf(&name[0], "FRAM (%i kB)", size >> 10);
			break;
		case 3:
			sprintf(&name[0], "Flash (%i kB)", size >> 10);
			break;
		default:
			sprintf(&name[0], "unknown");
			break;
		}
	}
	consoleClear();
	iprintf("%s", name);

	// 4) Special properties (infrared device...)
	consoleSetWindow(&upperScreen, 10, 5, 22, 1);
	consoleClear();
	memset(&name[0], 0, MAXPATHLEN);
	//if (auxspi_has_infrared()) {
	if (ir) {
		sprintf(&name[0], "Infrared");
	} else {
		sprintf(&name[0], "----");
	}
	iprintf("%s", name);
	
	// Slot 2 status
	// 5) GBA game id
	consoleSetWindow(&upperScreen, 10, 7, 22, 1);
	consoleClear();
	memset(&name[0], 0, MAXPATHLEN);
	if (ezflash) {
		if (ezflash == 0x89168916)
			sprintf(name, "3in1 (512M)");
		else if (ezflash == 0x227E2218)
			sprintf(name, "3in1 (256M V2)");
		else if (ezflash == 0x227E2202)
			sprintf(name, "3in1 (256M V1)");
		else
			sprintf(name, "3in1 (unknown type)");
	} else if (gba)
		sprintf(name, "%.4s", (char*)0x080000ac);
	else if (slot2)
		sprintf(name, "Flash Card");
	else if (dstype == 0)
		sprintf(name, "----");
	if (dstype == 0)
		iprintf("%s", name);

	// 6) GBA game name
	consoleSetWindow(&upperScreen, 10, 8, 22, 1);
	consoleClear();
	memset(&name[0], 0, MAXPATHLEN);
	if (ezflash)
		sprintf(name, "3in1");
	else if (gba)
		sprintf(name, "%.12s", (char*)0x080000a0);
	else if (slot2)
		sprintf(name, "Flash Card");
	else if (dstype == 0)
		sprintf(name, "----");
	if (dstype == 0)
		iprintf(name);

	// 7) GBA save size
	consoleSetWindow(&upperScreen, 10, 9, 22, 1);
	consoleClear();
	memset(&name[0], 0, MAXPATHLEN);
	if (ezflash)
		sprintf(name, "SRAM");
	else if (gba) {
		u8 type = gbatype;
		u8 size = gbaGetSaveSizeLog2(type);
		switch (type) {
			case 1:
			case 2:
				sprintf(name, "EEPROM (%i bytes)", 1 << size);
				break;
			case 3:
				sprintf(name, "SRAM (%i kB)", 1 << (size - 10));
				break;
			case 4:
			case 5:
				sprintf(name, "Flash (%i kB)", 1 << (size - 10));
				break;
			default:
				sprintf(name, "(none)");
		}
	}
	else if (slot2)
		sprintf(name, "Flash Card");
	else if (dstype == 0)
		sprintf(name, "----");
	if (dstype == 0)
		iprintf(name);

	// 8) GBA special stuff
	consoleSetWindow(&upperScreen, 10, 10, 22, 1);
	consoleClear();
	memset(&name[0], 0, MAXPATHLEN);
	if (ezflash)
		sprintf(name, "NOR + PSRAM");
	else if (gba)
		sprintf(name, "(unsupported)");
	else if (slot2)
		sprintf(name, "----");
	else if (dstype == 0)
		sprintf(name, "----");
	if (dstype == 0)
		iprintf(name);
}

void displayPrintLower()
{
	consoleSelect(&lowerScreen);
	consoleSetWindow(&lowerScreen, 0, 0, 32, 24);
	consoleClear();
	iprintf("+------------------------------+");
	for (int i = 0; i < 6; i++) {
		iprintf("|                              |");
	}
	iprintf("+------------------------------+");
	iprintf("+------------------------------+");
	for (int i = 0; i < 6; i++) {
		iprintf("|                              |");
	}
	iprintf("+------------------------------+");
	iprintf("+------------------------------+");
	for (int i = 0; i < 6; i++) {
		iprintf("|                              |");
	}
	iprintf("+------------------------------+");

	consoleSetWindow(&lowerScreen, 1, 1, 30, 6);
	iprintf("\n\n            BACKUP\n");
	iprintf("         Game -> .sav");

	consoleSetWindow(&lowerScreen, 1, 9, 30, 6);
	iprintf("\n\n            RESTORE\n");
	iprintf("         .sav -> Game");

	consoleSetWindow(&lowerScreen, 1, 17, 30, 6);
	iprintf("\n             RESET\n\n");
	iprintf("    WIPES OUT ALL SAVE DATA\n");
	iprintf("         ON YOUR GAME !");
}

void displayMessage(const char *msg)
{
  consoleSelect(&upperScreen);
  consoleSetWindow(0, 0, 16, 32, 6);
  consoleClear();

  iprintf("%s", msg);
}

void displayPrintState(const char *txt)
{
  swiWaitForVBlank();
  consoleSelect(&upperScreen);
  consoleSetWindow(0, 0, 22, 32, 1);
  consoleClear();
  iprintf("%s", txt);
}


void displayProgressBar(int cur, int max0)
{
  swiWaitForVBlank();
  consoleSelect(&upperScreen);
  consoleSetWindow(0, 0, 23, 32, 1);
  consoleClear();

  char buffer[33];

  int percent = float(cur)/float(max0)*100;
  if (percent > 100)
    percent = 100;
  sprintf(&buffer[14], "%i%%", percent);

  buffer[0] = '[';
  int steps = float(cur)/float(max0)*30;
  if (steps > 30)
    steps = 30;
  for (int i = 1; i <= 30; i++) {
    if ((i >= 14) && (i <= 15))
      continue;
    if ((i == 16) && (percent >= 10))
      continue;
    if ((i == 17) && (percent == 100))
      continue;
    if (i <= steps)
      buffer[i] = '#';
    else
      buffer[i] = '-';
  }
  buffer[31] = ']';
  buffer[32] = 0;

  iprintf("%s", buffer);
}

