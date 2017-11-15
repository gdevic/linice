/******************************************************************************
*                                                                             *
*   Module:     loader.c                                                      *
*                                                                             *
*   Date:       09/05/00                                                      *
*                                                                             *
*   Copyright (c) 2001 Goran Devic                                            *
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

        This module contains main functions for Linice loader/translator.

        Options:

        Translate symbol information - create a symbol file:
        =====================================================

        linsym --translate path/program  -t -> creates path/program.sym
        --translate:publics                 -> include only public symbols
        --translate:typeinfo                -> include publics+type info
        --translate:symbols                 -> include all symbol info
        --translate:source                  -> include all + source files
        --translate:package                 -> include all + source files

        --source:{list of dirs;}         -p -> dirs to search for sources

        --output:file_name               -o -> set the symbol file name

        --prompt                         -p -> prompt for missing source files

        Load debugee:
        ==============

        linsym --load {file}             -l -> loads module as a process
        --load:BREAK {file}                 -> break on init_module()
        --load:NOBREAK {file}               -> do not break on main()
        (for applications, break is default; for modules, nobreak is default)
        --args:{program arguments}       -a -> use these arguments
        --args:"{program arguments}         -> use quote if there are spaces

        Load/Unload linice debugger module:
        ===================================

        linsym --install{:System.map}    -i -> load debugger, specify System.map file
        linsym --uninstall               -x -> unloads debugger

        Loads/Unloads symbol table:
        ============================

        linsym --sym:{list of sym files;}-s -> loads symbol file(s)
        linsym --unload:{symbol files;}  -u -> unloads symbol table(s)

        Miscellaneous:
        ===============

        linsym --logfile:{file}          -g -> saves linice history into that file
               --logfile:{file},APPEND      ->  optionally appending

        linsym --help                    -h -> print command line symtax

        linsym --verbose                 -v -> verbose output

        Test/Debug:
        ============

        linsym --capture:{option}        -c -> capture ioctl

*******************************************************************************
*                                                                             *
*   Major changes:                                                            *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 09/05/00   Initial version                                      Goran Devic *
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

#define PATH_MAX    128
static char sOutputFile[PATH_MAX];      // Contains the name of the output file

int nTranslate = TRAN_PACKAGE;          // Default translation is all
char *pTranslate = NULL;                // File to translate
char *pOutput = NULL;                   // Default output file is "input_file".sym
char *pSource = ".";                    // Source search path is current directory
char *pLoad = NULL;                     // Need to specify loading module
char *pArgs = NULL;                     // Default no arguments
char *pSym = NULL;                      // Default symbol table to load/unload
char *pLogfile = "linice.log";          // Default logfile name
char *pSystemMap = NULL;                // User supplied System.map file
char *pCap = NULL;                      // Capture option string
unsigned int opt;                       // Various option flags
int nVerbose;                           // Verbose level

/******************************************************************************
*                                                                             *
*   Local Defines, Variables and Macros                                       *
*                                                                             *
******************************************************************************/

int strnicmp( const char *s1, const char *s2, size_t n );
int strcmpi( const char *s1, const char *s2 );
int tolower(int);

extern BOOL OptInstall(char *pSystemMap);
extern void OptUninstall();
extern void OptAddSymbolTable(char *sName);
extern void OptRemoveSymbolTable(char *sName);
extern void OptTranslate(char *pathOut, char *pathIn, char *pathSources, int nLevel);
extern void OptLogHistory(void);
extern void OptCapture(char *pStr);


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
#ifndef WINDOWS
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
*   void OptHelp(int shorth)                                                  *
*                                                                             *
*******************************************************************************
*
*   prints the command options and short (1), or long (2) help
*
******************************************************************************/
void OptHelp(int shorth)
{
    if( shorth )
    {
        printf("Usage: LINSYM [options] [<program-name>]\n");
        printf("Use LINSYM -h or --help for extended help\n\n");
    }
    else
    {
        printf("  -v:{0-3} or --verbose:{0-3}    Verbose level (0=silent)\n");
//        printf("\n");
        printf("       Example: --verbose:3\n");
//        printf("\n");
        printf("  -i{:System.map} or --install   Installs Linice debugger module\n");
//        printf("\n");
        printf("       Example: --install\n");
//        printf("\n");
        printf("  -x or --uninstall              Uninstalls Linice debugger module\n");
//        printf("\n");
        printf("       Example: --uninstall\n");
//        printf("\n");
        printf("  -t or --translate:             Specify options for symbol translation\n");
        printf("    [PUBLICS|TYPEINFO|SYMBOLS|SOURCE|*PACKAGE*]\n");
//        printf("\n");
        printf("       Example: --translate:source Myapp\n");
//        printf("\n");
        printf("  -l or --load:                  Specify options for loading module or symbols\n");
        printf("    [BREAK|NOBREAK]\n");
//        printf("\n");
        printf("       Example: --load:break module.o\n");
        printf("       Example: --load:nobreak a.out\n");
//        printf("\n");
        printf("  -o or --output:<filename>    Specify alternate filename for translation\n");
//        printf("\n");
        printf("       Example: --output:MySymbols.sym\n");
//        printf("\n");
        printf("  -p or --source:<path>[;<path>]       Specify path(s) for source searches\n");
//        printf("\n");
        printf("       Example: --source:/myproject/source;/myproject/include\n");
//        printf("\n");
        printf("  -a or --args:<arg-string>            Specify program argument for loading\n");
//        printf("\n");
        printf("       Example: --args:\"-ftest.c -x -d\"\n");
//        printf("\n");
        printf("  -p or --prompt                       Prompt for missing source files\n");
//        printf("\n");
        printf("       Example: --prompt\n");
//        printf("\n");
        printf("  -s or --sym:<filename>[;<filename>]  Load symbol file(s)\n");
//        printf("\n");
        printf("       Example: --sym:MyProg.sym\n");
//        printf("\n");
        printf("  -u or --unload:<name>[;<name>]       Unload symbol table(s)\n");
//        printf("\n");
        printf("       Example: --unload:MyProg\n");
//        printf("\n");
        printf("  -g or --logfile[:<filename>]         Save the Linice history buffer\n");
        printf("    [,APPEND]\n");
//        printf("\n");
        printf("       Example: --logfile:Mylog.log,append\n");
//        printf("\n");
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

    pSource = "./";                     // Default source path is the current directory
    opt = OPT_VERBOSE;                  // Default option
    nVerbose = 1;                       // Default verbose level

    // Print the basic banner
    printf("\nLinice Debugger ");
    printf("Symbol Translator/Loader Version %d.%02d\n", LINSYMVER >> 8, LINSYMVER & 0xFF);

    printf("(C) Goran Devic, 2000-2003\n\n");

    // If there were no arguments, just print help and exit (help exits)
    if( argn==1 )
        OptHelp(1);

    //------------------------------------------------------------------------
    // Parse command line arguments and assign opt bits and strings
    //------------------------------------------------------------------------
    for( i=1; i<argn; i++)
    {
        if( !strnicmp(argp[i], "--translate", 11) || !strnicmp(argp[i], "-t", 2) )
        {
            ptr = argp[i] + (*(argp[i]+1)=='-'? 11 : 2);

            // --translate {file}
            // --translate:publics {file}
            // --translate:typeinfo {file}
            // --translate:symbols {file}
            // --translate:source {file}
            // --translate:package {file}
            opt |= OPT_TRANSLATE;

            if( !strcmpi(ptr, ":publics" ))
                nTranslate = TRAN_PUBLICS;
            if( !strcmpi(ptr, ":typeinfo" ))
                nTranslate = TRAN_TYPEINFO;
            if( !strcmpi(ptr, ":symbols" ))
                nTranslate = TRAN_SYMBOLS;
            if( !strcmpi(ptr, ":source" ))
                nTranslate = TRAN_SOURCE;
            if( !strcmpi(ptr, ":package" ))
                nTranslate = TRAN_PACKAGE;

            if( i+1<argn )
            {
                pTranslate = argp[i+1];  // Assign path/name of the file to translate
                i++;
            }
            else
                opt |= OPT_HELP;    // Did not provide file name to translate
        }
        else
        if( !strnicmp(argp[i], "--source:", 9) || !strnicmp(argp[i], "-p", 2) )
        {
            ptr = argp[i] + (*(argp[i]+1)=='-'? 9 : 2);

            // --source:{dirs to search for source}
            opt |= OPT_SOURCE;
            pSource = ptr;
        }
        else
        if( !strcmpi(argp[i], "--prompt") || !strcmpi(argp[i], "-p") )
        {
            // --prompt  says to prompt for the missing source (default is no prompt)
            opt |= OPT_PROMPT;
        }
        else
        if( !strnicmp(argp[i], "--output:", 9) || !strnicmp(argp[i], "-o:", 3) )
        {
            ptr = argp[i] + (*(argp[i]+1)=='-'? 9 : 3);

            // --output:{output_file}
            opt |= OPT_OUTPUT;
            pOutput = ptr;
        }
        else
        if( !strnicmp(argp[i], "--load", 6) || !strnicmp(argp[i], "-l", 2) )
        {
            ptr = argp[i] + (*(argp[i]+1)=='-'? 6 : 2);

            // --load [exec-file | symbol-file]
            // --load:BREAK ..
            // --load:NOBREAK ..
            if( i+1<argn )
            {
                if( !strcmpi(ptr, ":break") )
                    opt |= OPT_LOAD_BREAK;
                if( !strcmpi(ptr, ":nobreak") )
                    opt |= OPT_LOAD_NOBREAK;

                pLoad = argp[i+1];  // Assign path/name of the module to load
                i++;
            }
            else
                opt |= OPT_HELP;    // Did not provide file name to load
            opt |= OPT_LOAD;
        }
        else
        if( !strnicmp(argp[i], "--args:", 7) || !strnicmp(argp[i], "-a:", 3) )
        {
            ptr = argp[i] + (*(argp[i]+1)=='-'? 7 : 3);

            // --args:{arguments}
            // --args:"arguments"
            opt |= OPT_ARGS;
            pArgs = ptr;
        }
        else
        if( !strnicmp(argp[i], "--sym:", 6) || !strnicmp(argp[i], "-s:", 3) )
        {
            ptr = argp[i] + (*(argp[i]+1)=='-'? 6 : 3);

            // --sym:{symbols-to-load}
            opt |= OPT_SYM;
            pSym = ptr;
        }
        else
        if( !strnicmp(argp[i], "--unload:", 9) || !strnicmp(argp[i], "-u:", 3) )
        {
            ptr = argp[i] + (*(argp[i]+1)=='-'? 9 : 3);

            // --unload:{symbols-to-unload}
            opt |= OPT_UNLOAD;
            pSym = ptr;
        }
        else
        if( !strnicmp(argp[i], "--install", 9) || !strnicmp(argp[i], "-i", 2) )
        {
            ptr = argp[i] + (*(argp[i]+1)=='-'? 9 : 2);

            // --install  install debugger
            // --install:System.map
            opt |= OPT_INSTALL;
            if( *ptr == ':' )
            {
                pSystemMap = ptr + 1;
            }
        }
        else
        if( !strcmpi(argp[i], "--uninstall") || !strcmpi(argp[i], "-x") )
        {
            // --uninstall  uninstalls debugger
            opt |= OPT_UNINSTALL;
        }
        else
        if( !strnicmp(argp[i], "--logfile", 9) || !strnicmp(argp[i], "-g", 2) )
        {
            ptr = argp[i] + (*(argp[i]+1)=='-'? 9 : 2);

            // --logfile                        - log to a default logfile
            // --logfile:{log_file}             - specify a logfile
            // --logfile,APPEND                 - append to a default logfile
            // --logfiled:{logfile},APPEND      - append to a specified logfile
            opt |= OPT_LOGFILE;

            // Check if we have a new logfile specified
            if( *ptr == ':' )
            {
                ptr++;
                pLogfile = ptr;                 // New log file is specified
            }

            // Find the end of the specified file name (it is either end of string
            // or a comma for append sub option
            if( strchr(ptr, ',') )
            {
                ptr = strchr(ptr, ',');
                *ptr = 0;                   // Zero-terminate the log file name
                ptr++;
            }

            // Check if we need to append instead of create/truncate logfile
            if( !strcmpi(ptr, "append") )
                opt |= OPT_LOGFILE_APPEND;
        }
        else
        if( !strcmpi(argp[i], "--help") || !strcmpi(argp[i], "-h") )
        {
            // --help  display help information and exit
            opt |= OPT_HELP;
        }
        else
        if( !strnicmp(argp[i], "--verbose", 9) || !strnicmp(argp[i], "-v:", 3) )
        {
            ptr = argp[i] + (*(argp[i]+1)=='-'? 9 : 2);

            // --verbose:{0,1,2,3}   display more output information
            if( *ptr == ':' )
            {
                ptr++;

                // 1 is default verbose level, so switch to another
                switch( *ptr )
                {
                    case '0':   opt &= ~OPT_VERBOSE;  break;     // 0 means no output
                    case '2':   nVerbose = 2;  break;
                    case '3':   nVerbose = 3;  break;
                }
            }
        }
        else
        if( !strnicmp(argp[i], "--capture:", 10) || !strnicmp(argp[i], "-c:", 3) )
        {
            ptr = argp[i] + (*(argp[i]+1)=='-'? 10 : 3);

            // --capture:{option}
            opt |= OPT_CAPTURE;
            pCap = ptr;
        }
        else
        {
            printf("Error - Unknown option: %s\n\n", argp[i]);

            // Unknown option? We should exit in that case
            return(-1);
        }

        if( opt & OPT_HELP )
        {
            // Print help and EXIT (you can't do anything if you want help :-)
            OptHelp(0);
        }
    }

    //------------------------------------------------------------------------
    // Start executing command line options
    //------------------------------------------------------------------------

    if( opt & OPT_LOGFILE )
    {
        VERBOSE1 printf("Logfile: %s %s\n", pLogfile, (opt & OPT_LOGFILE_APPEND)? "APPEND":"");
    }

    // Now we need to do some adjustments:
    // If an output symbol file is not given, use input+".sym"
    if( opt & OPT_TRANSLATE )
    {
        if( pOutput==NULL )
        {
            strcpy(sOutputFile, pTranslate);
            strcat(sOutputFile, ".sym");
        }
        // Assign the "real" pointer to an output file
        pOutput = sOutputFile;
    }

    // If we need to load debugger, do it first
    if( opt & OPT_INSTALL )
    {
        // If installation failed, we need to exit
        if( OptInstall(pSystemMap)==FALSE )
            exit(-1);
    }

    // Translate the symbols
    if( opt & OPT_TRANSLATE )
        OptTranslate(pOutput, pTranslate, pSource, nTranslate);

    // Load symbol(s) into debugger
    if( opt & OPT_SYM )
    {
        char *pDelim;
        do
        {
            pDelim = strchr(pSym, ';');
            if( pDelim ) *pDelim = 0;

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
            pDelim = strchr(pSym, ';');
            if( pDelim ) *pDelim = 0;

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

    // Test capture option
    if( opt & OPT_CAPTURE )
        OptCapture(pCap);

    return( 0 );
}


int _open(const char *path, int oflag)
{
    return(open(path, oflag));
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
