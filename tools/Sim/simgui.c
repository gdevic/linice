/******************************************************************************
*                                                                             *
*   Module:     Simgui.c                                                      *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       06/08/97                                                      *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************

    Module Description:

        This is the main module for the simulator shell (GUI portion)

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     REV   DESCRIPTION OF CHANGES                         AUTHOR      *
* --------   ----  ---------------------------------------------  ----------- *
* 06/08/97   1.00  Original                                       Goran Devic *
* --------   ----  ---------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include <windows.h>                    // Include Windows support
#include <stdio.h>                      // Include standard I/O header file
#include <malloc.h>                     // Include memory operation defines
#include <commctrl.h>                   // Include common controls
#include <stdarg.h>                     // Include variable arguments file

#include "Globals.h"                    // Include global definitions

#include "resource.h"                   // Include its own resource definitions

#include "sim.h"                        // Include sim header file

/******************************************************************************
*                                                                             *
*   Global Variables                                                          *
*                                                                             *
******************************************************************************/

char * sSimClass   = SIM_CLASS;
char * sSimCaption = SIM_CAPTION;
char * sSimMenu    = SIM_MENU_NAME;

HWND        hSim;                       // Sim window handle
HWND        hDebugDlg;                  // Debug window handle
HWND        hList;                      // Handle of the list item in the debug window
HANDLE      SimInst;                    // Sim program instance handle

extern TREGS Regs;                      // Virtual CPU register structure

/******************************************************************************
*                                                                             *
*   Local Defines, Variables and Macros                                       *
*                                                                             *
******************************************************************************/

static HMENU hMenu;                     // Handle of a menu
static HICON hIcon;                     // Handle of Sim icon
LOGFONT font;                           // Font type
HFONT hFont;                            // Font handle
#define MAX_YLINES          60          // Maximum number of lines
DWORD yLines = 25;                      // Number of lines in Y direction

#define VGA_TIMER           1           // Timer number for VGA refresh
int nTimer;                             // Timer handler


/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

long WindowProc( HWND, unsigned, UINT, LONG );
long DebugProc( HWND, unsigned, UINT, LONG );
LRESULT CALLBACK SetCPURegsWnd(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

/******************************************************************************
*                                                                             *
*   void EchoError()                                                          *
*                                                                             *
*******************************************************************************
*
*   This function posts a message box with the LastError code
*
******************************************************************************/
void EchoError()
{
    char sError[32];

    sprintf( sError, "Error %d", GetLastError() );

    MessageBox( NULL,
        sError,
        sSimCaption,
        MB_ICONSTOP | MB_OK );
}


/******************************************************************************
*                                                                             *
*   WinMain                                                                   *
*                                                                             *
*******************************************************************************
*
*   Initialization and message loop
*
******************************************************************************/
int APIENTRY WinMain( HINSTANCE this_inst, HINSTANCE prev_inst, LPSTR cmdline, int cmdshow )
{
    MSG         msg;
    WNDCLASS    wc;
    RECT        rc;                     // Generic window rectangle


    // There can be only one instance of this application in the system.
    // Look if the window with our class and caption is registered

    if( FindWindow( sSimClass, sSimCaption ) != NULL )
    {
        // Found a previous instance, inform the user and exit

        MessageBox( NULL,
            NAME" already running!",
            sSimCaption,
            MB_ICONSTOP | MB_OK );

        return( 0 );
    }

    SimInst = this_inst;


    // Init the sim state
    SimInit();


    hIcon = LoadIcon( this_inst, SIM_ICON_NAME );

    // Register window class for the application

    wc.style         = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = (LPVOID) WindowProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = this_inst;
    wc.hIcon         = hIcon;
    wc.hCursor       = LoadCursor( NULL, IDC_ARROW );
    wc.hbrBackground = NULL;
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = sSimClass;

    if( RegisterClass( &wc ) == FALSE )
    {
        // Cannot register class, return

        EchoError();
        return( FALSE );
    }


    // Load the menu

    hMenu = LoadMenu( SimInst, sSimMenu );

    if( hMenu == NULL )
    {
        // Cannot load menu

        EchoError();
        return( FALSE );
    }

    // Make sure that the common control dll is loaded

    InitCommonControls();

    // Create working font
    memset(&font, 0, sizeof(font));

    font.lfHeight = -15;
    font.lfWeight = 400;
    font.lfOutPrecision = 3;
    font.lfClipPrecision = 2;
    font.lfQuality = DEFAULT_QUALITY;
    font.lfPitchAndFamily = 0x31;
    strcpy(font.lfFaceName, "Fixedsys");

    hFont = CreateFontIndirect(&font);


    //-------------------------------------------------------------------------
    // Create the main window
    //-------------------------------------------------------------------------
    hSim = CreateWindow(sSimClass, sSimCaption, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        80*8 + GetSystemMetrics(SM_CXSIZEFRAME) * 2,
        -font.lfHeight * yLines + GetSystemMetrics(SM_CYSIZEFRAME) * 2 + GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYMENU),
        NULL, NULL, SimInst, NULL);

    if( hSim == NULL )
    {
        // Unable to create window.

        EchoError();

        return( FALSE );
    }


    SetMenu(hSim, hMenu);

    // Display window

    ShowWindow( hSim, cmdshow );
    UpdateWindow( hSim );

    //-------------------------------------------------------------------------
    // Create a Debug window and show it to the right of the main window
    //-------------------------------------------------------------------------
    GetWindowRect(hSim, &rc);
    hDebugDlg = CreateDialog(SimInst, (LPCTSTR)IDD_DEBUG, hSim, (DLGPROC)DebugProc);
    MoveWindow(hDebugDlg, rc.right, rc.top, 500, rc.bottom-rc.top, TRUE);
    ShowWindow(hDebugDlg, TRUE);


    // Set VGA update timer
    nTimer = SetTimer(hSim, VGA_TIMER, 100, NULL);

    // Message pump...

    while( GetMessage( &msg, NULL, 0, 0 ) )
    {
        TranslateMessage( &msg );
        DispatchMessage( &msg );
    }

    return( msg.wParam );
}


/******************************************************************************
*                                                                             *
*   void WinPaint(HWND hWnd)
*                                                                             *
*******************************************************************************
*
*   Paints the main window with the VGA buffer in the sim allocated memory
*
******************************************************************************/
void WinPaint(HWND hWnd)
{
    //  R     G     B
    static const DWORD vga_fore[16] = {
    RGB( 0x00, 0x00, 0x00 ),                   // Color 0  BLACK
    RGB( 0x00, 0x00, 0xFF ),                   // Color 1  BLUE
    RGB( 0x00, 0xFF, 0x00 ),                   // Color 2  GREEN
    RGB( 0x00, 0xFF, 0xFF ),                   // Color 3  CYAN
    RGB( 0xFF, 0x00, 0x00 ),                   // Color 4  RED
    RGB( 0x80, 0x00, 0x80 ),                   // Color 5  MAGENTA
    RGB( 0xC0, 0xC0, 0xC0 ),                   // Color 6  BROWN
    RGB( 0xFF, 0xFF, 0xFF ),                   // Color 7  GREY
    RGB( 0x40, 0x40, 0x40 ),                   // Color 8  DARK GREY
    RGB( 0x00, 0x00, 0x80 ),                   // Color 9  LIGHT BLUE
    RGB( 0x00, 0x80, 0x00 ),                   // Color 10 LIGHT GREEN
    RGB( 0x00, 0x80, 0x80 ),                   // Color 11 LIGHT CYAN
    RGB( 0x80, 0x00, 0x00 ),                   // Color 12 LIGHT RED
    RGB( 0x40, 0x00, 0x40 ),                   // Color 13 LIGHT MAGENTA
    RGB( 0xFF, 0xFF, 0x00 ),                   // Color 14 YELLOW
    RGB( 0xFF, 0xFF, 0xFF )                    // Color 15 WHITE
    };

    static const DWORD vga_back[16] = {
    RGB( 0x00, 0x00, 0x00 ),                   // Color 0  BLACK
    RGB( 0x00, 0x00, 0xFF ),                   // Color 1  BLUE
    RGB( 0x00, 0xFF, 0x00 ),                   // Color 2  GREEN
    RGB( 0x00, 0xFF, 0xFF ),                   // Color 3  CYAN
    RGB( 0xFF, 0x00, 0x00 ),                   // Color 4  RED
    RGB( 0x80, 0x00, 0x80 ),                   // Color 5  MAGENTA
    RGB( 0xC0, 0xC0, 0xC0 ),                   // Color 6  BROWN
    RGB( 0x80, 0x80, 0x80 ),                   // Color 7  GREY
    };

    PAINTSTRUCT ps;
    HDC hdc;
    int x, y;
    BYTE bAttr;
    static char Char;
    static int counter = 0;

    hdc = BeginPaint(hWnd, &ps);
    SelectObject(hdc, hFont);

    for(y=0; y<(int)yLines; y++)
    {
        for(x=0; x<80; x++)
        {
            Char = *(pPageOffset + 0xB8000 + x*2 + y*160);
            bAttr = *(BYTE *)(pPageOffset + 0xB8000 + x*2 + y*160 + 1);

            // This is a decorative touch
            if(Char<32) Char = '-';

            SetTextColor(hdc, vga_fore[bAttr & 0xF]);
            SetBkColor(hdc, vga_back[(bAttr >> 4) & 0x7]);
            TextOut(hdc, x * 8, y * -font.lfHeight, &Char, 1);
        }
#if 0
        // Overlay the logo, why not?  :)
#define LOGO    "LINICE SIM (c) 2003 Goran Devic"
        SetTextColor(hdc, RGB( 0xFF, 0xFF, 0xFF ));
        SetBkColor(hdc, RGB( 0x00, 0x00, 0x00 ));
        TextOut(hdc, (80 - sizeof(LOGO))*8, (yLines-2) * -font.lfHeight, LOGO, sizeof(LOGO)-1);
#endif
    }

    // Get the cursor blinking
    counter++;
    SetTextColor(hdc, RGB( 0xFF, 0xFF, 0xFF ));
    SetBkColor(hdc, RGB( 0x00, 0x00, 0x00 ));
    x = (int)ReadCRTC(0xE) * 256 + ReadCRTC(0xF);
    if(counter&2)
        TextOut(hdc, (x % 80)*8, (x/80) * -font.lfHeight - 2, "_", 1);
    else
        TextOut(hdc, (x % 80)*8, (x/80) * -font.lfHeight, " ", 1);

//    ReleaseDC(hWnd, hdc); 
    EndPaint(hWnd, &ps);
}


/******************************************************************************
*                                                                             *
*   WindowProc                                                                *
*                                                                             *
*******************************************************************************
*
*   The main window procedure
*
******************************************************************************/
LONG WindowProc( HWND hWnd, unsigned Msg, UINT wParam, LONG lParam )
{
    // Switch on a windows message

    if( Msg==WM_COMMAND )
    {
        // Switch on a command messages

        switch( LOWORD( wParam ) )
        {
        case IDM_INSTALL:
                SimInstall();
            break;

        case IDM_UNINSTALL:
                SimUninstall();
            break;

        case IDM_LOAD_SYMBOL_FILE:
                SimLoadSymbolFile();
            break;

        case IDM_UNLOAD_SYMBOL_FILE:
                SimUnloadSymbolFile();
            break;

        case IDM_LOAD_CAPTURE:
                SimLoadCapture();
            break;

        case IDM_INT1:
                SimINT1();
            break;

        case IDM_SETCPUREGS:
                DialogBox(SimInst, (LPCTSTR)IDD_SETCPUREGS, hWnd, (DLGPROC)SetCPURegsWnd);
            break;

        case MENU_EXIT:
                // Send a message to itself to exit

                PostMessage( hSim, WM_CLOSE, 0, 0 );

            break;
        };
    }

    // Switch on rest of windows messages

    switch( Msg )
    {
       // --------------------------------------------------------------------
        case WM_CLOSE:
                DestroyWindow( hSim );
#if 0
                // User tries to abort the program.  Ask him if he really
                // wants to do so

                result = MessageBox(
                    hSim,
                    "Really quit Sim?",
                    sSimCaption,
                    MB_APPLMODAL | MB_YESNO | MB_ICONQUESTION );

                if( result==IDYES )
                {
                    // Yes - really quit

                    DestroyWindow( hSim );
                }
#endif
            break;

        // --------------------------------------------------------------------
        case WM_DESTROY:
                PostQuitMessage( 0 );
            break;

        // --------------------------------------------------------------------
//        case WM_CHAR:
//                SimKey(wParam, lParam);
//            break;

        case WM_KEYUP:
        case WM_KEYDOWN:
            // A key has beed pressed or released

            SimKey(wParam, lParam);

            // We are processing this message, so return 0
            return 0;

        // --------------------------------------------------------------------
        case WM_TIMER:
            switch(wParam)
            {
            case VGA_TIMER:
                InvalidateRect(hWnd, NULL, TRUE);
                break;
            }
            break;

        // --------------------------------------------------------------------
        case WM_PAINT:
                WinPaint(hWnd);
            break;

        // --------------------------------------------------------------------
        default:
            return( DefWindowProc( hWnd, Msg, wParam, lParam ) );
    }

    // By default, return 0 for all messages that were handled but not
    // explicitly terminated

    return( 0L );
}

void InterruptPoll(void)
{
    Sleep(50);
}


/******************************************************************************
*                                                                             *
*   long DebugProc( HWND, unsigned, UINT, LONG )
*                                                                             *
*******************************************************************************
*
*   The debug window procedure
*
******************************************************************************/
long DebugProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message)
    {
    case WM_INITDIALOG:
        // Debug window is being initialized
        hList = GetDlgItem(hDlg, IDC_DEBUG_LIST);

        SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)(LPCTSTR) "Debug output: printk()");

        return TRUE;

    case WM_SIZE:
        // Resize the window: resize child items and redraw them
        MoveWindow(hList, 12, 12, LOWORD(lParam) - 24, HIWORD(lParam)-24, TRUE);
        ListView_RedrawItems(hList, 0, ListView_GetItemCount(hList));

        break;
    }

    return FALSE;
}

/******************************************************************************
*                                                                             *
*   SetCPURegsWnd(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
*                                                                             *
*******************************************************************************
*   Set CPU registers.
******************************************************************************/
LRESULT CALLBACK SetCPURegsWnd(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    char buf[32];

    switch (message)
    {
    case WM_INITDIALOG:
        {
            // Fill in the current file name and the state of autoincrement

            sprintf(buf, "%08X", Regs.eax);  SetDlgItemText(hDlg, IDC_EAX, buf);
            sprintf(buf, "%08X", Regs.ebx);  SetDlgItemText(hDlg, IDC_EBX, buf);
            sprintf(buf, "%08X", Regs.ecx);  SetDlgItemText(hDlg, IDC_ECX, buf);
            sprintf(buf, "%08X", Regs.edx);  SetDlgItemText(hDlg, IDC_EDX, buf);
            sprintf(buf, "%08X", Regs.esi);  SetDlgItemText(hDlg, IDC_ESI, buf);
            sprintf(buf, "%08X", Regs.edi);  SetDlgItemText(hDlg, IDC_EDI, buf);
            sprintf(buf, "%08X", Regs.esp);  SetDlgItemText(hDlg, IDC_ESP, buf);
            sprintf(buf, "%08X", Regs.ebp);  SetDlgItemText(hDlg, IDC_EBP, buf);
            sprintf(buf, "%08X", Regs.eip);  SetDlgItemText(hDlg, IDC_EIP, buf);

            sprintf(buf, "%04X", Regs.cs);  SetDlgItemText(hDlg, IDC_CS, buf);
            sprintf(buf, "%04X", Regs.ds);  SetDlgItemText(hDlg, IDC_DS, buf);
            sprintf(buf, "%04X", Regs.es);  SetDlgItemText(hDlg, IDC_ES, buf);
            sprintf(buf, "%04X", Regs.fs);  SetDlgItemText(hDlg, IDC_FS, buf);
            sprintf(buf, "%04X", Regs.gs);  SetDlgItemText(hDlg, IDC_GS, buf);
            sprintf(buf, "%04X", Regs.ss);  SetDlgItemText(hDlg, IDC_SS, buf);

            return TRUE;
        }

    case WM_COMMAND:
        switch( LOWORD(wParam) )
        {
            case IDOK:
                // Store away new selection (file name and the autoincrement check)

                GetDlgItemText(hDlg, IDC_EAX, buf, sizeof(buf)); sscanf(buf,"%X", &Regs.eax);
                GetDlgItemText(hDlg, IDC_EBX, buf, sizeof(buf)); sscanf(buf,"%X", &Regs.ebx);
                GetDlgItemText(hDlg, IDC_ECX, buf, sizeof(buf)); sscanf(buf,"%X", &Regs.ecx);
                GetDlgItemText(hDlg, IDC_EDX, buf, sizeof(buf)); sscanf(buf,"%X", &Regs.edx);
                GetDlgItemText(hDlg, IDC_ESI, buf, sizeof(buf)); sscanf(buf,"%X", &Regs.esi);
                GetDlgItemText(hDlg, IDC_EDI, buf, sizeof(buf)); sscanf(buf,"%X", &Regs.edi);
                GetDlgItemText(hDlg, IDC_ESP, buf, sizeof(buf)); sscanf(buf,"%X", &Regs.esp);
                GetDlgItemText(hDlg, IDC_EBP, buf, sizeof(buf)); sscanf(buf,"%X", &Regs.ebp);
                GetDlgItemText(hDlg, IDC_EIP, buf, sizeof(buf)); sscanf(buf,"%X", &Regs.eip);

                GetDlgItemText(hDlg, IDC_CS, buf, sizeof(buf)); sscanf(buf,"%X", &Regs.cs);
                GetDlgItemText(hDlg, IDC_DS, buf, sizeof(buf)); sscanf(buf,"%X", &Regs.ds);
                GetDlgItemText(hDlg, IDC_ES, buf, sizeof(buf)); sscanf(buf,"%X", &Regs.es);
                GetDlgItemText(hDlg, IDC_FS, buf, sizeof(buf)); sscanf(buf,"%X", &Regs.fs);
                GetDlgItemText(hDlg, IDC_GS, buf, sizeof(buf)); sscanf(buf,"%X", &Regs.gs);
                GetDlgItemText(hDlg, IDC_SS, buf, sizeof(buf)); sscanf(buf,"%X", &Regs.ss);

                // NOTE - This case falls-through ...

            case IDCANCEL:
                // Cancel the dialog box
                EndDialog(hDlg, LOWORD(wParam));
                return TRUE;
        }
    }

    return FALSE;
}


/******************************************************************************
*                                                                             *
*   int printk(char *format, ...)
*                                                                             *
*******************************************************************************
*   This is the kernel debug print function
******************************************************************************/
int printk(char *format, ...)
{
    va_list arg;                        // Variable argument list pointer
    char *p;                            // Generic pointer
    int i;                              // Standard printf return value
    static char printbuf[256];          // We print into this buffer
#if 1
    va_start( arg, format );
    i = vsprintf( printbuf, format, &arg );
    va_end( arg );

    // Remove 0xA and 0xD codes from the message
    while(p=strchr(printbuf, 0xA)) *p=' ';
    while(p=strchr(printbuf, 0xD)) *p=' ';

    SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)(LPCTSTR) printbuf);

    // Make sure that the last line is visible

    SendMessage(hList, LB_SETTOPINDEX, SendMessage(hList, LB_GETCOUNT, 0, 0)-1, 0);
#endif
    return i;
}


void Message(char *msg)
{
    MessageBox(hSim, msg, "Sim message", MB_ICONHAND);
}
