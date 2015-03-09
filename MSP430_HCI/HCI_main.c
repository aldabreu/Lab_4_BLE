#include <msp430.h>
#include "Scheduler.h"

#define UART_TASK_ID 0

//Static Global Definitions for Main ISR's
   //--------------------------------------------------------------
	static RingBuffer_s *UART_A_RXCircBuf;
	static RingBuffer_s *UART_A_TXCircBuf;
	static RingBuffer_s *UART_B_RXCircBuf;
	static RingBuffer_s *UART_B_TXCircBuf;

	static uint8 ERRORFLAG = 0;



 //---------------------------------------------------------------

void main(void) {
	WDTCTL = WDTPW | WDTHOLD;	// Stop Watchdog timer

	// Initialize the Memory Allocation System
	osal_mem_init();

	//Initialize UART RX TX Buffers
	UART_A_RXCircBuf = initializeBuffer(RXBUFFER_INIT);
	//UART_A_TXCircBuf = initializeBuffer(TXBUFFER_INIT);
	//UART_B_RXCircBuf = initializeBuffer(RXBUFFER_INIT);
	//UART_B_TXCircBuf = initializeBuffer(TXBUFFER_INIT);

	//Start the HCI system
	scheduler_start_system();





/*---------------------------------------------------------------------
Test Code Block
*/


/*
-----------------------------------------------------------------------
*/
}


// Vector 0 - no interrupt
// Vector 2 - RXIFG
// Vector 4 - TXIFG

#pragma vector=USCI_A1_VECTOR
__interrupt void USCI_A1_ISR(void)
{
	//Flag to skip current RX Packet if all buffers are full
	static uint8 SkipPacket = FALSE;
	static uint8 pktType = 0;


	//Number of bytes either received or transmitted
		 static uint8 TxByteCtr = 0;
		 static uint8 RxByteCtr = 0;

	//Index of Buffer currently being used
		 static uint8 currTxBuf = 0;
		 static uint8 currRxBuf = 0;
	//Index of end of data
		// static uint8 RxPktEnd = BUFFERSIZE;
		 static uint8 RxPktEnd = 44;
		 static uint8 TxPktEnd = 0;

	//Status of adding to a buffer
		 uint8 addStatus = SUCCESS;
		 volatile uint8 rxByte = 0;

		 uint8 pktLenIndex = 0;








	switch(__even_in_range(UCA1IV,4))
	{
	case UARTTXINT:
			{
			 //UCA1TXBUF = 'a';

			}break;
	case UARTRXINT:
	{

		 //Read Received Byte into Circular RX Buffer
		 rxByte = UCA1RXBUF;
		 RxByteCtr++;
		 /*Buffer writing procedure
		  * 0)Update NumOfBufInUse
			1)Test to see if the overall Circular Buffer is fully in use - STOP ERROR
			2)Find open Buffer,Set in use flag,increment Total buffers in use
			3)Write to same open buffer until : Full or End transmission
			4)If Full and Transmission not ended:Send MSG with Full Buffer->DO NOT reset rxByteCtr,read into new buffer for next rxByte->Send MSG & Set Event when Complete

			Cases: 	1) End of incoming data packet
					2)Buffer is full BUT Data not complete
					3)Buffer Not full AND Data not complete
						3A)RxByte == 3, parse data packet length
						3B) DO nothing




				*/

		 //Update NumOfBufInUse
		 int i;
		 for(i = 0; i < NUMOFBUFFERS;i++)
		 {
			 UART_A_RXCircBuf->numOfBufInUse = 0;
			 //Check Each Linear Buffer to see if it's in use
			 if(UART_A_RXCircBuf->circBuffer[i]->isInUse == 1)
				 UART_A_RXCircBuf->numOfBufInUse++;
		 }









		 //Save Packet type
		 if(RxByteCtr == 1)
			 pktType = rxByte;

		 //Buffer's not full and NOT skipping a previous Packet
		 if((!isCircBufFull(UART_A_RXCircBuf)) && (!SkipPacket))
		 {
			 //Find a new Buffer if the one is currently being used
			 if(UART_A_RXCircBuf->circBuffer[currRxBuf]->isInUse)
			 {
				 currRxBuf = findOpenBuffer(UART_A_RXCircBuf,currRxBuf);
			 }

			 //Add Data to circular Buffer
			 addStatus =  addToBuffer(UART_A_RXCircBuf,currRxBuf,rxByte);

			  if(addStatus == SUCCESS)
			  {
				  //End of data Reached
				  if(RxByteCtr == RxPktEnd)
				  {
					  //Reset Byte Counter
					  RxByteCtr = 0;
					  RxPktEnd = BUFFERSIZE;

					  //Set in use flag so it doesen't get overwritten
					  UART_A_RXCircBuf->circBuffer[currRxBuf]->isInUse = 1;
					  UART_A_RXCircBuf->numOfBufInUse++;

					  //Send MSG
					  scheduler_send_Msg(UART_TASK_ID,UART_A_RX_EVT,UART_A_RXCircBuf->circBuffer[currRxBuf]);
					  //Set Event
				  }

				  //Check for full buffer && Not finished with incoming Packet data
				  else if((isBuffFull((*(UART_A_RXCircBuf->circBuffer[currRxBuf])))))
				  {

					  UART_A_RXCircBuf->numOfBufInUse++;

					  //Make sure there's enough space for the rest of the packet
					  if((!isCircBufFull(UART_A_RXCircBuf)))
					  {
						  //Just don't RESET RX Byte Counter
						  UART_A_RXCircBuf->circBuffer[currRxBuf]->isInUse = 1;

						  //SEND SCHED MSG
						  scheduler_send_Msg(UART_TASK_ID,UART_A_RX_EVT,UART_A_RXCircBuf->circBuffer[currRxBuf]);
					  }
					  else
					  {
						  UART_A_RXCircBuf->numOfBufInUse--;
						  //Not enough room, skip rest of packet and set error Flag
						  SkipPacket  = 1;
						  ERRORFLAG = 1;
					  }
				  }
				  else if(RxByteCtr == 0x03)
				  {
					 switch(pktType)
					 {
						case CMDPKT:
							pktLenIndex = CMDDATALENINDEX; break;
						case EVTPKT:
							pktLenIndex = EVTDATALENINDEX; break;
						case DATAPKT:
							pktLenIndex = CMDDATALENINDEX; break;
					 }
					 //End of data transmission
					 RxPktEnd = UART_A_RXCircBuf->circBuffer[currRxBuf]->linBuffer[pktLenIndex] + RXOFFSET;
				  }

			  }
			  //ADD STATUS FAILURE
			  else
			  {
				//SET SOME KIND OF FLAG
				  ERRORFLAG = 1;
			  }

		 }
		 else
		 {
			 //Circular Buffer is Full so skip the current packet and try again later
			 SkipPacket = TRUE;
			 //Continue to count bytes BUT just to make sure to start at next valid buffer

			 if((RxByteCtr == 0x03) && (pktType == EVTPKT))
				 RxPktEnd = rxByte + RXOFFSET;
			 else if((RxByteCtr == 0x04) && (pktType == (CMDPKT || DATAPKT)))
				 RxPktEnd = rxByte + RXOFFSET;


			 if(RxByteCtr == RxPktEnd)
			 {
				 //Reset Byte Counter
				 RxByteCtr = 0;
				 RxPktEnd = BUFFERSIZE;
				 //Packet Skip Completed
				 SkipPacket = FALSE;
				 pktType = 0;
			 }
		 }

	}break;


	default: break;
	}

}

