/*
 * savegame_manager: a tool to backup and restore savegames from Nintendo
 *  DS cartridges. Nintendo DS and all derivative names are trademarks
 *  by Nintendo. EZFlash 3-in-1 is a trademark by EZFlash.
 *
 * gba.cpp: Functions for working with the GBA-slot on a Nintendo DS.
 *    EZFlash 3-in-1 functions are found in dsCard.h/.cpp
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

#include <sys/iosupport.h>
#include <unistd.h>

#include <stdio.h>
#include <string.h>
#include <math.h>

#include <dirent.h>

#include "gba.h"
#include "dsCard.h"


bool slot2Init()
{
	static bool doonce = false;
	static bool init_3in1 = false;
	
	if (doonce)
		return init_3in1;
	
	doonce = true;
	OpenNorWrite();
	uint32 test = ReadNorFlashID();
	CloseNorWrite();
	
	init_3in1 = test;
	if (test)
		return true;
	else
		return false;
}


// -----------------------------------------------------
#define MAGIC_EEPR 0x52504545
#define MAGIC_SRAM 0x4d415253
#define MAGIC_FLAS 0x53414c46

#define MAGIC_H1M_ 0x5f4d3148

saveTypeGBA GetSlot2SaveType(cartTypeGBA type) {
	if (type == CART_GBA_NONE)
		return SAVE_GBA_NONE;
		
	// Search for any one of the magic version strings in the ROM.
	uint32 *data = (uint32*)0x08000000;
	
	for (int i = 0; i < (0x02000000 >> 2); i++, data++) {
		if (*data == MAGIC_EEPR)
			return SAVE_GBA_EEPROM_8; // TODO: Try to figure out 512 bytes version...
		if (*data == MAGIC_SRAM)
			return SAVE_GBA_SRAM_32;
		if (*data == MAGIC_FLAS) {
			uint32 *data2 = data + 1;
			if (*data2 == MAGIC_H1M_)
				return SAVE_GBA_FLASH_128;
			else
				return SAVE_GBA_FLASH_64;
		}
	}
	return SAVE_GBA_NONE;
};

cartTypeGBA GetSlot2Type(uint32 id)
{		
	if (id == 0x53534150)
		// All conventional GBA flash cards identify themselves as "PASS"
		return CART_GBA_FLASH;
	else {
		return CART_GBA_GAME;
	}
};

void IdentifySlot2(dataSlot2 &data)
{
// FIXME...
#if 0
	// Identify an EZFlash 3in1
	OpenNorWrite();
	cartTypeGBA ezflash = slot2IsEZFlash3in1(data.ez_ID);
	//chip_reset();
	CloseNorWrite();
	if (ezflash != CART_GBA_NONE) {
    data.type = cartTypeGBA(ezflash);
    return; // 3in1 has no classic save memory
  } else {
    // it's not a 3in1
    sGBAHeader *gba = (sGBAHeader*)0x08000000;
  	memcpy(&data.name[0], &gba->title[0], 12);
	  data.name[12] = 0;
	  memcpy(&data.iid, &gba->gamecode[0], 4);
	  data.cid[4] = 0;
		data.type = GetSlot2Type(data.iid);
  }

	data.save = GetSlot2SaveType(data.type);
#endif
};

// -----------------------------------------------------------
bool gbaIsGame()
{
	// look for some magic bytes of the compressed Nintendo logo
	uint32 *data = (uint32*)0x08000004;
	
	if (*data == 0x51aeff24) {
		data ++; data ++;
		if (*data == 0x0a82843d)
			return true;
	}
	return false;
}

uint8 gbaGetSaveType()
{
	// Search for any one of the magic version strings in the ROM. They are always dword-aligned.
	uint32 *data = (uint32*)0x08000000;
	
	for (int i = 0; i < (0x02000000 >> 2); i++, data++) {
		if (*data == MAGIC_EEPR)
			return 1; // TODO: Try to figure out how to ID the 512 bytes version...
		if (*data == MAGIC_SRAM)
			return 3;
		if (*data == MAGIC_FLAS) {
			uint32 *data2 = data + 1;
			if (*data2 == MAGIC_H1M_)
				return 4;
			else
				return 5;
		}
	}
	
	return 0;
}

uint32 gbaGetSaveSize(uint8 type)
{
	if (type == 255)
		type = gbaGetSaveType();
	
	switch (type) {
		case 1:
			return 512; // ???
		case 2:
			return 8192;
		case 3:
			return 32768;
		case 4:
			return 65536;
		case 5:
			return 131072;
		case 0:
		default:
			return 0;
	}
}
