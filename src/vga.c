/******************************************************************************
*                                                                             *
*   Module:     vga.c                                                         *
*                                                                             *
*   Date:       11/01/00                                                      *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************

    Module Description:

        VGA text output driver

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 11/01/00   Original                                             Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include "clib.h"                       // Include C library header file

#include "intel.h"                      // Include Intel defines

#include "i386.h"                       // Include assembly code

#include "ice.h"                        // Include global structures

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

// Define text buffer to store underlying VGA text when debugger pops up

static WORD textbuf[ 80 * 25 ];

#define HOST_MEMORY     0xB8000

typedef struct
{
    BYTE CRTC[0x19];                    // CRTC Registers
    
} TVGA;

static TVGA vga;

static const crtc_enable[0x19] = {
    1, 1, 1, 1, 1, 1, 1, 0, 
    0, 0, 1, 1, 1, 1, 1, 1, 
    0, 0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0
};

/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

static void SaveBackground(void);
static void RestoreBackground(void);

/******************************************************************************
*                                                                             *
*   void VgaInit(void)                                                        *
*                                                                             *
*******************************************************************************
*
*   Initializes VGA output driver
*
******************************************************************************/
void VgaInit(TVIDEO *video)
{
    video->SaveBackground = SaveBackground;
    video->RestoreBackground = RestoreBackground;
}    


static BYTE ReadCRTC(BYTE index)
{
    outp(0x3D4, index);
    return( inp(0x3D5) );
}    

static void WriteCRTC( BYTE index, BYTE value )
{
    outp(0x3D4, index);
    outp(0x3D5, value);    
}    

/******************************************************************************
*                                                                             *
*   static void SaveBackground(void)                                          *
*                                                                             *
*******************************************************************************
*
*   Saves the VGA text screen background and prepares driver for displaying
*
******************************************************************************/
static void SaveBackground(void)
{
    BYTE index;

    WriteCRTC(0xC, 18);
    WriteCRTC(0xD, 24);

    for( index=0; index<0x19; index++)
    {
        if( crtc_enable[index] )
        {
            vga.CRTC[index] = ReadCRTC(index);
            printk("<1> %02X = %02X\n", index, vga.CRTC[index] );
        }
    }

    printk("<1> Start address: %02X%02X\n", vga.CRTC[0xC], vga.CRTC[0xD] );

    WriteCRTC(0xD, 13);
}   
 

/******************************************************************************
*                                                                             *
*   static void RestoreBackground(void)                                       *
*                                                                             *
*******************************************************************************
*
*   Restores VGA text screen
*
******************************************************************************/
static void RestoreBackground(void)
{
    int i;

    printk("<1>VGA display subsystem:\n");
}   
 
