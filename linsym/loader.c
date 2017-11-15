/******************************************************************************
*                                                                             *
*   Module:     loader.c                                                      *
*                                                                             *
*   Date:       03/05/01                                                      *
*                                                                             *
*   Copyright (c) 2001 Goran Devic                                            *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************

    Module Description:

        This module contains

*******************************************************************************
*                                                                             *
*   Major changes:                                                            *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 03/05/01   Initial version                                      Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include <unistd.h>                     // Include standard UNIX header file
#include <errno.h>                      // Include errno
#include <stdlib.h>                     // Include standard library
#include <sys/wait.h>                   // Include waitpid
#include <string.h>                     // Include strings header file
#include <sys/stat.h>                   // Include file operations
#include <fcntl.h>                      // Include file control file
#include <stdio.h>                      // Include standard io file

/******************************************************************************
*                                                                             *
*   Global Variables                                                          *
*                                                                             *
******************************************************************************/

int VER_LOADER = (0x00 << 8) + 0x01;    // Major/minor version number

int VER_LOADER_BUILD = 1;

extern char **environ;

/******************************************************************************
*                                                                             *
*   Local Defines, Variables and Macros                                       *
*                                                                             *
******************************************************************************/

extern void OptInstall();
extern void OptAddSymbolTable(char *sName);
extern void OptRemoveSymbolTable(char *sName);

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
}


/******************************************************************************
*                                                                             *
*   void OptUninstall()                                                       *
*                                                                             *
*******************************************************************************
*
*   Removes linice.o module
*
******************************************************************************/
void OptUninstall()
{
    int status;

    // Unload the linice.o device driver module
    status = system2("rmmod linice");

    // rmmod must return 0 when uninstalling a module correctly
    if( status==0 )
    {
        printf("Linice module removed\n");
    }
    else
    {
        printf("Error unloading linice module (%d)!\n", status);
    }
}

/******************************************************************************
*                                                                             *
*   void OptHelp()                                                            *
*                                                                             *
*******************************************************************************
*
*   prints the command options and short help
*
******************************************************************************/
void OptHelp()
{
    printf("LOADER linice loader/symbol translator\n");
    printf("Arguments:\n");
    printf("   -i --install          loads debugger into the memory\n");
    printf("   -s --symbol <name>    adds a symbol table\n");
    printf("   -r --remove <name>    removes a symbol table\n");
    printf("   -t --translate <map> <sym> translates a map file into a symbol table\n");
    printf("   -u --uninstall        removes debugger from the memory\n");
    printf("   -h --help             this help\n");
    printf("   -v --version          print version number\n");
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
    printf("LOADER version %d.%02d build %d\n",
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
*   Command line options:
*       -h  or  --help           prints short help
*       -i  or  --install        installs debugger module (same as `insmod linice.o')
*       -s  or  --symbol         adds a symbol table (made by 'nm' command) <name>
*       -r  or  --remove         removes a symbol table <name>
*       -t  or  --translate      translate map file <name1) into a symbol table <name2)
*       -u  or  --uninstall      removes the debugger module (same as `rmmod linice')
*       -v  or  --version        prints linice version
*
******************************************************************************/
int main(int argn, char *argp[])
{
    int i;
    char *ptr;

    if( argn==1 )
    {
        OptHelp();
    }
    else
    {
        // Parse command line arguments

        for( i=1; i<argn; i++)
        {
            // See if it is an option

            if( *argp[i]=='-' )
            {
                ptr = argp[i] + 1;

                if( (strcmp(ptr, "i")==0) || (strcmp(ptr, "-install")==0) )
                    OptInstall();

                if( (strcmp(ptr, "u")==0) || (strcmp(ptr, "-uninstall")==0) )
                    OptUninstall();

                if( (strcmp(ptr, "s")==0) || (strcmp(ptr, "-symbol")==0) )
                {
                    OptAddSymbolTable(argp[i+1]);
                    i++;
                }

                if( (strcmp(ptr, "r")==0) || (strcmp(ptr, "-remove")==0) )
                {
                    OptRemoveSymbolTable(argp[i+1]);
                    i++;
                }

                if( (strcmp(ptr, "t")==0) || (strcmp(ptr, "-translate")==0) )
                {
                    OptTranslateSymbolTable(argp[i+1], argp[i+2]);
                    i+=2;
                }

                if( (strcmp(ptr, "h")==0) || (strcmp(ptr, "-help")==0) )
                    OptHelp();

                if( (strcmp(ptr, "v")==0) || (strcmp(ptr, "-version")==0) )
                    OptVersion();
            }
            else
                OptHelp();
        }
    }

    return( 0 );
}
