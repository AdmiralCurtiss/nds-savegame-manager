/*
 * savegame_manager: a tool to backup and restore savegames from Nintendo
 *  DS cartridges. Nintendo DS and all derivative names are trademarks
 *  by Nintendo. EZFlash 3-in-1 is a trademark by EZFlash.
 *
 * auxspi.cpp: A thin reimplementation of the AUXSPI protocol
 *   (high level functions)
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

#include "auxspi.h"
#include "auxspi_core.cpp"
#include "hardware.h"

#include <algorithm>

using std::max;

extern u8 data[0x8000];


// ========================================================
//  local functions
uint8 jedec_table(uint32 id)
{
	switch (id) {
	// 256 kB
	case 0x204012:
	case 0x621600:
		return 0x12;
	// 512 kB
	case 0x204013:
	case 0x621100:
		return 0x13;
	// 1 MB
	case 0x204014:
		return 0x14;
	// 8 MB (Band Brothers DX)
	case 0x204017:
		return 0x17;
	default:
		return 0;
	};
}

uint8 type2_size()
{
	static const uint32 offset0 = (8*1024-1);        //      8KB
	static const uint32 offset1 = (2*8*1024-1);      //      16KB
	u8 buf1;     //      +0k data        read -> write
	u8 buf2;     //      +8k data        read -> read
	u8 buf3;     //      +0k ~data          write
	u8 buf4;     //      +8k data new    comp buf2
	auxspi_read_data(offset0, &buf1, 1, 2);
	auxspi_read_data(offset1, &buf2, 1, 2);
	buf3=~buf1;
	auxspi_write_data(offset0, &buf3, 1, 2);
	auxspi_read_data (offset1, &buf4, 1, 2);
	auxspi_write_data(offset0, &buf1, 1, 2);
	if(buf4!=buf2)      //      +8k
		return 0x0d;  //       8KB(64kbit)
	else
		return 0x10; //      64KB(512kbit)
}

// ========================================================

uint8 auxspi_save_type()
{
	uint32 jedec = auxspi_save_jedec_id(); // 9f
	int8 sr = auxspi_save_status_register(); // 05
	
	if ((sr & 0xfd) == 0xF0 && (jedec == 0x00ffffff)) return 1;
	if ((sr & 0xfd) == 0x00 && (jedec == 0x00ffffff)) return 2;
	if ((sr & 0xfd) == 0x00 && (jedec != 0x00ffffff)) return 3; // should also cover Pokemon HG/SS
	
	return 0;
}

uint32 auxspi_save_size()
{
	return 1 << auxspi_save_size_log_2();
}

uint8 auxspi_save_size_log_2()
{
	uint8 type = auxspi_save_type();
	switch (type) {
	case 1:
		return 0x09; // 512 bytes
		break;
	case 2:
		return type2_size();
		break;
	case 3:
		return jedec_table(auxspi_save_jedec_id());
		break;
	default:
		return 0;
	}
}

uint32 auxspi_save_jedec_id()
{
	uint32 id = 0;
	auxspi_disable_infrared();
	auxspi_open(0);
	auxspi_write(0x9f);
	id |= auxspi_read() << 16;
	id |= auxspi_read() << 8;
	id |= auxspi_read();
	auxspi_close();
	return id;
}

uint8 auxspi_save_status_register()
{
	uint8 sr = 0;
	auxspi_disable_infrared();
	auxspi_open(0);
	auxspi_write(0x05);
	sr = auxspi_read();
	auxspi_close();
	return sr;
}

void auxspi_read_data(uint32 addr, uint8* buf, uint16 cnt, uint8 type)
{
	if (type == 0)
		type = auxspi_save_type();
	if (type == 0)
		return;

	auxspi_disable_infrared();
	auxspi_open(0);
	auxspi_write(0x03 | ((type == 1) ? addr>>8<<3 : 0));

    if (type == 3) {
		auxspi_write((addr >> 16) & 0xFF);
    } 
    
    if (type >= 2) {
		auxspi_write((addr >> 8) & 0xFF);
    }

	auxspi_write(addr & 0xFF);

    while (cnt > 0) {
        *buf++ = auxspi_read();
        cnt--;
    }
	
	auxspi_close();
}

void auxspi_write_data(uint32 addr, uint8 *buf, uint16 cnt, uint8 type)
{
/*
	if (type == 0)
		type = auxspi_save_type();
	if (type == 0)
		return;
		*/

	uint32 addr_end = addr + cnt;
	int i;
    int maxblocks = 32;
    if(type == 1) maxblocks = 16;
    if(type == 2) maxblocks = 32;
    if(type == 3) maxblocks = 256;

	// we can only write a finite amount of data at once, so we need a separate loop
	//  for multiple passes.
	while (addr < addr_end) {
		auxspi_disable_infrared();
		auxspi_open(0);
		// set WEL (Write Enable Latch)
		auxspi_write(0x06);
		auxspi_close_lite();
		
		auxspi_disable_infrared();
		auxspi_open(0);
		// send initial "write" command
        if(type == 1) {
			auxspi_write(0x02 | (addr & BIT(8)) >> (8-3));
			auxspi_write(addr & 0xFF);
        }
        else if(type == 2) {
			auxspi_write(0x02);
            auxspi_write((addr >> 8) & 0xff);
            auxspi_write(addr & 0xFF);
        }
        else if(type == 3) {
			auxspi_write(0x02);
            auxspi_write((addr >> 16) & 0xff);
            auxspi_write((addr >> 8) & 0xff);
            auxspi_write(addr & 0xFF);
        }

		for (i=0; addr < addr_end && i < maxblocks; i++, addr++) { 
			auxspi_write(*buf++);
        }
		auxspi_close_lite();

		// wait for programming to finish
		//auxspi_wait_wip();
		// wait programming to finish
		auxspi_disable_infrared();
		auxspi_open(0);
		auxspi_write(5);
		do { REG_AUXSPIDATA = 0; auxspi_wait_busy(); } while (REG_AUXSPIDATA & 0x01);	// WIP (Write In Progress) ?
        auxspi_wait_busy();
		auxspi_close();
	}
}

void auxspi_disable_infrared()
{
	if (with_infrared)
		auxspi_disable_infrared_core();
}

bool auxspi_has_infrared()
{
	return with_infrared;
}

void auxspi_erase()
{
	uint8 type = auxspi_save_type();
	if (type == 3) {
		uint8 size;
		size = 1 << (auxspi_save_size_log_2() - 16);
		for (int i = 0; i < size; i++) {
			auxspi_disable_infrared();
			auxspi_open(0);
			// set WEL (Write Enable Latch)
			auxspi_write(0x06);
			auxspi_close_lite();
			
			auxspi_disable_infrared();
			auxspi_open(0);
			auxspi_write(0xd8);
			auxspi_write(i);
			auxspi_write(0);
			auxspi_write(0);
			auxspi_close_lite();
			
			// wait for programming to finish
			//auxspi_wait_wip();
			auxspi_disable_infrared();
			auxspi_open(0);
			auxspi_write(5);
			do { REG_AUXSPIDATA = 0; auxspi_wait_busy(); } while (REG_AUXSPIDATA & 0x01);	// WIP (Write In Progress) ?
			auxspi_wait_busy();
			auxspi_close();
		}
	} else {
		int8 size = 1 << max(0, (auxspi_save_size_log_2() - 15));
		memset(data, 0, 0x8000);
		for (int i = 0; i < size; i++) {
			auxspi_write_data(i << 15, data, 0x8000, type);
		}
	}
}

void auxspi_wait_wip()
{
	auxspi_disable_infrared();
	auxspi_open(0);
	auxspi_write(0x05);
	uint8 sr;
	do {
		sr = auxspi_read();
	} while (sr & 0x01);
	auxspi_close();
}
