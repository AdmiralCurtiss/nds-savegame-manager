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
#include "globals.h"
#include "strings.h"


PrintConsole upperScreen;
PrintConsole lowerScreen;



//===========================================================
void displayInit()
{
	videoSetMode(MODE_0_2D);
	vramSetBankA(VRAM_A_MAIN_BG);
	consoleInit(&upperScreen, 3,BgType_Text4bpp, BgSize_T_256x256, 31, 0, true, true);

	videoSetModeSub(MODE_0_2D);
	vramSetBankC(VRAM_C_SUB_BG);
	consoleInit(&lowerScreen, 3,BgType_Text4bpp, BgSize_T_256x256, 31, 0, false, true);
}

void displayTitle()
{
	displayMessageF(STR_TITLE_MSG);
	
	displayStateF(STR_STR, "Press (B) to continue");
	while (!(keysCurrent() & KEY_B));
}

void displayPrintUpper()
{
	bool gba = (mode == 1);
	u32 dstype = (mode == 3) ? 1 : 0;
	bool ir = (slot_1_type == 1) ? true : false;

	// print upper screen (background)
	consoleSelect(&upperScreen);
	consoleSetWindow(&upperScreen, 0, 0, 32, 24);
	consoleClear();
	iprintf("Mode     :\n");
	iprintf("Memory   :\n");
	iprintf("--- SLOT 1 ---------------------");
	iprintf("Game ID  :\n");
	iprintf("Game name:\n");
	iprintf("Game save:\n");
	iprintf("Special  :\n");
	iprintf("--- SLOT 2 ---------------------");
	if (dstype > 0)
		iprintf("This device has no Slot 2\n");
	else {
		iprintf("Game ID  :\n");
		iprintf("Game name:\n");
		iprintf("Game save:\n");
		iprintf("Special  :\n");
	}
	
	// print upper screen
	consoleSetWindow(&upperScreen, 10, 3, 22, 4);
	consoleClear();
	consoleSetWindow(&upperScreen, 10, 8, 22, 4);
	consoleClear();
	
	// fetch cartridge header (maybe, calling "cardReadHeader" on a FC messes with libfat!)
	sNDSHeader nds;
	if (slot_1_type != 2)
		cardReadHeader((uint8*)&nds);
	
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
		sprintf(&name[0], "3in1");
		break;
	case 3:
		sprintf(&name[0], "DSi/SD");
		break;
	case 4:
		sprintf(&name[0], "Slot 2");
		break;
	case 5:
		sprintf(&name[0], "Download Play");
		break;
	default:
		sprintf(&name[0], "* ??? *");
		break;
	}
	consoleClear();
	iprintf("%s", name);
	// 0.5) memory buffer size
	consoleSetWindow(&upperScreen, 10, 1, 20, 1);
	iprintf("%i kB", size_buf >> 10);
	
	// 1) The cart id.
	consoleSetWindow(&upperScreen, 10, 3, 22, 1);
	sprintf(&name[0], "----");
	if (slot_1_type == 2) {
		sprintf(&name[0], "Flash Card");
	} else {
		memcpy(&name[0], &nds.gameCode[0], 4);
		name[4] = 0x00;
	}
	consoleClear();
	iprintf("%s", name);

	// 2) The cart name.
	consoleSetWindow(&upperScreen, 10, 4, 22, 1);
	sprintf(&name[0], "----");
	if (slot_1_type == 2) {
		sprintf(&name[0], "Flash Card");
	} else {
		memcpy(&name[0], &nds.gameTitle[0], 12);
		name[12] = 0x00;
	}
	consoleClear();
	iprintf("%s", name);

	// 3) The save type
	consoleSetWindow(&upperScreen, 10, 5, 22, 1);
	sprintf(&name[0], "----");
	if (slot_1_type == 2) {
		sprintf(&name[0], "Flash Card");
	} else {
		uint8 type = auxspi_save_type(ir);
		uint8 size = auxspi_save_size_log_2(ir);
		switch (type) {
		case 1:
			sprintf(&name[0], "Eeprom (%i Bytes)", size);
			break;
		case 2:
			sprintf(&name[0], "FRAM (%i kB)", 1 << (size - 10));
			break;
		case 3:
			if (size == 0)
				sprintf(&name[0], "Flash (ID:%x)", auxspi_save_jedec_id(ir));
			else
				sprintf(&name[0], "Flash (%i kB)", 1 << (size - 10));
			break;
		default:
			sprintf(&name[0], "???");
			break;
		}
	}
	consoleClear();
	iprintf("%s", name);

	// 4) Special properties (infrared device...)
	consoleSetWindow(&upperScreen, 10, 6, 22, 1);
	consoleClear();
	memset(&name[0], 0, MAXPATHLEN);
	if (slot_1_type == 1) {
		sprintf(&name[0], "Infrared");
	} else {
		sprintf(&name[0], "----");
	}
	iprintf("%s", name);
	
	// Slot 2 status
	// 5) GBA game id
	consoleSetWindow(&upperScreen, 10, 8, 22, 1);
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
			sprintf(name, "3in1 (???M)");
	} else if (gba)
		sprintf(name, "%.4s", (char*)0x080000ac);
	else if (slot2 > 0)
		sprintf(name, "Flash Card");
	else if (dstype == 0)
		sprintf(name, "----");
	if (dstype == 0)
		iprintf("%s", name);

	// 6) GBA game name
	consoleSetWindow(&upperScreen, 10, 9, 22, 1);
	consoleClear();
	memset(&name[0], 0, MAXPATHLEN);
	if (ezflash)
		sprintf(name, "3in1");
	else if (gba)
		sprintf(name, "%.12s", (char*)0x080000a0);
	else if (slot2 > 0)
		sprintf(name, "Flash Card");
	else if (dstype == 0)
		sprintf(name, "----");
	if (dstype == 0)
		iprintf(name);

	// 7) GBA save size
	consoleSetWindow(&upperScreen, 10, 10, 22, 1);
	consoleClear();
	memset(&name[0], 0, MAXPATHLEN);
	if (ezflash)
		sprintf(name, "SRAM");
	else if (gba) {
		saveTypeGBA type = GetSlot2SaveType(CART_GBA_GAME);
		u8 size = gbaGetSaveSizeLog2(type);
		switch (type) {
			case SAVE_GBA_EEPROM_05:
			case SAVE_GBA_EEPROM_8:
				sprintf(name, "EEPROM (%i bytes)", 1 << size);
				break;
			case SAVE_GBA_SRAM_32:
				sprintf(name, "SRAM (%i kB)", 1 << (size - 10));
				break;
			case SAVE_GBA_FLASH_64:
			case SAVE_GBA_FLASH_128:
				sprintf(name, "Flash (%i kB)", 1 << (size - 10));
				break;
			default:
				sprintf(name, "(none)");
		}
	}
	else if (slot2 > 0)
		sprintf(name, "Flash Card");
	else if (dstype == 0)
		sprintf(name, "----");
	if (dstype == 0)
		iprintf(name);

	// 8) GBA special stuff
	consoleSetWindow(&upperScreen, 10, 11, 22, 1);
	consoleClear();
	memset(&name[0], 0, MAXPATHLEN);
	if (ezflash)
		sprintf(name, "NOR + PSRAM");
	else if (gba)
		// TODO: test for RTC, add function for syncing RTC?
		sprintf(name, "???");
	else if (slot2 > 0)
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
	iprintf("\n             RESET\n");
	iprintf(stringsGetMessageString(STR_MM_WIPE));
}

void displayPrintState(const char *txt)
{
  swiWaitForVBlank();
  consoleSelect(&upperScreen);
  consoleSetWindow(0, 0, 22, 32, 1);
  consoleClear();
  iprintf("%s", txt);
}

// ----- internal function
char *ParseLine(char *start, const char *end, int &length)
{
	// This function takes a line and does a quick word-wrap, by fitting it into a certain fixed-length line
	int len = 0;
	char *cur = start;
	char *separator = start;
	
	while (start < end) {
		if (*start == '\n') {
			length = 1;
			return start;
		}
		
		// look for a working "start" position
		if ((*start == ' ') || (*start == '\n') || (*start == '\t')) {
			start++; cur++; separator++;
			continue;
		}
		
		// do count characters
		if (len < length) {
			cur++; len++;
			if ((*cur == ' ') || (*cur == '\n') || (*cur == '\t') || (*cur == '\0')) {
				separator = cur;
				if ((*cur == '\n')  || (*cur == '\0'))
					break;
			}
		} else {
			// emergency exit
			if (!separator)
				separator = cur;
			break;
		}
	}
	
	length = separator - start;
	return start;
}
// ------------------

void displayStateF(int id, ...)
{
	// prevent flickering
	swiWaitForVBlank();
	
	va_list argp;
	va_start(argp, id);
	memset(txt, 0, 256);
	vsnprintf(txt, 256, stringsGetMessageString(id), argp);
	va_end(argp);
	
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
  
  if (max0 == 0)
	buffer[0] = 0;

  iprintf("%s", buffer);
}

void displayMessageF(int id, ...)
{
	va_list argp;
	va_start(argp, id);
	memset(txt, 0, 256);
	vsnprintf(txt, 256, stringsGetMessageString(id), argp);
	va_end(argp);
	
	consoleSelect(&upperScreen);
	consoleSetWindow(&upperScreen, 0, 16, 32, 6);
	consoleClear();
	
	char *start = txt;
	char *end = start + strlen(txt);
	for (int i = 0; i < 6; i++) {
		int l;
		l = 32;
		start = ParseLine(start, end, l);
		consoleSetWindow(&upperScreen, 0, i+16, 32, 1);
		char tmp = *(start+l);
		*(start+l) = '\0';
		iprintf(start);
		*(start+l) = tmp;
		start += l+1;
		if (start >= end)
			break;
	}
}

void displayMessage2F(int id, ...)
{
	va_list argp;
	va_start(argp, id);
	memset(txt, 0, 256);
	vsnprintf(txt, 256, stringsGetMessageString(id), argp);
	va_end(argp);
	
	consoleSelect(&lowerScreen);
	consoleSetWindow(&lowerScreen, 0, 0, 32, 24);
	consoleClear();
	
	char *start = txt;
	char *end = start + strlen(txt);
	for (int i = 0; i < 20; i++) {
		int l;
		l = 28;
		start = ParseLine(start, end, l);
		consoleSetWindow(&lowerScreen, 2, i+2, 28, 1);
		char tmp = *(start+l);
		*(start+l) = '\0';
		iprintf(start);
		*(start+l) = tmp;
		start += l+1;
		if (start >= end)
			break;
	}
}

void displayWarning2F(int id, ...)
{
	va_list argp;
	va_start(argp, id);
	memset(txt, 0, 256);
	vsnprintf(txt, 256, stringsGetMessageString(id), argp);
	va_end(argp);
	
	consoleSelect(&lowerScreen);
	consoleSetWindow(&lowerScreen, 0, 0, 32, 24);
	consoleClear();
	
	iprintf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
	for (int i = 0; i < 22; i++)
		iprintf("!                              !");
	iprintf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
	
	char *start = txt;
	char *end = start + strlen(txt);
	for (int i = 0; i < 20; i++) {
		int l;
		l = 28;
		start = ParseLine(start, end, l);
		consoleSetWindow(&lowerScreen, 2, i+2, 28, 1);
		char tmp = *(start+l);
		*(start+l) = '\0';
		iprintf(start);
		*(start+l) = tmp;
		start += l+1;
		if (start >= end)
			break;
	}
}
