/******************************************************************************
*                                                                             *
*   Module:     malloc.c                                                      *
*                                                                             *
*   Date:       02/26/96                                                      *
*                                                                             *
*   Copyright (c) 1996-2005 Goran Devic                                       *
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

        This module contains functions for memory allocation on a specific
        heap.

        Prior to using a specific heap, that linear memory has to be
        initialized by calling Init_Alloc().

*******************************************************************************
*                                                                             *
*   Major changes:                                                            *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 02/26/96   Initial version                                      Goran Devic *
* 09/08/97   New Init_Alloc                                       Goran Devic *
* 09/08/00   Modified for Linice                                  Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include "module-header.h"              // Include types commonly defined for a module

#include "clib.h"                       // Include C library header file
#include "iceface.h"                    // Include iceface module stub protos
#include "ice.h"                        // Include main debugger structures

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

#define MALLOC_COOKIE   0xABCD9876      // Safety signature

typedef struct                          // Free node structure
{
    DWORD size;                         // Size of a free block
    struct Tmalloc *next;               // Pointer to the next free block

} Tmalloc;


// Access macros

#define TM              (Tmalloc *)
#define STM             (struct Tmalloc *)

#define HEADER_SIZE     sizeof(Tmalloc) // Define header size constant


/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

/******************************************************************************
*                                                                             *
*   void DumpHeap(BYTE *pHeap)                                                *
*                                                                             *
*******************************************************************************
*
*   Dumps the free list of a heap nodes. This is used for debugging of the heap
*   allocations.
*
*   Where:
*       pHeap is the requested heap
*
******************************************************************************/
void DumpHeap(BYTE *pHeap)
{
    Tmalloc *p;

    // Traverse the free list and dump it
    p = TM(pHeap);

    while( p )
    {
        dprinth(1, "%08X  %X (%d)", (DWORD)p, p->size, p->size);

        p = TM(p->next);
    }
}

/******************************************************************************
*                                                                             *
*   BYTE * _Init_Alloc( BYTE *pRamStart, DWORD dwRamSize )                    *
*                                                                             *
*******************************************************************************
*
*   This function initializes a buffer for the internal memory allocation pool.
*
*   Where:
*       pRamStart - beginning address of the linear buffer memory
*       dwRamTop  - size in bytes of the buffer memory
*
*   Returns:
*       Memory handle to be passed to subsequent mallocHeap() and freeHeap()
*               functions. That is the "heap".
*       This is actually the starting address pRamStart.
*
*       NULL - memory could not be initialized (invalid size/address)
*
******************************************************************************/
static BYTE * _Init_Alloc( BYTE *pRamStart, DWORD dwRamSize )
{
    Tmalloc *pMalloc;
    BYTE * pFree;

    // Some sanity checking

    if( dwRamSize < 32 )
        return( NULL );

    // Set the dummy free structure at the beginning of the free block to
    // easily traverse the linked list (a sentinel)

    pMalloc = TM(pRamStart);            // Get the buffer start
    pFree = (char*)pMalloc;             // Set the free list beginning

    pMalloc->size = 0;                  // No one can request that much!
    pMalloc->next = STM(pMalloc + 1);   // Next structure immediately follows

    pMalloc = TM(pMalloc->next);        // Next free block header
    pMalloc->size = dwRamSize - HEADER_SIZE; // That's how much is really free
    pMalloc->next = NULL;               // Last block in the list

    // Return the address of a heap that is now initialized

    return( pFree );
}

/******************************************************************************
*                                                                             *
*   BYTE *memInitHeap(UINT size)                                              *
*                                                                             *
*******************************************************************************
*
*   Allocated and initializes a local memory heap.
*
*   Where:
*       size is the requested size of the heap
*
*   Returns:
*       Memory handle to be passed to subsequent mallocHeap() and freeHeap()
*               functions. That is the "heap".
*       NULL - memory could not be initialized
*
******************************************************************************/
BYTE *memInitHeap(UINT size)
{
    BYTE *pHeap;

    // Allocate memory from the kernel non-paged pool

    pHeap = ice_vmalloc(size);
    if( pHeap != NULL )
    {
        // Initialize our new heap
        pHeap = _Init_Alloc(pHeap, size);
    }

    return( pHeap );
}

/******************************************************************************
*                                                                             *
*   void memFreeHeap(BYTE *pHeap)                                             *
*                                                                             *
*******************************************************************************
*
*   Frees the local memory heap.
*
*   Where:
*       pHeap is the handle of the heap, returned by memInitHeap
*
******************************************************************************/
void memFreeHeap(BYTE *hHeap)
{
    ice_vfree((char *) hHeap);
}

/******************************************************************************
*                                                                             *
*   char *mallocHeap(BYTE *pHeap, UINT size)                                  *
*                                                                             *
*******************************************************************************
*
*   Allocates a block of memory within the Linice internal heap.
*
*   Where:
*       pHeap is the requested heap
*       size is the requested size of the memory block
*
*   Returns:
*       Pointer to newly allocated block
*       NULL - memory could not be allocated
*
******************************************************************************/
char *mallocHeap(BYTE *pHeap, UINT size)
{
    Tmalloc *pLast;
    Tmalloc *pNew;

    // This should really be some sort of assert...

    if( pHeap == NULL )
        return(NULL);

    // If the requested size is 0, do nothing.

    if( (size == 0) )
        return NULL;

    // Set the size to be a multiple of 4 to keep the allignemnt
    // Also, add the size of the header to be allocated

    size = ((size+3) & 0xFFFFFFFC) + HEADER_SIZE;

    // Debug allocations:
    //dprinth(1, "malloc(%d)", size);

    // Traverse the free list and find the first block large enough for the
    // block of the requested size

    pLast = TM(pHeap);
    pNew  = TM(pLast->next);

    // This effectively implements the first-fit memory allocation strategy

    while( (pNew != NULL) && (pNew->size < size ))
    {
        pLast = pNew;
        pNew  = TM(pNew->next);
    }


    // Check if we could not find the block large enough and are at the end of
    // the list

    if( pNew==NULL )
        return( NULL );

    // A free memory block that was found is large enough for the request, but
    // maybe too large, so we may want to split it:

    // Two things can happen now: either we will link another free block
    // at the end of the allocated one, or not.  Linking another block will
    // increase fragmentation so we will do it only if the space that remains
    // is larger or equal to 16 bytes (arbitrary value)

    if( pNew->size >= size+16+HEADER_SIZE )
    {
         // Link in another free block

         pLast->next = (struct Tmalloc*)((int)pNew + size);
         pLast = TM(pLast->next);               // Point to the new free block

         pLast->next = pNew->next;              // Link in the next free block
         pLast->size = pNew->size - size;

         pNew->size = size;                     // Set the allocated block size
         pNew->next = STM(MALLOC_COOKIE);       // And the debug cookie

         return (void*)((int)pNew + HEADER_SIZE);
    }
    else
    {
         // There was not enough free space to link in new free node, so just
         // allocate the whole block.

         pLast->next = pNew->next;              // Skip the newly found space

         // pNew->size is all the size of a block. No need to change.

         pNew->next = STM(MALLOC_COOKIE);       // Set the debug cookie

         return (void*)((int)pNew + HEADER_SIZE);
    }
}

/******************************************************************************
*                                                                             *
*   void freeHeap(BYTE *pHeap, void *mPtr )                                   *
*                                                                             *
*******************************************************************************
*
*   Frees the memory that was allocated using mallocHeap() from the internal
*   memory allocation pool (heap).
*
*   The pointer mPtr can be NULL.
*
*   Where:
*       pHeap is the requested heap
*       pMem - pointer to a memory block to be freed
*
******************************************************************************/
void freeHeap(BYTE *pHeap, void *mPtr )
{
    Tmalloc *pLast;
    Tmalloc *pMem;
    Tmalloc *pMalloc;


    // Return if pointer is NULL (should not happen)

    if( mPtr==NULL )
        return;

    // Get the allocation structure

    pMalloc = (Tmalloc*)((int)mPtr - HEADER_SIZE);


    // Check for the magic number to ensure that the right block was passed

    if( (int)pMalloc->next != MALLOC_COOKIE )
    {
        // Should print some error message in the future
        //printf(" *** ERROR - Magic Number Wrong: %08X ***\n",(int)pMalloc->next );

        return;
    }

    // Now we have to return the block to the list of free blocks, so find the
    // place in the list to insert it.  The free list is ordered by the address
    // of the blocks that it holds

    pLast = TM(pHeap);
    pMem  = TM(pLast->next);

    // Traverse the free list and find where the new block should be inserted

    while( (pMem != NULL) && (pMem < pMalloc) )
    {
        pLast = pMem;
        pMem  = TM(pMem->next);
    }


    // If pMem is NULL, the block to be freed lies after the last node in the
    // free list, so link it at the end.

    if( pMem == NULL )
    {
        pLast->next = STM(pMalloc); // Last node in the free list
        pMalloc->next = NULL;       // Terminate it

        // The new, last free block may be merged with the preceeding one

        if( (int)pLast + pLast->size == (int)pMalloc )
        {
            pLast->size += pMalloc->size;
            pLast->next = NULL;
        }

        return;
    }


    // Now pLast points to the last node before pMalloc, and pMem points to
    // the next node.  They just have to be linked now.

    pLast->next = STM(pMalloc);
    pMalloc->next = STM(pMem);


    // If pMem node is immediately after pMalloc, they will be merged.

    if( (int)pMalloc + pMalloc->size == (int)pMem )
    {
        // Merge new node and the successor pointed by pMem

        pMalloc->size += pMem->size;
        pMalloc->next = pMem->next;
    }


    // If the newly freed node is after another free node, merge them

    if( (int)pLast + pLast->size == (int)pMalloc )
    {
        pLast->size += pMalloc->size;
        pLast->next = pMalloc->next;
    }

    return;
}


/******************************************************************************
*                                                                             *
*   int _Alloc_Check( BYTE *pHeap, DWORD dwInitSize )                         *
*                                                                             *
*******************************************************************************
*
*   This function traverses the memory structures and checks if the allocation
#   links are in order.  It also adds up the free (availble) memory to be
#   allocated.
*
*   Where:
*       pHeap - handle of a heap as returned by Init_Alloc()
#       dwInitSize - initially requested memory size in bytes
*
*   Returns:
*       Positive number - available memory in bytes
#       Negative number - data structure memory check error:
#           -1  - pHeap is NULL
#           -2  - init size parameter is too small
#           -3  - pointer out of bounds
#           -4  - size out of bounds
#
******************************************************************************/
#if 0
static int _Alloc_Check( BYTE *pHeap, DWORD dwInitSize )
{
    Tmalloc *pLast;
    Tmalloc *pNew;
    BYTE *pEnd;
    int nFree = 0;


    // Check the pointer to heap

    if( pHeap == NULL )  return( -1 );

    // Check the size

    if( dwInitSize < 32 )  return( -2 );

    pEnd = pHeap + dwInitSize;

    pLast = TM(pHeap);
    pNew  = TM(pLast->next);

    if( ((BYTE *)pNew < pHeap) || ((BYTE *)pNew > pEnd) )  return( -3 );

    // Traverse the list and check all the nodes

    while( pNew != NULL )
    {
        if( (pNew->size < 4) || (pNew->size + (BYTE *)pNew > pEnd) ) return( -4 );

        // Add the free size

        nFree += pNew->size;

        pLast = pNew;
        pNew  = TM(pNew->next);

        if( pNew != NULL )
            if( ((BYTE *)pNew < pHeap) || ((BYTE *)pNew >= pEnd) )  return( -3 );
    }

    return( nFree );
}
#endif

/******************************************************************************
*                                                                             *
*   int _ReAlloc( BYTE * pHeap, void * ptr, size_t new_size )                 *
*                                                                             *
*******************************************************************************
*
*   This function changes the size of an allocated memory block.  If the new
#   size is smaller than the old one, it frees some memory from the end of
#   a block (if the difference is reasonably significant).  If the new size
#   is larger than the old size and there is sufficient free space left after
#   a memory block, this function will change the size.  It will fail if there
#   is not enough free space immediately after the block.  It will not
#   change the base pointer or move a block.
*
*   Where:
*       pHeap - handle of a heap as returned by Init_Alloc()
#       ptr - pointer to a memory block to be resized, this pointer should be
#             the one returned by kMalloc()
#       new_size - the new size in bytes
*
*   Returns:
*       ptr - pointer to the memory block (same as argument ptr)
#       NULL if the block cannot be resized
*
******************************************************************************/
#if 0   // ==============   WORK IN PROGRESS   ==============
        // Since `96  :-P
static int _ReAlloc( BYTE * pHeap, void * ptr, size_t new_size )
{
    Tmalloc *pLast;
    Tmalloc *pMem;
    Tmalloc *pMalloc;


    // Return if anything is NULL (should not happen)

    if( (pHeap==NULL) || (ptr==NULL) || (new_size==0) )
        return( NULL );

    // Check that the ptr pointer is valid

    pMalloc = (Tmalloc*)((int)mPtr - HEADER_SIZE);

    // Check for the magic number to ensure that the right block was passed

    if( (int)pMalloc->next != MALLOC_COOKIE )
    {
        /* Should print some error message in the future                       */
        //printf(" *** ERROR - Magic Number Wrong: %08X ***\n",(int)pMalloc->next );

        return NULL;
    }

    // We need to find the last free node before this one

    pLast = TM(pHeap);
    pMem  = TM(pLast->next);

    // Traverse the free list and find the last node before the `ptr'-one

    while( (pMem != NULL) && (pMem < pMalloc) )
    {
        pLast = pMem;
        pMem  = TM(pMem->next);
    }

    // If the new size if smaller, this function always succeeds

    if( new_size < pMalloc->size )
    {
        return( ptr );
    }
    else
        if( new_size > pMalloc->size )
        {
            // If the new size is larger than the old size, check if there
            // is enough free space after this block

            return( NULL );
        }

    // Return ok

    return( ptr );
}

#endif
