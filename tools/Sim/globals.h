/******************************************************************************
*                                                                             *
*   Module:     Globals.h                                                     *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       06/08/97                                                      *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************

    Module Description:

        This is a header file for the Linice simulation environment

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     REV   DESCRIPTION OF CHANGES                         AUTHOR      *
* --------   ----  ---------------------------------------------  ----------- *
* 06/08/97   1.00  Original                                       Goran Devic *
* --------   ----  ---------------------------------------------  ----------- *
*******************************************************************************
*   Important Defines                                                         *
******************************************************************************/
#ifndef _GLOBALS_H_
#define _GLOBALS_H_


/******************************************************************************
*                                                                             *
*   Include Files                                                             *
*                                                                             *
******************************************************************************/

/******************************************************************************
*                                                                             *
*   Global Defines, Variables and Macros                                      *
*                                                                             *
******************************************************************************/

#define NAME                "Linice Simulation Environment"
#define VERSION             "Release 1.01"
#define ABOUT               "Sim Copyright 2003 Goran Devic"
#define SIM_CLASS           "SimClass"  // Class name of the Sim
#define SIM_CAPTION         "Sim"       // Caption on the main window
#define SIM_MENU_NAME       "Menu"      // Resource name of the menu
#define SIM_ICON_NAME       "Icon"      // Resource icon name

// Menu items

#define MENU_ABOUT          100
#define MENU_EXIT           101

extern char   * sSimCaption;            // Program caption text

extern HWND     hSim;                   // Sim window handle
extern HANDLE   SimInst;                // Sim program instance handle

/******************************************************************************
*                                                                             *
*   External Functions                                                        *
*                                                                             *
******************************************************************************/

extern BOOL OptInstall(char *pSystemMap);


#endif  //  _GLOBALS_H_
