





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

	extern uint8 UART_ProcessEvent(uint8 taskId,uint8 events);
	uint8 BLE_ProcessEvent(uint8 taskId,uint8 events){
		return 1;
	}
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
		struct QueueNode_s * temp = createNode(data);
		if(temp != NULL)
		{
			enqueue(eventMessageArray[taskId][eventFlag - 1],temp);
			scheduler_set_Evt(taskId,eventFlag);
			return SUCCESS;
		}
		else
			return FAILURE;
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
	void *nodeData;
	//Check for valid Task input
	if(taskId < tasksCnt)
	{
		//Valid Events [1->8], array from 0 to 7
		struct QueueNode_s * tempNode = dequeue(eventMessageArray[taskId][eventFlag - 1]);
		if(tempNode!= NULL)
		{
			//Store Data temporarily
			nodeData = tempNode->data;

			//Deallocate memory used by node, NOT data
			deleteQueueNode(tempNode);

			return(nodeData);
		}
		else
			return NULL;
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


	int i,j;
	//Allocate Queue Memory
	for(i = 0; i < NUMOFTASKS; i++)
	{
		for(j = 0; j < NUMOFEVENTS; j++)
		{
			eventMessageArray[i][j] = initializeQueue();
			if(eventMessageArray[i][j] == NULL)
				return FAILURE;
		}
	}







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
			while(1);

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


	/*
	static char *temp2,*temp;
	temp = (char*)osal_mem_alloc(10);
	int i;
	for(i = 0; i < 10; i++)
		temp[i] = 'a';

	scheduler_send_Msg(0,1,(void *)temp);

	temp2 = (char*)scheduler_receive_Msg(0,1);
*/



	uint8 taskId = 0;
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
 *   This function adds an input node to the back of a queue structure.
 *
 * @param   Queue_s * structure,QueueNode_s * structure
 *
 * @return  status: SUCCESS or FAILURE
 */

uint8 enqueue(Queue_s *inputQueue,struct QueueNode_s *newNode)
{
	//Check input
	if((newNode == NULL) || (inputQueue == NULL))
		return FAILURE;

	//Case 1 : length = 0
	if(inputQueue->head == NULL)
	{
		newNode->next = NULL;
		inputQueue->tail = newNode;
		inputQueue->head = newNode;
		inputQueue->length++;

	}

	//Case 2 : length  >= 1
	else
	{
		newNode->next = NULL;
		inputQueue->tail->next = newNode;
		inputQueue->tail = inputQueue->tail->next;
		inputQueue->length++;
	}

	return SUCCESS;

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
struct QueueNode_s * dequeue(Queue_s *inputQueue)
{
	//Check if Queue is Empty
	if(isEmpty(inputQueue) || (inputQueue == NULL))
		return NULL;

	struct QueueNode_s *temp = inputQueue->head;
	//Case 1 : length  = 1
    if(inputQueue ->head == inputQueue->tail)
	{

		inputQueue->head = NULL;
		inputQueue->tail = NULL;
		inputQueue->length--;
	}
	//Case 1 : length  > 1
	else
	{
		//struct QueueNode_s *temp = inputQueue->head;
		//Reassign Head pointer
		inputQueue->head = inputQueue->head->next;
		inputQueue->length--;
	}

	return temp;
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

Queue_s* initializeQueue()
{
	//Allocate Memory for Queue Data structure
	Queue_s *temp_queue = (Queue_s *)osal_mem_alloc(sizeof(Queue_s));	//OSALMEMALLOC
	if(temp_queue != NULL)
	{
		temp_queue->head = NULL;
		temp_queue->tail = NULL;
		temp_queue->length = 0;

		return temp_queue;
	}
	else
		return NULL;
}

/*********************************************************************
 * @fn      isEmpty
 *
 * @brief
 *
 *   This function checks to see if a Queue is empty.
 *
 * @param   Queue_s * structure
 *
 * @return  True(1) or False(0)
 */
uint8 isEmpty(Queue_s *inputQueue)
{
	if(inputQueue != NULL)
	{
		if(inputQueue->tail == NULL)
			return 1;
		else
			return 0;
	}
	return 1;
}


/*********************************************************************
 * @fn      deleteQueueNode
 *
 * @brief
 *
 *   This function frees memory used by a queue Node.
 *
 * @param   QueueNode_s * structure
 *
 * @return
 */
void deleteQueueNode(struct QueueNode_s *inputNode)
{
	//Check if input is NULL
	if(inputNode != NULL)
	{
		osal_mem_free((void*)inputNode); //OSALMEMFREE()
	}
}

/*********************************************************************
 * @fn      deleteQueueNode
 *
 * @brief
 *
 *   This function frees all memory used by a queue Node.
 *
 * @param   QueueNode_s * structure
 *
 * @return
 */
void deleteQueueNodeData(struct QueueNode_s *inputNode)
{
	//Check if input is NULL
	if(inputNode != NULL)
	{
		if(inputNode ->data != NULL)
		{
			osal_mem_free((void*)(inputNode->data));
		}
		osal_mem_free((void*)inputNode); //OSALMEMFREE()
	}
}






/*********************************************************************
 * @fn      createNode
 *
 * @brief
 *
 *   This function allocates memory for a new QueueNode_s and places input data inside it.
 *
 * @param  void * : input data
 *
 * @return   QueueNode_s * : structure
 */
struct QueueNode_s *createNode(void *data)
{

	struct QueueNode_s *newNode = (struct QueueNode_s *)osal_mem_alloc(sizeof(struct QueueNode_s)); //OSAL MEMALLOC
	if(newNode != NULL)
	{
		newNode->data = data;
		newNode->next = NULL;

		return newNode;
	}

	else
		return NULL;
 }



