/******************************************************************************
*                                                                             *
*   Module:     extension.c                                                   *
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

    This is a sample code using the debugger extension interface.

    It implements one dot command and two extensions to the expression
    evaluator.

        Dot command: TEST
        Optional parameters: an expression to evaluate

        Expression evaluator token: const
        Returns value: 0x12345678

        Expression evaluator token: cube()
        Returns value: cube of the parameter given in brackets

    The sample code also handles the notifications, but does nothing except
    prints their name.

*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/string.h>

MODULE_LICENSE("GPL");

#include <linux/version.h>

#include "LiniceExt.h"                  // Include the extension header file

MODULE_AUTHOR("Goran Devic");
MODULE_DESCRIPTION("Linice Extension Module");

/******************************************************************************
*                                                                             *
*   Local Defines, Variables and Macros                                       *
*                                                                             *
******************************************************************************/

// Extension structure - you may have more than one, each registering its own
// dot command. Each needs to be registered (and unregistered!) individually.
static TLINICEEXT ext;
static int fRegistered = 0;             // Flag that we are registered

/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

/******************************************************************************
*   int DotCommand(char *pCommand)                                            *
*******************************************************************************
*
*   This is the handler for the extension command.
*
******************************************************************************/
static int DotCommand(char *pCommand)
{
    TLINICEREGS *cpuRegs;               // Debugee current register state
    char key;                           // Key that we depressed
    char buf[80];                       // Temp line buffer
    int dump;                           // Temp dump buffer
    int result;                         // Result of an argument expression

    // If there was any expression specified as an argument, evaluate it
    // Note that the space that separates command from the rest of the arguments
    // will also appear in the pCommand string.
    if( *pCommand )
    {
        if( ext.Eval(&result, pCommand, NULL) )
            ext.dprint("Argument = %d", result);
        else
            ext.dprint("Error evaluating argument expression '%s'", pCommand);
    }

    // We will need to access debugee CPU state, so let's do it first
    cpuRegs = ext.GetRegs();

    do
    {
        ext.dprint("");                 // Empty line
        ext.dprint("Please select from the menu:");
        ext.dprint(" 0 .. Show CS:EIP");
        ext.dprint(" 1 .. Disassemble at CS:EIP");
        ext.dprint(" 2 .. Dump int at kernel DS:ESI");
        ext.dprint(" 3 .. Show GDT of CS");
        ext.dprint(" 9 .. Exit this menu");
        ext.dprint("");                 // Empty line

        key = ext.Getch(1);

        switch( key )
        {
            case '0':   // Show CS:EIP
                ext.dprint("CS:EIP = %04X:%08X", cpuRegs->cs, cpuRegs->eip);
            break;

            case '1':   // Disassemble at CS:EIP
                // If the given selector is 0, kernel CS will be used instead
                ext.Disasm(buf, cpuRegs->cs, cpuRegs->eip);
                ext.dprint("%s", buf);
            break;

            case '2':   // Dump 8 bytes at DS:ESI
                // Verify the address is accessible. If the given selector
                // is 0, kernel DS will be used instead.
                if( ext.MemVerify(0, cpuRegs->esi, 8) )
                {
                    // This will of course default to kernel DS.
                    // Since kernel DS can addresses 4Gb, that is not the problem
                    // in most of the cases when we need a memory access.
                    dump = *(int *)cpuRegs->esi;

                    ext.dprint("*esi = %08X", dump);
                }
                else
                    ext.dprint("*DS:ESI is not accessible!");
            break;

            case '3':   // Execute Linice command
                sprintf(buf, "gdt %x", cpuRegs->cs);
                ext.Execute(buf);
            break;
        }
    }
    while( key!='9' );

    return( 0 );
}

/******************************************************************************
*   void Notify(int Notification)                                             *
*******************************************************************************
*
*   This is the handler for the Linice notification callback.
*
******************************************************************************/
static void Notify(int Notification)
{
    // Various kinds of notifications
    switch( Notification )
    {
        case PEXT_NOTIFY_ENTER:
            ext.dprint("Notification: PEXT_NOTIFY_ENTER");
            break;

        case PEXT_NOTIFY_LEAVE:
            ext.dprint("Notification: PEXT_NOTIFY_LEAVE");
            break;
    }
}

/******************************************************************************
*   int QueryToken(int *pResult, char *pToken, int len)                       *
*******************************************************************************
*
*   This is the handler for the expression token extension.
*
******************************************************************************/
static int QueryToken(int *pResult, char *pToken, int len)
{
    int result;                         // Result of the evaluation
    char *pSubEnd;                      // Pointer to a subexpression end
    char buf[80];                       // Temp line buffer

    // This sample expression token is 'const'

    if( len==5 && !strncmp(pToken, "const", 5) )
    {
        // Return a magic value for our token
        *pResult = 0x12345678;          // Store the result
        return( 5 );                    // "Eat up" 5 characters
    }

    // The following code implements the 'cube()' function.
    // It shows how to parse a pair of parenthesis and call
    // expression evaluator recursively.
    // You can even do expressions like: ? cube(cube(2))

    if( len==4 && !strncmp(pToken, "cube(", 5) )
    {
        // Call the expression evaluator with the content of the parens
        // Note the last argument is the address of our expression pointer
        // variable, so it will be incremented as needed
        pToken = pToken + 5;

        // Copy the subexpression into a temp buffer; find the outer end
        pSubEnd = strrchr(pToken, ')');
        if( pSubEnd )
        {
            strncpy(buf, pToken, pSubEnd-pToken);
            buf[pSubEnd-pToken] = '\0';

            if( ext.Eval(&result, buf, NULL) )
            {
                // Calculate the cube and store it
                *pResult = result * result * result;

                // Return the total number of characterd that we recognized
                return( pSubEnd - pToken + 5 + 1);
            }
        }
    }

    // By default, we did not recognize the token, so return 0
    return( 0 );
}


/******************************************************************************
*   init_module()                                                             *
*******************************************************************************
*
*   Module init function called at module load.
*
******************************************************************************/
int init_module()
{
    int result;

    // Initialize our extension structure
    memset(&ext, 0, sizeof(TLINICEEXT));
    ext.version = LINICEEXTVERSION;
    ext.size    = LINICEEXTSIZE;

    // Provide the DOT name of the extension we want to implement
    ext.pDotName = "test";
    ext.pDotDescription = "Sample Linice extension code";

    // Fill up the function callbacks that we will provide
    ext.Command    = DotCommand;
    ext.Notify     = Notify;
    ext.QueryToken = QueryToken;

    // Call to register our extension
    result = LiniceRegisterExtension(&ext);

    // These are the possible return values from the register function
    switch( result )
    {
        case EXTREGISTER_OK:
            ext.dprint("Registration: EXTREGISTER_OK");
            fRegistered = 1;
            break;

        case EXTREGISTER_BADVERSION:
            break;

        case EXTREGISTER_BADHEADER:
            break;

        case EXTREGISTER_DUPLICATE:
            break;
    }

    // Return if we could not register it
    if( result!=EXTREGISTER_OK )
        return( 0 );

    // Using our dprint function - note we dont put "\n" at the end!
    ext.dprint("Hello from the sample extension code!");

    return( 0 );
}

/******************************************************************************
*   cleanup_module()                                                          *
*******************************************************************************
*
*   Module cleanup function called at module unload.
*
******************************************************************************/
void cleanup_module()
{
    // It is safe to refer to "ext" only if we were properly registered
    if( fRegistered )
    {
        // Unregister extension
        ext.dprint("Unregistering the sample extension.");

        LiniceUnregisterExtension(&ext);

        // After this point 'ext' contains no valid entries...
        // Linice redirected them to a safe handler just for your protection :)
    }
}

