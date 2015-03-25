/**************************************************************************************************
  Filename:       Scheduler.c
  Revised:        $Date: 2012-02-02 12:55:32 -0800 (Thu, 02 Mar 2015) $
  Revision:       $Revision: 1 $

  Description:    This API allows the software components in the Z-stack to be written
                  independently of the specifics of the operating system, kernel or tasking
                  environment (including control loops or connect-to-interrupt systems).


**************************************************************************************************/

/*********************************************************************
 * INCLUDES
 */
	#include "Scheduler.h"



/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

/*
 * Tasks array with event handler Functions.
 */
	const pTaskEventHdrfn tasksArr[NUMOFTASKS] = {
		 UART_ProcessEvent,
		 BLE_ProcessEvent,
		 SensorTag_ProcessEvent,
		 Transciever_ProcessEvent
	};



		uint8 SensorTag_ProcessEvent(uint8 taskId,uint8 events){
		return 1;
	}
	 uint8 Transciever_ProcessEvent(uint8 taskId,uint8 events){
		return 1;
	}




/*
 * Event Flags for each Task.
 */
	const uint8 tasksCnt = sizeof( tasksArr ) / sizeof( tasksArr[0] );

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */

/*
 * Message array holding a queue data structure for each tasks' events
 */
	Queue_s * eventMessageArray[NUMOFTASKS][NUMOFEVENTS];
	uint8 ERRORFLAG = 0;

/*********************************************************************
 * LOCAL VARIABLES
 */
	//Global event flags array for each task.
	static uint8 events[NUMOFTASKS];
	//Holds ID of active task.
	static uint8 activeTaskId;

/*********************************************************************
 * HELPER FUNCTIONS
 */

/*********************************************************************
 * @fn      scheduler_send_Msg
 *
 * @brief
 *
 *    This function is called to set the event flags for a task. The
 *    event passed in is OR'd into the task's event variable.
 *
 * @param   uint8 task_id - receiving tasks ID
 * @param   uint8 event_flag - what event to set
 * @param	void * data - Block of data to be sent
 * @return  SUCCESS, INVALID_TASK
 */
uint8 scheduler_send_Msg(uint8 taskId,uint8 eventFlag,void *data)
{
	//Check if valid taskId->Allocate new Queue Node->add data to node->add to queue for specific event-> set event flag
	//Check for valid Task input
	if(taskId < tasksCnt)
	{
		//enqueue(eventMessageArray[taskId][eventFlag - 1],data);
		//-----------------------------------------------------------------------
				//TEST CODE
				//__delay_cycles(1000);

				if(enqueue(eventMessageArray[taskId][eventFlag - 1],data) == FAILURE)
				{
					ERRORFLAG = SCHEDULER_QUEUE_ERROR;
				}

		//-----------------------------------------------------------------------

		return SUCCESS;
	}
	else
		return FAILURE;

}


/*********************************************************************
 * @fn      scheduler_recieve_Msg
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
void * scheduler_receive_Msg(uint8 taskId,uint8 eventFlag)
{
	//Check for valid Task input
	if(taskId < tasksCnt)
	{
		//Valid Events [1->8], array from 0 to 7
		void *temp = dequeue(eventMessageArray[taskId][eventFlag - 1]);
		return temp;
	}
	else
		return NULL;

}


/*********************************************************************
 * @fn      scheduler_clear_Evt
 *
 * @brief
 *
 *    This function is called to clear the event flags for a task. The
 *    event passed in is masked out of the task's event variable.
 *
 * @param   uint8 task_id - receiving tasks ID
 * @param   uint8 event_flag - what event to clear
 *
 * @return  SUCCESS, INVALID_TASK
 */
uint8 scheduler_clear_Evt(uint8 taskId, uint8 eventFlag)
{
	if(taskId < tasksCnt)
	{
		events[taskId] &= ~eventFlag;
		return SUCCESS;
	}
	else
		return FAILURE;
}



/*********************************************************************
 * @fn      scheduler_set_Evt
 *
 * @brief
 *
 *    This function is called to set the event flags for a task. The
 *    event passed in is masked to the task's event variable.
 *
 * @param   uint8 task_id - receiving tasks ID
 * @param   uint8 event_flag - what event to clear
 *
 * @return  SUCCESS, INVALID_TASK
 */


uint8 scheduler_set_Evt(uint8 taskId, uint8 eventFlag)
{
	if(taskId < tasksCnt)
	{
		events[taskId] |= eventFlag;
		return SUCCESS;
	}
	else
		return FAILURE;
}


/*********************************************************************
 * Main Scheduler FUNCTIONS
 */
/*********************************************************************
 * @fn      scheduler_Init_Tasks
 *
 * @brief
 *
 *    This function calls Task specific Initialization functions
 *
 * @param   void
 *
 * @return  void
 */

void scheduler_Init_Tasks()
{
	UART_Init();
	BLE_Init();


}
/*********************************************************************
 * @fn      scheduler_init_system
 *
 * @brief
 *
 *   Calls function to set up the MSP430f5529 and Bluetooth HCI
 *
 * @param   void
 *
 * @return  SUCCESS
 */

uint8 scheduler_init_system( void )
{


	initializeMessageArray();



/*
 * TEST CODE FOR QUEUE
	char *temp = (char*)osal_mem_alloc(3);
	temp[0] = 'A';
	//enqueue(eventMessageArray[0][0],(void*)temp);
	char*temp2 = (char*)osal_mem_alloc(3);
	//temp2=(char*)dequeue(eventMessageArray[0][0]);
	scheduler_send_Msg(0,1,temp);
	scheduler_send_Msg(0,1,temp);
	scheduler_send_Msg(0,1,temp);
	scheduler_send_Msg(0,1,temp);


	if(scheduler_send_Msg(0,1,temp) == FAILURE)
		while(1);
*/

	//Scheduler_MCU_Init() //Set Clock Rate etc.


  // Initialize the system tasks.
  scheduler_Init_Tasks();

  // Setup efficient search for the first free block of heap.
  osal_mem_kick();

  return (SUCCESS);
}





/*********************************************************************
 * @fn      osal_start_system
 *
 * @brief
 *
 *   This function calls the system wide initialization function inaddition to
 *	 placing the scheduler_run_system in an infinite loop.
 *
 * @param   void
 *
 * @return  none
 */
void scheduler_start_system( void )
{
	//Lock system if initialization Fails
		if(scheduler_init_system() != SUCCESS)
			ERRORFLAG = SCHEDULER_INIT_FAILURE;

		//Infinite Loop
		while(1){
			scheduler_run_system();
		}
}
/*********************************************************************
 * @fn      scheduler_run_system
 *
 * @brief
 *
 *   This function will make one pass through the taskEvents array
 *   and call the task_event_processor() function for the first task that
 *   is found with at least one event pending. If there are no pending
 *   events (all tasks), this function puts the processor into Sleep.
 *
 * @param   void
 *
 * @return  none
 */

void scheduler_run_system( void )
{
	uint8 taskId = 0;

	//Check for potential program wide errors
	if(ERRORFLAG)
	{
		while(1);
	}


	do
	{
		//Find highest priority Task
		if(events[taskId])
		{
			break;
		}
	}while(++taskId < tasksCnt);


	if(taskId < tasksCnt)
	{
		HAL_ENTER_CRITICAL_SECTION();

		//Temporary events
		uint8 temp_events = events[taskId];
		events[taskId] = 0;

		HAL_EXIT_CRITICAL_SECTION();

		activeTaskId = taskId;
		//Call event handler function with its events
		temp_events = (*tasksArr[taskId])(taskId,temp_events);
		activeTaskId = NO_TASK_ACTIVE;

		HAL_ENTER_CRITICAL_SECTION();

		//Add any unfinished events
		events[taskId] |= temp_events;



		HAL_EXIT_CRITICAL_SECTION();


		 //_bis_SR_register(LPM0_bits + GIE);	   // Enter LPM0, interrupts enabled

	}
}





/*********************************************************************
 * @fn      enqueue
 *
 * @brief
 *
 *   This function adds an input data to the back of a queue structure.
 *
 * @param   Queue_s * structure,void * data
 *
 * @return  status: SUCCESS or FAILURE
 */
uint8 enqueue(Queue_s *inputQueue,void* data)
{
	uint8 next_head = (inputQueue->head + 1) % NUM_OF_QUEUE_ELEMENTS;
	//Queue Still has space
	if(next_head != inputQueue->tail)
	{
		//Add data to queue
		inputQueue->pQueue[inputQueue->head] = data;
		inputQueue->head = next_head;

		return SUCCESS;
	}
	else
		return FAILURE;


}


/*********************************************************************
 * @fn      dequeue
 *
 * @brief
 *
 *   This function removes a node fron the front of the queue.
 *
 * @param   Queue_s * structure
 *
 * @return  QueueNode_s * : element removed
 */
void * dequeue(Queue_s *inputQueue)
{
	void *temp;
	//Queue is not empty
	if(inputQueue->head != inputQueue->tail)
	{
		temp = inputQueue->pQueue[inputQueue->tail];

		inputQueue->tail = (inputQueue->tail + 1) % NUM_OF_QUEUE_ELEMENTS;

		return temp;
	}
	else
		return NULL;
}
/*********************************************************************
 * @fn      initializeQueue
 *
 * @brief
 *
 *   This function creates a new Queue and initialzes the head/tail pointers to NULL.
 *
 * @param
 *
 * @return  Queue_s * structure
 */
//Allocate new memory for queue with NUM_OF_QUEUE_ELEMENTS(Array size)
Queue_s* initializeQueue()
{
	Queue_s* tempQueue = (Queue_s *) osal_mem_alloc(sizeof(Queue_s));

	tempQueue->tail = 0;
	tempQueue->head = 0;

	return tempQueue;
}

/*********************************************************************
 * @fn      initializeMessageArray
 *
 * @brief
 *
 *   This function allocates memory for a the global queue structure with unique queue structures
 *	 for all events and tasks.
 *
 * @param  void
 *
 * @return   void
 */

void initializeMessageArray()
{
	int i,j;
	//Allocate Queue Memory
	for(i = 0; i < NUMOFTASKS; i++)
	{
		for(j = 0; j < NUMOFEVENTS; j++)
		{
			eventMessageArray[i][j] = initializeQueue();
		}
	}
}

