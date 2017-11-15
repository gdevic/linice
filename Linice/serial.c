/******************************************************************************
*                                                                             *
*   Module:     serial.c                                                      *
*                                                                             *
*   Date:       05/01/00                                                      *
*                                                                             *
*   Copyright (c) 2000 Goran Devic                                            *
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

        This module implements input/output functionality for serial port
		(COM1/COM2/COM3/COM4)

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 05/01/00   Original                                             Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include "module-header.h"              // Versatile module header file

#include "clib.h"                       // Include C library header file
#include "ice.h"                        // Include main debugger structures
#include "ibm-pc.h"                     // Include hardware defines


/******************************************************************************
*                                                                             *
*   Global Variables                                                          *
*                                                                             *
******************************************************************************/

extern TOUT outVT100;

/******************************************************************************
*                                                                             *
*   Local Defines, Variables and Macros                                       *
*                                                                             *
******************************************************************************/

#define MAX_SERIAL_BUFFER      4000     // Output serial queue len

static int port[4] = { 0x3F8, 0x2F8, 0x3E8, 0x2E8 };
static int irq[4]  = { 4, 3, 4, 3 };

typedef struct                          // Define serial connection structure
{
    int com;                            // COM port number
    int port;                           // Serial port #
    int irq;                            // IRQ: 2 or 3
    int baud;                           // Baud rate number
    int rate;                           // Baud rate translated
    DWORD sent;                         // Number of bytes sent over the port
    DWORD parity, overrun, framing, brk;// Number of these errors
    int head, tail;                     // Head and Tail of the buffer:
    BYTE outBuffer[MAX_SERIAL_BUFFER];  // Serial output asynchronous buffer

} TSerial;

// Define serial port default setting: COM1, 9600 baud
static TSerial Serial = { 1, 0x3F8, 3, 0x9600, 0x0C, };

#if SERIAL_POLLING
static char *smode="Polling";
#else
static char *smode="Ints";
#endif

/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

extern int InitVT100(void);
extern void VT100Input(BYTE data);

/******************************************************************************
*                                                                             *
*   int SerialInit(int com, int baud)                                         *
*                                                                             *
*******************************************************************************
*
*   Initializes serial interface.
*
*   Where:
*       com is the com port to use: 1,2,3,4
*       baud is the baud rate
*
*   If com==0, default (or previously set) port and rate will be used.
*
******************************************************************************/
int SerialInit(int com, int baud)
{
    // If we supplied the new port and baud numbers, set them up
    if( com != 0 )
    {
        Serial.com = com;
        com = com - 1;

        Serial.port = port[com];
        Serial.irq  = irq[com];
        Serial.baud = baud;

        switch( baud )
        {
            case 0x2400:    Serial.rate = 0x30; break;
            case 0x9600:    Serial.rate = 0x0C; break;
            case 0x19200:   Serial.rate = 0x06; break;
            case 0x38400:   Serial.rate = 0x03; break;
            case 0x57600:   Serial.rate = 0x02; break;
            case 0x115200:  Serial.rate = 0x01; break;
        }
    }
    // Initialize serial port - we'd rather not have interrupts at this time
    LocalCLI();

    // + 3 Data format register:
    // [7]   Access to divisor latch
    // [6]   BRK off
    // [5:3] parity - 000 none
    // [2]   stop 0 - 1 bit
    // [1:0] 11 - 8 data bits
    outp(Serial.port + 3, 0x83);

    // Output baud rate parameter
    outp(Serial.port + 0, Serial.rate);
    outp(Serial.port + 1, 0);

    // Revert the DLATCH to 0
    outp(Serial.port + 3, 3);

#if SERIAL_POLLING
    // We still expect interrupt on receiver data in
    outp(Serial.port + 1, 0x01);

    // DTR + RTS set
    outp(Serial.port + 4, 0x08 | 0x03);
#else
    // + 1 Interrupt enable register:
    // [3] Issue interrupt on state-change of a RS-232 input line
    // [2] Issue interrupt on parity, overrun, framing error or break
    // [1] Issue interrupt when out buffer is empty (ready to send)
    // [0] Issue interrupt when byte arrives (incomming)
    outp(Serial.port + 1, 0x06);

    // + 2 Interrupt identification line:
    // [2] \   00 - change of RS232 line     01 - output buffer empty
    // [1]  \  10 - data received            11 - error or break
    // [0] 1 - no pending int.  0 - interrupt pending

    // + 4 Modem control register:
    // [3] Must be set to allow interrupts
    // [1] RTS
    // [0] DTR
    outp(Serial.port + 4, 0x08 | 0x02 | 0x01);

#endif // SERIAL_POLLING

    Serial.sent = 0;
    Serial.parity = Serial.overrun = Serial.framing = Serial.brk = 0;
    Serial.head = 0;
    Serial.tail = 0;

    // Empty the input queue
    while( inp(Serial.port + 5) & 1 )
        inp(Serial.port + 0);

    // Done with initialization of the serial port
    LocalSTI();

    // Now we can initialize and start using a VT100 terminal
    InitVT100();

    return(0);
}


/******************************************************************************
*                                                                             *
*   void SerialPrintStat()                                                    *
*                                                                             *
*******************************************************************************
*
*   Prints information about the usage of the serial connection.
*
******************************************************************************/
void SerialPrintStat()
{
    if( pOut == &outVT100 )
        dprinth(1, "Serial is VT100: COM%d %x baud (IRQ=%d IO=%X) Sent: %d P%d O%d F%d B%d  %s",
            Serial.com, Serial.baud, Serial.irq, Serial.port, Serial.sent,
            Serial.parity, Serial.overrun, Serial.framing, Serial.brk, smode );
    else
        dprinth(1, "Serial is OFF");
}


/******************************************************************************
*                                                                             *
*   void SerialHandler(int IRQ)                                               *
*                                                                             *
*******************************************************************************
*
*   This handler is used when the debugger has control.
*
*   This is a low-level serial port handler. The event could be a serial
*   input character from the remote terminal or a serial mouse.
*
*   Where:
*       IRQ is 0 for COM1, COM3 or 1 for COM2, COM4
*
******************************************************************************/
void SerialHandler(int IRQ)
{
    BYTE status, data;
    static BYTE lastData;

    // If mouse has a control over the serial port, send it there

    // Queue mouse packets from the serial port:
    //
    //  D7   D6   D5   D4   D3   D2   D1   D0
    //  X    1    LB   RB   Y7   Y6   X7   X6
    //  X    0    X5   X4   X3   X2   X1   X0
    //  X    0    Y5   Y4   Y3   Y2   Y1   Y0
    //


    // Otherwise, it is a remote terminal
    // See if our serial port initiated this interrupt
    status = inp(Serial.port + 2);
    if( (status & 1)==0 )
    {
        // Pending interrupt was initiated by our serial interface
        switch( (status >> 1) & 3 )
        {
            case 0x00:      // change of RS232 line
                break;

            case 0x01:      // Output buffer empty
                // See if we have another byte to send
                if( Serial.head != Serial.tail )
                {
                    data = Serial.outBuffer[Serial.tail];
                    Serial.tail++;
                    if( Serial.tail>=MAX_SERIAL_BUFFER )
                        Serial.tail = 0;

                    outp(Serial.port + 0, data);
                    lastData = data;
                    Serial.sent++;
                }
                break;

            case 0x02:      // Data received in input buffer
                    data = inp(Serial.port);

                    // We dont support mouse yet, so assume it is a serial
                    // remote terminal sending us a character which we will
                    // accept only if serial terminal is enabled (and active)

                    if( pOut == &outVT100 )
                    {
                        VT100Input(data);
                    }

                break;

            case 0x03:      // Error or break
                    // Read status to clear pending interrupt
                    status = inp(Serial.port + 5);
                    if( status & (1<<2) ) Serial.parity++;
                    if( status & (1<<1) ) Serial.overrun++;
                    if( status & (1<<3) ) Serial.framing++;
                    if( status & (1<<4) ) Serial.brk++;

                    // Resend the data byte
                    outp(Serial.port + 0, lastData);
                break;
        }
    }

    // Or just a junk
}


#if SERIAL_POLLING
BYTE SerialIn()
{
    BYTE status;
    BYTE value;

    do
    {
        status = inp(Serial.port + 5);
        if( (status & 1)==1 )
        {
            value = inp(Serial.port);

            return( value );
        }

        value = inp(0x80);
    }
    while( TRUE );
}
#else
#endif

/******************************************************************************
*                                                                             *
*   void SerialOut(BYTE data)                                                 *
*                                                                             *
*******************************************************************************
*
*   Sends one byte through a serial port (via output buffer)
*
*   Where:
*       data is the byte to transmit
*
******************************************************************************/
#if SERIAL_POLLING
void SerialOut(BYTE data)
{
    BYTE status;

    // If the transmitter buffer is empty, send a byte. Otherwise, poll it
    deb.timer[0] = 50;                // 1/2 second
    do
    {
        status = inp(Serial.port + 5);
        if( status & (1<<5) )
        {
            // Transmitter buffer is empty..
            outp(Serial.port + 0, data);
            Serial.sent++;

            return;
        }
    }
    while( deb.timer[0] );
}
#else
void SerialOut(BYTE data)
{
    BYTE status;

    // If the transmitter buffer is empty, send a byte. Otherwise, queue it
    status = inp(Serial.port + 5);
    if( status & (1<<5) )
    {
        // Transmitter buffer is empty..
        outp(Serial.port + 0, data);
        Serial.sent++;
    }
    else
    {
        // Transmitter buffer is full, queue the byte
        Serial.outBuffer[Serial.head] = data;

        // If we got ahead with the head, poll until we transmit a byte out
        // Use the debugger timer to time out after a while
        //
        deb.timer[0] = 50;            // 1/2 second
        while( (Serial.head + 1)==Serial.tail && deb.timer[0]) ;

        LocalCLI();

        Serial.head++;

        if( Serial.head>=MAX_SERIAL_BUFFER )
            Serial.head = 0;

        LocalSTI();
    }
}
#endif // SERIAL_POLLING

/******************************************************************************
*                                                                             *
*   void SerialOutString(char *str)                                           *
*                                                                             *
*******************************************************************************
*
*   Sends a complete string to a serial port
*
******************************************************************************/
void SerialOutString(char *str)
{
    while( *str )
        SerialOut(*str++);
}


/******************************************************************************
*                                                                             *
*   void SerialMouse(int x, int y)                                            *
*                                                                             *
*******************************************************************************
*
*   Not used yet.
*
******************************************************************************/
void SerialMouse(int x, int y)
{
}

