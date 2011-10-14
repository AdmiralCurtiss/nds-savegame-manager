/*
 * savegame_manager: a tool to backup and restore savegames from Nintendo
 *  DS cartridges. Nintendo DS and all derivative names are trademarks
 *  by Nintendo. EZFlash 3-in-1 is a trademark by EZFlash.
 *
 * strings.cpp: Localised message strings
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
  This file implements the functions and data structures required for localised messages.
 */


#include "strings.h"
#include "libini.h"
#include <nds.h>
#include "fileselect.h"
#include "globals.h"
#include "display.h"

// fallback string resources
#include "strings_fallback.inc"

using namespace std;

// a global string array
char **message_strings;

// ---------------------------------------------------
//  local functions
//

void AddString(uint32 id, ini_fd_t ini)
{
	// first, try to read string from ini file
	sprintf(txt, "%i", id);
	bool found = false;
	if (ini) {
		if (ini_locateKey(ini, txt) == 0) {
			if (ini_readString(ini, txt, 256) > 0) {
				found = true;
			}
		}
	}

	if (!found) {
		// if no ini file was found, use the fallback striung provided here
		message_strings[id] = const_cast<char*>(strings_fallback[id]);
	} else {
		// did load string, so prepare a buffer for it and move text
		message_strings[id] = new char[strlen(txt)+1];
		memcpy(message_strings[id], txt, strlen(txt)+1);
	}
}

// ---------------------------------------------------
//  global functions
//
bool stringsLoadFile(const char *fname)
{
#ifdef DEBUG
	iprintf("Trying to access strings file.\n");
#endif
	// load ini file...
	ini_fd_t ini = 0;
	if (fileExists(fname))
		ini = ini_open(fname, "r", "");
	
	// On the DSi/sudokuhax platform, "fileExists" always returns true, so we need to test if
	//  we actually got something. This is merged in the macro, but we keep some debug output.
#ifdef DEBUG
	if (!ini) {
		iprintf("Parsing %s failed!\n", fname);
	}
#endif

	message_strings = new char*[STR_LAST];
	if (ini)
		ini_locateHeading(ini, "");
	
#if 0
//#ifdef ENABLE_CUSTOM_FONT
	// EXPERIMENTAL: load and init a custom font
	if (ini_locateKey(ini, "font") == 0) {
		ini_readString(ini, txt, 256);
		if (strlen(txt)) {
			if (fileExists(txt)) {
				// load and init custon font; code borrowed from custom_font demo
				u32 fs = fileSize(txt);
				static u32 min_fs = 32*256;
				if (fs >= min_fs) {
					// TODO: maybe this should be a fixed size?
					void *font0 = malloc(fileSize(txt));
					FILE *file = fopen(txt, "rb");
					fread(font0, 1, fs, file);
					fclose(file);

					// borrowed from libnds source
					ConsoleFont font;
					font.gfx = (u16*)font0 + (fs - 16*256); // 16 bit-pointer, so only "16*"
					font.pal = 0; // single color!
					font.numColors =  0;
					font.numChars = 256; // this is fixed; it should be enough for everybody
					font.bpp = 4; // 4 bits per pixel
					font.asciiOffset = 0; // we cover 256 letters (well, we try)
					font.convertSingleColor = true; // for now, we only want one color!
				
					consoleSetFont(&upperScreen, &font);
					consoleSetFont(&lowerScreen, &font);
					free(font0);
				}
			}
		}
	}
#endif

	AddString(STR_EMPTY, ini);
	AddString(STR_STR, ini);
	//
	AddString(STR_MM_WIPE, ini);
	AddString(STR_TITLE_MSG, ini);
	AddString(STR_BOOT_NO_INI, ini);
	AddString(STR_BOOT_MODE_UNSUPPORTED, ini);
	AddString(STR_BOOT_DLDI_ERROR, ini);
	//
	AddString(STR_HW_SWAP_CARD, ini);
	AddString(STR_HW_CARD_UNREADABLE, ini);
	AddString(STR_HW_WRONG_GAME, ini);
	AddString(STR_HW_PLEASE_REBOOT, ini);
	//
	AddString(STR_HW_SELECT_FILE, ini);
	AddString(STR_HW_SELECT_FILE_OW, ini);
	AddString(STR_HW_SEEK_UNUSED_FNAME, ini);
	AddString(STR_ERR_NO_FNAME, ini);
	//
	AddString(STR_HW_FORMAT_GAME, ini);
	AddString(STR_HW_WRITE_GAME, ini);
	AddString(STR_HW_READ_GAME, ini);
	AddString(STR_HW_WRITE_FILE, ini);
	AddString(STR_HW_READ_FILE, ini);
	//
	AddString(STR_HW_3IN1_FORMAT_NOR, ini);
	AddString(STR_HW_3IN1_PREPARE_REBOOT, ini);
	AddString(STR_HW_3IN1_PLEASE_REBOOT, ini);
	AddString(STR_HW_3IN1_CLEAR_FLAG, ini);
	AddString(STR_HW_3IN1_DONE_DUMP, ini);
	AddString(STR_HW_3IN1_ERR_IDMODE, ini);
	AddString(STR_HW_3IN1_ERR_NOR, ini);
	//
	AddString(STR_HW_FTP_SEEK_AP, ini);
	AddString(STR_HW_FTP_ERR_AP, ini);
	AddString(STR_HW_FTP_SEEK_FTP, ini);
	AddString(STR_HW_FTP_ERR_FTP, ini);
	AddString(STR_HW_FTP_LOGIN, ini);
	AddString(STR_HW_FTP_ERR_LOGIN, ini);
	AddString(STR_HW_FTP_DIR, ini);
	AddString(STR_HW_FTP_SLOW, ini);
	AddString(STR_HW_FTP_READ_ONLY, ini);
	//
	AddString(STR_HW_WARN_DELETE, ini);
	AddString(STR_HW_DID_DELETE, ini);
	//
	AddString(STR_FS_READ, ini);
	AddString(STR_FS_WRITE, ini);
	//

	// delete temp file (which is a remnant of inilib)
	remove("/tmpfile");
	
	// Convert manual newline commands added as plaintext in the translation file.
	for (int i = 0; i < STR_LAST; i++) {
		char *ptr = message_strings[i];
		while ((ptr = strchr(ptr, '\\')) != NULL) {
			if (strlen(ptr+1)) {
				if (ptr[1] == 'n') {
					ptr[0] = '\n';
					memmove(ptr+1, ptr+2, strlen(ptr)-1);
				}
			}
		}
	}
	
	return true;
}

const char *stringsGetMessageString(int id)
{
	if (id > STR_LAST)
		return 0;
	else
		return message_strings[id];
}
