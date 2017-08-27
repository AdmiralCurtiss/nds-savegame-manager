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

#include <cstdio>

#include "display.h"
#include "strings.h"
#include "globals.h"

#include "fileselect.h"


//extern u8 *data;
//extern u32 size_buf;

// ---------------------------------------------------------------------------------
void ftpGetFileList(const char *dir, netbuf *ctrl, int &num)
{
	char *buf = (char*)data;
	memset(buf, 0, size_buf);
	FtpDirBuf((char*)data, size_buf, "/", ctrl);
	//FtpDir("/dir.txt", "/", ctrl);
	char *ptr = buf; // scan pointer
	char *ptr2 = buf; // write pointer
	// count number of entries and condense them for further use
	char *begin = 0, *end;
	while ((end = strchr(ptr, '\n')) != NULL) {
		char c;
		char fname[512];
		if (!strlen(ptr))
			break;
		sscanf(ptr, "%c%*s %*s %*s %*s %*s %*s %*s %*s %[^\n]", &c, fname); // gets 'd' or 'f' - dir or file
		begin = strstr(ptr, fname);
		if (!begin)
			break;
		int flen = end - begin + 1;
		if (flen <= 1)
			break; // some FTP servers send an empty line at the end, so prevent an integer overflow
		if (c == 'd')
			*ptr2 = 'd';
		else
			*ptr2 = 'f';
		ptr2++;
		memcpy(ptr2, begin, flen);
		ptr = end + 1;
		ptr2 += flen;
		num++;
	}
}

void fileGetFileList(const char *dir, int &num)
{
	char *buf = (char*)data;
	int idx = 0;
	memset(buf, 0, size_buf);
	
	DIR *pdir;
	struct dirent *pent;
	num = 0;
	pdir=opendir(dir);
	if (pdir) {
		while ((pent=readdir(pdir)) !=NULL) {
			char fullname[256];
			sprintf(fullname, "%s/%s", dir, pent->d_name);
			struct stat statbuf;
			stat(fullname, &statbuf);
			
			// TODO: check for buffer overflow!
			if (idx + strlen(pent->d_name) < size_buf-2) {
				if (S_ISDIR(statbuf.st_mode)) {
					int len = sprintf(buf, "d%s\n", pent->d_name);
					buf += len;
					idx += len;
				} else {
					int len = sprintf(buf, "f%s\n", pent->d_name);
					buf += len;
					idx += len;
				}
				num++;
			} else {
				// buffer overflow imminent
				break;
			}
		}
	}
	if (pdir)
		closedir(pdir);
}

void filePrintFileList(const char *dir, int first, int select, int count, bool cancel = false)
{
	if (select < first)
		select = first;

	consoleSelect(&lowerScreen);
	consoleSetWindow(&lowerScreen, 0, 0, 32, 19);
	consoleClear();

	char *buf = (char*)data;
	for (int i = 0; i < first; i++) {
		buf = strchr(buf, '\n') + 1;
	}

	for (int i = first; (i < first + 18) && (i < count); i++) {
		char *newline = strchr(buf, '\n');
		int linelen = newline - buf;
		
		*newline = 0;
		if (*buf == 'd') {
			if (i != select)		
				iprintf("[%.29s]\n", buf+1);
			else
				iprintf("-->[%.26s]\n", buf+1);
		} else {
			if (i != select)
				iprintf("%.31s\n", buf+1);
			else
				iprintf("-->%.28s\n", buf+1);
		}
		*newline = '\n';
		buf += linelen+1;
	}
	
	consoleSelect(&lowerScreen);
	consoleSetWindow(&lowerScreen, 0, 18, 32, 6);
	consoleClear();
	iprintf("================================");
	if (cancel)
		iprintf(stringsGetMessageString(STR_FS_WRITE));
	else
		iprintf(stringsGetMessageString(STR_FS_READ));
}

// ========================***************************==============================
void fileSelect(const char *startdir, char *out_dir, char *out_fname, netbuf *buf,
  bool allow_cancel, bool allow_up)
{
	bool select = false;
	static int size_list = 18;
	
	int num_files = 0;
	int sel_file = 0;
	int first_file = 0;
	
	// get list of files in current dir
	if (buf)
		ftpGetFileList("/", buf, num_files);
	else
		fileGetFileList(startdir, num_files);
	filePrintFileList(startdir, first_file, sel_file, num_files, allow_cancel);
		
	while (!select) {
		// get_event
		uint32 keys = keysCurrent();
		
		// handle_event
		if (keys & KEY_DOWN) {
			if (sel_file < num_files-1) {
				sel_file++;
				if (sel_file >= first_file + size_list - 1)
					first_file = sel_file - (size_list - 1);
				filePrintFileList(startdir, first_file, sel_file, num_files, allow_cancel);
			} else {
				// wrap around to top of list
				sel_file = 0;
				first_file = 0;
				filePrintFileList(startdir, first_file, sel_file, num_files, allow_cancel);
			}
		} else if (keys & KEY_UP) {
			if (sel_file > 0) {
				sel_file--;
				if (sel_file < first_file)
					first_file = sel_file;
				filePrintFileList(startdir, first_file, sel_file, num_files, allow_cancel);
			} else {
				// wrap around to bottom of list
				sel_file = num_files - 1;
				first_file = sel_file - (size_list - 1);
				filePrintFileList(startdir, first_file, sel_file, num_files, allow_cancel);
			}
		} else if (keys & KEY_A) {
			// get selected file name
			char *buf2 = (char*)data;
			char fname[128];
			for (int i = 0; i < sel_file; i++) {
				buf2 = strchr(buf2, '\n') + 1;
			}
			char c;
			sscanf(buf2, "%c%[^\n]", &c, fname); // gets 'd' or 'f' - dir or file			
			// special cases
			if (strcasecmp(fname, ".") == 0)
				// don't do anything
				continue;
			if (strcasecmp(fname, "..") == 0) {
				// return to parent function (usually results in "back to higher level")
				while (keysCurrent() & (KEY_A | KEY_B));
				if (!allow_up)
					continue;
				if (buf)
					FtpChdir("..", buf);
				return;
			}
			
			// get selected file properties
			char fullname[512];
			sprintf(fullname, "%s%s", startdir, fname);
			struct stat statbuf;
			stat(fullname, &statbuf);
			if (c == 'd') {
				char fullpath[512];
				sprintf(fullpath, "%s%s/", startdir, fname);
				if (buf)
					FtpChdir(fname, buf);
				fileSelect(fullpath, out_dir, out_fname, buf, allow_cancel, true);
				if ((keysCurrent() & (KEY_L | KEY_R)) && allow_cancel)
					return;
				while (keysCurrent() & (KEY_A | KEY_B));
				
				// did we select a file? if so, keep returning to parent calling function
				if (strlen(out_fname))
					return;
				
				// we did not select anything, so regenerate the old list
				num_files = 0;
				if (buf)
					ftpGetFileList("/", buf, num_files);
				else
					fileGetFileList(startdir, num_files);
				filePrintFileList(startdir, first_file, sel_file, num_files, allow_cancel);
				continue;
			} else {
				sprintf(out_dir, "%s", startdir);
				sprintf(out_fname, "%s", fname);
				while (keysCurrent() & (KEY_A | KEY_B));
				return;
			}
		} else if (keys & KEY_B) {
			if (!allow_up)
				continue;
			if (buf)
				FtpChdir("..", buf);
			return;
		} else if (keys & (KEY_L | KEY_R)) {
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
// This is a standard textbook-style "fileExists" function.
bool fileExists(const char *fname)
{
	struct stat statbuf;
	memset(&statbuf, 0, sizeof(statbuf));
    if (stat(fname, &statbuf) != 0)
		return false;
	else {
		// on dsiwarehaxx, this additional check is required
		if (statbuf.st_size)
			return true;
		else
			return false;
	}
}

uint32 fileSize(const char *fname)
{
	struct stat statbuf;
    if (stat(fname, &statbuf) == 0)
		return statbuf.st_size;
	else
		return 0;
}
