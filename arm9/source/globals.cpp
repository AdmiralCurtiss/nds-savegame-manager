/*
 * savegame_manager: a tool to backup and restore savegames from Nintendo
 *  DS cartridges. Nintendo DS and all derivative names are trademarks
 *  by Nintendo. EZFlash 3-in-1 is a trademark by EZFlash.
 *
 * globals.cpp: global varibles, defines etc.
 *
 * Copyright (C) Pokedoc (2011)
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

#include "globals.h"

u8 *data;
u32 size_buf;

u32 slot_1_type = ~0;

char ftp_ip[16] = "ftp_ip";
char ftp_user[64] = "ftp_user";
char ftp_pass[64] = "ftp_pass";
int ftp_port = 0;

int ir_delay = 1200;

char device[16] = "/";

char txt[256] = "";

u32 mode = 0;
u32 ezflash = 0;

int slot2 = -1;

u32 extra_id[EXTRA_ARRAY_SIZE];
u8 extra_size[EXTRA_ARRAY_SIZE];
