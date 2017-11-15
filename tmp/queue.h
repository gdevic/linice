/******************************************************************************
*                                                                             *
*   Module:     Queue.h                                                       *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       8/29/97                                                       *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************
.-
    Module Description:

        This is a header file for the queue functions.

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
            fFound  = QFind( &q, fmCmp, const element )

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
* 8/29/97    1.00  Original                                       Goran Devic *
* --------   ----  ---------------------------------------------  ----------- *
*******************************************************************************
*   Important Defines                                                         *
******************************************************************************/
#ifndef _QUEUE_H_
#define _QUEUE_H_

/******************************************************************************
*                                                                             *
*   Include Files                                                             *
*                                                                             *
******************************************************************************/

/******************************************************************************
*                                                                             *
*   Global Defines, Variables and Macros                                      *
*                                                                             *
******************************************************************************/

typedef struct TNodePtr pTNode;

typedef struct TNodePtr
{
    void * p;                           // Pointer to the data element
    pTNode *pPrev, *pNext;              // Doubly linked list pointers

} TNode;

typedef struct
{
    TNode *pHead, *pTail, *pCur;        // Queue pointers
    unsigned int nSize;                 // Number of nodes in a queue

} TQueue;


/******************************************************************************
*                                                                             *
*   External Functions                                                        *
*                                                                             *
******************************************************************************/

extern int    QCheck( TQueue *q );

extern void   QCreate( TQueue *q );
extern void   QDestroy( char * pHeap, TQueue *q );

extern TQueue* QCreateAlloc( char * pHeap );
extern void    QDestroyAlloc( char * pHeap, TQueue *q );

extern int    QPush( char * pHeap, TQueue * q, void * p );
extern void * QPop( char * pHeap, TQueue * q );

extern int    QInsert( char * pHeap, TQueue * q, void * p );
extern int    QAdd( char * pHeap, TQueue * q, void * p );
extern void * QDelete( char * pHeap, TQueue * q );

extern void * QFirst( TQueue * q );
extern void * QLast( TQueue * q );
extern void * QCurrent( TQueue * q );
extern void * QNext( TQueue * q );
extern void * QPrev( TQueue * q );

extern int    QIsEmpty( TQueue * q );
extern int    QFind( TQueue * q, int fnCmp( void *p1, const void *p2), const void * p);

extern int    QEnqueue( char * pHeap, TQueue * q, void * p );
extern void * QDequeue( char * pHeap, TQueue * q );

extern int    QPriorityEnqueue( char * pHeap, TQueue * q, int fnCmp( void *p1, void *p2), void * p);

extern int    QSort( char * pHeap, TQueue * q, int fnCmp( void *p1, void *p2));


#endif //  _QUEUE_H_
