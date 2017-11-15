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

        linsym --ver                     -v -> prints debugger version

        linsym --help                    -h -> print command line symtax

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

#include "loader.h"                     // Include global protos

/******************************************************************************
*                                                                             *
*   Global Variables                                                          *
*                                                                             *
******************************************************************************/

int VER_LOADER = (0x00 << 8) + 0x01;    // Major/minor version number

int VER_LOADER_BUILD = 1;

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
char *pSystemMap = "/boot/System.map";  // Default System.map file
unsigned int opt;

/******************************************************************************
*                                                                             *
*   Local Defines, Variables and Macros                                       *
*                                                                             *
******************************************************************************/

extern void OptInstall(char *pSystemMap);
extern void OptUninstall();
extern void OptAddSymbolTable(char *sName);
extern void OptRemoveSymbolTable(char *sName);
extern void OptTranslate(char *pathOut, char *pathIn, char *pathSources, int nLevel);


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

    if( command==NULL )
        return( -1 );

    pid = fork();
    if( pid==-1 )
        return( -1 );
    if( pid==0 )
    {
        char *argv[4];
        argv[0] = "sh";
        argv[1] = "-c";
        argv[2] = command;
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
        printf("Usage: LINSYM [options] [<module-name>]\n");
        printf("Use LINSYM --help for extended help\n\n");
    }
    else
    {
        printf("\t-i{:System.map} or --install   Installs Linice debugger module\n");
        printf("\n");
        printf("\t\tExample: --install\n");
        printf("\n");
        printf("\t-x or --uninstall   Uninstalls Linice debugger module\n");
        printf("\n");
        printf("\t\tExample: --uninstall\n");
        printf("\n");
        printf("\t-t or --translate:  Specify options for symbol translation\n");
        printf("\t\t[PUBLICS|TYPEINFO|SYMBOLS|SOURCE|*PACKAGE*]\n");
        printf("\n");
        printf("\t\tExample: --translate:source Myapp\n");
        printf("\n");
        printf("\t-l or --load:  Specify options for loading a module and symbols\n");
        printf("\t\t[BREAK|NOBREAK]\n");
        printf("\n");
        printf("\t\tExample: --load:break module.o\n");
        printf("\t\tExample: --load:nobreak a.out\n");
        printf("\n");
        printf("\t-o or --output:<filename>  Specify alternate filename for translation\n");
        printf("\n");
        printf("\t\tExample: --output:MySymbols.sym\n");
        printf("\n");
        printf("\t-p or --source:<path>[;<path>] Specify path(s) for source searches\n");
        printf("\n");
        printf("\t\tExample: --source:/myproject/source;/myproject/include\n");
        printf("\n");
        printf("\t-a or --args:<arg-string>  Specify program argument for loading\n");
        printf("\n");
        printf("\t\tExample: --args:\"-ftest.c -x -d\"\n");
        printf("\n");
        printf("\t-p or --prompt Prompt for missing source files\n");
        printf("\n");
        printf("\t\tExample: --prompt\n");
        printf("\n");
        printf("\t-s or --sym:<filename>[;<filename>]  Load symbol file(s)\n");
        printf("\n");
        printf("\t\tExample: --sym:MyProg.sym\n");
        printf("\n");
        printf("\t-u or --unload:<name>[;<name>]  Unload symbol table(s)\n");
        printf("\n");
        printf("\t\tExample: --unload:MyProg\n");
        printf("\n");
        printf("\t-g or --logfile[:<filename>]  Save the Linice history buffer\n");
        printf("\t\t[,APPEND]\n");
        printf("\n");
        printf("\t\tExample: --logfile:Mylog.log,append\n");
        printf("\n");
        printf("\t-v or --version  Show version information for Linice\n");
        printf("\n");
        printf("\t\tExample: --version\n");
        printf("\n");
        printf("\t-h or --help  Display this help information\n");
        printf("\n");
        printf("\t\tExample: --help\n");
        printf("\n");
    }

    exit(0);
}

/******************************************************************************
*                                                                             *
*   void OptVersion()                                                         *
*                                                                             *
*******************************************************************************
*
*   prints the linice version number
*
******************************************************************************/
void OptVersion()
{
    printf("Linsym Translator/Loader Version %d.%02d build %d\n",
        VER_LOADER >> 8, VER_LOADER & 0xFF, VER_LOADER_BUILD);
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
    char *ptr;

    pSource = "./";                     // Default source path is the current directory
    opt = 0;

    // Print the basic banner
    printf("\nLinice Debugger Symbol Translator/Loader version %d.%02d\n", VER_LOADER >> 8, VER_LOADER & 0xFF);
    printf("(C) Goran Devic, 2000-2001\n\n");

    // If there were no arguments, just print help and exit (help exits)
    if( argn==1 )
        OptHelp(1);

    // Parse command line arguments and assign opt bits
    for( i=1; i<argn; i++)
    {
        if( !strncmp(argp[i], "--translate", 11) || !strncmp(argp[i], "-t", 2) )
        {
            ptr = argp[i] + (*(argp[i]+1)=='-'? 11 : 2);

            // --translate {file}
            // --translate:publics {file}
            // --translate:typeinfo {file}
            // --translate:symbols {file}
            // --translate:source {file}
            // --translate:package {file}
            opt |= OPT_TRANSLATE;

            if( !strcmp(ptr, ":publics" ))
                nTranslate = TRAN_PUBLICS;
            if( !strcmp(ptr, ":typeinfo" ))
                nTranslate = TRAN_TYPEINFO;
            if( !strcmp(ptr, ":symbols" ))
                nTranslate = TRAN_SYMBOLS;
            if( !strcmp(ptr, ":source" ))
                nTranslate = TRAN_SOURCE;
            if( !strcmp(ptr, ":package" ))
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
        if( !strncmp(argp[i], "--source:", 9) || !strncmp(argp[i], "-p", 2) )
        {
            ptr = argp[i] + (*(argp[i]+1)=='-'? 9 : 2);

            // --source:{dirs to search for source}
            opt |= OPT_SOURCE;
            pSource = ptr;
        }
        else
        if( !strcmp(argp[i], "--prompt") || !strcmp(argp[i], "-p") )
        {
            // --prompt  says do not prompt for the missing source
            opt |= OPT_PROMPT;
        }
        else
        if( !strncmp(argp[i], "--output:", 9) || !strncmp(argp[i], "-o:", 3) )
        {
            ptr = argp[i] + (*(argp[i]+1)=='-'? 9 : 3);

            // --output:{output_file}
            opt |= OPT_OUTPUT;
            pOutput = ptr;
        }
        else
        if( !strncmp(argp[i], "--load", 6) || !strncmp(argp[i], "-l", 2) )
        {
            ptr = argp[i] + (*(argp[i]+1)=='-'? 6 : 2);

            // --load [exec-file | symbol-file]
            // --load:BREAK ..
            // --load:NOBREAK ..
            if( i+1<argn )
            {
                if( !strcmp(ptr, ":break") )
                    opt |= OPT_LOAD_BREAK;
                if( !strcmp(ptr, ":nobreak") )
                    opt |= OPT_LOAD_NOBREAK;

                pLoad = argp[i+1];  // Assign path/name of the module to load
                i++;
            }
            else
                opt |= OPT_HELP;    // Did not provide file name to load
            opt |= OPT_LOAD;
        }
        else
        if( !strncmp(argp[i], "--args:", 7) || !strncmp(argp[i], "-a:", 3) )
        {
            ptr = argp[i] + (*(argp[i]+1)=='-'? 7 : 3);

            // --args:{arguments}
            // --args:"arguments"
            opt |= OPT_ARGS;
            pArgs = ptr;
        }
        else
        if( !strncmp(argp[i], "--sym:", 6) || !strncmp(argp[i], "-s:", 3) )
        {
            ptr = argp[i] + (*(argp[i]+1)=='-'? 6 : 3);

            // --sym:{symbols-to-load}
            opt |= OPT_SYM;
            pSym = ptr;
        }
        else
        if( !strncmp(argp[i], "--unload:", 9) || !strncmp(argp[i], "-u:", 3) )
        {
            ptr = argp[i] + (*(argp[i]+1)=='-'? 9 : 3);

            // --unload:{symbols-to-unload}
            opt |= OPT_UNLOAD;
            pSym = ptr;
        }
        else
        if( !strncmp(argp[i], "--install", 9) || !strncmp(argp[i], "-i", 2) )
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
        if( !strcmp(argp[i], "--uninstall") || !strcmp(argp[i], "-x") )
        {
            // --uninstall  uninstalls debugger
            opt |= OPT_UNINSTALL;
        }
        else
        if( !strncmp(argp[i], "--logfile", 9) || !strncmp(argp[i], "-g", 2) )
        {
            ptr = argp[i] + (*(argp[i]+1)=='-'? 9 : 2);

            // --logfile
            // --logfile:{log_file}
            // --logfiled:{logfile},APPEND
            if( *ptr == ':' )
            {
                int len;

                // Log file is specified
                pLogfile = ptr + 1;
                len = strlen(pLogfile);
                if( len > 8 && !strcmp(pLogfile+len-7, ",append") )
                {
                    opt |= OPT_LOGFILE_APPEND;
                    pLogfile[len-7] = 0;
                }
            }
            // Case where we append to a default logfile:
            if( !strcmp(ptr, ":append") )
                opt |= OPT_LOGFILE_APPEND;
        }
        else
        if( !strcmp(argp[i], "--ver") || !strcmp(argp[i], "-v") )
        {
            // --ver  display version
            opt |= OPT_VER;
        }
        else
        if( !strcmp(argp[i], "--help") || !strcmp(argp[i], "-h") )
        {
            // --help  display help information and exit
            opt |= OPT_HELP;
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
        OptInstall(pSystemMap);

    // Print the version
    if( opt & OPT_VER )
        OptVersion();

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

    // If uninstall debugger is needed, do it last
    if( opt & OPT_UNINSTALL )
        OptUninstall();

    return( 0 );
}


int _open(const char *path, int oflag)
{
    return(open(path, oflag));
}

