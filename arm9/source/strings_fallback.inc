/*
 * savegame_manager: a tool to backup and restore savegames from Nintendo
 *  DS cartridges. Nintendo DS and all derivative names are trademarks
 *  by Nintendo. EZFlash 3-in-1 is a trademark by EZFlash.
 *
 * strings_fallback.inc: Hard-coded strings to be used if no translation is provided
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
  This is a quick-and-dirty string resource file.
 */

const char *strings_fallback[STR_LAST] = {
	/* STR_EMPTY */ "",
	/* STR_STR */ "%s",
	//
	/* STR_MM_WIPE */ "\n    WIPES OUT ALL SAVE DATA\n         ON YOUR GAME !",
	/* STR_TITLE_MSG */ "DS savegame manager\nVersion %i.%i.%i%s\nBy Pokedoc",
	/* STR_BOOT_NO_INI */ "Unable to open ini file!\nPlease make sure that it is\n1. in this apps folder, or"
		  "\n2. in the root folder\nIf 1. does not work, use 2.",
	/* STR_BOOT_MODE_UNSUPPORTED */ "This mode is DISABLED.\nPlease restart the system.",
	/* STR_BOOT_DLDI_ERROR */ "DLDI initialisation error!",
	//
	/* STR_HW_SWAP_CARD */ "Please take out Slot 1 flash card and insert a game.\nPress A when done.",
	/* STR_HW_CARD_UNREADABLE */ "I can't read the game inserted in slot 1.\nPlease eject and retry.",
	/* STR_HW_WRONG_GAME */ "This game has a save chip that is bigger than the save you are going to write. Please insert a different game.\nPress A when done.",
	/* STR_HW_PLEASE_REBOOT */ "Done! Please power off\n(and restart if you want to do more).",
	//
	/* STR_HW_SELECT_FILE */ "Please select a .sav file.",
	/* STR_HW_SELECT_FILE_OW */ "Please select a file to overwrite, or press L+R in a folder to create a new file.",
	/* STR_HW_SEEK_UNUSED_FNAME */ "Please wait... searching for an unused filename.\nTrying: %s",
	/* STR_ERR_NO_FNAME */ "ERROR: Unable to get an unused nfilename! This means that you have more than 65536 saves!\n(wow!)",
	//
	/* STR_HW_FORMAT_GAME */ "Preparing to write to your game.\nPlease wait...",
	/* STR_HW_WRITE_GAME */ "Writing save to your game.\nPlease wait...",
	/* STR_HW_READ_GAME */ "Reading save from your game.\nPlease wait...",
	/* STR_HW_WRITE_FILE */ "Writing file:\n%s",
	/* STR_HW_READ_FILE */ "Reading file:\n%s",
	//
	/* STR_HW_3IN1_FORMAT_NOR */ "Preparing to write to the 3in1.\nPlease wait...",
	/* STR_HW_3IN1_PREPARE_REBOOT */ "Preparing reboot...",
	/* STR_HW_3IN1_PLEASE_REBOOT */ "Save has been written to the 3in1. Please power off and restart this tool to finish the dump.",
	/* STR_HW_3IN1_CLEAR_FLAG */ "Preparing to dump your save...\nPlease wait...",
	/* STR_HW_3IN1_DONE_DUMP */ "Done. Your game save has been dumped using your 3in1. Filename:\n%s\nPress (B) to continue.",
	/* STR_HW_3IN1_ERR_IDMODE */ "ERROR!\nID mode still active!",
	/* STR_HW_3IN1_ERR_NOR */ "ERROR!\nWriting to NOR failed!",
	//
	/* STR_HW_FTP_SEEK_AP */ "Connecting to an access point...\nplease wait...",
	/* STR_HW_FTP_ERR_AP */ "ERROR!\nCould not find a compatible Access Point. Please configure your WiFi Connection from a WiFi-enabled game!",
	/* STR_HW_FTP_SEEK_FTP */ "Connecting to an FTP server...\nplease wait...",
	/* STR_HW_FTP_ERR_FTP */ "ERROR!\nCould not find an FTP server. Please refer to the documentation.",
	/* STR_HW_FTP_LOGIN */ "Logging in...",
	/* STR_HW_FTP_ERR_LOGIN */ "ERROR!\nCould not log in to the FTP server. Please verify your username and password are correct in your ini file.",
	/* STR_HW_FTP_DIR */ "Reading FTP directory...",
	/* STR_HW_FTP_SLOW */ "FTP is slow, please wait...",
	/* STR_HW_FTP_READ_ONLY */ "WARNING:\nCould not write to your FTP server! Maybe you forgot to enable write access?",
	//
	/* STR_HW_WARN_DELETE */ "This will WIPE OUT your entire save! ARE YOU SURE?\n\nPress R+up+Y to confim!",
	/* STR_HW_DID_DELETE */ "Done. Your game save has been PERMANENTLY deleted.\n\nPlease restart your DS.",
	//
	/* STR_FS_READ */ "Please select a .sav file\n    (A) Select\n    (B) One directory up\n",
	/* STR_FS_WRITE */ "Please select a .sav file\n    (A) Select\n    (B) One directory up\n    (L+R) cancel (new file)"
};
