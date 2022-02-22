//-----------------------------LICENSE NOTICE------------------------------------
//  This file is part of CPCtelera: An Amstrad CPC Game Engine
//  Copyright (C) 2015 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU Lesser General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//------------------------------------------------------------------------------

#include <cpctelera.h>
#include "RSB1.h"
#include "RSB2.h"

//PALETTE={18 14 12 15 10 16 6 13 24 9 19 25 20 3 2 7}

const u8 PAL[16] =
{
    0x12, 0x0E, 0x0C, 0x0F, 0x0A, 0x10, 0x06, 0x0D,
    0x18, 0x09, 0x13, 0x19, 0x14, 0x03, 0x02, 0x07
};

void main(void) 
{
    cpct_disableFirmware();
    cpct_setVideoMode(0);
    cpct_fw2hw(PAL, 16);
    cpct_setPalette(PAL, 16);
    cpct_setBorder(PAL[9]);
    cpct_clearScreen(cpct_px2byteM0(1,1));
    cpct_drawSprite(g_RSB1_0, CPCT_VMEM_START, 20, 200);
    cpct_drawSprite(g_RSB1_1, CPCT_VMEM_START + 20, 20, 200);
    cpct_drawSprite(g_RSB2_0, CPCT_VMEM_START + 40, 20, 200);
    cpct_drawSprite(g_RSB2_1, CPCT_VMEM_START + 60, 20, 200);
    while (1);
}
