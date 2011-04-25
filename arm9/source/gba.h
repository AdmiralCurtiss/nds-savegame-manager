/*
 * savegame_manager: a tool to backup and restore savegames from Nintendo
 *  DS cartridges. Nintendo DS and all derivative names are trademarks
 *  by Nintendo. EZFlash 3-in-1 is a trademark by EZFlash.
 *
 * gba.h: header file for gba.cpp
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
#ifndef __SLOT2_H__
#define __SLOT2_H__

enum cartTypeGBA {
	CART_GBA_NONE = 0,
	CART_GBA_GAME,
	CART_GBA_EMULATOR,
	CART_GBA_3IN1_256_V1,
	CART_GBA_3IN1_256_V2,
	CART_GBA_3IN1_512_V3,
	CART_GBA_FLASH
};

enum saveTypeGBA {
	SAVE_GBA_NONE = 0,
	SAVE_GBA_EEPROM_05 = 0x00010001, // 512 bytes
	SAVE_GBA_EEPROM_8 = 0x00010002, // 8k
	SAVE_GBA_SRAM_32 = 0x00020001, // 32k
	SAVE_GBA_FLASH_64 = 0x00040001, // 64k
	SAVE_GBA_FLASH_128 = 0x00040002 // 128k
};

struct dataSlot2 {
	cartTypeGBA type;
	saveTypeGBA save;
	uint32 ez_ID;
	union {
		char cid[5];
		uint32 iid;
	};
	char name[13];
};


cartTypeGBA GetSlot2Type(uint32 id);
saveTypeGBA GetSlot2SaveType(cartTypeGBA type);
void IdentifySlot2(dataSlot2 &data);

bool slot2Init();

// --------------------
bool gbaIsGame();
uint8 gbaGetSaveType();
uint32 gbaGetSaveSize(uint8 type = 255);


#endif // __SLOT2_H__