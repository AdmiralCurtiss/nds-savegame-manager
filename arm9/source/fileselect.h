/*
 * savegame_manager: a tool to backup and restore savegames from Nintendo
 *  DS cartridges. Nintendo DS and all derivative names are trademarks
 *  by Nintendo. EZFlash 3-in-1 is a trademark by EZFlash.
 *
 * fileselect.h: header file for fileselect.cpp
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

#ifndef _FILE_SELECT
#define _FILE_SELECT


#define CONFIRMDELETE 5
#define LOADBIN 4
#define MAXBROWSERCYCLES 4

#define MAX_CUSTOM_LIST 8192

#include <unistd.h>
//#endif
//#if defined(_WIN32)
//#include <windows.h>
//#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
//#if defined(__unix__)
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "ftplib.h"


void fileSelect(const char *startdir, char *out_dir, char *out_fname, netbuf *buf = 0,
  bool allow_cancel = false, bool allow_up = true);

bool fileExists(const char *fname);
uint32 fileSize(const char *fname);

#endif
