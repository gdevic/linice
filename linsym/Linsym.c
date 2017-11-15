/******************************************************************************
*                                                                             *
*   Module:     Linsym.c                                                      *
*                                                                             *
*   Date:       09/05/00                                                      *
*                                                                             *
*   Copyright (c) 2000-2005 Goran Devic                                       *
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

        This module contains main functions for Linice loader/translator.


*******************************************************************************
*                                                                             *
*   Major changes:                                                            *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 09/05/00   Initial version                                      Goran Devic *
* 03/20/04   Renamed from loader.c into Linsym.c                  Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include <errno.h>                      // Include errno
#include <stdlib.h>                     // Include standard library
#include <string.h>                     // Include strings header file
#include <sys/stat.h>                   // Include file operations
#include <fcntl.h>                      // Include file control file
#include <stdio.h>                      // Include standard io file

#ifndef WIN32
#include <unistd.h>                     // Include standard UNIX header file
#include <sys/wait.h>                   // Include waitpid
#endif // WIN32

#include "ice-version.h"                // Include version file
#include "ice-ioctl.h"                  // Include shared header file
#include "loader.h"                     // Include global protos

/******************************************************************************
*                                                                             *
*   Global Variables                                                          *
*                                                                             *
******************************************************************************/

extern char **environ;

#define PATH_MAX    256
static char sOutputFile[PATH_MAX];      // Contains the name of the output file

char *pTranslate = NULL;                // File to translate
char *pOutput    = NULL;                // Default output file is <input_file>.sym
char *pPathSubst = NULL;                // Source path substitution string
char *pSym       = NULL;                // Default symbol table to load/unload
char *pLogfile   = "linice.log";        // Default logfile name
char *pSystemMap = NULL;                // User supplied System.map file
char *pCheck     = NULL;                // Check symbol file
unsigned int opt = 0;                   // Various option flags
int nVerbose     = 0;                   // Verbose level

/******************************************************************************
*                                                                             *
*   Local Defines, Variables and Macros                                       *
*                                                                             *
******************************************************************************/

int strcmpi( const char *s1, const char *s2 );
int tolower(int);

extern BOOL OptInstall(char *pSystemMap);
extern void OptUninstall();
extern void OptAddSymbolTable(char *sName);
extern void OptRemoveSymbolTable(char *sName);
extern void OptTranslate(char *pathOut, char *pathIn, char *pPathSubst);
extern void OptLogHistory(void);
extern void OptCheck(char *pFile);

/******************************************************************************
*                                                                             *
*   int system2(char *command)                                                *
*                                                                             *
*******************************************************************************
*
*   Simulates system() call with suid where calling the system() is not
*   advisable.
*
*   Where:
*       command is the command to be exec'd
*
*   Returns:
*       -1 if there was an error executing the command
*       >=0 (status) of executing the command
*
******************************************************************************/
int system2(char *command)
{
#ifndef WIN32
    int pid, status;
    char *argv[4];                      // Argument string pointers
    static char sExec[256];             // Line that we will be executing

    if( command==NULL )
        return( -1 );

    pid = fork();                       // Create a secondary process that will execute a line
    if( pid==-1 )
        return( -1 );

    if( pid==0 )
    {
        // Form the line to be executed.. Append pipe to dev/null if no verbose was set

        strcpy(sExec, command);

        if( !(opt & OPT_VERBOSE) )
        {
            // Ok, this is a little kludge worth explaining: if a line already contains
            // indirection > we can't redirect it further to dev null
            if( !strchr(sExec, '>') )
                strcat(sExec, " >& /dev/null");
        }

        argv[0] = "sh";
        argv[1] = "-c";
        argv[2] = sExec;
        argv[3] = 0;

        execve("/bin/sh", argv, environ);
        exit(127);
    }

    do
    {
        if( waitpid(pid, &status, 0)==-1 )
        {
            if( errno!=EINTR )
                return( -1 );
        }
        else
            return( status );
    } while( 1 );
#endif // WINDOWS
    return 0;
}


/******************************************************************************
*                                                                             *
*   void OptHelp(BOOL fLong)                                                  *
*                                                                             *
*******************************************************************************
*
*   prints the command options, with short or long (TRUE) description.
*
******************************************************************************/
void OptHelp(BOOL fLong)
{
    if( !fLong )
    {
        printf("Use LINSYM -h or --help for extended help\n\n");

        printf("               www.linice.com\n");
        printf("Send bug reports to:      bugs@linice.com\n");
        printf("Send feature requests to: features@linice.com\n");
        printf("Send comments to:         author@linice.com\n\n");
    }
    else
    {
        printf("  -i, --install                       Install Linice debugger module\n");
        printf("       Example: --install\n");

        printf("  -m, --map <System.map>              Specify alternate System.map file\n");
        printf("       Example: --map /boot/System.map\n");

        printf("  -x, --uninstall                     Uninstall Linice debugger module\n");
        printf("       Example: --uninstall\n");

        printf("  -t, --translate <module>            Translate and create a debug symbol file\n");
        printf("       Example: --translate MyProgram\n");

        printf("  -o, --output <filename>             Specify alternate name for translation\n");
        printf("       Example: --output MyProgram.sym\n");

        printf("  -p, --path <orig-path>:<new-path>   Specify source code path substitution\n");
        printf("       Example: --path /myproject/source:/mnt/source\n");

        printf("  -s, --sym <sym>[;<sym>]             Load symbol file(s)\n");
        printf("       Example: --sym MyProgram.sym\n");

        printf("  -u, --unload <sym>[;<sym>]          Unload symbol table(s)\n");
        printf("       Example: --unload MyProgram\n");

        printf("  -l, --logfile [<filename>][,append] Save the Linice history buffer\n");
        printf("       Example: --logfile Mylog.log,append\n");

        printf("  -v, --verbose {0-3}                 Verbose level (0=silent)\n");
        printf("       Example: --verbose 3\n");

        printf("\n");
    }

    exit(0);
}


/******************************************************************************
*                                                                             *
*   int init_module(void)                                                     *
*                                                                             *
*******************************************************************************
*
*   Linice module loader and symbol translator
*
******************************************************************************/
int main(int argn, char *argp[])
{
    int i;
    char *ptr;                          // Temporary pointer variable to use

    // Print the basic banner
    printf("\nLinice Debugger Symbol Translator/Loader Version %d.%d\n", LINSYMVER >> 8, LINSYMVER & 0xFF);
    printf("Linice and Linsym (C) 2005 by Goran Devic. All Rights Reserved.\n\n");

    // If there were no arguments, just print help and exit (help exits)
    if( argn==1 )
        OptHelp(FALSE);

    //------------------------------------------------------------------------
    // Parse command line arguments and assign opt bits and strings
    //------------------------------------------------------------------------
    for( i=1; i<argn; i++)
    {
        if( !strcmpi(argp[i], "--translate") || !strcmpi(argp[i], "-t") )
        {
            // --translate <file>
            opt |= OPT_TRANSLATE;

            if( i+1<argn )
            {
                i++;
                pTranslate = argp[i];

                VERBOSE1 printf("TRANSLATE %s\n", pTranslate);
            }
            else
                opt |= OPT_HELP;
        }
        else
        if( !strcmpi(argp[i], "--path") || !strcmpi(argp[i], "-p") )
        {
            // --path <prefix-path>:<new-path>
            opt |= OPT_PATH;

            if( i+1<argn )
            {
                i++;
                pPathSubst = argp[i];

                VERBOSE1 printf("PATH %s\n", pPathSubst);
            }
            else
                opt |= OPT_HELP;
        }
        else
        if( !strcmpi(argp[i], "--output") || !strcmpi(argp[i], "-o") )
        {
            // --output <output_file>
            opt |= OPT_OUTPUT;

            if( i+1<argn )
            {
                i++;
                pOutput = argp[i];

                VERBOSE1 printf("OUTPUT %s\n", pOutput);
            }
            else
                opt |= OPT_HELP;
        }
        else
        if( !strcmpi(argp[i], "--sym") || !strcmpi(argp[i], "-s") )
        {
            // --sym <symbols-to-load>[:<more-symbols>]
            opt |= OPT_SYM;

            if( i+1<argn )
            {
                i++;
                pSym = argp[i];

                VERBOSE1 printf("SYM %s\n", pSym);
            }
            else
                opt |= OPT_HELP;
        }
        else
        if( !strcmpi(argp[i], "--unload") || !strcmpi(argp[i], "-u") )
        {
            // --unload <symbols-to-unload>[:<more-symbols>]
            opt |= OPT_UNLOAD;

            if( i+1<argn )
            {
                i++;
                pSym = argp[i];

                VERBOSE1 printf("UNLOAD %s\n", pSym);
            }
            else
                opt |= OPT_HELP;
        }
        else
        if( !strcmpi(argp[i], "--install") || !strcmpi(argp[i], "-i") )
        {
            // --install  install debugger
            opt |= OPT_INSTALL;

            VERBOSE1 printf("INSTALL\n");
        }
        else
        if( !strcmpi(argp[i], "--map") || !strcmpi(argp[i], "-m") )
        {
            // --map <System.map>
            // We dont have option code in the 'opt' for map

            if( i+1<argn )
            {
                i++;
                pSystemMap = argp[i];

                VERBOSE1 printf("MAP %s\n", pSystemMap);
            }
            else
                opt |= OPT_HELP;
        }
        else
        if( !strcmpi(argp[i], "--uninstall") || !strcmpi(argp[i], "-x") )
        {
            // --uninstall  uninstalls debugger
            opt |= OPT_UNINSTALL;

            VERBOSE1 printf("INSTALL\n");
        }
        else
        if( !strcmpi(argp[i], "--logfile") || !strcmpi(argp[i], "-l") )
        {
            // --logfile                        - log to a default logfile
            // --logfile {log_file}             - specify a logfile
            // --logfile APPEND                 - append to a default logfile
            // --logfiled {logfile},APPEND      - append to a specified logfile
            opt |= OPT_LOGFILE;

            // Check if we have a new logfile specified
            if( i+1<argn )
            {
                i++;
                ptr = argp[i];

                // Lonely "append"
                if( !strcmpi(ptr, "append") )
                    opt |= OPT_LOGFILE_APPEND;
                else
                {
                    // We have a new file name
                    pLogfile = ptr;

                    // Find the end of the specified file name (it is either end of string
                    // or a comma for append sub option
                    if( strchr(ptr, ',') )
                    {
                        ptr = strchr(ptr, ',');
                        *ptr = 0;                   // Zero-terminate the log file name
                        ptr++;
                    }

                    // Check if we need to append instead of create/truncate existing logfile
                    if( !strcmpi(ptr, "append") )
                        opt |= OPT_LOGFILE_APPEND;
                }
            }

            VERBOSE1 printf("LOGFILE %s %s\n", pLogfile, (opt & OPT_LOGFILE_APPEND)? "APPEND":"");
        }
        else
        if( !strcmpi(argp[i], "--verbose") || !strcmpi(argp[i], "-v") )
        {
            // --verbose {0,1,2,3}   display more output information

            if( i+1<argn )
            {
                i++;
                ptr = argp[i];

                switch( *ptr )
                {
                    case '0':   opt &= ~OPT_VERBOSE;  break;     // 0 means no output
                    case '1':   nVerbose = 1;  break;
                    case '2':   nVerbose = 2;  break;
                    case '3':   nVerbose = 3;  break;
                    default:
                        opt |= OPT_HELP;
                }
                VERBOSE1 printf("VERBOSE %d\n", nVerbose);
            }
            else
                opt |= OPT_HELP;
        }
        else
        if( !strcmpi(argp[i], "--check") || !strcmpi(argp[i], "-c") )
        {
            // --check <file>
            opt |= OPT_CHECK;

            if( i+1<argn )
            {
                i++;
                pCheck = argp[i];

                VERBOSE1 printf("CHECK %s\n", pCheck);
            }
            else
                opt |= OPT_HELP;
        }
        else
        if( !strcmpi(argp[i], "--help") || !strcmpi(argp[i], "-h") )
        {
            // --help  display help information and exit
            opt |= OPT_HELP;
        }
        else
        {
            printf("Error - Unknown option: %s\n\n", argp[i]);

            // Unknown option? We should exit in that case, with some error code
            exit(1);
        }
    }

    //------------------------------------------------------------------------
    // Start executing command line options
    //------------------------------------------------------------------------

    if( opt & OPT_HELP )
    {
        // Print help and EXIT (you can't do anything if you want help :-)
        OptHelp(TRUE);
    }

    // If we need to load debugger, do it first
    if( opt & OPT_INSTALL )
    {
        // If installation failed, we need to exit
        if( OptInstall(pSystemMap)==FALSE )
            exit(-1);
    }

    // Now we need to do some adjustments:
    // If an output symbol file is not given, use input+".sym"
    if( opt & OPT_TRANSLATE )
    {
        // Form the "real" pointer to an output file string
        if( pOutput==NULL )
        {
            strcpy(sOutputFile, pTranslate);
            strcat(sOutputFile, ".sym");
        }
        else
            strcpy(sOutputFile, pOutput);

        // Translate the symbols
        OptTranslate(sOutputFile, pTranslate, pPathSubst);
    }

    // Load symbol(s) into debugger
    if( opt & OPT_SYM )
    {
        char *pDelim;
        do
        {
            pDelim = strchr(pSym, ':');
            if( pDelim ) *pDelim = 0;

            VERBOSE1 printf("Add symbol table %s\n", pSym);

            OptAddSymbolTable(pSym);

            pSym = pDelim + 1;
        }
        while( pDelim );
    }

    // Unload symbol table(s) from debugger
    if( opt & OPT_UNLOAD )
    {
        char *pDelim;
        do
        {
            pDelim = strchr(pSym, ':');
            if( pDelim ) *pDelim = 0;

            VERBOSE1 printf("Remove symbol table %s\n", pSym);

            OptRemoveSymbolTable(pSym);

            pSym = pDelim + 1;
        }
        while( pDelim );
    }

    // Get the history strings into a log file
    if( opt & OPT_LOGFILE )
    {
        OptLogHistory();
    }

    // If uninstall debugger is needed, do it last
    if( opt & OPT_UNINSTALL )
        OptUninstall();

    // Check symbol file option
    if( opt & OPT_CHECK )
        OptCheck(pCheck);

    return( 0 );
}

/******************************************************************************
*
*   int strnicmp( const char *s1, const char *s2, size_t n )
*
*      Compares two counted strings with case insensitivity.
*
*   Where:
*      s1 - pointer to string 1
*      s2 - pointer to string 2
*      n - maximum number of characters to compare
*
*   Returns:
*      An int that is greather than, equal to, or less than zero according to
*      the relative order of s1 and s2.
*
*      Example, if s1 > s2, return a positive value.
*
******************************************************************************/
int strnicmp( const char *s1, const char *s2, size_t n )
{
    if( n==0 ) return( 0 );
    while( 1 )
    {
        if (tolower(*s1) != tolower(*s2))
            return(tolower(*s1) - tolower(*s2));
        if (*s1 == 0 || --n == 0) return(0);
        s1++;
        s2++;
    }
}

/******************************************************************************
*
*   int tolower(int c)
*
*       Returns lowercased character.
*
*   Where:
*       c - any ASCII character
*
*   Returns:
*       'A' - 'Z' will return 'a' to 'z'
*       else return c unchanged.
*
******************************************************************************/
int tolower(int c)
{
    if( c>='A' && c<='Z' )
        return( c + 0x20 );

    return( c );
}

/******************************************************************************
*
*   int strcmpii( const char *s1, const char *s2 )
*
*      Compares s1 with s2 with case insensitivity.
*      This function is identical to stricmp()
*
*   Where:
*      s1 - pointer to string 1
*      s2 - pointer to string 2
*
*   Returns:
*      An int that is greather than, equal to, or less than zero according to
*      the relative order of s1 and s2.
*
*      Example, if s1 > s2, return a positive value.
*
******************************************************************************/
int strcmpi( const char *s1, const char *s2 )
{
    while( 1 )
    {
        if (tolower(*s1) != tolower(*s2)) return(tolower(*s1) - tolower(*s2));
        if (*s1 == 0) return(0);
        s1++;
        s2++;
    }
}
