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
#include <sys/unistd.h>
#include <algorithm>
#include <nds/dma.h>


#include <dirent.h>

#include "gba.h"
#include "dsCard.h"

#include "display.h"

inline u32 min(u32 i, u32 j) { return (i < j) ? i : j;}
inline u32 max(u32 i, u32 j) { return (i > j) ? i : j;}


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
		if (*data == MAGIC_EEPR) {
			// 2 versions: 512 bytes / 8 kB
			return 1; // TODO: Try to figure out how to ID the 512 bytes version... hard way? write/restore!
		}
		if (*data == MAGIC_SRAM) {
			// *always* 32 kB
			return 3;
		}
		if (*data == MAGIC_FLAS) {
			// 64 kB oder 128 kB
			uint32 *data2 = data + 1;
			if (*data2 == MAGIC_H1M_)
				return 5;
			else
				return 4;
		}
	}
	
	return 0;
}

uint32 gbaGetSaveSizeLog2(uint8 type)
{
	if (type == 255)
		type = gbaGetSaveType();
	
	switch (type) {
		case 1:
			return 9;
		case 2:
			return 13;
		case 3:
			return 15;
		case 4:
			return 16;
		case 5:
			return 17;
		case 0:
		default:
			return 0;
	}
}

uint32 gbaGetSaveSize(uint8 type)
{
	return 1 << gbaGetSaveSizeLog2(type);
}

// local function
//#define E_DEBUG
void gbaEepromRead8Bytes(u8 *out, u32 addr, bool short_addr = false)
{
	// waitstates
	//*(volatile unsigned short *)0x04000204 = 0x4317; 
#ifdef E_DEBUG
	char txt[64];
#endif

	u16 buf[68];
	// "read" command
	buf[0] = 1;
	buf[1] = 1;
	// write address
	if (short_addr) {
		buf[2] = addr >> 5;
		buf[3] = addr >> 4;
		buf[4] = addr >> 3;
		buf[5] = addr >> 2;
		buf[6] = addr >> 1;
		buf[7] = addr;
		buf[8] = 0;
#if 1
		// does not work
		char txt[512];
		sprintf(txt, "EXMEMCNT = %x", REG_EXMEMCNT);
		sysSetBusOwners(true, true);
		displayMessage(txt);
		//u32 ime = enterCriticalSection();
		// EXMEMCNT: 0..1: long sram wait state (18/8) - set to "2", i.e. not the slowest
		// EXMEMCNT: 2..3: long ROM wait state (18/8)
		REG_EXMEMCNT |= 15;
		REG_EXMEMCNT &= ~(1 << 4);
		// EXMEMCNT: ~4: 6 waitstates on 2nd access (should be 18 as well, but that's no option!)
		dmaCopy((u16*)&buf[0], (u32*)0x09ffff00, 9<<1);
		while (*(u16*)0x040000dc != 0);
		dmaCopy((u32*)0x09ffff00, (u16*)&buf[0], 68<<1);
		while (*(u16*)0x040000dc != 0);
		//leaveCriticalSection(ime);
#endif
#if 0
		// TEST
		//u32 ime = enterCriticalSection();
		REG_EXMEMCNT |= 3;
		u32 ie = REG_IE;
		REG_IE = 1 << 11; // DMA3 only
		dmaCopy((u16*)&buf[0], (u32*)0x09ffff00, 9 << 1);
		REG_EXMEMCNT |= 3;
		dmaCopy((u32*)0x09ffff00, (u16*)&buf[0], 68 << 1);
		REG_IE = ie;
		//leaveCriticalSection(ime);
#endif
#if 0
#define EEPROM *(u16*)0x0dffff00
		for (int i = 0; i < 9; i++) {
			EEPROM = buf[i];
			swiDelay(10);
		}
		swiDelay(100);
		for (int i = 0; i < 68; i++) {
			buf[i] = EEPROM;
			swiDelay(10);
		}
#endif
	} else {
		buf[2] = addr >> 13;
		buf[3] = addr >> 12;
		buf[4] = addr >> 11;
		buf[5] = addr >> 10;
		buf[6] = addr >> 9;
		buf[7] = addr >> 8;
		buf[8] = addr >> 7;
		buf[9] = addr >> 6;
		buf[10] = addr >> 5;
		buf[11] = addr >> 4;
		buf[12] = addr >> 3;
		buf[13] = addr >> 2;
		buf[14] = addr >> 1;
		buf[15] = addr;
		buf[16] = 0;
		dmaCopy((u16*)&buf[0], (u32*)0x09000000, 17);
		dmaCopy((u32*)0x09000000, (u16*)&buf[0], 68);
	}
	
   // Extract data (there is only one *bit* per halfword!)
   u16 *in_pos = &buf[4]; 
   u8 *out_pos = out; 
   u8 out_byte;
   for(s8 byte = 7; byte >= 0; --byte ) 
   { 
      out_byte = 0; 
      for(s8 bit = 7; bit >= 0; --bit ) 
      { 
#ifdef E_DEBUG
/*
		sprintf(txt, "byte: %i, Bit: %i, Val: %x", byte, bit, *in_pos);
		displayPrintState(txt);
		*/
#endif
		  out_byte += ((*in_pos++)&1)<<bit;
      } 
      *out_pos++ = out_byte ; 
   } 
   displayPrintState("Exit!");
}

bool gbaReadSave(u8 *dst, u8 src, u32 len, u8 type)
{
	int nbanks = 2; // for type 4,5
	
	switch (type) {
	case 1: {
		// FIXME: this does not work yet...
		int start, end;
		start = src >> 3;
		end = (src + len - 1) >> 3;
		u8 *tmp = (u8*)malloc((end-start+1) << 3);
		u8 *ptr = tmp;
		displayPrintState("Reading from Eeprom...");
		for (int j = start; j <= end; j++, ptr+=8) {
			gbaEepromRead8Bytes(ptr, j, true);
		}
		memcpy(dst, tmp, len);
		free(tmp);
		displayPrintState("Done!");
		break;
		}
	case 2: {
		break;
		}
	case 3: {
		// SRAM: blind copy
		int start = 0x0a000000 + src;
		u8 *tmpsrc = (u8*)start;
		sysSetBusOwners(true, true);
		for (u32 i = 0; i < len; i++, tmpsrc++, dst++)
			*dst = *tmpsrc;
		break;
		}
	case 4:
		nbanks = 1;
	case 5:
		// FLASH - must be opend by register magic, then blind copy
		nbanks = 2;
		for (int j = 0; j < nbanks; j++) {
			// we need to wait a few cycles before the hardware reacts!
			*(u8*)0x0a005555 = 0xaa;
			swiDelay(10);
			*(u8*)0x0a002aaa = 0x55;
			swiDelay(10);
			*(u8*)0x0a005555 = 0xb0;
			swiDelay(10);
			*(u8*)0x0a000000 = (u8)j;
			swiDelay(10);
			u32 start, sublen;
			if (j == 0) {
				start = 0x0a000000 + src;
				sublen = (src < 0x10000) ? min(len, (1 << 16) - src) : 0;
			} else if (j == 1) {
				start = max(0x09ff0000 + src, 0x0a000000);
				sublen = (src + len < 0x10000) ? 0 : min(len, len - (0x10000 - src));
			}
			u8 *tmpsrc = (u8*)start;
			sysSetBusOwners(true, true);
			for (u32 i = 0; i < sublen; i++, tmpsrc++, dst++)
				*dst = *tmpsrc;
		}
		break;
	}
	return true;
}

bool gbaIsAtmel()
{
	*(u8*)0x0a005555 = 0xaa;
	swiDelay(10);
	*(u8*)0x0a002aaa = 0x55;
	swiDelay(10);
	*(u8*)0x0a005555 = 0x90; // ID mode
	swiDelay(10);
	//
	u8 dev = *(u8*)0x0a000001;
	u8 man = *(u8*)0x0a000000;
	//
	*(u8*)0x0a005555 = 0xaa;
	swiDelay(10);
	*(u8*)0x0a002aaa = 0x55;
	swiDelay(10);
	*(u8*)0x0a005555 = 0xf0; // leave ID mode
	swiDelay(10);
	//
	char txt[128];
	sprintf(txt, "Man: %x, Dev: %x", man, dev);
	displayPrintState(txt);
	if ((man == 0x3d) && (dev == 0x1f))
		return true;
	else
		return false;
}

bool gbaWriteSave(u8 *dst, u8 src, u32 len, u8 type)
{
	int nbanks = 2; // for type 4,5
	
	switch (type) {
	case 1:
	case 2:
		// TODO: how does eeprom work?
		break;
	case 3: {
		// SRAM: blind write
		u32 start = 0x0a000000 + src;
		u8 *tmpsrc = (u8*)start;
		sysSetBusOwners(true, true);
		for (u32 i = 0; i < len; i++, tmpsrc++, dst++)
			*tmpsrc = *dst;
			swiDelay(10); // mabe we don't need this, but better safe than sorry
		break;
		}
	case 4: {
		bool atmel = gbaIsAtmel();
		if (atmel) {
			// only 64k, no bank switching required
			u32 len7 = len >> 7;
			u8 *tmpsrc = (u8*)(0x0a000000+src);
			for (u32 j = 0; j < len7; j++) {
				u32 ime = enterCriticalSection();
				*(u8*)0x0a005555 = 0xaa;
				swiDelay(10);
				*(u8*)0x0a002aaa = 0x55;
				swiDelay(10);
				*(u8*)0x0a005555 = 0xa0;
				swiDelay(10);
				for (int i = 0; i < 128; i++) {
					*tmpsrc = *dst;
					swiDelay(10);
				}
				leaveCriticalSection(ime);
				while (*tmpsrc != *dst) {swiDelay(10);}
			}
			break;
		}
		nbanks = 1;
		}
	case 5:
		// FLASH - must be opend by register magic, erased and then rewritten
		// FIXME: currently, you can only write "all or nothing"
		nbanks = 2;
		for (int j = 0; j < nbanks; j++) {
			displayPrintState("Switching Bank.");
			*(u8*)0x0a005555 = 0xaa;
			swiDelay(10);
			*(u8*)0x0a002aaa = 0x55;
			swiDelay(10);
			*(u8*)0x0a005555 = 0xb0;
			swiDelay(10);
			*(u8*)0x0a000000 = (u8)j;
			swiDelay(10);
			//
			u32 start, sublen;
			if (j == 0) {
				start = 0x0a000000 + src;
				sublen = (src < 0x10000) ? min(len, (1 << 16) - src) : 0;
			} else if (j == 1) {
				start = max(0x09ff0000 + src, 0x0a000000);
				sublen = (src + len < 0x10000) ? 0 : min(len, len - (0x10000 - src));
			}
			u8 *tmpsrc = (u8*)start;
			sysSetBusOwners(true, true);
			for (u32 i = 0; i < sublen; i++, tmpsrc++, dst++) {
				// we need to wait a few cycles before the hardware reacts!
				*(u8*)0x0a005555 = 0xaa;
				swiDelay(10);
				*(u8*)0x0a002aaa = 0x55;
				swiDelay(10);
				*(u8*)0x0a005555 = 0xa0; // write byte command
				swiDelay(10);
				//
				*tmpsrc = *dst;
				swiDelay(10);
				//
				while (*tmpsrc != *dst) {swiDelay(10);}
			}
		}
		break;
	}
	return true;
}

bool gbaFormatSave(u8 type)
{
	switch (type) {
		case 1:
		case 2:
			// TODO: eeprom is not supported yet
			break;
		case 3:
			break;
		case 4:
		case 5:
			*(u8*)0x0a005555 = 0xaa;
			swiDelay(10);
			*(u8*)0x0a002aaa = 0x55;
			swiDelay(10);
			*(u8*)0x0a005555 = 0x80; // erase command
			swiDelay(10);
			*(u8*)0x0a005555 = 0xaa;
			swiDelay(10);
			*(u8*)0x0a002aaa = 0x55;
			swiDelay(10);
			*(u8*)0x0a005555 = 0x10; // erase entire chip
			swiDelay(10);
			while (*(u8*)0x0a000000 != 0xff)
				swiDelay(10);
			break;
	}
}
