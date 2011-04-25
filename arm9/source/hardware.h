/*
 * savegame_manager: a tool to backup and restore savegames from Nintendo
 *  DS cartridges. Nintendo DS and all derivative names are trademarks
 *  by Nintendo. EZFlash 3-in-1 is a trademark by EZFlash.
 *
 * hardware.h: header file for header.cpp
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

#ifndef HARDWARE_H
#define HARDWARE_H

#include <nds.h>
#include <sys/unistd.h>

#define RS_BACKUP 0x4b434142

extern uint32 boot;

extern bool flash_card;

struct dsCardData
{
	uint32 data[4];
	char name[12];
};

void do_dump_nds_save_stage_1();
void do_dump_nds_save_stage_2(int size);
void do_restore_nds_save();

bool swap_cart();
bool is_flash_card();

void hwBackup3in1();
void hwDump3in1(uint32 size, const char *gamename);
void hwRestore3in1();
void hwRestore3in1_b(uint32 size);
void hwErase();

void hwBackupGBA();
void hwRestoreGBA();
void hwEraseGBA();

void hwFormatNor(uint32 page, uint32 count);

#endif
