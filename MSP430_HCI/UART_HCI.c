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
	case UART_A_TX_EVT:{

		uint8 tmpTxBuf = 0;
		//Receive message
		LinearBuffer_s *tempMsg = (LinearBuffer_s *)scheduler_receive_Msg(taskId,UART_A_TX_EVT);

		 //Find a new Buffer if the one is currently being used
		 if(UART_A_TXCircBuf->circBuffer[tmpTxBuf]->isInUse)
		 {
			 tmpTxBuf = findOpenBuffer(UART_A_TXCircBuf,tmpTxBuf);
		 }



		UART_A_TXCircBuf->circBuffer[tmpTxBuf]->linBuffer = tempMsg->linBuffer;
		UART_A_TXCircBuf->circBuffer[tmpTxBuf]->isInUse = INUSE;
		UART_A_TXCircBuf->circBuffer[tmpTxBuf]->dataEnd = tempMsg->dataEnd;
		UART_A_TXCircBuf->numOfBufInUse++;


		//Delete Message but not array
		osal_mem_free(tempMsg);


		//Mask out completed events
		if(getQueueLength(taskId,events) == 0)
			events &= ~UART_A_TX_EVT;

		UCA0IE |= UCTXIE;

//UCA1IE |= UCTXIE;	//Enable TX Interrupts on port A1

	} break;
	case UART_A_RX_EVT:
	{
//-----------------------------------------------------------------------
		//TEST CODE
		__delay_cycles(1000000);
		static int eventCount = 0;
		eventCount++;
		if(eventCount== 100)
		{
			eventCount = 0;

		}

//-----------------------------------------------------------------------
		//Current Data length = length from all received messages
		uint8 dataLength = 0;

		static uint8 currDataLength = 0;
		static evtPkt_Gen_s * tempEvtPkt = NULL;
		//Retrieve First relevant message for a specific event from queue
		LinearBuffer_s *tempMsg = (LinearBuffer_s *)scheduler_receive_Msg(taskId,UART_A_RX_EVT);

		//Update Data length
		currDataLength += tempMsg->dataEnd;

	//Check for First set of bytes of incoming packet
	if(currDataLength <= BUFFERSIZE)
	{
		//New event Packet
		tempEvtPkt = osal_mem_alloc(sizeof(evtPkt_Gen_s));

		dataLength = tempMsg->linBuffer[EVTDATALENINDEX];// - PKTOFFSET;

		//Allocate new array of datalen - PKTOFFSET bytes(Don't include header info in data section)
		uint8 *pktData = (uint8 *)osal_mem_alloc(dataLength);

		if(pktData == NULL)
			ERRORFLAG = UART_NULL_ERROR;

		tempEvtPkt->eventCode = tempMsg->linBuffer[1];
		tempEvtPkt->totalParamLen = dataLength;
		tempEvtPkt->pData = pktData;

	}




		//void copyArr(uint8 *src,uint8 *dst,uint8 src_start,uint8 src_end,uint8 dst_start)



		do{
			//Copy From queueBuffer
			if(currDataLength <= BUFFERSIZE)
					copyArr(tempMsg->linBuffer,tempEvtPkt->pData,PKTOFFSET,tempMsg->dataEnd,0);
			else
				copyArr(tempMsg->linBuffer,tempEvtPkt->pData,0,tempMsg->dataEnd,currDataLength - PKTOFFSET - tempMsg->dataEnd);



			//Release Buffer for UART ISR
			tempMsg->isInUse = 0;
			tempMsg->dataEnd = 0;
			UPDATE_BUFF_AMT = 1;

			//Get new MSG
			if(currDataLength < dataLength + PKTOFFSET)
			{
				tempMsg = (LinearBuffer_s *)scheduler_receive_Msg(taskId,UART_A_RX_EVT);

				//Update Data length
				currDataLength += tempMsg->dataEnd;
			}
			else
			{
				switch((uint8)tempEvtPkt->pData[1])
				{
				case GAP_EVT:
				{
					//Send MSG
					scheduler_send_Msg(BLE_TASK_ID,GAP_EVT_EVT,(void*)tempEvtPkt);
					//Set Event
					scheduler_set_Evt(BLE_TASK_ID,GAP_EVT_EVT);

				}break;
				case GAP_CMD:
				{
					//Set Event
					scheduler_set_Evt(BLE_TASK_ID,GAP_CMD_EVT);

				}break;
				case GATT_EVT:
				{
					//Send MSG
					scheduler_send_Msg(BLE_TASK_ID,GATT_EVT_EVT,(void*)tempEvtPkt);
					//Set Event
					scheduler_set_Evt(BLE_TASK_ID,GATT_EVT_EVT);

				}break;
				case GATT_CMD:
				{
					//Set Event
					scheduler_set_Evt(BLE_TASK_ID,GATT_CMD_EVT);

				}break;

				};

				//Reset for next packet
				currDataLength = 0;

				break;
			}
		}while(1);

		LinearBuffer_s * tmpHead = (LinearBuffer_s *)peekQueue(taskId,events);

		if(tmpHead!= NULL)
		{
			//Check if more events of this type need to be processed
			if((uint8)tmpHead->linBuffer[2] <= (getQueueLength(taskId,events) * BUFFERSIZE))
				break;
		}

		//Mask out completed event
		if(getQueueLength(taskId,events) == 0)
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
/*
	P4SEL |= BIT4+BIT5;                       // P3.3,4 = USCI_A0 TXD/RXD
	UCA1CTL1 |= UCSWRST;                      // **Put state machine in reset**
	UCA1CTL1 |= UCSSEL_2;                     // SMCLK
	UCA1BR0 = 0xD9;   //0x2C; //                //  0x2C,0x0A = 25Mhz 9600 | 0xD9,0x00 = 25MHz 115200
	UCA1BR1 = 0; //0x0A;//                              // 25MHz 115200
	UCA1MCTL = 0;           // --Modulation UCBRSx=1, UCBRFx=0
	UCA1CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**

	UCA1IE |= UCRXIE;                         // Enable USCI_A1 RX interrupt
*/

	P3SEL |= BIT3 + BIT4;                      // P3.3,4 = USCI_A0 TXD/RXD
	UCA0CTL1 |= UCSWRST;                      // **Put state machine in reset**
	UCA0CTL1 |= UCSSEL_2;                     // SMCLK
	UCA0BR0 = 0xD9;   //0x2C; //                //  0x2C,0x0A = 25Mhz 9600 | 0xD9,0x00 = 25MHz 115200
	UCA0BR1 = 0; //0x0A;//                              // 25MHz 115200
	UCA0MCTL = 0;           // --Modulation UCBRSx=1, UCBRFx=0
	UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**

	UCA0IE |= UCRXIE;                         // Enable USCI_A1 RX interrupt






	_bis_SR_register(GIE);
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

/*********************************************************************
 * @fn      copyArr
 *
 * @brief
 *
 *      This function copies the elements of a source array into the destination array.
 *
 * @param   uint8 *src - Source array
 * @param   uint8 *dst	- Destination Array
 *
 * @param	uint8 src_start - Starting index for source array(To be copied from)
 * @param	uint8 src_end - Ending index for source array
 * @param	uint8 dst_start - sStarting index for destination array
 *
 * @return  void
 */
void copyArr(uint8 *src,uint8 *dst,uint8 src_start,uint8 src_end,uint8 dst_start)
{

	uint8 dst_currEl = dst_start;
	uint8 src_currEl = src_start;

	while(src_currEl < src_end)
	{
		dst[dst_currEl] = src[src_currEl];
		src_currEl++;
		dst_currEl++;
	}
}


/*********************************************************************
 * @fn     name
 *
 * @brief
 *
 *    This function returns the message data in the event messages array
 *	  corresponding to the input event and taskId.
 *
 * @param   uint8 task_id - task ID
 * @param   uint8 event	  - event
 *
 * @return  void * data - Data stored at front of queue for specific task & event
 */
