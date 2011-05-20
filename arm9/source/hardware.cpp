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
#include <fat.h>

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
#include "dsi.h"

using namespace std;

static u32 pitch = 0x40000;


// ---------------------------------------------------------------------
bool swap_cart()
{
	sNDSHeader nds;
	nds.gameTitle[0] = 0;
	
	while (!nds.gameTitle[0]) {
		displayMessage2F(STR_HW_SWAP_CARD);

		bool swap = false;
		while (!swap) {
			if (keysCurrent() & KEY_A) {
				// identify hardware
				slot_1_type = get_slot1_type();
				// don't try to dump a flash card
				if (slot_1_type == 2)
					continue;

				sysSetBusOwners(true, true);
				// this will break DLDI on the Cyclops Evolution, but we need it anyway.
				cardReadHeader((u8*)&nds);
				displayPrintUpper();
				if (!nds.gameTitle[0]) {
					displayMessage2F(STR_HW_CARD_UNREADABLE);
					continue;
				}
				
				return true;
			}
		}
	}
	
	return true;
}

u32 get_slot1_type()
{
	sysSetBusOwners(true, true);

	// Trying to read the save size in IR mode will fail on non-IR devices.
	// If we have success, it is an IR device.
	u8 size2 = auxspi_save_size_log_2(true);
	if (size2 > 0)
		return 1;
	
	// It is not an IR game, so maybe it is a regular game.
	u8 size1 = auxspi_save_size_log_2(false);
	if (size1 > 0)
		return 0;
	
	// No save size test returned a save chip, so it must be a flash card,
	//  or a type 3 save with an unknown JEDEC ID. Unfortunately, due to
	//  different behaviour of flash cards, we can't test for this case,
	//  or the flash card may be recognised as "type 3 game".
	/*
	u32 id2 = auxspi_save_jedec_id(true);
	u32 id1 = auxspi_save_jedec_id(false);
	if (id2 != 0x00ffffff)
		return 1;
	if (id1 != 0x00ffffff)
		return 0;
		*/
	
	return 2;
}

bool swap_card_game(uint32 size)
{
	return false;
}

// This function is called on boot; it detects the hardware configuration and selects the mode.
u32 hwDetect()
{
	// Identify Slot 1 devide. This is used by pretty much everything.
	slot_1_type = get_slot1_type();

	// First, look for a DSi running in DSi mode.
	if (isDsi()) {
		size_buf = 1 << 23; // 8 MB memory buffer
		return 3;
	}
	size_buf = 1 << 21; // 2 MB memory buffer
	
	// Identify Slot 2 device.
	// First, look for an EZFlash 3in1
	uint32 ime = enterCriticalSection();
	sysSetBusOwners(true, true);
	OpenNorWrite();
	ezflash = ReadNorFlashID();
	CloseNorWrite();
	leaveCriticalSection(ime);
	chip_reset();
	if (ezflash)
		return 2;
	
	// Okay, maybe it is a regular GBA game instead
	if (gbaIsGame())
		return 1;
	
	// Maybe it is a Slot 2 flash card.

	// Detecting slot 2 flash cards is very evil:
	// - They don't usually support argv, so we can't simply test that we are
	//  running from "fat2".
	// - We need a passme compatible slot 1 device, i.e. we can't verify that
	//  there is a flash card in slot 1 (which usually is).
	// - There is only a limited number of Slot 2 flash cards on the market, so
	//  we could test for this (I think there is a library for this). But if we
	//  boot from Slot 1, a positive Slot 2 test does not mean that we did boot
	//  from there - the card may be sitting there unused!
	//
	// --> So we select slot 2 mode the "dumb" way - with an ini parameter. Unless you swap
	//  the same microSD card between Slot 1 and Slot 2, this should be the safest mode.
	// (If you know a smarter way of doing this, feel free to commit a patch!)
	//
	if (slot2)
		return 4;

	// Try to identify download play mode. This also introduces various complications:
	// - The latest libnds versions include code for the SD-slot on the DSi. It does not work
	//  from flash cards or download play, but it may still result in a positive DLDI driver
	//  initialisation. So we can't simply run "fatInitDefault" and test return values.
	// - Currently, download play mode aims to "insert game first, then run dlp", i.e.
	//  it does not support hotswapping the game. So we select this mode if there is *no*
	//  flash card inserted in Slot 1.
	//
	if (slot_1_type != 2)
		return 5;
	
	// Nothing unique found, so enter WiFi mode
	return 0;
}

// --------------------------------------------------------
void hwFormatNor(uint32 page, uint32 count)
{
	uint32 ime = hwGrab3in1();
	SetSerialMode();
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

	bool ir = (slot_1_type == 1) ? true : false;
	uint8 size = auxspi_save_size_log_2(ir);
	int size_blocks = 1 << max(0, (int8(size) - 18)); // ... in units of 0x40000 bytes - that's 256 kB
	uint8 type = auxspi_save_type(ir);

	displayMessage2F(STR_HW_3IN1_FORMAT_NOR);
	hwFormatNor(0, size_blocks+1);

	displayMessage2F(STR_HW_READ_GAME);
	if (size < 15)
		size_blocks = 1;
	else
		size_blocks = 1 << (uint8(size) - 15);
	u8 *test = (u8*)malloc(0x8000);
	if (test == NULL)
		while(1);
	u32 LEN = min(1 << size, 0x8000);
	
	for (int i = 0; i < size_blocks; i++) {
		displayProgressBar(i+1, size_blocks);
		auxspi_read_data(i << 15, data, LEN, type, ir);
		uint32 ime = hwGrab3in1();
		SetSerialMode();
		WriteNorFlash((i << 15) + pitch, data, LEN);
		hwRelease3in1(ime);
		// test if NOR memory is sane, i.e. if we can read what we just wrote
		ime = hwGrab3in1();
		ReadNorFlash(test, (i << 15) + pitch, LEN);
		hwRelease3in1(ime);
		if (*((vuint16 *)(FlashBase+0x2002)) == 0x227E) {
			displayMessage2F(STR_HW_3IN1_ERR_IDMODE);
			while(1);
		}
		if (memcmp(test, data, LEN)) {
			displayMessage2F(STR_HW_3IN1_ERR_NOR);
			while(1);
		}
	}
	free(test);

	// Write a flag to tell the app what happens on restart.
	// This is necessary, since some DLDI drivers cease working after swapping a card.
	// Fix for dead batteries: don't write "reboot" values to SRAM, but to NOR.
	displayMessage2F(STR_HW_3IN1_PREPARE_REBOOT);
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
	WriteNorFlash(0, data, 0x8000);
	hwRelease3in1(ime);
	
	displayMessage2F(STR_HW_3IN1_PLEASE_REBOOT);
	
	while(1) {};
}

void hwDump3in1(uint32 size, const char *gamename)
{
	u32 size_blocks = 1 << max(0, (uint8(size) - 18));

	displayMessageF(STR_HW_SELECT_FILE_OW);
	
	char path[256];
	char fname[256] = "";
	fileSelect("/", path, fname, 0, true, false);
	
	// look for an unused filename
	if (!fname[0]) {
		uint32 cnt = 0;
		sprintf(fname, "/%s.%i.sav", gamename, cnt);
		displayMessage2F(STR_HW_SEEK_UNUSED_FNAME, fname);
		while (fileExists(fname)) {
			if (cnt < 65536)
				cnt++;
			else {
				displayWarning2F(STR_ERR_NO_FNAME);
				while(1);
			}
			sprintf(fname, "%s.%i.sav", gamename, cnt);
		}
	}
	char fullpath[256];
	sprintf(fullpath, "%s/%s", path, fname);
	displayMessage2F(STR_HW_WRITE_FILE, fullpath);
	
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

	displayMessage2F(STR_HW_3IN1_DONE_DUMP, fullpath);
	while (!(keysCurrent() & KEY_B)) {};
}

void hwRestore3in1()
{
	char path[256];
	char fname[256];
	memset(fname, 0, 256);
	displayMessageF(STR_HW_SELECT_FILE); // lower screen is used for file browser
	fileSelect("/", path, fname, 0, false, false);
	char msg[256];
	// This does not have to be translated.
	// FIXME: make this more meaningful!
	sprintf(msg, "%s/%s", path, fname);
	
	FILE *file = fopen(msg, "rb");
	uint32 size0 = fileSize(msg);
	
	uint8 size = 1;
	// crude log2 function
	while (size0 > (1 << size)) {
		size++;
	}
	int size_blocks = 1 << max(0, int8(size) - 18); // ... in units of 0x40000 bytes - that's 256 kB
	
	displayMessage2F(STR_HW_3IN1_FORMAT_NOR);
	hwFormatNor(1, size_blocks);

	displayMessage2F(STR_HW_READ_FILE, msg);
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
			displayMessage2F(STR_HW_3IN1_ERR_IDMODE);
			while(1);
		}
		if (memcmp(test, data, LEN)) {
			displayMessage2F(STR_HW_3IN1_ERR_NOR);
			while(1);
		}
	}
	displayProgressBar(1, 1);
	fclose(file);
	free(test);
	
	hwRestore3in1_b(1 << size);
}

void hwRestore3in1_b(uint32 size_file)
{
	bool ir = (slot_1_type == 1) ? true : false;

	// Third, swap in a new game
	uint32 size = auxspi_save_size_log_2(ir);
	while ((size_file < size) || (slot_1_type == 2)) {
		if (slot_1_type == 2)
			displayMessage2F(STR_HW_SWAP_CARD);
		else if (size_file < size)
			displayMessage2F(STR_HW_WRONG_GAME);
		swap_cart();
		ir = (slot_1_type == 1) ? true : false;
		size = auxspi_save_size_log_2(ir);
	}
	displayPrintUpper();
	
	uint8 type = auxspi_save_type(ir);
	if (type == 3) {
		displayMessage2F(STR_HW_FORMAT_GAME);
		auxspi_erase(ir);
	}

	// And finally, write the save!
	displayMessage2F(STR_HW_WRITE_GAME);
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
		auxspi_write_data(i << shift, data, LEN, type, ir);
	}
	displayProgressBar(1, 1);

	displayMessage2F(STR_HW_PLEASE_REBOOT);	
	while (1) {};
}

// ------------------------------------------------------------
void hwErase()
{
	bool ir = (slot_1_type == 1) ? true : false;
	displayMessage2F(STR_HW_WARN_DELETE);
	while (!(keysCurrent() & (KEY_UP | KEY_R | KEY_Y))) {};
	auxspi_erase(ir);
}

// ------------------------------------------------------------
void hwBackupFTP(bool dlp)
{
	netbuf *buf, *ndata;
	int j;
	static int jmax = 10;
	bool ir = (slot_1_type == 1) ? true : false;

	// Dump save and write it to FTP server
	// First: swap card
	if (!dlp)
		swap_cart();
	displayPrintUpper();
	uint8 size = auxspi_save_size_log_2(ir);
	int size_blocks = 1 << max(0, (int8(size) - 18)); // ... in units of 0x40000 bytes - that's 256 kB
	uint8 type = auxspi_save_type(ir);
	if (size < 15)
		size_blocks = 1;
	else
		size_blocks = 1 << (uint8(size) - 15);

	// Second: connect to FTP server
	displayMessage2F(STR_HW_FTP_SEEK_AP);
	if (!Wifi_InitDefault(true)) {
		displayWarning2F(STR_HW_FTP_ERR_AP);
		while(1);
	}
	displayMessage2F(STR_HW_FTP_SEEK_FTP);
	char fullname[512];
	sprintf(fullname, "%s:%i", ftp_ip, ftp_port);
	j = 0;
	while (!FtpConnect(fullname, &buf)) {
		j++;
		if (j >= jmax) {
			displayWarning2F(STR_HW_FTP_ERR_FTP);
			while(1);
		}
		swiDelay(10000);
	}
	displayMessage2F(STR_HW_FTP_LOGIN);
	j = 0;
	while (!FtpLogin(ftp_user, ftp_pass, buf)) {
		j++;
		if (j >= jmax) {
			displayWarning2F(STR_HW_FTP_ERR_LOGIN);
			while(1);
		}
		swiDelay(10000);
	}
	
	char fdir[256] = "";
	char fname[256] ="";
	memset(fname, 0, 256);
	displayMessageF(STR_HW_SELECT_FILE_OW);
	displayStateF(STR_HW_FTP_DIR);
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
			displayMessage2F(STR_HW_SEEK_UNUSED_FNAME, fname);
			if (cnt < 65536)
				cnt++;
			else {
				displayWarning2F(STR_ERR_NO_FNAME);
				while(1);
			}
			sprintf(fname, "%.12s.%i.sav", nds.gameTitle, cnt);
		}
	}
	
	// Fourth: dump save
	displayStateF(STR_EMPTY);
	// This may not have to be translated.
	// FIXME: make this more meaningful!
	sprintf(fullname, "%s%s", fdir, fname);
	FtpAccess(fname, FTPLIB_FILE_WRITE, FTPLIB_IMAGE, buf, &ndata);
	u32 length = 0x200;
	if (size < 9)
		length = 1 << size;
	for (int i = 0; i < (1 << (size - 9)); i++) {
		displayProgressBar(i+1, (size_blocks << 6));
		auxspi_read_data(i << 9, (u8*)&data[0], length, type, ir);
		u32 out = 0;
		while (out < length) {
			u32 delta = FtpWrite((u8*)&data[out], length-out, ndata);
			out += delta;
			if (delta == 0) {
				displayMessage2F(STR_HW_FTP_READ_ONLY);
			} else {
				displayMessage2F(STR_EMPTY);
			}
			if (delta < length) {
				displayStateF(STR_HW_FTP_SLOW);
			} else {
				displayStateF(STR_EMPTY);
			}
		}
	}
	FtpClose(ndata);
	FtpQuit(ndata);
	
	Wifi_DisconnectAP();

	if (dlp) {
		displayMessage2F(STR_HW_PLEASE_REBOOT);
		while(1);
	}
}

void hwRestoreFTP(bool dlp)
{
	netbuf *buf, *ndata;
	int j;
	static int jmax = 10;
	bool ir = (slot_1_type == 1) ? true : false;

	// Dump save and write it to FTP server
	// First: connect to FTP server
	displayMessage2F(STR_HW_FTP_SEEK_AP);
	if (!Wifi_InitDefault(true)) {
		displayWarning2F(STR_HW_FTP_ERR_AP);
		while(1);
	}
	displayMessage2F(STR_HW_FTP_SEEK_FTP);
	char fullname[512];
	sprintf(fullname, "%s:%i", ftp_ip, ftp_port);
	j = 0;
	while (!FtpConnect(fullname, &buf)) {
		j++;
		if (j >= jmax) {
			displayWarning2F(STR_HW_FTP_ERR_FTP);
			while(1);
		}
		swiDelay(10000);
	}
	displayMessage2F(STR_HW_FTP_LOGIN);
	j = 0;
	while (!FtpLogin(ftp_user, ftp_pass, buf)) {
		j++;
		if (j >= jmax) {
			displayWarning2F(STR_HW_FTP_ERR_LOGIN);
			while(1);
		}
		swiDelay(10000);
	}

	// Second: select a filename
	char fdir[256] = "";
	char fname[256] ="";
	memset(fname, 0, 256);
	displayMessageF(STR_HW_SELECT_FILE);
	displayStateF(STR_STR, "FTP: dir");
	fileSelect("/", fdir, fname, buf, false, false);
	
	// Third: swap card
	if (!dlp)
		swap_cart();
	displayPrintUpper();
	uint8 size = auxspi_save_size_log_2(ir);
	uint8 type = auxspi_save_type(ir);

	// Fourth: read file
	displayStateF(STR_EMPTY);
	FtpChdir(fdir, buf);
	// FIXME: make this more meaningful!
	sprintf(fullname, "%s%s", fdir, fname);
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
		displayMessageF("Save larger than available RAM.\nRestoring will be dangerous!\n\nPress Start + Select to proceed");
		while (!(keysCurrent() & (KEY_START | KEY_SELECT)));
#else
		// TODO: translate this message!
		displayMessage2F(STR_STR, "Save larger than available RAM.\nRestoring is buggy, so\nI will stop here.\n(Try Rudolphs tools.)");
		while (1);
#endif
	}
	if ((type == 3) && (insecure)) {
		displayMessage2F(STR_HW_FORMAT_GAME);
		auxspi_erase(ir);
	}
	FtpAccess(fname, FTPLIB_FILE_READ, FTPLIB_IMAGE, buf, &ndata);
	u8 *pdata = data;
	for (int i = 0; i < num_blocks_ftp; i++) {
		displayProgressBar(i+1, num_blocks_ftp);
		int in = 0;
		while (in < 512) {
			in += FtpRead((u8*)&pdata[in], 512-in, ndata);
			if (in < 512)
				displayStateF(STR_HW_FTP_SLOW);
			else
				displayStateF(STR_EMPTY);
		}
		// does not fit into memory: unsafe mode
		// TODO: reactivate this!
		if (insecure) {
			displayStateF(STR_STR, "Writing save (insecure!)");
			for (int j = 0; j < 1 << (9-shift); j++) {
				auxspi_write_data((i << 9)+(j << shift), ((u8*)pdata)+(j<<shift), LEN, type, ir);
			}
		}
		pdata += 512;
	}
	// Write to game (safe mode)
	if (!insecure) {
		if (type == 3) {
			displayMessage2F(STR_HW_FORMAT_GAME);
			auxspi_erase(ir);
		}
		for (int i = 0; i < (1 << (size - shift)); i++) {
			displayMessage2F(STR_HW_WRITE_GAME);
			displayProgressBar(i+1, 1 << (size - shift));
			auxspi_write_data(i << shift, ((u8*)data)+(i<<shift), LEN, type, ir);
		}
	}
	FtpClose(ndata);
	FtpQuit(ndata);

	Wifi_DisconnectAP();

	if (dlp) {
		displayMessage2F(STR_HW_PLEASE_REBOOT);
		while(1);
	}
}

// ------------------------------------------------------------
void hwBackupGBA(u8 type)
{
	if ((type == 0) || (type > 5))
		return;
	
	if ((type == 1) || (type == 2)) {
		// This is not to be translated, it will be removed at some point.
		displayMessageF(STR_STR, "I can't read this save type\nyet. Please use Rudolphs tool\ninstead.");
		return;
	}
	
	char path[256];
	char fname[256] = "";
	char *gamename = (char*)0x080000a0;
	fileSelect("/", path, fname, 0, true, false);
	// look for an unused filename
	if (!fname[0]) {
		uint32 cnt = 0;
		sprintf(fname, "/%.12s.%i.sav", gamename, cnt);
		displayMessage2F(STR_HW_SEEK_UNUSED_FNAME, fname);
		while (fileExists(fname)) {
			if (cnt < 65536)
				cnt++;
			else {
				displayWarning2F(STR_ERR_NO_FNAME);
				while(1);
			}
			sprintf(fname, "/%.12s.%i.sav", gamename, cnt);
		}
	}
	char fullpath[512];
	sprintf(fullpath, "%s/%s", path, fname);
	
	displayMessage2F(STR_HW_READ_GAME);
	uint32 size = gbaGetSaveSize(type);
	gbaReadSave(data, 0, size, type);
	
	displayMessage2F(STR_HW_WRITE_FILE, fullpath);
	FILE *file = fopen(fullpath, "wb");
	fwrite(data, 1, size, file);
	fclose(file);

	displayStateF(STR_STR, "Done!");
	while(1);
}

void hwRestoreGBA()
{
	u8 type = gbaGetSaveType();
	if ((type == 0) || (type > 5))
		return;
	
	if ((type == 1) || (type == 2)) {
		// This is not to be translated, it will be removed at some point.
		displayMessageF(STR_STR, "I can't write this save type\nyet. Please use Rudolphs tool\ninstead.");
		return;
	}
	
	uint32 size = gbaGetSaveSize(type);
	
	char path[256];
	char fname[256] = "";
	fileSelect("/", path, fname, 0);
	char fullpath[512];
	sprintf(fullpath, "%s/%s", path, fname);

	displayMessage2F(STR_HW_READ_FILE, fname);
	FILE *file = fopen(fullpath, "rb");
	fread(data, 1, size, file);
	fclose(file);
	
	if ((type == 4) || (type == 5)) {
		displayMessage2F(STR_HW_FORMAT_GAME);
		gbaFormatSave(type);
	}
	
	displayMessage2F(STR_HW_WRITE_GAME);
	gbaWriteSave(0, data, size, type);

	displayMessage2F(STR_HW_PLEASE_REBOOT);
	while(1);
}

void hwEraseGBA()
{
	// TODO: implement chip erase!
	u8 type = gbaGetSaveType();
	if ((type == 0) || (type > 5))
		return;
	
	memset(data, 0, gbaGetSaveSize(type));
	
	switch (type) {
	}
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
