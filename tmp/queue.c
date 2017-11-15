/******************************************************************************
*                                                                             *
*   Module:     Queue.c                                                       *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       8/28/97                                                       *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************
.-
    Module Description:

    This module contains the code for the linked lists, stacks and queues
    management.  That includes support for sorted lists, sorting of lists and
    priority queuing.

    ----

    Passing a NULL pointer to any of the functions is illegal, they are
    not checked for performance reasons.  Use the function QCheck to check
    the consistency of the doubly linked list structures.  This function
    has no side effects and can detect very reliably if a list is damaged.

    Usage:

    Create Data Structure
    ---------------------

            q = QCreateAlloc()
            if q==NULL then Fail

                or

            TQueue q
            QCreate( &q )

    Destruct Data Structure
    -----------------------

            QDestroyAlloc( &q )

                or

            QDestroy( &q )

    Stack Operations
    ----------------

            fResult = QPush( &q, element )
            element = QPop( &q )
            fEmpty  = QIsEmpty( &q )
            element = QCurrent( &q )   (peek at the next one to pop)

    Linked List Operations
    ----------------------

            fResult = QInsert( &q, element )
            fResult = QAdd( &q, element )
            element = QDelete( &q )
            fEmpty  = QIsEmpty( &q )

            element = QFirst( &q )     (traverse the doubly linked list)
            element = QLast( &q )
            element = QNext( &q )
            element = QPrev( &q )
            element = QCurrent( &q )

    Queue Operations
    ---------------------

            fResult = QEnqueue( &q, element )
            element = QDequeue( &q )
            fEmpty  = QIsEmpty( &q )
            element = QCurrent( &q )   (peek at the next one to dequeue)

    Sorted List / Priority Queue Operations
    ---------------------------------------

            fResult = QSort( &q )

            fResult = QPriorityEnqueue( &q, fmCmp, element )
            element = QDequeue( &q )
            fEmpty  = QIsEmpty( &q )
            element = QCurrent( &q )   (peek at the next one to dequeue)

-.
*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     REV   DESCRIPTION OF CHANGES                         AUTHOR      *
* --------   ----  ---------------------------------------------  ----------- *
* 8/28/97    1.00  Original                                       Goran Devic *
* --------   ----  ---------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include <stdlib.h>

#include "Intel.h"                      // MM needs this include

#include "MM.h"                         // Include memory management

#include "Queue.h"                      // Include its own header structures

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

/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/


/******************************************************************************
*                                                                             *
*   int QCheck( TQueue *q )                                                   *
*                                                                             *
*******************************************************************************
*
*   This function check the queue structure.  It traverses all the nodes and
#   checks with certain probability that all the links are ok.
*
*   Where:
*       q - Queue descriptor
*
*   Returns:
*       0 if structure is ok
#       1 if the q pointer is NULL
#       2 size is 0, but pHead is not NULL
#       3 size is 0, but pTail is not NULL
#       4 size is 0, but pCur is not NULL
#       5 last node next pointer is not NULL
#       6 queue tail does not point to last node
#       7 linked list broken
#       8 back link of a linked list broken
#       9 pCur pointer is lost
*
******************************************************************************/
int QCheck( TQueue *q )
{
    TNode *pCur, *pPrev;
    int i, fFoundCur;

    // First, check the q pointer that is not NULL

    if( q == NULL )
        return( 1 );

    // If the size is 0, all the pointers should be NULL

    if( q->nSize == 0 )
    {
        if( q->pHead != NULL )
            return( 2 );
        if( q->pTail != NULL )
            return( 3 );
        if( q->pCur != NULL )
            return( 4 );

        return( 0 );
    }

    // There are some elements in the queue.  Traverse the list and check the
    // memory pointer consistency

    fFoundCur = 0;
    pPrev = NULL;
    pCur = q->pHead;

    for( i=q->nSize; i; i-- )
    {
        // Current should never be NULL

        if( pCur == NULL )
            return( 7 );

        // Previous shoud point to the previous node

        if( pCur->pPrev != pPrev )
            return( 8 );

        // Look for the pCur pointer

        if( q->pCur == pCur )
            fFoundCur++;

        pPrev = pCur;
        pCur = pCur->pNext;
    }

    // At the end of the list, next should be NULL and tail should point here

    if( pPrev->pNext != NULL )
        return( 5 );

    if( q->pTail != pPrev )
        return( 6 );

    // Did we found pCur pointer ?

    if( fFoundCur != 1 )
        return( 9 );

    // If we have reached this point, the structure is probably ok

    return( 0 );
}


/******************************************************************************
*                                                                             *
*   void QCreate( TQueue *q )                                                 *
*                                                                             *
*******************************************************************************
*
*   Initializes a queue descriptor.  Descriptor structure must already exist.
#   If it needs to be allocated, use function QCreateAlloc() instead.
*
*   Where:
*       q - Address of a TQueue structure, `Queue descriptor'
*
*   Returns:
*       void
*
******************************************************************************/
void QCreate( TQueue *q )
{
    q->pHead = q->pTail = q->pCur = NULL;
    q->nSize = 0;
}



/******************************************************************************
*                                                                             *
*   TQueue * QCreateAlloc( char * pHeap )                                     *
*                                                                             *
*******************************************************************************
*
*   Allocates and initializes a queue descriptor.
*
*   Where:
*       pHeap - pointer to a allocation heap
*
*   Returns:
*       Newly created queue descriptor.
#       NULL if the queue structure cannot be allocated.
*
******************************************************************************/
TQueue * QCreateAlloc( char * pHeap )
{
    TQueue * q;

    q = (TQueue *) _kMalloc( pHeap, sizeof(TQueue) );

    if( q != NULL )
    {
        q->pHead = q->pTail = q->pCur = NULL;
        q->nSize = 0;

    }

    return( q );
}



/******************************************************************************
*                                                                             *
*   void QDestroy( char * pHeap, TQueue *q )                                  *
*                                                                             *
*******************************************************************************
*
*   Destroys the list created by QCreate function.  All the nodes are freed,
#   but the TQueue structure q is not (since it was created by the user).
#
#   Note that any allocation associated with the elements of a queue is not
#   freed -- it is up to the user to traverse and free every element.
*
*   Where:
*       pHeap - pointer to a allocation heap
*       q - Queue descriptor initialized by QCreate function
*
*   Returns:
*       void
*
******************************************************************************/
void QDestroy( char * pHeap, TQueue *q )
{
    TNode *pCur;

    // Traverse the list and free all the nodes

    while( q->pHead != NULL )
    {
        // Clear the elements for additional safety

        q->pHead->pNext = q->pHead->pPrev = NULL;
        q->pHead->p = (void *) 0xBAD;

        // Traverse the list

        pCur = q->pHead->pNext;
        _kFree( pHeap, q->pHead );
        q->pHead = pCur;
    }

    q->pHead = q->pTail = q->pCur = NULL;
    q->nSize = 0;
}



/******************************************************************************
*                                                                             *
*   void QDestroyAlloc( char * pHeap, TQueue *q )                             *
*                                                                             *
*******************************************************************************
*
*   Destroys the list created by QCreateAlloc function.  All the nodes are
#   freed and the queue structure q is freed as well.
#
#   Note that any allocation associated with the elements of a queue is not
#   freed -- it is up to the user to traverse and free every element.
*
*   Where:
*       pHeap - pointer to a allocation heap
*       q - Queue descriptor returned from QCreateAlloc function
*
*   Returns:
*       void
*
******************************************************************************/
void QDestroyAlloc( char * pHeap, TQueue *q )
{
    QDestroy( pHeap, q );

    _kFree( pHeap, q );
}


/******************************************************************************
*                                                                             *
*   STACK FUNCTIONS                                                           *
*                                                                             *
******************************************************************************/


/******************************************************************************
*                                                                             *
*   int QPush( char * pHeap, TQueue * q, void * p )                           *
*                                                                             *
*******************************************************************************
*
#   STACK OPERATION: Push
#
*   Pushes an element at the top of the stack.  The current node points to
#   that new element on a stack.
*
*   Where:
*       pHeap - pointer to a allocation heap
*       q - Queue descriptor
#       p - element/pointer to an element
*
*   Returns:
#       0 if operation failed (cannot allocate memory for structure)
#       n, n>0 - if operation succeded (number of nodes)
*
******************************************************************************/
int QPush( char * pHeap, TQueue * q, void * p )
{
    TNode * pFirst;

    pFirst = _kMalloc( pHeap, sizeof(TNode) );

    if( pFirst == NULL )
        return( 0 );

    pFirst->pPrev = NULL;
    pFirst->pNext = q->pHead;

    if( q->pTail == NULL )
        q->pTail = pFirst;
    else
        q->pHead->pPrev = pFirst;

    // Link in the new node

    q->pHead = pFirst;

    // Set up the element

    pFirst->p = p;

    // Set the current node to the first one

    q->pCur = pFirst;

    return( ++ q->nSize );
}



/******************************************************************************
*                                                                             *
*   void * QPop( char * pHeap, TQueue * q )                                   *
*                                                                             *
*******************************************************************************
*
#   STACK OPERATION: Pop
*
#   Pops an element from the top of the list and deallocates its storage.
#   The current node is set to the next top node on a stack.
*
*   Where:
*       pHeap - pointer to a allocation heap
*       q - Queue descriptor
*
*   Returns:
*       Data value associated with the top element
#       NULL if the stack is empty
*
******************************************************************************/
void * QPop( char * pHeap, TQueue * q )
{
    TNode * pTop;
    void * p;

    if( q->pHead == NULL )
        return( NULL );

    pTop = q->pHead;
    p = q->pHead->p;

    if( -- q->nSize == 0 )
    {
        q->pHead = q->pTail = q->pCur = NULL;
    }
    else
    {
        q->pCur = q->pHead = q->pHead->pNext;
        q->pHead->pPrev = NULL;
    }

    _kFree( pHeap, pTop );

    return( p );
}


/******************************************************************************
*                                                                             *
*   LINKED LIST FUNCTIONS                                                     *
*                                                                             *
******************************************************************************/


/******************************************************************************
*                                                                             *
*   int QInsert( char * pHeap, TQueue * q, void * p )                         *
*                                                                             *
*******************************************************************************
*
*   Inserts an element in front of the current one.  By default, the
#   current position is the head of a list.  By using QFirst, QLast, QNext and
#   QPrev, the insert position can be moved anywhere in the linked list.
#
#   Note that this function cannot add element at the end of a list (if a
#   list is not empty).  Use QAdd function in order to add new element after
#   the current one.
*
#   After the new node is inserted, the current pointer is set to it.
#
*   Where:
*       pHeap - pointer to a allocation heap
*       q - Queue descriptor
#       p - element/pointer to an element
*
*   Returns:
#       0 if operation failed (cannot allocate memory for structure)
#       n, n>0 - if operation succeded (number of nodes)
*
******************************************************************************/
int QInsert( char * pHeap, TQueue * q, void * p )
{
    TNode * pNew;


    pNew = _kMalloc( pHeap, sizeof(TNode) );

    if( pNew == NULL )
        return( 0 );

    if( q->pHead == NULL )
    {
        // List is initially empty

        pNew->pNext = NULL;
        pNew->pPrev = NULL;

        q->pHead = q->pTail = pNew;
    }
    else
    {
        pNew->pNext = q->pCur;
        pNew->pPrev = q->pCur->pPrev;

        // Modify previous node to link new one after it

        if( q->pCur->pPrev != NULL )
        {
            q->pCur->pPrev->pNext = pNew;
        }
        else
        {
            // That was the first node... need to modify q

            q->pHead = pNew;
        }

        // Modify pCur node to insert new node in front of it

        q->pCur->pPrev = pNew;
    }

    pNew->p = p;

    q->pCur = pNew;

    return( ++ q->nSize );
}



/******************************************************************************
*                                                                             *
*   int QAdd( char * pHeap, TQueue * q, void * p )                            *
*                                                                             *
*******************************************************************************
*
*   Adds an element after the current one.  By default, the
#   current position is the head of a list.  By using QFirst, QLast, QNext and
#   QPrev, the insert position can be moved anywhere in the linked list.
#
#   Note that this function cannot add element at the start of the list (if a
#   list is not empty).  Use QInsert function in order to add new element in
#   front of the current one.
*
#   After a new node is added, the current pointer is set to it.
#
*   Where:
*       pHeap - pointer to a allocation heap
*       q - Queue descriptor
#       p - element/pointer to an element
*
*   Returns:
#       0 if operation failed (cannot allocate memory for structure)
#       n, n>0 - if operation succeded (number of nodes)
*
******************************************************************************/
int QAdd( char * pHeap, TQueue * q, void * p )
{
    TNode * pNew;


    pNew = _kMalloc( pHeap, sizeof(TNode) );

    if( pNew == NULL )
        return( 0 );

    if( q->pHead == NULL )
    {
        // List is initially empty

        pNew->pNext = NULL;
        pNew->pPrev = NULL;

        q->pHead = q->pTail = pNew;
    }
    else
    {
        pNew->pNext = q->pCur->pNext;
        pNew->pPrev = q->pCur;

        // Modify the next node to link new one before it

        if( q->pCur->pNext != NULL )
        {
            q->pCur->pNext->pPrev = pNew;
        }
        else
        {
            // That was the last node... need to modify q

            q->pTail = pNew;
        }

        // Modify pCur node to insert new node after it

        q->pCur->pNext = pNew;

    }

    pNew->p = p;

    q->pCur = pNew;

    return( ++ q->nSize );
}



/******************************************************************************
*                                                                             *
*   void * QDelete( char * pHeap, TQueue * q )                                *
*                                                                             *
*******************************************************************************
*
*   Deletes and frees the memory of the current node.  The current list pointer
#   is set to the node immediately following it (or preceding it if that was
#   the last node in a list).
*
*   Where:
*       pHeap - pointer to a allocation heap
*       q - Queue descriptor
*
*   Returns:
*       Data value associated with the element that is deleted
#       NULL if the list was empty
*
******************************************************************************/
void * QDelete( char * pHeap, TQueue * q )
{
    TNode * pFree;
    void * p;

    if( q->pCur == NULL )
        return( NULL );

    pFree = q->pCur;

    // Modify the node ahead of the current one to skip it

    if( pFree->pPrev != NULL )
    {
        pFree->pPrev->pNext = pFree->pNext;
    }
    else
    {
        // There were no nodes ahead... need to modify q

        q->pHead = pFree->pNext;
    }

    // Modify the node after the current one

    if( pFree->pNext != NULL )
    {
        pFree->pNext->pPrev = pFree->pPrev;
        q->pCur = pFree->pNext;
    }
    else
    {
        // There were no nodes after the current one... need to modify q

        q->pCur = q->pTail = pFree->pPrev;
    }

    // Finally, free the node

    q->nSize--;

    p = pFree->p;
    _kFree( pHeap, pFree );

    return( p );
}



/******************************************************************************
*                                                                             *
*   void * QFirst( TQueue * q )                                               *
*                                                                             *
*******************************************************************************
*
*   Returns the first element in a linked list and sets the list pointer
#   to it for other traversal functions.
*
*   Where:
*       q - Queue descriptor
*
*   Returns:
*       Data value associated with the first element
#       NULL if the list is empty
*
******************************************************************************/
void * QFirst( TQueue * q )
{
    if( q->nSize == 0 )
        return( NULL );

    q->pCur = q->pHead;

    return( q->pHead->p );
}



/******************************************************************************
*                                                                             *
*   void * QLast( TQueue * q )                                                *
*                                                                             *
*******************************************************************************
*
*   Returns the last element in a linked list and sets the list pointer
#   to it for other traversal functions.
*
*   Where:
*       q - Queue descriptor
*
*   Returns:
*       Data value associated with the last element
#       NULL if the list is empty
*
******************************************************************************/
void * QLast( TQueue * q )
{
    if( q->nSize == 0 )
        return( NULL );

    q->pCur = q->pTail;

    return( q->pTail->p );
}



/******************************************************************************
*                                                                             *
*   void * QCurrent( TQueue * q )                                             *
*                                                                             *
*******************************************************************************
*
*   Returns the current node value.
*
*   Where:
*       q - Queue descriptor
*
*   Returns:
*       Data value associated with the last element
#       NULL if the list is empty
*
******************************************************************************/
void * QCurrent( TQueue * q )
{
    if( q->pCur == NULL )
        return( NULL );

    return( q->pCur->p );
}



/******************************************************************************
*                                                                             *
*   int QIsEmpty( TQueue * q )                                                *
*                                                                             *
*******************************************************************************
*
*   This function returns `0' if the queue is empty and a number of nodes
#   in a queue if the queue is not empty.
*
*   Where:
*       q - Queue descriptor
*
*   Returns:
*       0 - if the queue is empty
#       n, n>0 - if the queue is not empty (number of nodes)
*
******************************************************************************/
int QIsEmpty( TQueue * q )
{
    return( q->nSize );
}



/******************************************************************************
*                                                                             *
*   void * QNext( TQueue * q )                                                *
*                                                                             *
*******************************************************************************
*
*   Returns the next element in the linked list.  Use QFirst function to
#   reset the list pointer to a first element.
*
*   Where:
*       q - Queue descriptor
*
*   Returns:
*       Data value associated with the current element
#       NULL if there are no more list elements or list was empty
*
******************************************************************************/
void * QNext( TQueue * q )
{
    if( q->pCur == NULL )
        return( NULL );

    if( q->pCur->pNext == NULL )
        return( NULL );

    q->pCur = q->pCur->pNext;

    return( q->pCur->p );
}



/******************************************************************************
*                                                                             *
*   void * QPrev( TQueue * q )                                                *
*                                                                             *
*******************************************************************************
*
*   Returns the previous element in the doubly linked list.  Use QLast
#   function to initially set the list pointer to the last element.
*
*   Where:
*       q - Queue descriptor
*
*   Returns:
*       Data value associated with the previous element
#       NULL if there are no more list elements or list was empty
*
******************************************************************************/
void * QPrev( TQueue * q )
{
    if( q->pCur == NULL )
        return( NULL );

    if( q->pCur->pPrev == NULL )
        return( NULL );

    q->pCur = q->pCur->pPrev;

    return( q->pCur->p );
}



/******************************************************************************
*                                                                             *
*   int QFind( TQueue * q,                                                    #
#              int fnCmp( void *p1, const void *p2), const void * p)          *
*                                                                             *
*******************************************************************************
*
*   Traverses the linked list q and looks for the element p by the means of
#   provided compare function fnCmp.  If that function returns True for any
#   visited node, True is returned and the current pointer is positioned at the
#   found element, use QCurrent to retrieve it.
*
#   If NULL is given as a compare function, elements will be compared as
#   integers on their pointer values `p'.
#
*   Where:
*       q - Queue descriptor
#       fnCmp - compare function:  (BOOL) p1 == p2 ?
#       p - element/pointer to an element that is passed as p2 argument to
#           the compare function.
*
*   Returns:
*       0 if search failed.  The current pointer is set to the last node.
#       1 if search succeeded.  Use QCurrent to retrieve it.
*
******************************************************************************/
int QFind( TQueue * q, int fnCmp( void *p1, const void *p2), const void * p)
{
    // Reset the current list head to the beginning of the list

    if( q->nSize == 0 )
        return( 0 );

    q->pCur = q->pHead;

    // Loop for all nodes

    while( q->pCur != NULL )
    {
        // If the compare function is NULL, use integer compare

        if( fnCmp==NULL )
        {
            if( (int) p == (int) q->pCur->p )
                return( 1 );
        }
        else
        {
            if( fnCmp( q->pCur->p, p ) )
                return( 1 );
        }

        // Next node in a list

        q->pCur = q->pCur->pNext;
    }

    // Search has failed

    q->pCur = q->pTail;

    return( 0 );
}



/******************************************************************************
*                                                                             *
*   QUEUE FUNCTIONS                                                           *
*                                                                             *
******************************************************************************/


/******************************************************************************
*                                                                             *
*   int QEnqueue( char * pHeap, TQueue * q, void * p )                        *
*                                                                             *
*******************************************************************************
*
#   QUEUE OPERATION: Enqueue
#
#   Enqueues an element on the queue head.  This function is euqivalent to
#   the push operation except that the current node points to the the tail
#   node so it can be examined by QCurrent function.
*
*   Where:
*       pHeap - pointer to a allocation heap
*       q - Queue descriptor
#       p - element/pointer to an element
*
*   Returns:
#       0 if operation failed (cannot allocate memory for structure)
#       n, n>0 - if operation succeded (number of nodes)
*
******************************************************************************/
int QEnqueue( char * pHeap, TQueue * q, void * p )
{
    TNode * pFirst;

    pFirst = _kMalloc( pHeap, sizeof(TNode) );

    if( pFirst == NULL )
        return( NULL );

    pFirst->pPrev = NULL;
    pFirst->pNext = q->pHead;

    if( q->pTail == NULL )
        q->pTail = pFirst;
    else
        q->pHead->pPrev = pFirst;

    q->pHead = pFirst;
    q->pCur = q->pTail;

    pFirst->p = p;

    return( ++ q->nSize );
}



/******************************************************************************
*                                                                             *
*   void * QDequeue( char * pHeap, TQueue * q )                               *
*                                                                             *
*******************************************************************************
*
#   QUEUE OPERATION: Dequeue
#
#   Dequeues an element from the queue tail.
#
*   Deletes and frees the memory.  The current node pointer is set to the new
#   tail node or NULL if there is no mode nodes in a queue.
*
*   Where:
*       pHeap - pointer to a allocation heap
*       q - Queue descriptor
*
*   Returns:
*       Pointer to the tail node element.
#       NULL if the queue was empty
*
******************************************************************************/
void * QDequeue( char * pHeap, TQueue * q )
{
    TNode * pFree;
    void * p;

    // If the queue was empty, return

    if( q->pTail == NULL )
        return( NULL );

    pFree = q->pTail;

    // Modify the node ahead of the current one to skip it

    if( pFree->pPrev != NULL )
    {
        pFree->pPrev->pNext = pFree->pNext;
    }
    else
    {
        // There were no nodes ahead... need to modify q

        q->pHead = pFree->pNext;
    }

    // Set the new tail pointer and current pointer to a new tail node

    q->pCur = q->pTail = pFree->pPrev;

    // Finally, free the node

    q->nSize--;

    p = pFree->p;
    _kFree( pHeap, pFree );

    return( p );
}



/******************************************************************************
*                                                                             *
*   PRIORITY QUEUE FUNCTIONS                                                  *
*                                                                             *
******************************************************************************/


/******************************************************************************
*                                                                             *
* int QPriorityEnqueue( char * pHeap, TQueue * q,                             #
#                       int fnCmp( void *p1, void *p2), void * p )            #
*                                                                             *
*******************************************************************************
*
*   Enqueues an element in the priority queue.  The compare function may be
#   given as an argument; if set to NULL, an integer compare on the element
#   pointer p will be used.
#
#   The queue will remain sorted and the next QDequeue will remove the node
#   with the highest priority value.
*
#   It is important not to mix regular queue and priority queue operations
#   on the same queue because this function counts on a queue already being
#   priority sorted.
#
*   Where:
*       pHeap - pointer to a allocation heap
*       q - Queue descriptor
#       fnCmp - compare function:  (BOOL) p1 < p2 ?
#       p - element/pointer to an element
*
*   Returns:
#       0 if operation failed (cannot allocate memory for structure)
#       n, n>0 - if operation succeded (number of nodes)
#
******************************************************************************/
int QPriorityEnqueue( char * pHeap, TQueue * q, int fnCmp( void *p1, void *p2), void * p)
{
    // If the list is originally empty, simply insert a new node

    if( q->pHead == NULL )
        return( QInsert( pHeap, q, p ) );

    // Traverse the list and find the place to insert a new node

    q->pCur = q->pHead;

    while( q->pCur != NULL )
    {
        if( fnCmp==NULL?
            (int) p < (int) q->pCur->p :     // Compare as integers
            fnCmp( p, q->pCur->p) )          // Compare using user's fnCmp
        {
            // The new element is less than the current one, needs to be
            // inserted ahead of it

            QInsert( pHeap, q, p );

            // Set the current pointer to the tail of a queue

            q->pCur = q->pTail;

            return( q->nSize );
        }

        q->pCur = q->pCur->pNext;
    }

    // If we have reached the end of a list, add a new element

    q->pCur = q->pTail;

    return( QAdd( pHeap, q, p ) );
}


/******************************************************************************
*                                                                             *
*   SORTED LIST FUNCTIONS                                                     *
*                                                                             *
******************************************************************************/


/******************************************************************************
*                                                                             *
*   int QSort( char * pHeap, TQueue * q, int fnCmp( void *p1, void *p2))      *
*                                                                             *
*******************************************************************************
*
*   This function sorts the linked list.  The compare function may be
#   given as an argument; if set to NULL, an integer compare on the element
#   pointer p will be used.
#
#   Elements are sorted in ascending order.
*
*   Where:
*       pHeap - pointer to a allocation heap
*       q - Queue descriptor
#       fnCmp - compare function:  (BOOL) p1 < p2 ?
*
*   Returns:
#       0 if operation failed (cannot allocate memory for temporary structure)
*       number of nodes in a list
*
******************************************************************************/
int QSort( char * pHeap, TQueue * q, int fnCmp( void *p1, void *p2))
{
    TQueue Queue;
    TNode * pCur;

    // If there are less than 2 elements, list does not need a sort

    if( QIsEmpty( q ) < 2 )
        return( QIsEmpty( q ) );

    // Initialize temporary queue

    Queue.pHead = Queue.pTail = Queue.pCur = NULL;
    Queue.nSize = 0;

    pCur = q->pHead;

    while( pCur != NULL )
    {
        // Priority enqueue one old queue element

        if( QPriorityEnqueue( pHeap, &Queue, fnCmp, pCur->p) == 0 )
        {
            // Unable to priority enqueue new element, delete new nodes

            QDestroy( pHeap, &Queue );

            return( 0 );
        }

        // Traverse to the next old queue element

        pCur = pCur->pNext;
    }

    // Destroy the original queue node list (not the queue descriptor)

    QDestroy( pHeap, q );

    // Set the original queue to point to a newly created sorted list

    q->pHead = q->pCur = Queue.pHead;
    q->pTail = Queue.pTail;
    q->nSize = Queue.nSize;

    return( q->nSize );
}



/******************************************************************************
*                                                                             *
*   TESTS                                                                     *
*                                                                             *
******************************************************************************/
#if 0

// These are made before explicit heap additions, so they need to be updated

#define CHECK printf("Check: %d  size: %d\n", QCheck(q), QIsEmpty(q) )

void main()
{
    TQueue * q;

    q = QCreateAlloc();

    CHECK;

    if( q != NULL )
    {
#if 0
        printf("Pop %d  size: %d\n", (int) QPop(q), QIsEmpty(q) );
        QPush( q, 1 );
        QPush( q, 2 );
        QPush( q, 3 );
        CHECK;
        printf("Pop %d  size: %d\n", (int) QPop(q), QIsEmpty(q) );
        printf("Pop %d  size: %d\n", (int) QPop(q), QIsEmpty(q) );
        QPush( q, 3 );
        CHECK;
        printf("Pop %d  size: %d\n", (int) QPop(q), QIsEmpty(q) );
        printf("Pop %d  size: %d\n", (int) QPop(q), QIsEmpty(q) );
        printf("Pop %d  size: %d\n", (int) QPop(q), QIsEmpty(q) );
        printf("Pop %d  size: %d\n", (int) QPop(q), QIsEmpty(q) );
        CHECK;
#endif
#if 0
        QInsert( q, 10 );
        printf("First: %d\n", QFirst(q) );
        printf("Next:  %d\n", QNext(q) );
        printf("Last:  %d\n", QLast(q) );
        CHECK;
        QInsert( q, 11 );
        QInsert( q, 12 );
        QInsert( q, 13 );
        CHECK;
        printf("First: %d\n", QFirst(q) );
        printf("Next:  %d\n", QNext(q) );
        printf("Next:  %d\n", QNext(q) );
        printf("Next:  %d\n", QNext(q) );
        printf("Next:  %d\n", QNext(q) );
        CHECK;
        printf("Last:  %d\n", QLast(q) );
        printf("Prev:  %d\n", QPrev(q) );
        printf("Prev:  %d\n", QPrev(q) );
        printf("Prev:  %d\n", QPrev(q) );
        printf("Prev:  %d\n", QPrev(q) );
        CHECK;
#endif
#if 0
        QAdd( q, 10 );
        printf("First: %d\n", QFirst(q) );
        printf("Next:  %d\n", QNext(q) );
        printf("Last:  %d\n", QLast(q) );
        CHECK;
        QAdd( q, 11 );
        QAdd( q, 12 );
        QAdd( q, 13 );
        CHECK;
        printf("First: %d\n", QFirst(q) );
        printf("Next:  %d\n", QNext(q) );
        printf("Next:  %d\n", QNext(q) );
        printf("Next:  %d\n", QNext(q) );
        printf("Next:  %d\n", QNext(q) );
        CHECK;
        printf("Last:  %d\n", QLast(q) );
        printf("Prev:  %d\n", QPrev(q) );
        printf("Prev:  %d\n", QPrev(q) );
        printf("Prev:  %d\n", QPrev(q) );
        printf("Prev:  %d\n", QPrev(q) );
        CHECK;
        printf("First: %d\n", QFirst(q) );
        printf("Delete:%d\n", QDelete(q) );
        CHECK;
#endif
#if 0
        QEnqueue( q, 20 );
        QEnqueue( q, 21 );
        CHECK;
        printf("Dequeue: %d\n", QDequeue(q) );
        printf("Dequeue: %d\n", QDequeue(q) );
        printf("Dequeue: %d\n", QDequeue(q) );
        CHECK;
        QEnqueue( q, 22 );
        QEnqueue( q, 23 );
        CHECK;
        printf("Dequeue: %d\n", QDequeue(q) );
        printf("Dequeue: %d\n", QDequeue(q) );
        printf("Dequeue: %d\n", QDequeue(q) );
        CHECK;
#endif
#if 0
        QPriorityEnqueue( q, NULL, 100 );
        QPriorityEnqueue( q, NULL, 110 );
        QPriorityEnqueue( q, NULL, 120 );
        CHECK;
        printf("Dequeue: %d\n", QDequeue(q) );
        printf("Dequeue: %d\n", QDequeue(q) );
        printf("Dequeue: %d\n", QDequeue(q) );
        printf("Dequeue: %d\n", QDequeue(q) );
        CHECK;
        QPriorityEnqueue( q, NULL, 120 );
        QPriorityEnqueue( q, NULL, 110 );
        QPriorityEnqueue( q, NULL, 100 );
        printf("Dequeue: %d\n", QDequeue(q) );
        printf("Dequeue: %d\n", QDequeue(q) );
        printf("Dequeue: %d\n", QDequeue(q) );
        printf("Dequeue: %d\n", QDequeue(q) );
        CHECK;
#endif
#if 1
        QInsert( q, (void *) 250 );
        QInsert( q, (void *) 230 );
        QInsert( q, (void *) 220 );
        QInsert( q, (void *) 240 );
        QInsert( q, (void *) 200 );
        QInsert( q, (void *) 220 );
        CHECK;
        QSort( q, NULL );
        CHECK;
        QFirst( q );
        while( QIsEmpty( q ) )
        {
        printf("Delete: %d\n", QDelete(q) );
        }
        CHECK;
#endif
    }
    else
        printf("Cannot create list!\n");
}

#endif
