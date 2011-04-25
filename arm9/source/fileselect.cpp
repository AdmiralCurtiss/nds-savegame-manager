/*
 * savegame_manager: a tool to backup and restore savegames from Nintendo
 *  DS cartridges. Nintendo DS and all derivative names are trademarks
 *  by Nintendo. EZFlash 3-in-1 is a trademark by EZFlash.
 *
 * fileselect.cpp: Functions for browsing files on the Flash Card.
 *    Also a happy collection of other file-related functions.
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
#include <fat.h>
#include <nds/arm9/console.h>
#include <sys/dir.h>
#include <sys/unistd.h>

#include <stdio.h>
#include <dirent.h>

#include "display.h"

char fnames[256][256];

// ---------------------------------------------------------------------------------
void fileGetFileList(const char *dir, uint32 &num)
{
	DIR *pdir;
	struct dirent *pent;
	num = 0;

	pdir=opendir(dir);

	if (pdir) {
		while ((pent=readdir(pdir)) !=NULL) {
			sprintf(&fnames[num][0], "%s", pent->d_name);
			num++;
			if (num == 256)
				break;
		}
	}
	if (pdir)
		closedir(pdir);
}

void filePrintFileList(const char *dir, uint32 first, uint32 select, uint32 count, bool cancel = false)
{
	if (select < first)
		select = first;

	consoleSelect(&lowerScreen);
	consoleSetWindow(&lowerScreen, 0, 0, 32, 19);
	consoleClear();

	for (uint32 i = first; (i < first + 18) && (i < count); i++) {
		struct stat statbuf;
		char fullpath[512];
		sprintf(fullpath, "%s/%s", dir, fnames[i]);
    	stat(fullpath, &statbuf);
		if (S_ISDIR(statbuf.st_mode)) {
			if (i != select)		
				iprintf("[%.29s]\n", fnames[i]);
			else
				iprintf("-->[%.26s]\n", fnames[i]);
		} else {
			if (i != select)
				iprintf("%.31s\n", fnames[i]);
			else
				iprintf("-->%.28s\n", fnames[i]);
		}
	}

	consoleSelect(&lowerScreen);
	consoleSetWindow(&lowerScreen, 0, 18, 32, 6);
	consoleClear();
	iprintf("================================");
	iprintf("Please select a .sav file\n");
	iprintf("    (A) Select\n");
	if (cancel)
		iprintf("    (B) cancel");
}

// ========================***************************==============================
void fileSelect(const char *startdir, char *out_dir, char *out_fname, bool allow_cancel)
{
	bool select = false;
	
	uint32 num_files = 0;
	uint32 sel_file = 0;
	uint32 first_file = 0;
	
	// get list of files in current dir
	fileGetFileList(startdir, num_files);
	filePrintFileList(startdir, first_file, sel_file, num_files, allow_cancel);
		
	while (!select) {
		// get_event
		uint32 keys = keysCurrent();
		
		// handle_event
		if (keys & KEY_DOWN) {
			if (sel_file < num_files-1) {
				sel_file++;
				filePrintFileList(startdir, first_file, sel_file, num_files, allow_cancel);
				if (sel_file > first_file + 18)
					first_file = sel_file - 18;
			}
		} else if (keys & KEY_UP) {
			if (sel_file > 0) {
				sel_file--;
				filePrintFileList(startdir, first_file, sel_file, num_files, allow_cancel);
				if (sel_file < first_file)
					first_file = sel_file;
			}
		} else if (keys & KEY_A) {
			// TODO: Okay, my first attempt at implementing a fuill file browser
			//  failed, so I am disabling changing directory for now. If anybody
			//  feels inclined to fix it, go ahead.
			if (stricmp(fnames[sel_file], ".") == 0)
				continue;
			if (stricmp(fnames[sel_file], "..") == 0)
				//return;
				continue;
			char fullname[512];
			sprintf(fullname, "%s/%s", startdir, fnames[sel_file]);
			struct stat statbuf;
			stat(fullname, &statbuf);
			if (S_ISDIR(statbuf.st_mode)) {
			/*
				char fullpath[512];
				sprintf(fullpath, "%s/%s", startdir, fnames[sel_file]);
				fileSelect(fullpath, out_dir, out_fname, allow_cancel);
				for (int i = 0; i < 5; i++)
					swiWaitForVBlank();
				if (strlen(out_fname))
					return;
					*/
				continue;
			} else {
				sprintf(out_dir, "%s", startdir);
				sprintf(out_fname, "%s", fnames[sel_file]);
				return;
			}
			select = true;
			
		} else if (keys & KEY_B) {
			if (allow_cancel) {
				sprintf(out_dir, "%s", startdir);
				out_fname[0] = 0;
				return;
			}
		}
		
		for (int i = 0; i < 5; i++)
			swiWaitForVBlank();
	}
}

// -------------------------------------------------------
bool fileExists(const char *fname)
{
	struct stat statbuf;
    if (stat(fname, &statbuf) != 0)
		return false;
	else
		return true;
}

uint32 fileSize(const char *fname)
{
	struct stat statbuf;
    if (stat(fname, &statbuf) == 0)
		return statbuf.st_size;
	else
		return 0;
}
