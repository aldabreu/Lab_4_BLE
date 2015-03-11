/**************************************************************************************************
  Filename:       UART_HCI.c
  Revised:        $Date: 2015-03-01 (Sun, 1 Mar 2015) $
  Revision:       $Revision: 35416 $

  Description:    UART Host Controller Interface between MSP430F5529 and CC2540

**************************************************************************************************/

/*********************************************************************
 * INCLUDES
 */
#include "UART_HCI.h"


/*********************************************************************
 * CONSTANTS
 */


/*********************************************************************
 * TYPEDEFS
 */




/*********************************************************************
 * GLOBAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS EXT
 */

/*********************************************************************
 * LOCAL VARIABLES Static
 */

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

uint8 UART_ProcessEvent(uint8 taskId,uint8 events)
{
	//Check which event was triggered->receive message->parse->set GAP/GATT EVT Flag etc../allocate new msg->
	//delete node ->reset inUse flag-> return undone events

	switch(events)
	{
	case UART_A_TX_EVT: break;
	case UART_A_RX_EVT:
	{

		__delay_cycles(10000);
		LinearBuffer_s *tempMsg = (LinearBuffer_s *)scheduler_receive_Msg(taskId,events);

		//Release Buffer for UART ISR
		tempMsg->isInUse = 0;
		tempMsg->dataEnd = 0;


		/*



		//New event Packet
		evtPkt_Gen_s * tempEvtPkt = osal_mem_alloc(sizeof(evtPkt_Gen_s));

		//Current Data length = length from all received messages
		uint8 pktDataLength = 0,currDataLength = 0;

		//Retrieve ALL relevant messages for a specific event from queue
		LinearBuffer_s *tempMsg = (LinearBuffer_s *)scheduler_receive_Msg(taskId,events);

		dataLength = tempMsg->linBuffer[EVTDATALENINDEX];

		//Allocate new array of datalen - PKTOFFSET bytes(don't include header info in data section)
		uint8 *pktData = (uint8 *)osal_mem_alloc(dataLength);


		//void copyArr(uint8 *src,uint8 *dst,uint8 src_start,uint8 src_end,uint8 dst_start)


		tempEvtPkt->eventCode = tempMsg->linBuffer[1];
		tempEvtPkt->totalParamLen = dataLength;
		tempEvtPkt->pData = pktData;

		do{


			//Copy From queueBuffer
			if(currDataLength = 0)
					copyArr(tempMsg->linBuffer,pktData,PKTOFFSET,tempMsg->linBuffer->dataEnd,currDataLength);
			else
				copyArr(tempMsg->linBuffer,pktData,0,tempMsg->linBuffer->dataEnd,currDataLength - PKTOFFSET);

			//Update Data length
			currDataLength += tempMsg->dataEnd;

			//Release Buffer for UART ISR
			tempMsg->isInUse = 0;
			tempMsg->dataEnd = 0;

			deleteQueueNode(tempMsg);



			//Get new MSG
			if(currDataLength < dataLength + PKTOFFSET)
			{
				*tempMsg = (LinearBuffer_s *)scheduler_receive_Msg(taskId,events);
			}
			else
				break;

		}while(1);


*/

















		//Mask out completed events
		events &= ~UART_A_RX_EVT;


	}break;
	case UART_B_TX_EVT: break;
	case UART_B_RX_EVT: break;

	};

	return events;
}


/*********************************************************************
 * @fn          UART_Init
 *
 * @brief      UART Initialization function for both UART Ports
 * 			   115200 Baud, 8N1
 *
 * @param void
 *
 * @return void
 */

void UART_Init()
{
	P4SEL |= BIT4+BIT5;                       // P3.3,4 = USCI_A0 TXD/RXD
	UCA1CTL1 |= UCSWRST;                      // **Put state machine in reset**
	UCA1CTL1 |= UCSSEL_2;                     // SMCLK
	UCA1BR0 = 9;                              // 1MHz 115200 (see User's Guide)
	UCA1BR1 = 0;                              // 1MHz 115200
	UCA1MCTL |= UCBRS_1 + UCBRF_0;            // Modulation UCBRSx=1, UCBRFx=0
	UCA1CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
	//UCA1IE |= UCTXIE;                         // Enable USCI_A0 TX interrupt


	UCA1IE |= UCRXIE;                         // Enable USCI_A1 RX interrupt


}















/*********************************************************************
 * @fn          initializeBuffer
 *
 * @brief       Allocates Memory on the Heap for the a Circular Buffer
 *
 * @params uint8 RxBuffer  - Type of Buffer to create RXBUFFER or TXBUFFER(1 or 0)
 *
 * @return RingBuffer_s newBuffer  -    Pointer to the newly allocated Buffer
 */

RingBuffer_s * initializeBuffer(uint8 RXBuffer)
{
	//Create memory blocks for both circular and Linear Buffers
	RingBuffer_s *inputBuffer;
	uint8 currBuffer;

	inputBuffer = (RingBuffer_s*)osal_mem_alloc(sizeof(RingBuffer_s));
	inputBuffer->numOfBufInUse = 0;

	//Allocate memory for the LinearBuffer structure and internal Buffer(array)
	for(currBuffer = 0; currBuffer < NUMOFBUFFERS;currBuffer++)
	{
		inputBuffer->circBuffer[currBuffer] = (LinearBuffer_s *)osal_mem_alloc(sizeof(LinearBuffer_s));

		if(RXBuffer)	//Do not allocate Linear Array for Transmit Buffers
			{

				inputBuffer->circBuffer[currBuffer]->linBuffer = (uint8 *)osal_mem_alloc(BUFFERSIZE);

			}else
						inputBuffer->circBuffer[currBuffer]->linBuffer = NULL;

			//Initialize internal Buffer
			inputBuffer->circBuffer[currBuffer]->isInUse = 0; //Reset all flags
			inputBuffer->circBuffer[currBuffer]->dataEnd = 0; //Initially empty arrays


	}
	char tempchar = 'A';
	for(currBuffer = 0; currBuffer < NUMOFBUFFERS;currBuffer++)
	{

		int i;

		for(i = 0; i < BUFFERSIZE;i++)
			inputBuffer->circBuffer[currBuffer]->linBuffer[i] = tempchar;
		tempchar++;

	}







	return inputBuffer;
}

/*********************************************************************
 * @fn          addToBuffer
 *
 * @brief       Adds a byte to a linear array in the circular Buffer
 *
 * @params 	RingBuffer_s - input Buffer to be added to
 * 			uint8 currBuffer - Linear Buffer to add Byte
 * 			uint8 byteInput - Byte to add
 *
 * @return uint8 STATUS - SUCCESS or FAILURE
 */
uint8 addToBuffer(RingBuffer_s *inputBuffer,uint8 currBuffer,uint8 byteInput)
{
	//Check if current buffer selection is in range
	if(!(isBuffInRange(currBuffer)))
		return FAILURE;


	//Check for Valid inputBuffer
	if((inputBuffer != NULL) && (inputBuffer->circBuffer[currBuffer] != NULL) && (inputBuffer->circBuffer[currBuffer]->linBuffer != NULL))
	{

			//Check if the internal Buffer attempting to be written to is full
			if(isBuffFull((*(inputBuffer->circBuffer[currBuffer]))))
				return FAILURE;

			else{
					inputBuffer->circBuffer[currBuffer]->linBuffer[(inputBuffer->circBuffer[currBuffer]->dataEnd)] = byteInput;

					inputBuffer->circBuffer[currBuffer]->dataEnd++;

					return SUCCESS;
				}

	}
		return FAILURE;
}


/*********************************************************************
 * @fn          removeFromBuffer
 *
 * @brief       Remove a byte from a linear array in the circular Buffer
 *
 * @params 	RingBuffer_s - Input buffer to be removed from
 * 			uint8 currBuffer - Linear Buffer to add Byte
 * 			uint8 *byteLocationt - Pointer to byte to store removal
 *
 * @return uint8 STATUS - SUCCESS or FAILURE
 */
uint8 removeFromBuffer(RingBuffer_s *inputBuffer,uint8 currBuffer,uint8 *ByteLocation)
{
	//Check if current buffer selection is in range
	if(!(isBuffInRange(currBuffer)))
		return FAILURE;

	//Check for Valid inputBuffer
	if((inputBuffer != NULL) && (inputBuffer->circBuffer[currBuffer] != NULL) && (inputBuffer->circBuffer[currBuffer]->linBuffer != NULL))
	{

			//Check if buffer is empty
			if(!isBuffEmpty((*(inputBuffer->circBuffer[currBuffer]))))
			{
				inputBuffer->circBuffer[currBuffer]->dataEnd--;
				uint8 temp_dataEnd = inputBuffer->circBuffer[currBuffer]->dataEnd;

				*ByteLocation = inputBuffer->circBuffer[currBuffer]->linBuffer[temp_dataEnd];

				return SUCCESS;
			}


	}
	return FAILURE;

}

/*********************************************************************
 * @fn          findOpenBuffer
 *
 * @brief       Find first Rx or TX UART Buffer not in use
 *
 * @params 	RingBuffer_s - Input buffer
 * 			uint8 currBuffer - Linear Buffer to check if in use
 *
 * @return uint8 STATUS - Index of first buffer not in use
 */

uint8 findOpenBuffer(RingBuffer_s *UART_A_RXCircBuf,uint8 currBuf)
{
	uint8 currIndex = currBuf;
	do{

		if(UART_A_RXCircBuf->circBuffer[currIndex]->isInUse == INUSE)
			currIndex = (currIndex + 1) % NUMOFBUFFERS;
		else break;
	}
	while(currIndex != currBuf);

	return currIndex;

}


/**************************************************************************************************
 * End of Functions for Circular Buffer
 */





/**************************************************************************************************
 * @fn          FUNCTION NAME
 *
 * @brief       This function ...
 *
 * input parameters
 *
 * INPUT PARAMS
 *
 * output parameters
 *
 * OUTPUT PARAMS
 *
 * @return     RETURN EXPLANATION
 */
