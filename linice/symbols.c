/******************************************************************************
*                                                                             *
*   Module:     symbols.c                                                     *
*                                                                             *
*   Date:       10/21/00                                                      *
*                                                                             *
*   Copyright (c) 2000-2004 Goran Devic                                       *
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

        This module contains code for symbols access from the symbol table

*******************************************************************************
*                                                                             *
*   Major changes:                                                            *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 10/21/00   Initial version                                      Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include "module-header.h"              // Include types commonly defined for a module

#include "clib.h"                       // Include C library header file
#include "iceface.h"                    // Include iceface module stub protos
#include "ice.h"                        // Include main debugger structures
#include "debug.h"                      // Include our dprintk()

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

static TSYMTYPEDEF1 TypeUnsignedInt =
{
    0,      // maj
    0,      // min
    "",     // pName
    "\4",   // pDef -> TYPE_UNSIGNED_INT
    0       // file_id
};

/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

extern TSYMTYPEDEF1 *Type2Typedef(char *pTypeName, int nLen, WORD file_id);
extern void TypedefCanonical(TSYMTYPEDEF1 *pType1);
extern void *SymTabFindSection(TSYMTAB *pSymTab, BYTE hType);
extern BOOL FindModule(TMODULE *pMod, char *pName, int nNameLen);
extern DWORD fnPtr(DWORD arg);
extern int GetTokenLen(char *pToken);

/******************************************************************************
*                                                                             *
*   void ParseLocalSymbol(TExItem *item, TSYMFNSCOPE1 *pLocal)                *
*                                                                             *
*******************************************************************************
*
*   Given the token within the function scope that contains the symbol,
*   fill in the TExItem structure.
*
*   Where:
*       item is the TExItem to fill in
*       pLocal is the pointer to local scope token to use as a source
*
******************************************************************************/
void ParseLocalSymbol(TExItem *item, TSYMFNSCOPE1 *pLocal)
{
    TSYMTYPEDEF1 *pType1;               // Pointer to the resulting variable type
    PSTR pDef;                          // Pointer to the type definition portion of the name string

    // The local variable may be kept in the register or on the stack
    switch( pLocal->TokType )
    {
        case TOKTYPE_LCSYM:
            // Local static symbol has absolute address
        case TOKTYPE_LSYM:
            // Local symbol is also on the stack addresses by SS:EBP
        case TOKTYPE_PARAM:
            // Parameter is on the stack, addressed with SS:EBP
            item->bType = EXTYPE_SYMBOL;
            item->pData = (BYTE *) ((int)pLocal->param + (int)deb.r->ebp);

            item->wSel  = deb.r->ss;        // This is valid only for LSYM and PARAM
            item->Data  = pLocal->param;    // Keep the EBP offset in the Data field
            break;

        case TOKTYPE_RSYM:
            // Variable is stored in a CPU register, we can point to a register file
            item->bType = EXTYPE_REGISTER;

            // This is the gcc i386 register allocation
            switch( pLocal->param )
            {
                case 0x0:     item->pData = (BYTE*) &deb.r->eax;     break;
                case 0x1:     item->pData = (BYTE*) &deb.r->ecx;     break;
                case 0x2:     item->pData = (BYTE*) &deb.r->edx;     break;
                case 0x3:     item->pData = (BYTE*) &deb.r->ebx;     break;
                case 0x4:     item->pData = (BYTE*) &deb.r->esp;     break;
                case 0x5:     item->pData = (BYTE*) &deb.r->ebp;     break;
                case 0x6:     item->pData = (BYTE*) &deb.r->esi;     break;
                case 0x7:     item->pData = (BYTE*) &deb.r->edi;     break;
                case 0x8:     item->pData = (BYTE*) &deb.r->eip;     break;
                case 0x9:     item->pData = (BYTE*) &deb.r->eflags;  break;
                case 0xA:     item->pData = (BYTE*) &deb.r->cs;      break;
                case 0xB:     item->pData = (BYTE*) &deb.r->ss;      break;
                case 0xC:     item->pData = (BYTE*) &deb.r->ds;      break;
                case 0xD:     item->pData = (BYTE*) &deb.r->es;      break;
                case 0xE:     item->pData = (BYTE*) &deb.r->fs;      break;
                case 0xF:     item->pData = (BYTE*) &deb.r->gs;      break;
            }
            break;
    }

    // Get the type of the symbol, default to unsigned int
    memcpy(&item->Type, &TypeUnsignedInt, sizeof(TSYMTYPEDEF1));

    pDef = strchr(pLocal->pName, '(');
    if( pDef )
    {
        // Found the type portion, get the type pointer
        pType1 = Type2Typedef(pDef, 0, deb.pFnScope->file_id);
        if( pType1 )
        {
            // Make the resulting type canonical
            memcpy(&item->Type, pType1, sizeof(TSYMTYPEDEF1));
            TypedefCanonical(&item->Type);
        }
    }
}

/******************************************************************************
*                                                                             *
*   BOOL FindLocalSymbol(TExItem *item, char *pName, int nNameLen)            *
*                                                                             *
*******************************************************************************
*
*   Search for the symbol value and type of a given symbol name within
*   the local scope.
*
*   Where:
*       item - item descriptor to store the final value
*       pName - name of the symbol to look for
*       nNameLen - pointer to symbol name length
*   Returns:
*       TRUE if a symbol is found, pType1 is stored, pNameLen may be adjusted
*       FALSE if a symbol is not found
*
******************************************************************************/
BOOL FindLocalSymbol(TExItem *item, char *pName, int nNameLen)
{
    TSYMFNSCOPE *pFnScope;              // Pointer to a local scope (shorthand)
    TSYMFNSCOPE1 *pLocal;               // Local scope variable descriptor
    DWORD dwOffset;                     // Current EIP offset within a function
    int i;                              // Loop index

    // Search the local scope
    if( deb.pFnScope )
    {
        pFnScope = deb.pFnScope;

        dwOffset = deb.r->eip - pFnScope->dwStartAddress;

        // Traverse function scope array from back to front until we find we passed our offset
        for(i = pFnScope->nTokens-1; i>=0; i-- )
        {
            if( pFnScope->list[i].TokType==TOKTYPE_RBRAC || pFnScope->list[i].TokType==TOKTYPE_LBRAC )
            {
                if( pFnScope->list[i].param >= dwOffset )
                {
                    // We found where our eip currently is. Search backward for the symbol name.
                    // TODO: We should skip any increasing LBRAC that could form alternate "bubble" sub-scope
                    while( --i>=0 )
                    {
                        // Ignore RBRAC and LBRAC...
                        if( pFnScope->list[i].TokType==TOKTYPE_RBRAC || pFnScope->list[i].TokType==TOKTYPE_LBRAC )
                        {
                            ;
                        }
                        else
                        {
                            // Compare the symbol name: The local symbol has to terminate with ':'
                            if( *(pFnScope->list[i].pName+nNameLen)==':' && !strnicmp(pName, pFnScope->list[i].pName, nNameLen) )
                            {
                                // Names match! Store the information about the symbol
                                pLocal = &pFnScope->list[i];

                                ParseLocalSymbol(item, pLocal);

                                return( TRUE );
                            }
                        }
                    }
                }
            }
        }
    }

    return( FALSE );
}

/******************************************************************************
*                                                                             *
*   BOOL FindStaticSymbol(TExItem *item, char *pName, int nNameLen)           *
*                                                                             *
*******************************************************************************
*
*   Search for the symbol value and type of a given symbol name within
*   the file static scope.
*
*   Where:
*       item - item descriptor to store the final value
*       pName - name of the symbol to look for
*       nNameLen - pointer to symbol name length
*   Returns:
*       TRUE if a symbol is found, pType1 is stored, pNameLen may be adjusted
*       FALSE if a symbol is not found
*
******************************************************************************/
BOOL FindStaticSymbol(TExItem *item, char *pName, int nNameLen)
{
    TSYMSTATIC *pStatic;                // Pointer to the static symbol header
    TSYMTYPEDEF1 *pType1;               // Pointer to the resulting variable type
    int i;                              // Loop index

    // We have to be within some function scope
    if( deb.pFnScope )
    {
        // Within the current symbol table, find the static symbol descriptor for the current file id

        pStatic = (TSYMSTATIC *)SymTabFindSection(deb.pSymTabCur, HTYPE_STATIC);

        if( pStatic )
        {
            // Found one static symbol record, check that it belongs to the current file_id
            // TODO: Can the pointer to static record be part of the deb.context variable?

            if( pStatic->file_id==deb.pFnScope->file_id )
            {
                // The file id match, so we can loop and search for the name match

                for(i=0; i<pStatic->nStatics; i++ )
                {
                    // Compare the symbol name: The static symbol has to terminate with ':'
                    if( *(pStatic->list[i].pName+nNameLen)==':' && !strnicmp(pName, pStatic->list[i].pName, nNameLen) )
                    {
                        // Found the matching name of the static variable
                        // Fill in the item structure

                        item->bType = EXTYPE_SYMBOL;
                        item->pData = (BYTE*) pStatic->list[i].dwAddress;

                        // Get the type of the symbol
                        pType1 = Type2Typedef(pStatic->list[i].pDef, 0, pStatic->file_id);
                        if( pType1 )
                        {
                            // Make the resulting type canonical
                            memcpy(&item->Type, pType1, sizeof(TSYMTYPEDEF1));
                            TypedefCanonical(&item->Type);
                        }
                        else
                            memcpy(&item->Type, &TypeUnsignedInt, sizeof(TSYMTYPEDEF1));

                        return( TRUE );
                    }
                }
            }
        }
    }

    return( FALSE );
}

/******************************************************************************
*                                                                             *
*   BOOL FindGlobalSymbol(TExItem *item, char *pName, int nNameLen)           *
*                                                                             *
*******************************************************************************
*
*   Search for the symbol value and type of a given symbol name within
*   the symbol global scope.
*
*   Where:
*       item - item descriptor to store the final value
*       pName - name of the symbol to look for
*       nNameLen - pointer to symbol name length
*   Returns:
*       TRUE if a symbol is found, pType1 is stored, pNameLen may be adjusted
*       FALSE if a symbol is not found
*
******************************************************************************/
BOOL FindGlobalSymbol(TExItem *item, char *pName, int nNameLen)
{
    TSYMGLOBAL *pGlobal;                // Pointer to the global symbol header
    TSYMTYPEDEF1 *pType1;               // Pointer to the resulting variable type
    UINT i;                             // Loop index

    // We have to be within some function scope
    if( deb.pFnScope )
    {
        // Within the current symbol table, find the global symbol descriptor

        pGlobal = (TSYMGLOBAL *)SymTabFindSection(deb.pSymTabCur, HTYPE_GLOBALS);

        if( pGlobal )
        {
            // Found a global symbol record, look for the name match

            for(i=0; i<pGlobal->nGlobals; i++ )
            {
                // Compare the symbol name: The global symbol has to terminate with a zero
                if( !stricmp(pName, pGlobal->list[i].pName) )
                {
                    // Found the matching name of the global variable
                    // Fill in the item structure

                    item->bType = EXTYPE_SYMBOL;

                    // TODO: I am puzzled. For some global symbols, we need & dereference... Investigate...
                    item->pData = (BYTE*) pGlobal->list[i].dwStartAddress;

                    // Get the type of the symbol. The file ID is stored with the global symbol
                    pType1 = Type2Typedef(pGlobal->list[i].pDef, 0, pGlobal->list[i].file_id);
                    if( pType1 )
                    {
                        // Make the resulting type canonical
                        memcpy(&item->Type, pType1, sizeof(TSYMTYPEDEF1));
                        TypedefCanonical(&item->Type);
                    }
                    else
                        memcpy(&item->Type, &TypeUnsignedInt, sizeof(TSYMTYPEDEF1));

                    return( TRUE );
                }
            }
        }
    }

    return( FALSE );
}

/******************************************************************************
*                                                                             *
*   BOOL FindModuleSymbol(TExItem *item, char *pName, int nNameLen)           *
*                                                                             *
*******************************************************************************
*
*   Search for the symbol value and type of a given symbol name within
*   the module chain scope. The symbol name has to be given in the form:
*
*       module!export
*       |-----------| nNameLen
*
*   Where:
*       item - item descriptor to store the final value
*       pName - name of the symbol to look for
*       nNameLen - pointer to complete symbol name length
*   Returns:
*       TRUE if a symbol is found, pType1 is stored, pNameLen may be adjusted
*       FALSE if a symbol is not found
*
******************************************************************************/
BOOL FindModuleSymbol(TExItem *item, char *pName, int nNameLen)
{
    char *pSymName;                     // Pointer to a symbol name portion
    UINT count;                         // Symbol loop counter
    struct module_symbol* pSym;         // Pointer to a module symbol structure
    TMODULE Mod;                        // Current module internal structure


    // Find the symbol name portion of the input name
    if( (pSymName = memchr(pName, '!', nNameLen)) )
    {
        // Find the module that corresponds to the given module name
        if( !FindModule(&Mod, pName, pSymName-pName) )
            return( FALSE );

        // We got the module, look for the name
        pSymName++;                     // Skip the '!' delimiter
        nNameLen -= pSymName-pName;     // Adjust the name length to the symbol len

        // We got the right pointer to a module, find the symbol
        pSym = Mod.syms;

        for(count=0; count<Mod.nsyms; count++)
        {
            if( pSym->name[nNameLen]=='\0' && !strnicmp(pSym->name, pSymName, nNameLen) )
            {
                // Found the matching symbol name

                item->Data  = pSym->value;
        FoundValue:
                item->bType = EXTYPE_LITERAL;
                item->pData = (BYTE*) &item->Data;

                memcpy(&item->Type, &TypeUnsignedInt, sizeof(TSYMTYPEDEF1));

                return( TRUE );
            }

            pSym++;
        }

        // Two special cases of a symbol name that are really not exported,
        // but we still like to decode them are: init_module and cleanup_module
        // Still, we do it after all other symbol search fails...

        if( nNameLen==11 && !strnicmp(pSymName, "init_module", 11) )
        {
            // Store the value of that symbol from the module structure
            item->Data  = (DWORD) Mod.init;
            goto FoundValue;
        }

        if( nNameLen==14 && !strnicmp(pSymName, "cleanup_module", 14) )
        {
            // Store the value of that symbol from the module structure
            item->Data  = (DWORD) Mod.cleanup;
            goto FoundValue;
        }
    }

    return( FALSE );
}

/******************************************************************************
*                                                                             *
*   BOOL FindSymbol(TExItem *item, char *pName, int *pNameLen)                *
*                                                                             *
*******************************************************************************
*
*   Search for the symbol value and type of a given symbol name.
*
*   Where:
*       item - item descriptor to store the final value
*       pName - name of the symbol to look for
*       pNameLen - input: pointer to symbol name length
*                  output: if the symbol is longer (kernel!symbol format), the
*                          name legth is corrected
*   Returns:
*       TRUE if a symbol is found, pType1 is stored, pNameLen may be adjusted
*       FALSE if a symbol is not found
*
******************************************************************************/
BOOL FindSymbol(TExItem *item, char *pName, int *pNameLen)
{
    static char buf[MAX_STRING];        // Temp buffer for the symbol name
    int nNameLen;                       // Symbol name length

    // Basic parameter validation
    if( item && pNameLen )
    {
        nNameLen = *pNameLen;           // Get the symbol name length

        // Look for the module!symbol format
        if( *(pName+nNameLen)=='!' )
        {
            // The format is one of a module/export

            // Calculate the complete length past the "!" delimiter
            // Also add 1 to account for the "!"
            nNameLen += GetTokenLen(pName+nNameLen+1) + 1;

            if( FindModuleSymbol(item, pName, nNameLen) )
            {
                // Store the new name length and exit
                *pNameLen = nNameLen;

                return( TRUE );
            }
        }
        else
        {
            // The format is one of a standalone symbol name

            // Search the local symbols within the current function context
            // This also searche
            if( FindLocalSymbol(item, pName, nNameLen) )
                return( TRUE );

            // Search the file static symbols
            if( FindStaticSymbol(item, pName, nNameLen) )
                return( TRUE );

            // Search the global symbols
            if( FindGlobalSymbol(item, pName, nNameLen) )
                return( TRUE );

            // If everything else fails, look for the kernel export
            // For that, we will need to compose a temporary name
            nNameLen = sprintf(buf, "kernel!%s", pName);

            if( FindModuleSymbol(item, buf, nNameLen) )
                return( TRUE );
        }
    }

    return( FALSE );
}


/******************************************************************************
*                                                                             *
*   BOOL cmdExport(char *args, int subClass)                                  *
*                                                                             *
*******************************************************************************
*
*   Display exported symbols from a kernel module (or a kernel itself).
*
*   Where:
*       args - different ways to limit the list:
*              1) No arguments - list all exports from the kernel
*              2) [module_name] - list all exports from that module
*              3) [module_name]![partial-symbol] - list all matching symbols
*
******************************************************************************/
BOOL cmdExport(char *args, int subClass)
{
    TMODULE Mod;                        // Current module internal structure
    UINT count;                         // Symbol loop counter
    struct module_symbol* pSym;         // Pointer to a module symbol structure
    char *pSymName;                     // Pointer to a symbol name portion
    int nNameLen;                       // Total name length
    int nLine = 1;                      // Standard print counter

    // If no argument was given, assume kernel module
    if( !*args )
        args = "kernel";

    // Common case - find the optional '!' delimiter
    nNameLen = strlen(args);

    // Find the symbol name portion of the input name
    pSymName = memchr(args, '!', nNameLen);

    if( pSymName )
    {
        // Find the specified module name
        if( !FindModule(&Mod, args, pSymName-args) )
            return( TRUE );         // Could not find the module with that name

        // Found the delimiter, so we have the case (3)
        pSymName++;                     // Past the '!' symbol delimiter
        nNameLen -= pSymName - args;    // This is the length of the symbol name only
    }
    else
    {
        // No ! delimiter, but the module name is specified
        if( !FindModule(&Mod, args, nNameLen) )
            return( TRUE );         // Could not find the module with that name

        pSymName = "";              // List all exports
    }

    // Find the matching module for which we specified the name
    // pSymName points to the partial symbol name
    // List all symbols that match that partial name
    pSym = Mod.syms;

    // Print the module name in different color
    if( dprinth(nLine++, "%c%c%s", DP_SETCOLINDEX, COL_BOLD, Mod.name)==FALSE )
        return( TRUE );

    for(count=0; count<Mod.nsyms; count++)
    {
        if( !strnicmp(pSym->name, pSymName, strlen(pSymName)) )
        {
            if( dprinth(nLine++, "    %08X  %s", pSym->value, pSym->name)==FALSE )
                return( TRUE );
        }

        pSym++;
    }

    return( TRUE );
}

/******************************************************************************
*                                                                             *
*   BOOL cmdSymbol(char *args, int subClass)                                  *
*                                                                             *
*******************************************************************************
*
*   Display global symbols from the current symbol table
*
******************************************************************************/
BOOL cmdSymbol(char *args, int subClass)
{
    TSYMGLOBAL *pGlobals;
    int nLine = 1;
    UINT i;

    if( deb.pSymTabCur )
    {
        if( *args )
        {
            ;
        }
        else
        {
            // If no arguments, list all symbols:
            // Symbols are: Globals + static symbols from all files

            pGlobals = (TSYMGLOBAL *) SymTabFindSection(deb.pSymTabCur, HTYPE_GLOBALS);
            if( pGlobals )
            {
                // List the symbols from every segment

                for(i=0; i<pGlobals->nGlobals; i++ )
                {
                    if(dprinth(nLine++, " %08X %02d %s %s",
                        pGlobals->list[i].dwStartAddress,
                        pGlobals->list[i].bSegment,
                        pGlobals->list[i].bSegment==0x00? ".text  ":
                        pGlobals->list[i].bSegment==0x01? ".data  ":
                        pGlobals->list[i].bSegment==0x02? ".rodata":
                        pGlobals->list[i].bSegment==0x03? ".bss   ": "COMMON ",
                        pGlobals->list[i].pName)==FALSE)
                    return( TRUE );
                }
            }
        }
    }

    return( TRUE );
}


/******************************************************************************
*                                                                             *
*   BOOL cmdFile(char *args, int subClass)                                    *
*                                                                             *
*******************************************************************************
*
*   Change or display the current source file.
*
*   Symtax:
*       FILE *          - lists all symbol files in the current symbol table
*       FILE file-name  - switches to a given source file
*       FILE            - displays the name of the current source file
*
******************************************************************************/
BOOL cmdFile(char *args, int subClass)
{
    TSYMHEADER *pHead;                  // Generic section header
    TSYMSOURCE *pSrc;                   // Source file header
//  WORD fileID;
    int nLine = 1;

    if( deb.pSymTabCur )
    {
        if( *args )
        {
            if( *args=='*' )
            {
                // Print the current source file name

                pHead = deb.pSymTabCur->header;

                while( pHead->hType != HTYPE__END )
                {
                    if( pHead->hType == HTYPE_SOURCE )
                    {
                        pSrc = (TSYMSOURCE *)pHead;

                        // Print the source file path/name
                        if( (dprinth(nLine++, "%s", pSrc->pSourcePath))==FALSE )
                            break;
                    }

                    pHead = (TSYMHEADER*)((DWORD)pHead + pHead->dwSize);
                }
            }
            else
            {
                // Switch to a different source file

                // Loop over all the source files and compare to what we typed
                pHead = deb.pSymTabCur->header;

                while( pHead->hType != HTYPE__END )
                {
                    if( pHead->hType == HTYPE_SOURCE )
                    {
                        pSrc = (TSYMSOURCE *)pHead;

                        if( !strcmp(args, pSrc->pSourceName) )
                        {
                            deb.pSource = pSrc;             // New source descriptor
                            deb.codeFileTopLine = 1;        // Display at the first source line
                            deb.codeFileXoffset = 0;        // Reset the X offset

                            RecalculateDrawWindows();

                            return( TRUE );
                        }
                    }

                    pHead = (TSYMHEADER*)((DWORD)pHead + pHead->dwSize);
                }

                dprinth(nLine++, "Source file '%s' not found", args);
            }
        }
        else
        {
            // No arguments given - display the current source file name

            if( deb.pSource )
            {
                dprinth(nLine++, "%s", deb.pSource->pSourcePath);
            }
        }
    }
    else
        dprinth(1, "No symbol table loaded.");

    return( TRUE );
}

/******************************************************************************
*                                                                             *
*   char *SymAddress2Name(WORD wSel, DWORD dwOffset)                          *
*                                                                             *
*******************************************************************************
*
*   Translates address to a symbol name
*
*   Where:
*       wSel is the selector part of the address
*       dwOffset is the offset part of the address
*   Returns:
*       NULL if there are no symbols at that address
*       Address of the symbol name string
*
******************************************************************************/
char *SymAddress2Name(WORD wSel, DWORD dwOffset)
{
    void *pmodule;                      // Kernel pmodule pointer
    struct module_symbol* pSym;         // Pointer to a module symbol structure
    TMODULE Mod;                        // Current module internal structure
    int count;                          // Symbol loop counter

    // Storage for the return string
    static char sName[MAX_MODULE_NAME+1+MAX_SYMBOL_LEN+1];

    // Search the current symbol table first, if loaded
    if( deb.pSymTabCur )
    {
        ;
    }

    // Search kernel symbols and symbols exported by each module (ignore NULL)
    if( wSel==GetKernelCS() && dwOffset )
    {
        // Get the pointer to the module structure (internal)
        pmodule = ice_get_module(NULL, &Mod);

        while( pmodule )
        {
            // Check for the init_module and cleanup_module special cases
            if( dwOffset==(DWORD) Mod.init )
            {
                // Copy the module name and append a bang and init_module name
                sprintf(sName, "%s!init_module", Mod.name);

                return( sName );
            }

            if( dwOffset==(DWORD) Mod.cleanup )
            {
                // Copy the module name and append a bang and cleanup_module name
                sprintf(sName, "%s!cleanup_module", Mod.name);

                return( sName );
            }

            // Get the number of symbols exported by that module
            pSym = Mod.syms;

            for(count=0; count<Mod.nsyms; count++)
            {
                // TODO: Check the validity of these pointers before using them!

                if( pSym->value==dwOffset )
                {
                    // Found the matching symbol! Form its name and return. If a
                    // module name is found, print it, otherwise it is a kernel

                    sprintf(sName, "%s!%s", *Mod.name? Mod.name : "kernel", pSym->name);

                    return( sName );
                }

                pSym++;
            }

            // Get the next module in the linked list
            pmodule = ice_get_module(pmodule, &Mod);
        }
    }

    return(NULL);
}


/******************************************************************************
*                                                                             *
*   DWORD SymLinNum2Address(DWORD line)                                       *
*                                                                             *
*******************************************************************************
*
*   Returns the offset of the code that corresponds to the given source line
*   number in the current source file. (Used with dot-line_number expression).
*
*   Where:
*       line is the line number
*   Returns:
*       address of the code that corresponds to that line number
*       0 if no address found, no source loaded, etc.
*
******************************************************************************/
DWORD SymLinNum2Address(DWORD line)
{
    TSYMHEADER *pHead;                  // Generic section header
    TSYMFNLIN *pFnLin;
    WORD n, file_id;

    if( deb.pSymTabCur && deb.pSource )
    {
        pHead = deb.pSymTabCur->header;
        file_id = deb.pSource->file_id;

        while( pHead->hType != HTYPE__END )
        {
            // Scan all function line records to find our source/line number
            if( pHead->hType == HTYPE_FUNCTION_LINES )
            {
                pFnLin = (TSYMFNLIN *)pHead;

                for(n=0; n<pFnLin->nLines; n++)
                {
                    if( pFnLin->list[n].line==line && pFnLin->list[n].file_id==file_id )
                    {
                        return( pFnLin->dwStartAddress + pFnLin->list[n].offset );
                    }
                }
            }

            pHead = (TSYMHEADER*)((DWORD)pHead + pHead->dwSize);
        }
    }

    return( 0 );
}


/******************************************************************************
*                                                                             *
*   char *SymAddress2FunctionName(WORD wSel, DWORD dwOffset)                  *
*                                                                             *
*******************************************************************************
*
*   Returns the ASCIIZ name of the function at the given address
*
*   Where:
*       wSel is the selector part of the address
*       dwOffset is the offset part of the address
*   Returns:
*       NULL is no function can be found at that address
*       Function name string
*
******************************************************************************/
char *SymAddress2FunctionName(WORD wSel, DWORD dwOffset)
{
    TSYMGLOBAL *pGlobals;
    int i;
    char *pName;

    if( deb.pSymTabCur )
    {
        // Search global functions
        pGlobals = (TSYMGLOBAL *)SymTabFindSection(deb.pSymTabCur, HTYPE_GLOBALS);

        for(i=0; i<pGlobals->nGlobals; i++ )
        {
            if( pGlobals->list[i].dwStartAddress==dwOffset )
                return( pGlobals->list[i].pName );
        }
    }

    pName = SymAddress2Name(wSel, dwOffset);
    if( pName )
        return( pName );

    return( NULL );
}

/******************************************************************************
*                                                                             *
*   char *SymFnScope2Local(TSYMFNSCOPE *pFnScope, DWORD ebpOffset)            *
*                                                                             *
*******************************************************************************
*
*   Returns the name of the local variable or argument addressed using the
*   EBP register within a given function scope.
*
*   Note: Since this function is used only to get the variable name, and not
*         the type information, all type info will be stripped from the name
*
*   Where:
*       pFnScope is the function scope
*       ebpOffset is the offset used in the addressing
*   Returns:
*       Variable name
*       NULL
*
******************************************************************************/

// TODO: Local register variables + dont search all, but only until the current eip fn offset

char *SymFnScope2Local(TSYMFNSCOPE *pFnScope, DWORD ebpOffset)
{
    int i;
    char *pVar, *pType;
    static char sVar[MAX_SYMBOL_LEN+1]; // Temp store of the variable full name

    if( pFnScope )
    {
        // Traverse the function scope tokens and search for the ebp offset
        for(i=0; i<pFnScope->nTokens; i++ )
        {
            if( pFnScope->list[i].TokType==TOKTYPE_PARAM || pFnScope->list[i].TokType==TOKTYPE_LSYM )
            {
                if( pFnScope->list[i].param==ebpOffset )
                {
                    pVar = pFnScope->list[i].pName;

                    // Since that variable name contains the type description, we
                    // need to copy only the name portion

                    pType = strchr(pVar, ':');

                    if( pType )
                        i = MIN(pType-pVar, MAX_SYMBOL_LEN-1);
                    else
                        // This is just in case that we did not find the type delimiter ':'
                        i = MIN(strlen(pVar), MAX_SYMBOL_LEN-1);

                    // Copy the variable name portion of its definition
                    strncpy(sVar, pVar, i);
                    sVar[i] = 0;

                    return( sVar );
                }
            }
        }
    }

    return( NULL );
}

/******************************************************************************
*                                                                             *
*   BOOL cmdWhat(char *args, int subClass)                                    *
*                                                                             *
*******************************************************************************
*
*   For a given address, returns all symbol information it could find
*
******************************************************************************/
BOOL cmdWhat(char *args, int subClass)
{
    TADDRDESC Addr;                     // Address given
    char *pSymName;                     // Symbol that was found

    if( *args!=0 )
    {
        // Argument present: assume code selector
        evalSel = GetKernelCS();
        if( Expression(&Addr.offset, args, &args)  )
        {
            Addr.sel = evalSel;

            pSymName = SymAddress2Name(Addr.sel, Addr.offset);

            if( pSymName )
                dprinth(1, "%s", pSymName);
        }
    }
    else
    {
        // No arguments - Really, nothing can match that
        dprinth(1, "Nothing.");
    }

    return( TRUE );
}

