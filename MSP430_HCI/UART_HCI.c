


#define NUMOFBUFFERS 4
#define BUFFERSIZE 30
#define RXBUFFER_INIT 1
#define TXBUFFER_INIT 0

#define isBuffFull(LinBuff) ((LinBuff.dataEnd) == BUFFERSIZE)
#define isBuffEmpty(LinBuff) ((LinBuff.dataEnd) == 0)
#define isBuffInRange(BuffVal) ((BuffVal) < NUMOFBUFFERS)
typedef struct
{
	char *linBuffer;//[BUFFERSIZE];
	char isInUse; //Boolean true of false
	int dataEnd; //End of Valid Data


}LinearBuffer_s;


typedef struct
{
	LinearBuffer_s *circBuffer[NUMOFBUFFERS]; //The Actual Circular Buffer is an array of pointers

	int length;	//Length of the circular Array
	/*
	static inside ISR	static int currBuffer;	//Index of Linear Buffer being read/written to
	*/

}RingBuffer_s;



	//If RX Buffer set to 1, else TX Buffer set to 0
RingBuffer_s * initializeBuffer(int RXBuffer)
{

	//Create memory blocks for both circular and Linear Buffers
	RingBuffer_s *inputBuffer;
	int currBuffer;

	inputBuffer = (RingBuffer_s*)malloc(sizeof(RingBuffer_s));//OSALMEMALLOC(BUFFERSIZE);
	inputBuffer->length = NUMOFBUFFERS;

	//Allocate memory for the LinearBuffer structure and internal Buffer(array)
	for(currBuffer = 0; currBuffer < NUMOFBUFFERS;currBuffer++)
	{
		inputBuffer->circBuffer[currBuffer] = (LinearBuffer_s *)malloc(sizeof(LinearBuffer_s));//OSALMEMALLOC(BUFFERSIZE);

		if(RXBuffer)	//Do not allocate Linear Array for Transmit Buffers
			inputBuffer->circBuffer[currBuffer]->linBuffer = (char *)malloc(BUFFERSIZE * sizeof(char));//OSALMEMALLOC(BUFFERSIZE);
		else
			inputBuffer->circBuffer[currBuffer]->linBuffer = NULL;

		//Initialize internal Buffer
		inputBuffer->circBuffer[currBuffer]->isInUse = 0; //Reset all flags
		inputBuffer->circBuffer[currBuffer]->dataEnd = 0; //Initally empty arrays

		/*for(i = 0; i < inputBuffer->circBuffer[currBuffer]->dataEnd;i++)
			inputBuffer->circBuffer[currBuffer]->linBuffer[i] = 'A';
			*/
	}



		return inputBuffer;
}

//Return Status 0 = SUCCESS ,OTHER = FAILURE
//Add a BYTE to a buffer in the ring. Internal/External Buffer overflow possible
//			Circular Buffer input	,	linear Buffer you want to write to, byte you want to write
int addToBuffer(RingBuffer_s *inputBuffer,int currBuffer,char byteInput)
{
	//Check if current buffer selection is in range
	if(!(isBuffInRange(currBuffer)))
		return FAILURE;


	//Check for Valid inputBuffer
	if((inputBuffer != NULL) && (inputBuffer->circBuffer[currBuffer] != NULL) && (inputBuffer->circBuffer[currBuffer]->linBuffer != NULL))
	{
		//Check to see if the Buffer wanting to be used is in-use by another portion of the application
		if (!(inputBuffer->circBuffer[currBuffer]->isInUse))
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
	}
		return FAILURE;
}

int removeFromBuffer(RingBuffer_s *inputBuffer,int currBuffer,char &ByteLocation)
{
	//Check if current buffer selection is in range
	if(!(isBuffInRange(currBuffer)))
		return FAILURE;

	//Check for Valid inputBuffer
	if((inputBuffer != NULL) && (inputBuffer->circBuffer[currBuffer] != NULL) && (inputBuffer->circBuffer[currBuffer]->linBuffer != NULL))
	{
		//Check if Buffer is in use by another function
		if (!(inputBuffer->circBuffer[currBuffer]->isInUse))
		{

			//Check if buffer is empty
			if(!isBuffEmpty((*(inputBuffer->circBuffer[currBuffer]))))
			{
				cout << "DataEnd in Remove function is: " << 	inputBuffer->circBuffer[currBuffer]->dataEnd << endl;

				inputBuffer->circBuffer[currBuffer]->dataEnd--;
				int temp_dataEnd = inputBuffer->circBuffer[currBuffer]->dataEnd;

				ByteLocation = inputBuffer->circBuffer[currBuffer]->linBuffer[temp_dataEnd];
				cout << "Removing : " << ByteLocation << endl;

				return SUCCESS;
			}

		}
	}
	return FAILURE;

}







































/**************************************************************************************************
  Filename:       UART_HCI.c
  Revised:        $Date: 2015-03-01 (Sun, 1 Mar 2015) $
  Revision:       $Revision: 35416 $

  Description:    UART Host Controller Interface between MSP430F5529 and CC2540

**************************************************************************************************/

/*********************************************************************
 * INCLUDES
 */
#include "comdef.h"


/*********************************************************************
 * MACROS
 */

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

/*********************************************************************
 * @brief   Set a GAP Role parameter.
 *
 * Public function defined in peripheral.h.
 */