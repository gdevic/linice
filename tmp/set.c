/******************************************************************************
*                                                                             *
*   Module:     Set.c                                                         *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       9/9/97                                                        *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************
.-
    Module Description:

    This module contains the code for the `Set' command of the debugger.
-.
*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     REV   DESCRIPTION OF CHANGES                         AUTHOR      *
* --------   ----  ---------------------------------------------  ----------- *
* 9/9/97     1.00  Original                                       Goran Devic *
* --------   ----  ---------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include "Intel.h"                      // MM needs this include

#include "MM.h"                         // Include memory management

#include "eval.h"                       // For SetGetInteger function

#include "set.h"                        // Include its own header

#include "string.h"                     // Include string functions

/******************************************************************************
*                                                                             *
*   Global Variables                                                          *
*                                                                             *
******************************************************************************/

TQueue qSet;                            // Linked list of set keys and values

/******************************************************************************
*                                                                             *
*   Local Defines, Variables and Macros                                       *
*                                                                             *
******************************************************************************/

typedef struct
{
    char * sKey;                        // Pointer to a string containing a key
    char * sVal;                        // Pointer to a string cont. a value

} TSet;                                 // Set structure


/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/


/******************************************************************************
*                                                                             *
*   int fnSetCmp( void * s1, const void *s2 )                                 *
*                                                                             *
*******************************************************************************
*
*   This is a compare function for the set string search on a key string.
*
******************************************************************************/
int fnSetCmp( void * s1, const void *s2 )
{
    return( !strcmpi( ((TSet *) s1)->sKey, s2 ) );
}



/******************************************************************************
*                                                                             *
*   BOOL SetValue( char * sKey, char * sValue )                               *
*                                                                             *
*******************************************************************************
*
*   Adds a value to a key.  If a key already exists, it replaces it with a
#   new value.  If the value is empty string, it deletes a key from the list.
*
*   Where:
*       sKey - string with a `set' key
#       sValue - string with a `set' value
*
*   Returns:
*
*
******************************************************************************/
BOOL SetValue( char * sKey, char * sValue )
{
    TSet * pSet;
    char * pNewValue;
    int  nKeyLen, nValLen;


    // If a key is empty, return

    if( sKey==NULL )
        return( FALSE );

    nKeyLen = strlen(sKey);

    if( nKeyLen==0 )
        return( FALSE );

    // If the sValue is empty, we will possibly delete a key.

    if( sValue==NULL )
        nValLen = 0;
    else
        nValLen = strlen(sValue);

    // Find the key using the key search

    if( QFind( &qSet, fnSetCmp, sKey ) )
    {
        // Get the address of a set structure of a key

        pSet = (TSet *) QCurrent( &qSet );

        // Key was found If the sValue is empty, delete that node from the list

        if( nValLen==0 )
        {
            _kFree( pMemDeb, pSet->sKey );
            _kFree( pMemDeb, pSet->sVal );
            _kFree( pMemDeb, pSet );

            // Free the current node in a queue manager

            QDelete( pMemDeb, &qSet );

            return( TRUE );
        }

        // Node already exists, and the new value is given. Allocate space
        // for it and replace the old value

        pNewValue = (char *) _kMalloc( pMemDeb, nValLen + 1 );

        // We could not allocate space for the new value

        if( pNewValue==NULL )
            return( FALSE );

        // Free the old value

        _kFree( pMemDeb, pSet->sVal );

        // Copy and link a new value

        strcpy( pNewValue, sValue );

        pSet->sVal = pNewValue;

        return( TRUE );
    }

    // Key was not found.  If the value is empty, ignore the command

    if( nValLen==0 )
        return( TRUE );

    // Add a key-value to a list
    // Allocate memory for a `Set' structure

    pSet = (TSet *) _kMalloc( pMemDeb, sizeof(TSet) );

    if( pSet!=NULL )
    {
        // Allocate memory for the key string

        pSet->sKey = (char *) _kMalloc( pMemDeb, strlen(sKey) + 1 );

        if( pSet->sKey!=NULL )
        {
            // Allocate memory for the value string

            pSet->sVal = (char *) _kMalloc( pMemDeb, strlen(sValue) + 1);

            if( pSet->sVal!=NULL )
            {
                // Allocate new node in a linked list

                if( QAdd( pMemDeb, &qSet, pSet) != 0 )
                {
                    // Copy key and the value into memory

                    strcpy( pSet->sKey, sKey );
                    strcpy( pSet->sVal, sValue );

                    // Successful add
                }
                else
                {
                    // Could not allocate new node for a linked list

                    _kFree( pMemDeb, pSet->sVal );
                    _kFree( pMemDeb, pSet->sKey );
                    _kFree( pMemDeb, pSet );

                    return( FALSE );
                }
            }
            else
            {
                // Memory for value could not be allocated

                _kFree( pMemDeb, pSet->sKey );
                _kFree( pMemDeb, pSet );

                return( FALSE );
            }
        }
        else
        {
            // Memory for key could not be allocated

            _kFree( pMemDeb, pSet );

            return( FALSE );
        }
    }
    else
    {
        // pSet could not be allocated

        return( FALSE );
    }

    return( TRUE );
}



/******************************************************************************
*                                                                             *
*   void SetPrintAll()                                                        *
*                                                                             *
*******************************************************************************
*
*   This function prints all key/value pairs
*
******************************************************************************/
void SetPrintAll()
{
    TSet * pSet;

    pSet = QFirst( &qSet );

    while( pSet != NULL )
    {
        // Print the key and the value

        dprintf("\n%s = %s", pSet->sKey, pSet->sVal );

        // Next node in a list

        pSet = QNext( &qSet );
    }
}



/******************************************************************************
*                                                                             *
*   char * SetGetValue( char *sKey )                                          *
*                                                                             *
*******************************************************************************
*
*   Looks for the key and returns a value associated with the key.  If the key
#   does not exist, returns NULL.
*
*   Where:
*       sKey - the string denoting the key
*
*   Returns:
*       Pointer to a value string
#       NULL if a key does not exist
*
******************************************************************************/
char * SetGetValue( char *sKey )
{
    TSet * pSet;

    // Find a key

    if( QFind( &qSet, fnSetCmp, sKey ) )
    {
        // Get the address of a set structure of a key

        pSet = (TSet *) QCurrent( &qSet );

        // Return the `value' string

        return( pSet->sVal );
    }

    // Key was not found.  Return NULL

    return( NULL );
}


/******************************************************************************
*                                                                             *
*   int SetGetInteger( char *sKey )                                           *
*                                                                             *
*******************************************************************************
*
*   Looks for the key and returns an integer value that is evaluated from
#   the key.  If the key does not exist, it returns 0.  To check if a key
#   exist, use SetGetValue and check for NULL.
*
*   Where:
*       sKey - the string denoting the key
*
*   Returns:
*       integer value associated with the key
#       0 if a key does not exist
*
******************************************************************************/
int SetGetInteger( char *sKey )
{
    TSet * pSet;

    // Find a key

    if( QFind( &qSet, fnSetCmp, sKey ) )
    {
        // Get the address of a set structure of a key

        pSet = (TSet *) QCurrent( &qSet );

        // Return the evaluated `value' string

        return( nEvaluate( pSet->sVal, NULL ) );
    }

    // Key was not found.  Return 0

    return( 0 );
}
