/*
 * savegame_manager: a tool to backup and restore savegames from Nintendo
 *  DS cartridges. Nintendo DS and all derivative names are trademarks
 *  by Nintendo. EZFlash 3-in-1 is a trademark by EZFlash.
 *
 * auxspi.h: Header for auxspi.cpp
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
/*
  This is a thin reimplementation of the AUXSPI protocol at low levels.
  It is used to implement various experimental procedures to test accessing the HG/SS save chip. */

#ifndef SPI_BUS_H
#define SPI_BUS_H

#include <nds.h>


extern bool with_infrared;

uint8 auxspi_save_type(bool ir = false);
uint32 auxspi_save_size(bool ir = false);
uint8 auxspi_save_size_log_2(bool ir = false);
uint32 auxspi_save_jedec_id(bool ir = false);
uint8 auxspi_save_status_register(bool ir = false);
void auxspi_read_data(uint32 addr, uint8* buf, uint16 cnt, uint8 type = 0, bool ir = false);
void auxspi_write_data(uint32 addr, uint8 *buf, uint16 cnt, uint8 type = 0, bool ir = false);
void auxspi_disable_infrared();
void auxspi_erase(bool ir = false);
void auxspi_erase_sector(u32 sector, bool ir = false);

bool auxspi_has_infrared();

bool auxspi_is_unknown_type3();

#endif