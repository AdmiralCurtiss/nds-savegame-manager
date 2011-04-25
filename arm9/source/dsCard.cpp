/*
 * savegame_manager: a tool to backup and restore savegames from Nintendo
 *  DS cartridges. Nintendo DS and all derivative names are trademarks
 *  by Nintendo. EZFlash 3-in-1 is a trademark by EZFlash.
 *
 * dsCard.cpp: Functions for accessing an EZFlash 3-in-1.
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

#include "dscard.h"
#include "string.h"

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

// This is set by "ReadNORFlashID", which *must* *always* be called before anything else!
uint32 ID;

//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
//---------------------------------------------------
//DS ¿¨ »ù±¾²Ù×÷
void Enable_Arm9DS()
{
	REG_EXMEMCNT &= ~0x0800;
	//WAIT_CR &= ~0x0800;
}

void Enable_Arm7DS()
{
	REG_EXMEMCNT |= 0x0800;
	//WAIT_CR |= 0x0800;
}

void		OpenNorWrite()
{
	*(vuint16 *)0x9fe0000 = 0xd200;
	*(vuint16 *)0x8000000 = 0x1500;
	*(vuint16 *)0x8020000 = 0xd200;
	*(vuint16 *)0x8040000 = 0x1500;
	*(vuint16 *)0x9C40000 = 0x1500;
	*(vuint16 *)0x9fc0000 = 0x1500;
}


void		CloseNorWrite()
{
	*(vuint16 *)0x9fe0000 = 0xd200;
	*(vuint16 *)0x8000000 = 0x1500;
	*(vuint16 *)0x8020000 = 0xd200;
	*(vuint16 *)0x8040000 = 0x1500;
	*(vuint16 *)0x9C40000 = 0xd200;
	*(vuint16 *)0x9fc0000 = 0x1500;
}

void SetRompage(u16 page)
{
	*(vuint16 *)0x9fe0000 = 0xd200;
	*(vuint16 *)0x8000000 = 0x1500;
	*(vuint16 *)0x8020000 = 0xd200;
	*(vuint16 *)0x8040000 = 0x1500;
	*(vuint16 *)0x9880000 = page;
	*(vuint16 *)0x9fc0000 = 0x1500;
}
void SetRampage(u16 page)
{
	*(vu16 *)0x9fe0000 = 0xd200;
	*(vu16 *)0x8000000 = 0x1500;
	*(vu16 *)0x8020000 = 0xd200;
	*(vu16 *)0x8040000 = 0x1500;
	*(vu16 *)0x9c00000 = page;
	*(vu16 *)0x9fc0000 = 0x1500;
}
void SetSerialMode()
{
	
	*(vu16 *)0x9fe0000 = 0xd200;
	*(vu16 *)0x8000000 = 0x1500;
	*(vu16 *)0x8020000 = 0xd200;
	*(vu16 *)0x8040000 = 0x1500;
	*(vu16 *)0x9A40000 = 0xe200;
	*(vu16 *)0x9fc0000 = 0x1500;
	
}
uint32   ReadNorFlashID()
{
		vuint16 id1,id2,id3,id4;
		ID=0;
		//check intel 512M 3in1 card
		*((vuint16 *)(FlashBase+0)) = 0xFF ;
		*((vuint16 *)(FlashBase+0x1000*2)) = 0xFF ;
		*((vuint16 *)(FlashBase+0)) = 0x90 ;
		*((vuint16 *)(FlashBase+0x1000*2)) = 0x90 ;
		id1 = *((vuint16 *)(FlashBase+0)) ;
		id2 = *((vuint16 *)(FlashBase+0x1000*2)) ;
		id3 = *((vuint16 *)(FlashBase+1*2)) ;
		id4 = *((vuint16 *)(FlashBase+0x1001*2)) ;
		if(id3==0x8810)
			id3=0x8816;
		if(id4==0x8810)
			id4=0x8816;
		if( (id1==0x89)&& (id2==0x89) &&(id3==0x8816) && (id4==0x8816))
		{
			ID = 0x89168916;
			return 0x89168916;
		}
		//¼ì²â256M¿¨
		*((vuint16 *)(FlashBase+0x555*2)) = 0xAA ;
		*((vuint16 *)(FlashBase+0x2AA*2)) = 0x55 ;
		*((vuint16 *)(FlashBase+0x555*2)) = 0x90 ;

		*((vuint16 *)(FlashBase+0x1555*2)) = 0xAA ;
		*((vuint16 *)(FlashBase+0x12AA*2)) = 0x55 ;
		*((vuint16 *)(FlashBase+0x1555*2)) = 0x90 ;

		id1 = *((vuint16 *)(FlashBase+0x2)) ;
		id2 = *((vuint16 *)(FlashBase+0x2002)) ;
		if( (id1!=0x227E)|| (id2!=0x227E))
			return 0;
		
		id1 = *((vuint16 *)(FlashBase+0xE*2)) ;
		id2 = *((vuint16 *)(FlashBase+0x100e*2)) ;
		if(id1==0x2218 && id2==0x2218)			//H6H6
		{
			ID = 0x227E2218;
			return 0x227E2218;
		}
			
		if(id1==0x2202 && id2==0x2202)			//VZ064
		{
			ID = 0x227E2202;
			return 0x227E2202;
		}
		if(id1==0x2202 && id2==0x2220)			//VZ064
		{
			ID = 0x227E2202;
			return 0x227E2202;
		}
		if(id1==0x2202 && id2==0x2215)			//VZ064
		{
			ID = 0x227E2202;
			return 0x227E2202;
		}
		
		return 0;			
}
void chip_reset()
{
		if(ID==0x89168916)
		{
			*((vuint16 *)(FlashBase+0)) = 0x50 ;
			*((vuint16 *)(FlashBase+0x1000*2)) = 0x50 ;
			*((vuint16 *)(FlashBase+0)) = 0xFF ;
			*((vuint16 *)(FlashBase+0x1000*2)) = 0xFF ;	
			return;
		}
		*((vu16 *)(FlashBase)) = 0xF0 ;
		*((vu16 *)(FlashBase+0x1000*2)) = 0xF0 ;
		
		if(ID==0x227E2202)
		{
			*((vu16 *)(FlashBase+0x1000000)) = 0xF0 ;
			*((vu16 *)(FlashBase+0x1000000+0x1000*2)) = 0xF0 ;
		}
}

void Block_EraseIntel(u32 blockAdd)
{
	u32 loop;
	vu16 v1,v2;
	bool b512=false;
	if(blockAdd>=0x2000000)
	{
		blockAdd-=0x2000000;
		CloseNorWrite();
		SetRompage(768);
		OpenNorWrite();	
		b512=true;
	}
	if(blockAdd==0)
	{
		for(loop=0;loop<0x40000;loop+=0x10000)
		{
			*((vu16 *)(FlashBase+loop)) = 0x50 ;
			*((vu16 *)(FlashBase+loop+0x2000)) = 0x50 ;
			*((vu16 *)(FlashBase+loop)) = 0xFF ;
			*((vu16 *)(FlashBase+loop+0x2000)) = 0xFF ;
			*((vu16 *)(FlashBase+loop)) = 0x60 ;
			*((vu16 *)(FlashBase+loop+0x2000)) = 0x60;
			*((vu16 *)(FlashBase+loop)) = 0xD0 ;
			*((vu16 *)(FlashBase+loop+0x2000)) = 0xD0;
			*((vu16 *)(FlashBase+loop)) = 0x20 ;
			*((vu16 *)(FlashBase+loop+0x2000)) = 0x20;
			*((vu16 *)(FlashBase+loop)) = 0xD0 ;
			*((vu16 *)(FlashBase+loop+0x2000)) = 0xD0;
			
			do
			{
				v1 = *((vu16 *)(FlashBase+loop));
				v2 = *((vu16 *)(FlashBase+loop+0x2000));
			}
			while((v1!=0x80)||(v2!=0x80));
		}
	}
	else
	{
		*((vu16 *)(FlashBase+blockAdd)) = 0xFF ;
		*((vu16 *)(FlashBase+blockAdd+0x2000)) = 0xFF ;
		*((vu16 *)(FlashBase+blockAdd)) = 0x60 ;
		*((vu16 *)(FlashBase+blockAdd+0x2000)) = 0x60 ;
		*((vu16 *)(FlashBase+blockAdd)) = 0xD0 ;
		*((vu16 *)(FlashBase+blockAdd+0x2000)) = 0xD0 ;
		*((vu16 *)(FlashBase+blockAdd)) = 0x20 ;
		*((vu16 *)(FlashBase+blockAdd+0x2000)) = 0x20 ;
		*((vu16 *)(FlashBase+blockAdd)) = 0xD0 ;
		*((vu16 *)(FlashBase+blockAdd+0x2000)) = 0xD0 ;
		do
		{
			v1 = *((vu16 *)(FlashBase+blockAdd));
			v2 = *((vu16 *)(FlashBase+blockAdd+0x2000));
		
		}
		while((v1!=0x80)||(v2!=0x80));
	}
	if(b512)
	{
		CloseNorWrite();
		SetRompage(0);
		OpenNorWrite();	
		b512=true;
	}
}

void Block_Erase(u32 blockAdd)
{
		vu16 v1,v2;  
		u32 Address;
		u32 loop;
		u32 off=0;
		if(ID==0x89168916)
		{
			//intel 512 card
			Block_EraseIntel(blockAdd);
			return ;
		}
		if( (blockAdd>=0x1000000) &&  (ID==0x227E2202))
		{
			off=0x1000000;
			*((vu16 *)(FlashBase+off+0x555*2)) = 0xF0 ;
			*((vu16 *)(FlashBase+off+0x1555*2)) = 0xF0 ;
		}
		else
			off=0;
		Address=blockAdd;
		*((vu16 *)(FlashBase+0x555*2)) = 0xF0 ;
		*((vu16 *)(FlashBase+0x1555*2)) = 0xF0 ;
		
	
		if( (blockAdd==0) || (blockAdd==0x1FC0000) || (blockAdd==0xFC0000) || (blockAdd==0x1000000))
		{
			for(loop=0;loop<0x40000;loop+=0x8000)
			{
				*((vu16 *)(FlashBase+off+0x555*2)) = 0xAA ;
				*((vu16 *)(FlashBase+off+0x2AA*2)) = 0x55 ;
				*((vu16 *)(FlashBase+off+0x555*2)) = 0x80 ;
				*((vu16 *)(FlashBase+off+0x555*2)) = 0xAA ;
				*((vu16 *)(FlashBase+off+0x2AA*2)) = 0x55 ;
				*((vu16 *)(FlashBase+Address+loop)) = 0x30 ;
				
				*((vu16 *)(FlashBase+off+0x1555*2)) = 0xAA ;
				*((vu16 *)(FlashBase+off+0x12AA*2)) = 0x55 ;
				*((vu16 *)(FlashBase+off+0x1555*2)) = 0x80 ;
				*((vu16 *)(FlashBase+off+0x1555*2)) = 0xAA ;
				*((vu16 *)(FlashBase+off+0x12AA*2)) = 0x55 ;
				*((vu16 *)(FlashBase+Address+loop+0x2000)) = 0x30 ;
				
				*((vu16 *)(FlashBase+off+0x2555*2)) = 0xAA ;
				*((vu16 *)(FlashBase+off+0x22AA*2)) = 0x55 ;
				*((vu16 *)(FlashBase+off+0x2555*2)) = 0x80 ;
				*((vu16 *)(FlashBase+off+0x2555*2)) = 0xAA ;
				*((vu16 *)(FlashBase+off+0x22AA*2)) = 0x55 ;
				*((vu16 *)(FlashBase+Address+loop+0x4000)) = 0x30 ;
				
				*((vu16 *)(FlashBase+off+0x3555*2)) = 0xAA ;
				*((vu16 *)(FlashBase+off+0x32AA*2)) = 0x55 ; 
				*((vu16 *)(FlashBase+off+0x3555*2)) = 0x80 ;
				*((vu16 *)(FlashBase+off+0x3555*2)) = 0xAA ;
				*((vu16 *)(FlashBase+off+0x32AA*2)) = 0x55 ;
				*((vu16 *)(FlashBase+Address+loop+0x6000)) = 0x30 ;
				do
				{  
					
					v1 = *((vu16 *)(FlashBase+Address+loop)) ;
					v2 = *((vu16 *)(FlashBase+Address+loop)) ;
				}while(v1!=v2);
				do
				{
					
					v1 = *((vu16 *)(FlashBase+Address+loop+0x2000)) ;
					v2 = *((vu16 *)(FlashBase+Address+loop+0x2000)) ;
				}while(v1!=v2);
				do
				{
					
					v1 = *((vu16 *)(FlashBase+Address+loop+0x4000)) ;
					v2 = *((vu16 *)(FlashBase+Address+loop+0x4000)) ;
				}while(v1!=v2);
				do
				{
					
					v1 = *((vu16 *)(FlashBase+Address+loop+0x6000)) ;
					v2 = *((vu16 *)(FlashBase+Address+loop+0x6000)) ;
				}while(v1!=v2);
			}	
		}	
		else
		{
			*((vu16 *)(FlashBase+off+0x555*2)) = 0xAA ;
			*((vu16 *)(FlashBase+off+0x2AA*2)) = 0x55 ;
			*((vu16 *)(FlashBase+off+0x555*2)) = 0x80 ;
			*((vu16 *)(FlashBase+off+0x555*2)) = 0xAA ;
			*((vu16 *)(FlashBase+off+0x2AA*2)) = 0x55;
			*((vu16 *)(FlashBase+Address)) = 0x30 ;
			
			*((vu16 *)(FlashBase+off+0x1555*2)) = 0xAA ;
			*((vu16 *)(FlashBase+off+0x12AA*2)) = 0x55 ;
			*((vu16 *)(FlashBase+off+0x1555*2)) = 0x80 ;
			*((vu16 *)(FlashBase+off+0x1555*2)) = 0xAA ;
			*((vu16 *)(FlashBase+off+0x12AA*2)) = 0x55 ;
			*((vu16 *)(FlashBase+Address+0x2000)) = 0x30 ;
			
			do
			{
				v1 = *((vu16 *)(FlashBase+Address)) ;
				v2 = *((vu16 *)(FlashBase+Address)) ;
			}while(v1!=v2);
			do
			{
				v1 = *((vu16 *)(FlashBase+Address+0x2000)) ;
				v2 = *((vu16 *)(FlashBase+Address+0x2000)) ;
			}while(v1!=v2);
			
			*((vu16 *)(FlashBase+off+0x555*2)) = 0xAA ;
			*((vu16 *)(FlashBase+off+0x2AA*2)) = 0x55 ;
			*((vu16 *)(FlashBase+off+0x555*2)) = 0x80 ;
			*((vu16 *)(FlashBase+off+0x555*2)) = 0xAA ;
			*((vu16 *)(FlashBase+off+0x2AA*2)) = 0x55;
			*((vu16 *)(FlashBase+Address+0x20000)) = 0x30 ;
			
			*((vu16 *)(FlashBase+off+0x1555*2)) = 0xAA ;
			*((vu16 *)(FlashBase+off+0x12AA*2)) = 0x55 ;
			*((vu16 *)(FlashBase+off+0x1555*2)) = 0x80 ;
			*((vu16 *)(FlashBase+off+0x1555*2)) = 0xAA ;
			*((vu16 *)(FlashBase+off+0x12AA*2)) = 0x55 ;
			*((vu16 *)(FlashBase+Address+0x2000+0x20000)) = 0x30 ;
		
			do
			{
				v1 = *((vu16 *)(FlashBase+Address+0x20000)) ;
				v2 = *((vu16 *)(FlashBase+Address+0x20000)) ;
			}while(v1!=v2);
			do
			{
				v1 = *((vu16 *)(FlashBase+Address+0x2000+0x20000)) ;
				v2 = *((vu16 *)(FlashBase+Address+0x2000+0x20000)) ;
			}while(v1!=v2);	
		}
}
void 		ReadNorFlash(u8* pBuf,u32 address,u16 len)
{
	vu16 *p = (vu16 *)pBuf;
	u32 loop;
	bool b512=false;
	if(address>=0x2000000)//256M
	{
		CloseNorWrite();
		SetRompage(768);
		address-=0x2000000;
		b512=true;
	}
	Enable_Arm7DS();
	OpenNorWrite();
	if(ID==0x89168916)
	{
			*((vuint16 *)(FlashBase+address)) = 0x50 ;
			*((vuint16 *)(FlashBase+address+0x1000*2)) = 0x50 ;
			*((vuint16 *)(FlashBase+address)) = 0xFF ;
			*((vuint16 *)(FlashBase+address+0x1000*2)) = 0xFF ;
	}
	for(loop=0;loop<len/2;loop++)
	{
		p[loop]=*((vu16 *)(FlashBase+address+loop*2) );
	}	
	CloseNorWrite();
	Enable_Arm9DS();
	if(b512==true)
	{
		SetRompage(0);
	}
}

void WriteNorFlashINTEL(u32 address,u8 *buffer,u32 size)
{
	u32 mapaddress;
	u32 size2,lop,j;
	vu16* buf = (vu16*)buffer ;
	register u32 loopwrite,i ;
	vu16 v1,v2;
	
	bool b512=false;
	if(address>=0x2000000)
	{
		address-=0x2000000;
		CloseNorWrite();
		SetRompage(768);
		OpenNorWrite();	
		b512=true;
	}
	else
	{
		CloseNorWrite();
		SetRompage(0);
		OpenNorWrite();	
	}
	
	if(size>0x4000)
	{
		size2 = size >>1 ;
		lop = 2; 
	}
	else 
	{
		size2 = size  ;
		lop = 1;
	}
	mapaddress = address;

//	_consolePrintf("WriteNorFlashINTEL begin\n");
	for(j=0;j<lop;j++)
	{
		if(j!=0)
		{
			mapaddress += 0x4000;
			buf = (vu16*)(buffer+0x4000);
		}
		for(loopwrite=0;loopwrite<(size2);loopwrite+=64)
		{
//			_consolePrintf("WriteNorFlashINTEL begin 1\n");
			*((vu16 *)(FlashBase+mapaddress+(loopwrite>>1))) = 0x50;
			*((vu16 *)(FlashBase+mapaddress+(loopwrite>>1)+0x2000)) = 0x50;
				*((vu16 *)(FlashBase+mapaddress+(loopwrite>>1))) = 0xFF;
			*((vu16 *)(FlashBase+mapaddress+(loopwrite>>1)+0x2000)) = 0xFF;
			*((vu16 *)(FlashBase+mapaddress+(loopwrite>>1))) = 0xE8;
			*((vu16 *)(FlashBase+mapaddress+(loopwrite>>1)+0x2000)) = 0xE8;
			*((vu16 *)(FlashBase+mapaddress+(loopwrite>>1))) = 0x70;
			*((vu16 *)(FlashBase+mapaddress+(loopwrite>>1)+0x2000)) = 0x70;
			v1=v2=0;
			while((v1!= 0x80) || (v2 != 0x80) )
			{
				v1 = *((vu16 *)(FlashBase+mapaddress+(loopwrite>>1))) ;
				v2 = *((vu16 *)(FlashBase+mapaddress+(loopwrite>>1)+0x2000)) ;
			}
			*((vu16 *)(FlashBase+mapaddress+(loopwrite>>1))) = 0x0F;
			*((vu16 *)(FlashBase+mapaddress+(loopwrite>>1)+0x2000)) = 0x0F;
			for(i=0;i<0x10;i++)
			{
				*((vu16 *)(FlashBase+mapaddress+(loopwrite>>1)+i*2)) = buf[(loopwrite>>2)+i];
				*((vu16 *)(FlashBase+mapaddress+0x2000+(loopwrite>>1)+i*2)) = buf[0x1000+(loopwrite>>2)+i];
			}
			*((vu16 *)(FlashBase+mapaddress+(loopwrite>>1))) = 0xD0;
			*((vu16 *)(FlashBase+mapaddress+(loopwrite>>1)+0x2000)) = 0xD0;
			v1=v2=0;
//			_consolePrintf("WriteNorFlashINTEL begin 2\n");
			while((v1!= 0x80) || (v2 != 0x80) )
			{
				v1 = *((vu16 *)(FlashBase+mapaddress+(loopwrite>>1))) ;
				v2 = *((vu16 *)(FlashBase+mapaddress+(loopwrite>>1)+0x2000)) ;
				if( (v1==0x90) || (v2==0x90))
				{
					WriteSram(0xA000000,(u8 *)buf,0x8000);
//					_consolePrintf("Err \n");
					while(1);
					break;
				}
			}
//			_consolePrintf("WriteNorFlashINTEL begin 3\n");
		}
	}
	if(b512==true)
	{
		CloseNorWrite();
		SetRompage(0);
		OpenNorWrite();	
	}
}

void WriteNorFlash(u32 address,u8 *buffer,u32 size)
{
		if(ID==0x89168916)
		{
			WriteNorFlashINTEL(address,buffer,size);
			return;
		}
		vu16 v1,v2;
		register u32 loopwrite ;
		vu16* buf = (vu16*)buffer ;
		u32 size2,lop;
		u32 mapaddress;
		u32 j;
		v1=0;v2=1;
		u32 off=0;
		if( (address>=0x1000000) &&  (ID==0x227E2202))
		{
			off=0x1000000;
		}
		else
			off=0;
		if(size>0x4000)
		{
			size2 = size >>1 ;
			lop = 2; 
		}
		else 
		{
			size2 = size  ;
			lop = 1;
		}
		mapaddress = address;
		for(j=0;j<lop;j++)
		{
			if(j!=0)
			{
				mapaddress += 0x4000;
				buf = (vu16*)(buffer+0x4000);
			}
			for(loopwrite=0;loopwrite<(size2>>2);loopwrite++)
			{
				*((vu16 *)(FlashBase+off+0x555*2)) = 0xAA ;
				*((vu16 *)(FlashBase+off+0x2AA*2)) = 0x55 ;
				*((vu16 *)(FlashBase+off+0x555*2)) = 0xA0 ;
				*((vu16 *)(FlashBase+mapaddress+loopwrite*2)) = buf[loopwrite];
				
				*((vu16 *)(FlashBase+off+0x1555*2)) = 0xAA ;
				*((vu16 *)(FlashBase+off+0x12AA*2)) = 0x55 ;
				*((vu16 *)(FlashBase+off+0x1555*2)) = 0xA0 ;			
				*((vu16 *)(FlashBase+mapaddress+0x2000+loopwrite*2)) = buf[0x1000+loopwrite];
				do
				{
					v1 = *((vu16 *)(FlashBase+mapaddress+loopwrite*2)) ;
					v2 = *((vu16 *)(FlashBase+mapaddress+loopwrite*2)) ;
				}while(v1!=v2);
				do
				{
					v1 = *((vu16 *)(FlashBase+mapaddress+0x2000+loopwrite*2)) ;
					v2 = *((vu16 *)(FlashBase+mapaddress+0x2000+loopwrite*2)) ;
				}while(v1!=v2);
			}
		}	
}
void WriteSram(uint32 address, u8* data , uint32 size )
{	
	uint32 i ;
	for(i=0;i<size;i++)
		*(u8*)(address+i)=data[i];
}
void ReadSram(uint32 address, u8* data , uint32 size )
{
	uint32 i ;
	u16* pData = (u16*)data;
	for(i=0;i<size;i+=2)
	{
		pData[i>>1]=*(u8*)(address+i)+(*(u8*)(address+i+1)*0x100);
	}
}

void WritePSram(u8* address, u8* data , uint32 length )
{	
	vuint16 *pPatch=NULL;
	pPatch=(vuint16*)address;
	Enable_Arm7DS();
	CloseNorWrite();  
	SetRompage(192);
	OpenNorWrite();
	for(u32 pi=0;pi<length;pi+=2)
	{
		pPatch[pi>>1]= data[pi] + (data[pi+1]<<8);


	}
	CloseNorWrite();
	Enable_Arm9DS();
	
}
void ReadPSram(u8* address, u8* data , uint32 length )
{
	vuint16 *pPatch=NULL;
	pPatch=(vuint16*)address;
	Enable_Arm7DS();
	CloseNorWrite();  
	SetRompage(192);
	OpenNorWrite();
	for(u32 pi=0;pi<length;pi+=2)
	{
		data[pi]=pPatch[pi>>1] & 0xff; 
		data[pi+1]=(pPatch[pi>>1] &0xff00)>>8;

	}
	CloseNorWrite();
	Enable_Arm9DS();

}
void  OpenRamWrite()
{
	*(vu16 *)0x9fe0000 = 0xd200;
	*(vu16 *)0x8000000 = 0x1500;
	*(vu16 *)0x8020000 = 0xd200;
	*(vu16 *)0x8040000 = 0x1500;
	*(vu16 *)0x9C40000 = 0xA500;
	*(vu16 *)0x9fc0000 = 0x1500;
}

void CloseRamWrite()
{
	*(vu16 *)0x9fe0000 = 0xd200;
	*(vu16 *)0x8000000 = 0x1500;
	*(vu16 *)0x8020000 = 0xd200;
	*(vu16 *)0x8040000 = 0x1500;
	*(vu16 *)0x9C40000 = 0xA200;
	*(vu16 *)0x9fc0000 = 0x1500;
}
void SetShake(u16 data)
{
	*(vuint16 *)0x9fe0000 = 0xd200;
	*(vuint16 *)0x8000000 = 0x1500;
	*(vuint16 *)0x8020000 = 0xd200;
	*(vuint16 *)0x8040000 = 0x1500;
	*(vuint16 *)0x9E20000 = data;
	*(vuint16 *)0x9fc0000 = 0x1500;
}
#ifdef __cplusplus
}
#endif
