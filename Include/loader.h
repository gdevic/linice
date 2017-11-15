/******************************************************************************
*                                                                             *
*   Module:     loader.h                                                      *
*                                                                             *
*   Date:       11/26/00                                                      *
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

#ifndef _CPP
/******************************************************************************
*                                                                             *
*   Global Defines, Variables and Macros                                      *
*                                                                             *
******************************************************************************/

#define TRAN_PUBLICS        1
#define TRAN_TYPEINFO       2
#define TRAN_SYMBOLS        3
#define TRAN_SOURCE         4
#define TRAN_PACKAGE        5

extern int nTranslate;                  // Default translation is all
extern char *pTranslate;                // File to translate
extern char *pOutput;                   // Default output file is "input_file".sym
extern char *pSource;                   // Source search path is current directory
extern char *pLoad;                     // Need to specify loading module
extern char *pArgs;                     // Default no arguments
extern char *pSym;                      // Default symbol table to load/unload
extern char *pLogfile;                  // Default logfile name
unsigned int opt;                       // Various command line options

#endif // _CPP

#define OPT_TRANSLATE       0x00000001  // nTranslate -> level of translation
#define OPT_SOURCE          0x00000002  // pSource -> dirs to search for source
#define OPT_PROMPT          0x00000004  // Prompt for missing source
#define OPT_OUTPUT          0x00000008  // pOutput -> output file
#define OPT_LOAD            0x00000010  // pLoad -> module to load
#define OPT_LOAD_BREAK      0x00000020  // break upon load
#define OPT_LOAD_NOBREAK    0x00000040  // no break on load
#define OPT_ARGS            0x00000080  // pArgs -> program arguments
#define OPT_SYM             0x00000100  // pSym -> loads symbol(s) into debugger
#define OPT_UNLOAD          0x00000200  // pSym -> unloads symbol table(s)
#define OPT_INSTALL         0x00000400  // Install debugger
#define OPT_UNINSTALL       0x00000800  // Uninstall debugger
#define OPT_LOGFILE         0x00001000  // pLogfile is a logfile to output
#define OPT_LOGFILE_APPEND  0x00002000  // Append method on logfile
#define OPT_VER             0x00004000  // Version command
#define OPT_HELP            0x00008000  // Help command
#define OPT_VERBOSE         0x00010000  // Option verbose, make output informative


#endif //  _LOADER_H_

