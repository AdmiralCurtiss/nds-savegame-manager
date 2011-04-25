/*
 * savegame_manager: a tool to backup and restore savegames from Nintendo
 *  DS cartridges. Nintendo DS and all derivative names are trademarks
 *  by Nintendo. EZFlash 3-in-1 is a trademark by EZFlash.
 *
 * hardware.cpp: Functions used to communicate with slot-1 devices, including
 *    hotswap and identification of cartridges.
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
#include <nds/interrupts.h>
#include <nds/arm9/console.h>
#include <sys/dir.h>
#include <sys/unistd.h>

#include <stdio.h>
#include <algorithm>

#include "display.h"
#include "dsCard.h"
#include "gba.h"

#include "hardware.h"

#include "fileselect.h"

#include "auxspi.h"

using namespace std;

uint32 boot;

bool flash_card = true; // the homebrew will always start with a FC inserted
bool with_infrared = false; // ... which does not have an IR device

u8 data[0x8000];

// ---------------------------------------------------------------------
bool swap_cart()
{
	sNDSHeader nds;
	nds.gameTitle[0] = 0;
	
	while (!nds.gameTitle[0]) {
		// this is an attempt to account for bad contacts, where "cardreadheader" returns nothing.
		//  since this problem disappeared after adding this (as usual), I don't know if it works
	
		displayMessage("Please take out Slot 1 flash\ncart and insert a game\n\nPress A when done.");

		bool swap = false;
		while (!swap) {
			if (keysCurrent() & KEY_A) {
				// identify hardware
				sysSetBusOwners(true, true);
				cardReadHeader((u8*)&nds);
				if (!nds.gameTitle[0])
					continue;
			
				// Look for an IR-enabled game cart first. If the save size returns something
				//  nonzero, we hae found an IR-enabled game and we are done. If there is no
				//  IR device present, "size2" is always zero.
				with_infrared = true;
				uint32 size2 = auxspi_save_size_log_2();
				if (size2) {
					flash_card = false;
					return true;
				}
			
				// Second, test if this is a Flash Card, which has no save chip. Therefore,
				//  trying to identify this without IR-specific functions again returns zero.
				with_infrared = false;
				uint32 size1 = auxspi_save_size_log_2();
				if (!size1) {
					flash_card = true;
					return true;
				}
			
				// No special hardware found, so it must be a regular game.
				flash_card = false;
				return true;
			}
		}
	}
	
	return true;
}

bool is_flash_card()
{
	return flash_card;
}

// --------------------------------------------------------
void hwFormatNor(uint32 page, uint32 count)
{
	chip_reset();
	OpenNorWrite();
	SetSerialMode();
	displayPrintState("Formating NOR memory");
	displayProgressBar(0, count);
	for (int i = page; i < page+count; i++) {
		Block_Erase(i << 18);
		displayProgressBar(i+1, count);
	}
	CloseNorWrite();
}

// --------------------------------------------------------
void hwBackup3in1()
{
	uint8 size = auxspi_save_size_log_2();
	int size_blocks = 1 << max(0, (int8(size) - 18)); // ... in units of 0x40000 bytes - that's 256 kB
	uint8 type = auxspi_save_type();

	hwFormatNor(0, size_blocks);

	// Dump save and write it to NOR
	chip_reset();
	OpenNorWrite();
	displayPrintState("Writing save to NOR");
	if (size < 15)
		size_blocks = 1;
	else
		size_blocks = 1 << (uint8(size) - 15);
	for (int i = 0; i < size_blocks; i++) {
		displayProgressBar(i+1, size_blocks);
		auxspi_read_data(i << 15, data, 0x8000, type);
		WriteNorFlash(i << 15, data, 0x8000);
	}
	CloseNorWrite();

	// Write a flag to tell the app what happens on restart
	sNDSHeader nds;
	cardReadHeader((u8*)&nds);
	dsCardData data2;
	memset(&data2, 0, sizeof(data2));
	data2.data[0] = RS_BACKUP;
	data2.data[1] = 0;
	data2.data[2] = size;
	data2.data[3] = 0xffff00ff;
	memcpy(&data2.name[0], &nds.gameTitle[0], 12);
	WriteSram(0x0a000000, (u8*)&data2, sizeof(data2));
	char txt[128];
	sprintf(txt, "%.12s", &nds.gameTitle[0]);
	//memcpy(&txt[0], &nds.gameTitle[0], 12);
	//displayPrintState(txt);

	displayMessage("Save has been written to 3in1.\nPlease power off and restart\nthis tool.");

	while(1) {};
}

void hwDump3in1(uint32 size, const char *gamename)
{
	displayPrintState("Writing file to flash card");

	u32 size_blocks = 1 << max(0, (uint8(size) - 18));

	char path[256];
	char fname[256] = "";
	//fileSelect("/", path, fname/*, true*/);
	//if (!fname[0]) {
		uint32 cnt = 0;
		sprintf(fname, "/%s.%i.sav", gamename, cnt);
		while (fileExists(fname)) {
			if (cnt < 65536)
				cnt++;
			else {
				displayMessage("Unable to get a filename!\nThis means that you have more\nthan 65536 saves! (wow!)\nOops!");
				while(1);
			}
			sprintf(fname, "/%s.%i.sav", gamename, cnt);
		}
	//}
	char fullpath[512];
	//sprintf(fullpath, "%s/%s", path, fname);
	sprintf(fullpath, "%s", fname);
	displayMessage(fullpath);
  
	FILE *file = fopen(fullpath, "wb");
	//u8 *data = (u8*)malloc(0x8000);
	if (size < 15)
		size_blocks = 1;
	else
		size_blocks = 1 << (uint8(size) - 15);
	for (u32 i = 0; i < size_blocks; i++) {
		displayProgressBar(i+1, size_blocks);
		u32 LEN = 0x8000;
		ReadNorFlash(data, i << 15, LEN);
		fwrite(data, 1, LEN, file);
		//if (j != LEN)
			//iprintf ("WRITE ERROR %i\n", errno);
	}
	//free(data);
	fclose(file);

	displayPrintState("Done! Please power off...");
	while (1);
}

void hwRestore3in1()
{
	char path[256];
	char fname[256];
	displayMessage("Please select a .sav file.");
	fileSelect("/", path, fname);
	displayMessage("");
	char msg[256];
	sprintf(msg, "%s%s", path, fname);
	
	FILE *file = fopen(msg, "rb");
	uint32 size0 = fileSize(msg);
	uint8 size = 1;
	while (size0 > (1 << size)) {
		size++;
	}
	int size_blocks = 1 << max(0, int8(size) - 18); // ... in units of 0x40000 bytes - that's 256 kB
	
	hwFormatNor(0, size_blocks);

	// Read save and write it to NOR
	chip_reset();
	OpenNorWrite();
	SetSerialMode();
	displayPrintState("Writing save to NOR");
	if (size < 15)
		size_blocks = 1;
	else
		size_blocks = 1 << (uint8(size) - 15);
	for (int i = 0; i < size_blocks; i++) {
		displayProgressBar(i+1, size_blocks);
		fread(data, 1, 0x8000, file);
		WriteNorFlash(i << 15, data, 0x8000);
	}
	CloseNorWrite();
	fclose(file);
	
	// FIXME: for some weird reason, reading the save written to NOR right now managed to mangle
	//  precisely 2 bytes in my HG save, but not in my Platinum one. Therefore, we need to
	//  make this a two-stage process as well, to prevent this weitd glitch.
	//  (It appears in two different hardware revisions of the 3in1!)

	// Write a flag to tell the app what happens on restart
	sNDSHeader nds;
	cardReadHeader((u8*)&nds);
	dsCardData data2;
	memset(&data2, 0, sizeof(data2));
	data2.data[0] = RS_BACKUP;
	data2.data[1] = 0;
	data2.data[2] = size;
	data2.data[3] = 0xbad00bad;
	WriteSram(0x0a000000, (u8*)&data2, sizeof(data2));

	displayMessage("Thanks to some crazy bug,\nyou will need to restart your\nDS and this app. Sorry.");
	while(0);
}

void hwRestore3in1_b(uint32 size)
{

	// Third, swap in a new game
	swap_cart();
	displayPrintUpper();
	
	uint8 type = auxspi_save_type();
	if (type == 3) {
		displayPrintState("Formating Flash chip");
		auxspi_erase();
	}

	// And finally, write the save!
	displayPrintState("Restoring save");
	u32 LEN = 0, num_blocks = 0, shift = 0;
	switch (type) {
	case 1:
		shift = 4; // 16 bytes
		break;
	case 2:
		shift = 5; // 32 bytes
		break;
	case 3:
		shift = 8; // 256 bytes
		break;
	default:
		return;
	}
	LEN = 1 << shift;
	num_blocks = 1 << (size - shift);
	//data = (u8*)malloc(LEN);

	chip_reset();
	OpenNorWrite();
	SetSerialMode();
	for (unsigned int i = 0; i < num_blocks; i++) {
		if (i % (num_blocks >> 6) == 0)
			displayProgressBar(i+1, num_blocks);
		ReadNorFlash(data, i << shift, LEN);
		auxspi_write_data(i << shift, data, LEN, type);
	}
	CloseNorWrite();
	displayProgressBar(1, 1);

	
	displayPrintState("Done! Please turn your DS off...");
	
	while (1) {};
}

void hwErase()
{
	displayMessage("This will WIPE OUT your entire\nsave! ARE YOU SURE?\n\nPress R+up+Y to confim!");
	while (!(keysCurrent() & (KEY_UP | KEY_R | KEY_Y))) {};
	auxspi_erase();
	displayMessage("Done! Your save is gone!");
	while(0);
}

// ------------------------------------------------------------
void hwBackupGBA()
{
	// TODO: test this, implement missing bits!
	return;
	
	// TODO: select filename
	char fullname[512];
	
	u8 type = gbaGetSaveType();
	if (type == 0)
		return;
	uint32 size = gbaGetSaveSize(type);
	u8 nbanks = 2;
	
	switch (type) {
	case 1:
	case 2:
		// TODO: how to address this one?
		return;
	case 3: {
		// SRAM - this is easy, just functions like conventional RAM
		FILE *file = fopen(&fullname[0], "wb");
		for (u32 i = 0; i < size; i++) {
			fwrite((u8*)(0x0e000000+i), 1, 1, file);
		}
		fclose(file);
		return;
	}
	case 4:
		nbanks = 1;
	case 5: {
		// FLASH - must be opened by register magic
		FILE *file = fopen(&fullname[0], "wb");
		for (int j = 0; j < nbanks; j++) {
			*(u8*)0x0e005555 = 0xaa;
			*(u8*)0x0e002aaa = 0x55;
			*(u8*)0x0e005555 = 0xb0;
			*(u8*)0x0e000000 = j;
			for (int i = 0; i < 0x10000; i++) {
				fwrite((u8*)(0x0e000000+i), 1, 1, file);
			}
		}
		fclose(file);
		return;
	}
	}
}

void hwRestoreGBA()
{
	// TODO: test this, implement missing bits!
	return;
	
	// TODO: select filename
	char fullname[512];
	
	u8 type = gbaGetSaveType();
	if (type == 0)
		return;
	uint32 size = gbaGetSaveSize(type);
	u8 nbanks = 2;
	
	switch (type) {
	case 1:
	case 2:
		// TODO: how to address this one?
		return;
	case 3: {
		// SRAM - this is easy, just functions like conventional RAM
		FILE *file = fopen(&fullname[0], "rb");
		u8 *data = (u8*)malloc(size);
		fread(&data[0], 1, size, file);
		for (u32 i = 0; i < size; i++) {
			*(u8*)(0x0e000000+i) = data[i];
		}
		free(data);
		fclose(file);
		return;
	}
	case 4:
		nbanks = 1;
	case 5: {
		// FLASH - must be opened by register magic
		FILE *file = fopen(&fullname[0], "wb");
		for (int j = 0; j < nbanks; j++) {
			*(u8*)0x0e005555 = 0xaa;
			*(u8*)0x0e002aaa = 0x55;
			*(u8*)0x0e005555 = 0xb0;
			*(u8*)0x0e000000 = j;
			for (int i = 0; i < 0x10000; i++) {
				fwrite((u8*)(0x0e000000+i), 1, 1, file);
			}
		}
		fclose(file);
		return;
	}
	}
}

void hwEraseGBA()
{
	// TODO: implement chip erase!
}
