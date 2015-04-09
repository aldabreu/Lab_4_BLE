/**
 *  Author: Alexander D'Abreu
    Headerfile:       UART_HCI.h


  Revised:        $Date: 2015-3-1   (Sun, 1 Mar 2015) $
  Revision:       $Revision: 1 $

Description: 		Event Handler function and setup for the UART Ports on the MSP430f5529
 *					Used to communication via a set of Host-Controller Interface commands to the CC2540
 *					bluetooth low energy device.

**************************************************************************************************/
#ifndef UART_TASK_H_
#define UART_TASK_H_

/*********************************************************************
 * INCLUDES
 */
#include "comdef.h"
#include "hal_types.h"
#include "Mem_Manager.h"
#include "Scheduler.h"


/*********************************************************************
 * MACROS
 */
#define isCircBufFull(CircBuf) (CircBuf->numOfBufInUse == NUMOFBUFFERS)
#define isBuffFull(LinBuff) ((LinBuff.dataEnd) == BUFFERSIZE)
#define isBuffEmpty(LinBuff) ((LinBuff.dataEnd) == 0)
#define isBuffInRange(BuffVal) ((BuffVal) < NUMOFBUFFERS)

/*********************************************************************
 * CONSTANTS
 */

#define UARTTXINT 4
#define UARTRXINT 2

#define CMDPKT 0x01
#define DATAPKT 0x02
#define EVTPKT 0x04
#define PKTOFFSET 3



#define EVTDATALENINDEX 2


#define NUMOFBUFFERS 15
#define BUFFERSIZE 34

#define RXBUFFER_INIT 0x01
#define TXBUFFER_INIT 0x00
#define INUSE	0x01

//Event Flags
#define UART_A_TX_EVT BIT0
#define UART_A_RX_EVT BIT1

#define UART_B_TX_EVT BIT2
#define UART_B_RX_EVT BIT3
/*********************************************************************
 * TYPEDEFS
 */
//For Circular Buffer
typedef struct
{
	uint8 *linBuffer;//[BUFFERSIZE]
	uint8 isInUse; //Boolean true of false
	uint8 dataEnd; //End of Valid Data

}LinearBuffer_s;


typedef struct
{
	LinearBuffer_s *circBuffer[NUMOFBUFFERS]; //The Actual Circular Buffer is an array of pointers
	uint8 numOfBufInUse;	//Length of the circular Array

}RingBuffer_s;






//Structures for HCI Command and Event Packets
typedef struct {
	uint8 eventCode;
	uint8 totalParamLen;
	uint8 *pData;
}evtPkt_Gen_s;

typedef struct {
	 uint8 opCode[2];
	 uint8 * pData;
}cmdPkt_Gen_s;




/*********************************************************************
 * System Events
 */

/*********************************************************************
 * Global System Messages
 */

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */
extern RingBuffer_s *UART_A_TXCircBuf;
extern RingBuffer_s *UART_B_TXCircBuf;

/*********************************************************************
 * FUNCTIONS
 */

/*
* UART Initialization function.
	*/
extern void UART_Init();

/*
* UART Event Processing function for Receive and Transmission events .
	*/
extern uint8 UART_ProcessEvent(uint8 taskId,uint8 events);


/*
* Initializes the Circular Buffers for both Receive and Transmission functions.
	*/
extern RingBuffer_s * initializeBuffer(uint8 RXBuffer);

/*
* Adds a single character to a Linear Buffer within the UART Circular Buffers.
	*/
extern uint8 addToBuffer(RingBuffer_s *inputBuffer,uint8 currBuffer,uint8 byteInput);

/*
* Removes a specific Element from a linear buffer.
	*/
extern uint8 removeFromBuffer(RingBuffer_s *inputBuffer,uint8 currBuffer,uint8 *ByteLocation);

/*
* Searches through a circular buffer to find the first available linear buffer.
	*/
extern uint8 findOpenBuffer(RingBuffer_s * ,uint8 );

/*
* Copies bytes from one memory location to another.
	*/
extern void copyArr(uint8 *src,uint8 *dst,uint8 src_start,uint8 src_end,uint8 dst_start);

/*********************************************************************
*********************************************************************/



#endif /* UART_HCI_H_ */
