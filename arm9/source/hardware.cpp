/*
 * savegame_manager: a tool to backup and restore savegames from Nintendo
 *  DS cartridges. Nintendo DS and all derivative names are trademarks
 *  by Nintendo. EZFlash 3-in-1 is a trademark by EZFlash.
 *
 * hardware.cpp: Functions used to communicate with slot-1 devices, including
 *    hotswap and identification of cartridges.
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
#include <nds/interrupts.h>
#include <nds/arm9/console.h>
#include <sys/dir.h>
#include <sys/unistd.h>
#include <dswifi9.h>
#include <nds/card.h>

#include <stdio.h>
#include <algorithm>

#include "display.h"
#include "dsCard.h"
#include "ftplib.h"
#include "gba.h"
#include "globals.h"

#include "hardware.h"

#include "fileselect.h"

#include "auxspi.h"
#include "strings.h"

using namespace std;

static u32 pitch = 0x40000;


// ---------------------------------------------------------------------
bool swap_cart()
{
	sNDSHeader nds;
	nds.gameTitle[0] = 0;
	
	while (!nds.gameTitle[0]) {
		displayMessage2A(STR_HW_SWAP_CARD, false);

		bool swap = false;
		while (!swap) {
			if (keysCurrent() & KEY_A) {
				// identify hardware
				slot_1_type = get_slot1_type();
				sysSetBusOwners(true, true);
				cardReadHeader((u8*)&nds);
				//flash_card = false;
				displayPrintUpper();
				if (!nds.gameTitle[0])
					continue;
				
				return true;
			}
		}
	}
	
	return true;
}

u32 get_slot1_type()
{
	u8 size1 = auxspi_save_size_log_2(false);
	u8 size2 = auxspi_save_size_log_2(true);
	
	if (size1 == size2)
		return 2; // flash card
	
	if ((size1 == 0) && (size2 != 0))
		return 1; // ir device
	
	return 0; // regular game
}

bool swap_card_game(uint32 size)
{
	return false;
}

// --------------------------------------------------------
void hwFormatNor(uint32 page, uint32 count)
{
	uint32 ime = hwGrab3in1();
	SetSerialMode();
	//displayPrintState("Formating NOR memory");
	displayProgressBar(0, count);
	for (uint32 i = page; i < page+count; i++) {
		Block_Erase(i << 18);
		displayProgressBar(i+1-page, count);
	}
	hwRelease3in1(ime);
}

// --------------------------------------------------------
void hwBackupDSi()
{
	hwBackupFTP();
}

void hwRestoreDSi()
{
	hwRestoreFTP();

/*
	// only works from NOR!
	char path[256];
	char fname[256] = "";
	fileSelect("sd:/", path, fname, 0, true, false);
	*/
}

// --------------------------------------------------------
void hwBackup3in1()
{
	swap_cart();
	displayPrintUpper();

	uint8 size = auxspi_save_size_log_2();
	int size_blocks = 1 << max(0, (int8(size) - 18)); // ... in units of 0x40000 bytes - that's 256 kB
	uint8 type = auxspi_save_type();

	displayMessage2A(STR_HW_3IN1_FORMAT_NOR, false);
	hwFormatNor(0, size_blocks+1);

	displayMessage2A(STR_HW_3IN1_WRITE_NOR, false);
	if (size < 15)
		size_blocks = 1;
	else
		size_blocks = 1 << (uint8(size) - 15);
	u8 *test = (u8*)malloc(0x8000);
	u32 LEN = min(1 << size, 0x8000);
	
	for (int i = 0; i < size_blocks; i++) {
		displayProgressBar(i+1, size_blocks);
		auxspi_read_data(i << 15, data, LEN, type);
		uint32 ime = hwGrab3in1();
		SetSerialMode();
		WriteNorFlash((i << 15) + pitch, data, LEN);
		hwRelease3in1(ime);
		for (int j = 0; j < 4; j++) {
			uint32 ime = hwGrab3in1();
			ReadNorFlash(test, (i << 15) + pitch, LEN);
			hwRelease3in1(ime);
		}
		if (*((vuint16 *)(FlashBase+0x2002)) == 0x227E) {
			displayMessage("ERROR: ID mode active!");
			while(1);
		}
		if (memcmp(test, data, LEN)) {
			displayMessage("ERROR: verifying NOR failed");
			while(1);
		}
	}
	free(test);

	// Write a flag to tell the app what happens on restart.
	// This is necessary, since some DLDI drivers cease working after swapping a card.
	// Fix for dead batteries: don't write "reboot" values to SRAM, but to NOR.
	displayMessage2A(STR_HW_3IN1_PREPARE_REBOOT, false);
	//
	sNDSHeader nds;
	cardReadHeader((u8*)&nds); // on a Cyclops Evolution, this call *will* mess up your DLDI driver!
	dsCardData data2;
	memset(&data2, 0, sizeof(data2));
	data2.data[0] = RS_BACKUP;
	data2.data[1] = 0;
	data2.data[2] = size;
	data2.data[3] = 0xffff00ff;
	// We need to write the reboot flags to NOR, and we can only write in 32kB-blocks, at aligned offsets.
	// Therefore we prepare a bigger block and copy lots of data now.
	memcpy(&data2.name[0], &nds.gameTitle[0], 12);
	memset(data, 0, 0x8000);
	memcpy(&data[0x1000], (u8*)&data2, sizeof(data2));
	uint32 ime = hwGrab3in1();
	WriteNorFlash(0, data, 0x8000); // this should work - but it does not!
	hwRelease3in1(ime);
	
	displayMessage2A(STR_HW_3IN1_PLEASE_REBOOT, false);
	
	ime = hwGrab3in1();
	ReadNorFlash((u8*)&data2, 0x1000, sizeof(data2)); // this should work - but it does not!
	hwRelease3in1(ime);
	char txt[128];
	sprintf(txt, "%.12s", &data2.name[0]);
	displayMessage(txt);

	while(1) {};
}

void hwDump3in1(uint32 size, const char *gamename)
{
	u32 size_blocks = 1 << max(0, (uint8(size) - 18));

	displayMessageA(STR_HW_SELECT_FILE_OW);
	
	char path[256];
	char fname[256] = "";
	fileSelect("/", path, fname, 0, true, false);
	if (!fname[0]) {
		uint32 cnt = 0;
		sprintf(fname, "/%s.%i.sav", gamename, cnt);
		char txt[256];
		sprintf(txt, stringsGetMessageString(STR_HW_SEEK_UNUSED_FNAME), fname);
		displayMessage2(txt, false);
		while (fileExists(fname)) {
			if (cnt < 65536)
				cnt++;
			else {
				displayMessage2A(STR_ERR_NO_FNAME, true);
				while(1);
			}
			sprintf(fname, "%s.%i.sav", gamename, cnt);
		}
	}
	char fullpath[256];
	sprintf(fullpath, "%s/%s", path, fname);
	char txt[512];
	sprintf(txt, stringsGetMessageString(STR_HW_3IN1_DUMP), fullpath);
	displayMessage2(txt, false);

	FILE *file = fopen(fullpath, "wb");
	if (size < 15)
		size_blocks = 1;
	else
		size_blocks = 1 << (uint8(size) - 15);
	for (u32 i = 0; i < size_blocks; i++) {
		displayProgressBar(i+1, size_blocks);
		u32 LEN = min((u32)0x8000, (u32)1 << size);
		uint32 ime = hwGrab3in1();
		ReadNorFlash(data, (i << 15) + pitch, LEN);
		hwRelease3in1(ime);
		fwrite(data, 1, LEN, file);
	}
	fclose(file);

	sprintf(txt, stringsGetMessageString(STR_HW_3IN1_DONE_DUMP), fullpath);
	displayMessage2(txt, false);
	while (1);
}

void hwRestore3in1()
{
	char path[256];
	char fname[256];
	memset(fname, 0, 256);
	displayMessageA(STR_HW_SELECT_FILE); // lower screen is used for file browser
	fileSelect("/", path, fname, 0, false, false);
	char msg[256];
	sprintf(msg, "%s/%s", path, fname);
	displayMessage(msg);
	
	FILE *file = fopen(msg, "rb");
	uint32 size0 = fileSize(msg);
	
	uint8 size = 1;
	// crude log2 function
	while (size0 > (1 << size)) {
		size++;
	}
	int size_blocks = 1 << max(0, int8(size) - 18); // ... in units of 0x40000 bytes - that's 256 kB
	
	displayMessage2A(STR_HW_3IN1_FORMAT_NOR, false);
	hwFormatNor(1, size_blocks);

	displayMessage2A(STR_HW_3IN1_WRITE_NOR, false);
	if (size < 15)
		size_blocks = 1;
	else
		size_blocks = 1 << (uint8(size) - 15);
	u8 *test = (u8*)malloc(0x8000);
	uint32 LEN = min(1 << size, 0x8000);

	for (int i = 0; i < size_blocks; i++) {
		displayProgressBar(i, size_blocks);
		fread(data, 1, LEN, file);
		uint32 ime = hwGrab3in1();
		SetSerialMode();
		WriteNorFlash((i << 15) + pitch, data, LEN);
		ReadNorFlash(test, (i << 15) + pitch, LEN);
		hwRelease3in1(ime);
		if (*((vuint16 *)(FlashBase+0x2002)) == 0x227E) {
			displayMessage("ERROR: ID mode active!");
			while(1);
		}
		if (int err = memcmp(test, data, LEN)) {
			char txt[128];
			sprintf(txt, "ERROR: NOR %x/%x: %x -> %x", abs(err), LEN, data[err], test[err]);
			displayMessage(txt);
			while(1);
		}
	}
	displayProgressBar(1, 1);
	fclose(file);
	free(test);
	
	hwRestore3in1_b(size);
}

void hwRestore3in1_b(uint32 size_file)
{
	// Third, swap in a new game
	uint32 size = auxspi_save_size_log_2();
	while ((size_file < size) || (slot_1_type == 2)) {
		// TODO: make THIS error message more meaningful!
		displayPrintState("File too small or no save chip!");
		swap_cart();
		size = auxspi_save_size_log_2();
	}
	displayPrintUpper();
	
	uint8 type = auxspi_save_type();
	if (type == 3) {
		displayMessage2A(STR_HW_FORMAT_GAME, false);
		auxspi_erase();
	}

	// And finally, write the save!
	displayMessage2A(STR_HW_WRITE_GAME, false);
	u32 LEN = 0, num_blocks = 0, shift = 0;
	switch (type) {
	case 1:
		shift = 4; // 16 bytes
		break;
	case 2:
		shift = 5; // 32 bytes
		break;
	case 3:
		shift = 8; // 256 bytes
		break;
	default:
		return;
	}
	LEN = 1 << shift;
	num_blocks = 1 << (size - shift);

	for (unsigned int i = 0; i < num_blocks; i++) {
		if (i % (num_blocks >> 6) == 0)
			displayProgressBar(i+1, num_blocks);
		uint32 ime = hwGrab3in1();
		ReadNorFlash(data, (i << shift) + pitch, LEN);
		hwRelease3in1(ime);
		auxspi_write_data(i << shift, data, LEN, type);
	}
	displayProgressBar(1, 1);

	displayMessage2A(STR_HW_3IN1_RESTORE, false);
	
	while (1) {};
}

// ------------------------------------------------------------
void hwErase()
{
	displayMessage2A(STR_HW_WARN_DELETE, true);
	while (!(keysCurrent() & (KEY_UP | KEY_R | KEY_Y))) {};
	auxspi_erase();
	displayMessage2A(STR_HW_DID_DELETE, true);
	while(1);
}

// ------------------------------------------------------------
void hwBackupFTP(bool dlp)
{
	netbuf *buf, *ndata;
	int j;
	static int jmax = 10;

	// Dump save and write it to FTP server
	// First: swap card
	if (!dlp)
		swap_cart();
	displayPrintUpper();
	uint8 size = auxspi_save_size_log_2();
	int size_blocks = 1 << max(0, (int8(size) - 18)); // ... in units of 0x40000 bytes - that's 256 kB
	uint8 type = auxspi_save_type();
	if (size < 15)
		size_blocks = 1;
	else
		size_blocks = 1 << (uint8(size) - 15);

	// Second: connect to FTP server
	displayPrintState("FTP: connecting to AP");
	if (!Wifi_InitDefault(true)) {
		displayPrintState("FTP: ERROR: AP not found");
		while(1);
	}
	displayPrintState("FTP: connecting to FTP server");
	char fullname[512];
	sprintf(fullname, "%s:%i", ftp_ip, ftp_port);
	j = 0;
	while (!FtpConnect(fullname, &buf)) {
		j++;
		if (j >= jmax) {
			displayPrintState("FTP: ERROR: FTP server missing");
			while(1);
		}
		swiDelay(10000);
	}
	displayPrintState("FTP: login");
	j = 0;
	while (!FtpLogin(ftp_user, ftp_pass, buf)) {
		j++;
		if (j >= jmax) {
			displayPrintState("FTP: ERROR: login failed");
			while(1);
		}
		swiDelay(10000);
	}
	
	char fdir[256] = "";
	char fname[256] ="";
	memset(fname, 0, 256);
	displayMessageA(STR_HW_SELECT_FILE_OW);
	displayPrintState("FTP: dir");
	fileSelect("/", fdir, fname, buf, true, false);
	bool newfile;
	if (!fname[0])
		newfile = true;
	else
		newfile = false;
	
	// Third: get a new target filename
	FtpChdir(fdir, buf);
	if (!fname[0]) {
		sNDSHeader nds;
		cardReadHeader((u8*)&nds);
		uint32 cnt = 0;
		int tsize = 0;
		sprintf(fname, "%.12s.%i.sav", nds.gameTitle, cnt);
		while (FtpSize(fname, &tsize, FTPLIB_IMAGE, buf) != 0) {
		char txt[256];
			sprintf(txt, stringsGetMessageString(STR_HW_SEEK_UNUSED_FNAME), fname);
			displayMessage2(txt, false);
			if (cnt < 65536)
				cnt++;
			else {
				displayMessage2A(STR_ERR_NO_FNAME, true);
				while(1);
			}
			sprintf(fname, "%.12s.%i.sav", nds.gameTitle, cnt);
			displayPrintState(fname);
		}
	}
	
	// Fourth: dump save
	displayPrintState("");
	sprintf(fullname, "Writing file:\n%s%s", fdir, fname);
	displayMessage(fullname);
	FtpAccess(fname, FTPLIB_FILE_WRITE, FTPLIB_IMAGE, buf, &ndata);
	u32 length = 0x200;
	if (size < 9)
		length = 1 << size;
	for (int i = 0; i < (1 << (size - 9)); i++) {
		displayProgressBar(i+1, (size_blocks << 6));
		auxspi_read_data(i << 9, (u8*)&data[0], length, type);
		int out = 0;
		while (out < 512) {
			out += FtpWrite((u8*)&data[out], 512-out, ndata);
			if (out < 512) {
				displayPrintState(stringsGetMessageString(STR_HW_FTP_SLOW));
				/*
				debug++;
				if (debug >= 2048) {
					debug = 0;
					displayMessage2("Unable to reach FTP server. Make sure that you have WRITE ACCESS!"
				}
				*/
			} else {
				displayPrintState("");
			}
		}
	}
	FtpClose(ndata);
	FtpQuit(ndata);
	
	Wifi_DisconnectAP();

	if (dlp) {
		displayMessage("Done! Please turn off your DS.");
		while(1);
	}
}

void hwRestoreFTP(bool dlp)
{
	netbuf *buf, *ndata;
	int j;
	static int jmax = 10;

	// Dump save and write it to FTP server
	// First: connect to FTP server
	displayPrintState("FTP: connecting to AP");
	if (!Wifi_InitDefault(true)) {
		displayPrintState("FTP: ERROR: AP not found");
		while(1);
	}
	displayPrintState("FTP: connecting to FTP server");
	char fullname[512];
	sprintf(fullname, "%s:%i", ftp_ip, ftp_port);
	j = 0;
	while (!FtpConnect(fullname, &buf)) {
		j++;
		if (j >= jmax) {
			displayPrintState("FTP: ERROR: FTP server missing");
			while(1);
		}
		swiDelay(10000);
	}
	displayPrintState("FTP: login");
	j = 0;
	while (!FtpLogin(ftp_user, ftp_pass, buf)) {
		j++;
		if (j >= jmax) {
			displayPrintState("FTP: ERROR: login failed");
			while(1);
		}
		swiDelay(10000);
	}

	// Second: select a filename
	char fdir[256] = "";
	char fname[256] ="";
	memset(fname, 0, 256);
	displayMessage("Please select a file name to\nrestore.");
	displayPrintState("FTP: dir");
	fileSelect("/", fdir, fname, buf, true, false);
	
	// Third: swap card
	if (!dlp)
		swap_cart();
	displayPrintUpper();
	uint8 size = auxspi_save_size_log_2();
	uint8 type = auxspi_save_type();

	// Fourth: read file
	displayPrintState("");
	FtpChdir(fdir, buf);
	sprintf(fullname, "Reading file:\n%s%s", fdir, fname);
	displayMessage(fullname);
	u32 LEN = 0, num_blocks = 0, shift = 0;
	switch (type) {
	case 1:
		shift = 4; // 16 bytes
		break;
	case 2:
		shift = 5; // 32 bytes
		break;
	case 3:
		shift = 8; // 256 bytes
		break;
	default:
		return;
	}
	LEN = 1 << shift;
	int num_blocks_ftp = 1 << (size - 9);
	num_blocks = 1 << (size - shift);
	
	// if our save is small enough to fit in memory, use secure mode (i.e. read full file before erasing anything
	bool insecure = ((1 << size) > size_buf);
	if (insecure) {
#if 0
		displayMessage("Save larger than available RAM.\nRestoring will be dangerous!\n\nPress Start + Select to proceed");
		while (!(keysCurrent() & (KEY_START | KEY_SELECT)));
#else
		displayMessage("Save larger than available RAM.\nRestoring is buggy, so\nI will stop here.\n(Try Rudolphs tools.)");
		while (1);
#endif
	}
	if ((type == 3) && (insecure)) {
		displayPrintState("Formating Flash chip");
		auxspi_erase();
	}
	FtpAccess(fname, FTPLIB_FILE_READ, FTPLIB_IMAGE, buf, &ndata);
	u8 *pdata = data;
	for (int i = 0; i < num_blocks_ftp; i++) {
		displayProgressBar(i+1, num_blocks_ftp);
		int in = 0;
		while (in < 512) {
			in += FtpRead((u8*)&pdata[in], 512-in, ndata);
			if (in < 512)
				displayPrintState(stringsGetMessageString(STR_HW_FTP_SLOW));
			else
				displayPrintState("");
		}
		/*
		int out;
	    if ((out = FtpRead((u8*)pdata, 512, ndata)) < 512) {
			char dings[512];
			sprintf(dings, "Error: requested 512, got %i", out);
			displayPrintState(dings);
			while(1);
		}
		*/
		// does not fit into memory: insecure mode
		if (insecure) {
			//char bums[512];
			//sprintf(bums, "addr = %x", (i << 9)+(j << shift));
			displayPrintState("Writing save (insecure!)");
			for (int j = 0; j < 1 << (9-shift); j++) {
				auxspi_write_data((i << 9)+(j << shift), ((u8*)pdata)+(j<<shift), LEN, type);
			}
		}
		pdata += 512;
	}
	if (!insecure) {
		displayMessage("Got file, now writing to game");
		if (type == 3) {
			displayPrintState("Formating Flash chip");
			auxspi_erase();
		}
		for (int i = 0; i < (1 << (size - shift)); i++) {
			displayPrintState("Writing save");
			displayProgressBar(i+1, 1 << (size - shift));
			auxspi_write_data(i << shift, ((u8*)data)+(i<<shift), LEN, type);
		}
	}
	FtpClose(ndata);
	FtpQuit(ndata);

	Wifi_DisconnectAP();

	if (dlp) {
		displayMessage("Done! Please turn off your DS.");
		while(1);
	}
}

// ------------------------------------------------------------
void hwBackupGBA(u8 type)
{
	if ((type == 0) || (type > 5))
		return;
	
	if ((type == 1) || (type == 2)) {
		displayMessage("I can't read this save type\nyet. Please use Rudolphs tool\ninstead.");
		return;
	}

	char path[256];
	char fname[256] = "";
	char *gamename = (char*)0x080000a0;
	fileSelect("/", path, fname, 0, true, false);
	if (!fname[0]) {
		uint32 cnt = 0;
		sprintf(fname, "/%.12s.%i.sav", gamename, cnt);
		while (fileExists(fname)) {
			if (cnt < 65536)
				cnt++;
			else {
				displayMessage("Unable to get a filename!\nThis means that you have more\nthan 65536 saves! (wow!)\nOops!");
				while(1);
			}
			sprintf(fname, "/%.12s.%i.sav", gamename, cnt);
		}
	}
	char fullpath[512];
	sprintf(fullpath, "%s/%s", path, fname);
	displayMessage(fname);
	
	displayPrintState("Reading save from game");
	uint32 size = gbaGetSaveSize(type);
	gbaReadSave(data, 0, size, type);
	
	displayPrintState("Writing save to flash card");
	FILE *file = fopen(fullpath, "wb");
	fwrite(data, 1, size, file);
	fclose(file);

	displayPrintState("Done!");
	while(1);
}

void hwRestoreGBA()
{
	u8 type = gbaGetSaveType();
	if ((type == 0) || (type > 5))
		return;
	
	if ((type == 1) || (type == 2)) {
		displayMessage("I can't write this save type\nyet. Please use Rudolphs tool\ninstead.");
		return;
	}
	
	uint32 size = gbaGetSaveSize(type);
	
	char path[256];
	char fname[256] = "";
	fileSelect("/", path, fname, 0);
	char fullpath[512];
	sprintf(fullpath, "%s/%s", path, fname);
	displayMessage(fname);

	displayPrintState("Reading save from flash card");
	FILE *file = fopen(fullpath, "rb");
	fread(data, 1, size, file);
	fclose(file);
	
	if ((type == 4) || (type == 5)) {
		displayPrintState("Deleting old save.");
		gbaFormatSave(type);
	}
	
	displayPrintState("Writing save to game");
	gbaWriteSave(data, 0, size, type);

	displayPrintState("Done!");
	while(1);
}

void hwEraseGBA()
{
	// TODO: implement chip erase!
}

// -------------------------------------------------
uint32 hwGrab3in1()
{
	uint32 ime = enterCriticalSection();
	sysSetBusOwners(true, true);
	chip_reset();
	OpenNorWrite();
	return ime;
}

void hwRelease3in1(uint32 ime)
{
	CloseNorWrite();
	leaveCriticalSection(ime);
}
