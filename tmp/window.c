/******************************************************************************
*                                                                             *
*   Module:     window.c                                                      *
*                                                                             *
*   Date:       11/11/00                                                      *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************

    Module Description:

        Windows handling

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 11/11/00   Original                                             Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include "clib.h"                       // Include C library header file

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

typedef void (*TGetContentFunction)(char *sLine, DWORD nLine);

typedef struct
{
    DWORD   startingY;                  // Starting Y coordinate
    DWORD   sizeY;                      // Vertical size in lines
    
    TGetContentFunction GetContent;     // Get window content function

} TWindow;


#define WND_REGISTERS       0           // Registers window
#define WND_LOCALS          1           // Locals window
#define WND_WATCH           2           // Watch window
#define WND_DATA            3           // Data window
#define WND_CODE            4           // Code window
#define WND_STACK           5           // Stack window
#define WND_COMMAND         6           // Command window
#define WND_HELP            7           // Help line window

#define MAX_WND             (WND_HELP+1)

TWindow Window[ MAX_WND ];

char sLine[256];

/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

void InitWindow(void)
{
    memset(&Window, 0, sizeof(Window));
}    


/******************************************************************************
*                                                                             *
*   void RedrawScreen(void)                                                   *
*                                                                             *
*******************************************************************************
*
*   Redraws debugger screen
*
******************************************************************************/
void RedrawScreen(void)
{
    int i;
    DWORD y, loopY;

    pWnd = pWindow;
    y = 0;

    for( i=0; i<MAX_WND; i++)
    {
        if( Window[i].GetContent != NULL )
        {
            // Window exists, loop the Y coordinates and print them out

            for( loopY=0; loopY<Window[i].sizeY; loopY++, y++)
            {
                Window[i].GetContent( sLine, loopY );
                pTextOut->PrintLine( y, sLine );
            }
        }
    }
}    


