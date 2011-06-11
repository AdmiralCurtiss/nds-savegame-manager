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

using namespace std;

// a global string array
char **message_strings;

#define ADD_STRING(id,text) sprintf(txt, "%i", id);\
	if (ini_locateKey(ini, txt) == 0) {\
		ini_readString(ini, txt, 256);\
	} else {\
		strcpy(txt,text);\
	}\
	message_strings[id] = new char[strlen(txt)+1];\
	memcpy(message_strings[id], txt, strlen(txt)+1);


bool stringsLoadFile(const char *fname)
{
	// load ini file...
	ini_fd_t ini = 0;
	if (fileExists(fname))
		ini = ini_open(fname, "r", "");

	message_strings = new char*[STR_LAST];
	ini_locateHeading(ini, "");

	ADD_STRING(STR_EMPTY, "");
	ADD_STRING(STR_STR, "%s");
	//
	ADD_STRING(STR_MM_WIPE,"\n    WIPES OUT ALL SAVE DATA\n         ON YOUR GAME !");
	ADD_STRING(STR_TITLE_MSG,"DS savegame manager\nVersion 0.3.0\nBy Pokedoc");
	ADD_STRING(STR_BOOT_NO_INI,"Unable to open ini file!\nPlease make sure that it is\n1. in this apps folder, or"
		  "\n2. in the root folder\nIf 1. does not work, use 2.");
	ADD_STRING(STR_BOOT_MODE_UNSUPPORTED,"This mode is DISABLED.\nPlease restart the system.");
	ADD_STRING(STR_BOOT_DLDI_ERROR,"DLDI initialisation error!");
	//
	ADD_STRING(STR_HW_SWAP_CARD,"Please take out Slot 1 flash card and insert a game.\nPress A when done.");
	ADD_STRING(STR_HW_CARD_UNREADABLE,"I can't read the game inserted in slot 1.\nPlease eject and retry.");
	ADD_STRING(STR_HW_WRONG_GAME,"This game has a save chip that is bigger than the save you are going to write. Please insert a different game.\nPress A when done.");
	ADD_STRING(STR_HW_PLEASE_REBOOT,"Done! Please power off\n(and restart if you want to do more).");
	//
	ADD_STRING(STR_HW_SELECT_FILE,"Please select a .sav file.");
	ADD_STRING(STR_HW_SELECT_FILE_OW,"Please select a file to overwrite, or press L+R in a folder to create a new file.");
	ADD_STRING(STR_HW_SEEK_UNUSED_FNAME,"Please wait... searching for an unused filename.\nTrying: %s");
	ADD_STRING(STR_ERR_NO_FNAME,"ERROR: Unable to get an unused nfilename! This means that you have more than 65536 saves!\n(wow!)");
	//
	ADD_STRING(STR_HW_FORMAT_GAME,"Preparing to write to your game.\nPlease wait...");
	ADD_STRING(STR_HW_WRITE_GAME,"Writing save to your game.\nPlease wait...");
	ADD_STRING(STR_HW_READ_GAME,"Reading save from your game.\nPlease wait...");
	ADD_STRING(STR_HW_WRITE_FILE, "Writing file:\n%s");
	ADD_STRING(STR_HW_READ_FILE, "Reading file:\n%s");
	//
	ADD_STRING(STR_HW_3IN1_FORMAT_NOR,"Preparing to write to the 3in1.\nPlease wait...");
	ADD_STRING(STR_HW_3IN1_PREPARE_REBOOT,"Preparing reboot...");
	ADD_STRING(STR_HW_3IN1_PLEASE_REBOOT,"Save has been written to the 3in1. Please power off and restart this tool to finish the dump.");
	ADD_STRING(STR_HW_3IN1_CLEAR_FLAG,"Preparing to dump your save...\nPlease wait...");
	ADD_STRING(STR_HW_3IN1_DONE_DUMP,"Done. Your game save has been dumped using your 3in1. Filename:\n%s\nPress (B) to continue.");
	ADD_STRING(STR_HW_3IN1_ERR_IDMODE,"ERROR!\nID mode still active!");
	ADD_STRING(STR_HW_3IN1_ERR_NOR,"ERROR!\nWriting to NOR failed!");
	//
	ADD_STRING(STR_HW_FTP_SEEK_AP,"Connecting to an access point...\nplease wait...");
	ADD_STRING(STR_HW_FTP_ERR_AP,"ERROR!\nCould not find a compatible Access Point. Please configure your WiFi Connection from a WiFi-enabled game!");
	ADD_STRING(STR_HW_FTP_SEEK_FTP,"Connecting to an FTP server...\nplease wait...");
	ADD_STRING(STR_HW_FTP_ERR_FTP,"ERROR!\nCould not find an FTP server. Please refer to the documentation.");
	ADD_STRING(STR_HW_FTP_LOGIN,"Logging in...");
	ADD_STRING(STR_HW_FTP_ERR_LOGIN,"ERROR!\nCould not log in to the FTP server. Please verify your username and password are correct in your ini file.");
	ADD_STRING(STR_HW_FTP_DIR,"Reading FTP directory...");
	ADD_STRING(STR_HW_FTP_SLOW,"FTP is slow, please wait...");
	ADD_STRING(STR_HW_FTP_READ_ONLY,"WARNING:\nCould not write to your FTP server! Maybe you forgot to enable write access?");
	//
	ADD_STRING(STR_HW_WARN_DELETE,"This will WIPE OUT your entire save! ARE YOU SURE?\n\nPress R+up+Y to confim!");
	ADD_STRING(STR_HW_DID_DELETE,"Done. Your game save has been PERMANENTLY deleted.\n\nPlease restart your DS.");
	//
	ADD_STRING(STR_FS_READ,"Please select a .sav file\n    (A) Select\n    (B) One directory up\n");
	ADD_STRING(STR_FS_WRITE,"Please select a .sav file\n    (A) Select\n    (B) One directory up\n    (L+R) cancel (new file)");
	//

	// delete temp file (which is a remnant of inilib)
	remove("/tmpfile");
	
	// Convert manual newline commands added as plaintext in the translation file.
	// FIXME: this does not seem to work reliably yet (or maybe it is the print function).
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
