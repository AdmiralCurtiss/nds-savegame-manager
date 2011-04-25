/*
 * savegame_manager: a tool to backup and restore savegames from Nintendo
 *  DS cartridges. Nintendo DS and all derivative names are trademarks
 *  by Nintendo. EZFlash 3-in-1 is a trademark by EZFlash.
 *
 * dsCard.h: header file for dsCard.cpp
 *
 * Copyright (C) EZFlash Group (2006)
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

/**************************************************************************************************************
 * 此文件为 dsCard.h 文件的第二版 
 * 日期：2006年11月27日11点33分  第一版 version 1.0
 * 作者：aladdin
 * CopyRight : EZFlash Group
 * 
 **************************************************************************************************************/

#ifndef NDS_DSCARD_V2_INCLUDE
#define NDS_DSCARD_V2_INCLUDE

#include "nds.h"

	#ifdef __cplusplus
	extern "C" {
	#endif

#ifndef BYTE
typedef unsigned char BYTE;
#endif

#ifndef WORD
typedef unsigned short WORD;
#endif

#ifndef DWORD
typedef unsigned long DWORD;
#endif

#ifndef BOOL
typedef bool BOOL ;
#endif
// export interface




//---------------------------------------------------
//DS 卡 基本操作
		//Arm9 方面基本操作容许ARM7访问slot1
		void		Enable_Arm7DS(void);

		//Arm9 方面基本操作容许ARM9访问slot1
		void		Enable_Arm9DS(void);

		
		//下面是访问震动卡的函数
#define FlashBase		0x08000000
#define	_Ez5PsRAM 	0x08000000
		void		OpenNorWrite();
		void		CloseNorWrite();
		void      SetRompage(u16 page);
		void 		SetRampage(u16 page);
		void  	OpenRamWrite();
		void 		CloseRamWrite();
		void      SetSerialMode();
		uint32   ReadNorFlashID();
		void 		chip_reset();
		void 		Block_Erase(u32 blockAdd);
		void 		ReadNorFlash(u8* pBuf,u32 address,u16 len);
		void 		WriteNorFlash(u32 address,u8 *buffer,u32 size);
		void 		WriteSram(uint32 address, u8* data , uint32 size );
		void 		ReadSram(uint32 address, u8* data , uint32 size );
		void 		SetShake(u16 data);
		void WritePSram(u8* address, u8* data , uint32 length );
		void ReadPSram(u8* address, u8* data , uint32 length );
	#ifdef __cplusplus
	}
	#endif
#endif
