/*
 * savegame_manager: a tool to backup and restore savegames from Nintendo
 *  DS cartridges. Nintendo DS and all derivative names are trademarks
 *  by Nintendo. EZFlash 3-in-1 is a trademark by EZFlash.
 *
 * dsi.cpp: A happy collection of functions dealing with DSi-specific things
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

#include "dsi.h"

bool isDsi()
{
	// This code attempts to identify an application running in DSi mode. We
	//  make use of the fact that the DSi has 16 MB of memory, while the DS
	//  only has 4 MB. Since the Chinese iQue DS has 8 MB, we will read above
	//  this boundary.
	u32 test = *(u32*)0x02800000; // this is 8 MB into the main memory
	u32 test2 = *(u32*)0x02000000; // this is 8 MB into the main memory
	
	if (test == 0x00000000)
		return false;
	if (test == test2)
		return false;
	
	// Try writing to this address. If the value is accepted, we are on a DSi in DSi mode.
	*(u32*)0x02800000 = 0x12345678;
	if (*(u32*)0x02800000 == 0x12345678)
		return true;
	
	// The memory address does not exist, so we are on a DS (or a DSi in DS mode)
	return false;
}
