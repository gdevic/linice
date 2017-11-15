/******************************************************************************
*                                                                             *
*   Module:     mouse.c                                                       *
*                                                                             *
*   Date:       09/11/00                                                      *
*                                                                             *
*   Copyright (c) 1996-2001 Goran Devic                                       *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*   This source code and produced executable is copyrighted by Goran Devic.   *
*   This source, portions or complete, and its derivatives can not be given,  *
*   copied, or distributed by any means without explicit written permission   *
*   of the copyright owner. All other rights, including intellectual          *
*   property rights, are implicitly reserved. There is no guarantee of any    *
*   kind that this software would perform, and nobody is liable for the       *
*   consequences of running it. Use at your own risk.                         *
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
    pOut->mouse(mouseX / sens, mouseY / sens);
}


