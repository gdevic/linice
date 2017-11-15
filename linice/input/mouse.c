/******************************************************************************
*                                                                             *
*   Module:     mouse.c                                                       *
*                                                                             *
*   Date:       09/11/00                                                      *
*                                                                             *
*   Copyright (c) 1996-2005 Goran Devic                                       *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*   This program is free software; you can redistribute it and/or modify      *
*   it under the terms of the GNU General Public License as published by      *
*   the Free Software Foundation; either version 2 of the License, or         *
*   (at your option) any later version.                                       *
*                                                                             *
*   This program is distributed in the hope that it will be useful,           *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of            *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
*   GNU General Public License for more details.                              *
*                                                                             *
*   You should have received a copy of the GNU General Public License         *
*   along with this program; if not, write to the Free Software               *
*   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA   *
*                                                                             *
*******************************************************************************

    Module Description:

          This module contains the low-level PS/2 mouse input handler code.

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 09/11/00   Original                                             Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include "module-header.h"              // Versatile module header file

#include "clib.h"                       // Include C library header file
#include "ice.h"                        // Include main debugger structures
#include "ibm-pc.h"                     // Include hardware defines


/******************************************************************************
*                                                                             *
*   Global Variables                                                          *
*                                                                             *
******************************************************************************/

/******************************************************************************
*                                                                             *
*   Local Defines, Variables and Macros                                       *
*                                                                             *
******************************************************************************/

static int mouseX=0, mouseY=0;          // Mouse coordinates
static int sens = 8;                    // Mouse sensitivity

/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

/******************************************************************************
*                                                                             *
*   void MouseHandler(PTMPACKET pPacket)                                      *
*                                                                             *
*******************************************************************************
*
*   This handler is used when the debugger has control.
*
*   Where:
*       pPacket is internal mouse packet
*
******************************************************************************/
void MouseHandler(PTMPACKET pPacket)
{
    mouseX += pPacket->Xd;
    mouseY -= pPacket->Yd;

    //dprint("%02X %02X %d %d \n", mouseX, mouseY, pPacket->Xd, pPacket->Yd);

    if( mouseX < 0 )
        mouseX = 0;
    else
    if( mouseX > (pOut->sizeX-1) * sens )
        mouseX = (pOut->sizeX-1) * sens;

    if( mouseY < 0 )
        mouseY = 0;
    else
    if( mouseY > (pOut->sizeY-1) * sens )
        mouseY = (pOut->sizeY-1) * sens;

    // Call the function to display mouse

    // TODO: Revise whole mouse interface

//  pOut->mouse(mouseX / sens, mouseY / sens);
}


