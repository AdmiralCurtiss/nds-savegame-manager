/*
 * savegame_manager: a tool to backup and restore savegames from Nintendo
 *  DS cartridges. Nintendo DS and all derivative names are trademarks
 *  by Nintendo. EZFlash 3-in-1 is a trademark by EZFlash.
 *
 * display.h: header file for display.cpp
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

#ifndef SAVE_DISPLAY_H
#define SAVE_DISPLAY_H

#include <nds.h>
#include <nds/arm9/console.h>
#include <stdarg.h>


extern PrintConsole upperScreen;
extern PrintConsole lowerScreen;


void displayInit();
void displayTitle();

// Set "first" to true to tell the program that it has just booted, before anything was swapped.
//  This is a workaround for the Cyclops iEvolution, and an attempt to prevent
void displayPrintUpper(bool fc = false);
void displayPrintLower();

void displayMessageF(int id, ...);
void displayMessage2F(int id, ...);
void displayWarning2F(int id, ...);

void displayStateF(int id, ...);

void displayProgressBar(int cur, int max0);

void displayDebugF(const char *format, ...);



#endif
