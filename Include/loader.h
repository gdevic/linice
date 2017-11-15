/******************************************************************************
*                                                                             *
*   Module:     loader.h                                                      *
*                                                                             *
*   Date:       11/26/00                                                      *
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

        Loader/Translator shared global data

*******************************************************************************
*                                                                             *
*   Major changes:                                                            *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 11/26/00   Initial version                                      Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Important Defines                                                         *
******************************************************************************/
#ifndef _LOADER_H_
#define _LOADER_H_

/******************************************************************************
*                                                                             *
*   Global Defines, Variables and Macros                                      *
*                                                                             *
******************************************************************************/

extern char *pTranslate;                // File to translate
extern char *pOutput;                   // Default output file is "input_file".sym
extern char *pPathSubst;                // Source path substitution
extern char *pSym;                      // Default symbol table to load/unload
extern char *pLogfile;                  // Default logfile name
extern unsigned int opt;                // Various command line options
extern int nVerbose;                    // Verbose level


#define OPT_TRANSLATE       0x00000001  // nTranslate -> level of translation
#define OPT_PATH            0x00000002  // pPathSubst -> source path substitution
#define OPT_OUTPUT          0x00000008  // pOutput -> output file
#define OPT_LOAD            0x00000010  // pLoad -> module to load
#define OPT_SYM             0x00000100  // pSym -> loads symbol(s) into debugger
#define OPT_UNLOAD          0x00000200  // pSym -> unloads symbol table(s)
#define OPT_INSTALL         0x00000400  // Install debugger
#define OPT_UNINSTALL       0x00000800  // Uninstall debugger
#define OPT_LOGFILE         0x00001000  // pLogfile is a logfile to output
#define OPT_LOGFILE_APPEND  0x00002000  // Append method on logfile
#define OPT_HELP            0x00004000  // Help command
#define OPT_VERBOSE         0x00008000  // Option verbose, make output informative
#define OPT_CHECK           0x00010000  // Symbol test command

#define VERBOSE0            // 0 (default) simply means no extra output is desired
#define VERBOSE1            if(nVerbose==3 || nVerbose==2 || nVerbose==1)
#define VERBOSE2            if(nVerbose==3 || nVerbose==2)
#define VERBOSE3            if(nVerbose==3)

#endif //  _LOADER_H_

