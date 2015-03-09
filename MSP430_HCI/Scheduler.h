/**
  Headerfile:       Scheduler.h

  Author: Alexander D'Abreu
  Based upon Texas Instruments Operating Systems Abstraction layer
  task scheduling software

  Revised:        $Date: 2015-3-1   (Sun, 1 Mar 2015) $
  Revision:       $Revision: 1 $

  Description:    Main Scheduling system to manage system events to handle incoming HCI packets

**************************************************************************************************/
#ifndef SCHEDULER_H
#define SCHEDULER_H

/*********************************************************************
 * INCLUDES
 */
	#include "comdef.h"
	#include "UART_HCI.h"
	#include "GAP_HCI.h"
	#include "Mem_Manager.h"

/*********************************************************************
 * CONSTANTS
 */
#define NUMOFTASKS 4
#define NUMOFEVENTS 5
#define RXOFFSET 3
#define NO_TASK_ACTIVE 20
/*********************************************************************
 * TYPEDEFS
 */
struct QueueNode_s{
	void *data;
	struct QueueNode_s *next;

};
typedef struct{
	struct QueueNode_s *head,*tail;
	uint8 length;

}Queue_s;

//What each event handler function will need, each will return a status and take the
//event that caused the function to be called and the task id calling it to reference the global message array
typedef uint8 (*pTaskEventHdrfn)(uint8 taskId,uint8 events);


/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */
	extern	uint8 UART_ProcessEvent(uint8 taskId,uint8 events);
	extern	uint8 BLE_ProcessEvent(uint8 taskId,uint8 events);
	extern	uint8 SensorTag_ProcessEvent(uint8 taskId,uint8 events);
	extern  uint8 Transciever_ProcessEvent(uint8 taskId,uint8 events);

/*********************************************************************
 * FUNCTIONS
 */

/*
* Calls scheduler_init_tasks and run an infinite loop of scheduler_run_system.
	*/
	extern void scheduler_start_system( void );
/*
* Makes a single pass through the events array and calls callback functions if a event needs processing.
	*/
	extern void scheduler_run_system( void );
/*
* Initializes all Tasks and System level clock rates.
	*/
	extern uint8 scheduler_init_system( void );

/*
* Initializes all Tasks and System level clock rates.
	*/

	extern void schedulerInitTasks();




/*********************************************************************
* Message Sending Functions
 */

/*
* Sets an event for a specific task.
	*/
	extern uint8 scheduler_set_Evt(uint8 taskId,uint8 event_Flag);
/*
* Clears an event for a specific task.
	*/
	extern uint8 scheduler_clear_Evt(uint8 taskId, uint8 eventFlag);
/*
* Adds a message to the message queue for a specific task and event.
	*/
	extern uint8 scheduler_send_Msg(uint8 taskId,uint8 eventFlag,void *data);
	/*
* Returns a pointer to the data held in the message queue.
	*/
	extern void * scheduler_receive_Msg(uint8 taskId,uint8 eventFlag);
/*********************************************************************
* Message Sending Helpers
 */

/*
* Return a pointer to an newly allocated QueueNode structure.
	*/
	extern struct QueueNode_s *createNode(void *);

/*
* Return a pointer to an empty Queue structure.
	*/

	extern Queue_s* initializeQueue();

/*
* Return a status indicating if an input queue is empty.
	*/

	extern uint8 isEmpty(Queue_s *inputQueue);

/*
* Return status of the successful addition of a node to the back of the queue.
	*/

	extern uint8 enqueue(Queue_s *inputQueue,struct QueueNode_s *newNode);

/*
* Return a Queue_s structure from the front of the queue.
	*/

	extern struct QueueNode_s * dequeue(Queue_s *inputQueue);

/*
* Delete specific queue node by freeing its memory.
	*/
	extern void deleteQueueNode(struct QueueNode_s *inputNode);

/*
* Delete specific queue node and data by freeing its memory.
	*/
	extern void deleteQueueNodeData(struct QueueNode_s *);


/*********************************************************************
*********************************************************************/

 /* SCHEDULER_H */
#endif

