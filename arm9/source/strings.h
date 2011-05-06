/*
 * savegame_manager: a tool to backup and restore savegames from Nintendo
 *  DS cartridges. Nintendo DS and all derivative names are trademarks
 *  by Nintendo. EZFlash 3-in-1 is a trademark by EZFlash.
 *
 * strings.h: Header for strings.cpp
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
  This files is used to allow translations of the program. It defines many
  string IDs which may be used to overwrite default messages with an INI file.
 */

#ifndef STRINGS_H
#define STRINGS_H

enum {
  STR_TITLE_MSG,
  STR_BOOT_NO_INI,
  STR_BOOT_MODE_UNSUPPORTED,
  STR_BOOT_DLDI_ERROR,
  //
  STR_HW_SWAP_CARD,
  STR_HW_WRONG_GAME,
  STR_HW_PLEASE_REBOOT,
  //
  STR_HW_SELECT_FILE,
  STR_HW_SELECT_FILE_OW,
  STR_HW_SEEK_UNUSED_FNAME,
  STR_ERR_NO_FNAME,
  //
  STR_HW_FORMAT_GAME,
  STR_HW_WRITE_GAME,
  //
  STR_HW_3IN1_FORMAT_NOR,
  STR_HW_3IN1_WRITE_NOR,
  STR_HW_3IN1_PREPARE_REBOOT,
  STR_HW_3IN1_PLEASE_REBOOT,
  STR_HW_3IN1_CLEAR_FLAG,
  STR_HW_3IN1_DUMP,
  STR_HW_3IN1_DONE_DUMP,
  STR_HW_3IN1_RESTORE,
  STR_HW_3IN1_ERR_IDMODE,
  STR_HW_3IN1_ERR_NOR,
  //
  STR_HW_FTP_SEEK_AP,
  STR_HW_FTP_ERR_AP,
  STR_HW_FTP_SEEK_FTP,
  STR_HW_FTP_ERR_FTP,
  STR_HW_FTP_LOGIN,
  STR_HW_FTP_ERR_LOGIN,
  STR_HW_FTP_DIR,
  STR_HW_FTP_SLOW,
  STR_HW_FTP_READ_ONLY,
  //
  STR_HW_GBA_READ,
  STR_HW_GBA_DUMP,
  STR_HW_GBA_LOAD,
  STR_HW_GBA_RESTORE,
  //
  STR_HW_WARN_DELETE,
  STR_HW_DID_DELETE,
  //
  STR_LAST
};

bool stringsLoadFile(const char *fname);
const char *stringsGetMessageString(int id = 0);

#endif // STRINGS_H
