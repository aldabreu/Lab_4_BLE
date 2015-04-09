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
	#include "BLE_HCI.h"
	#include "Mem_Manager.h"

#define DEBUGMODE 0
/*********************************************************************
 * CONSTANTS
 */
#define NUMOFTASKS 4
#define NUMOFEVENTS 5
#define RXOFFSET 3
#define NO_TASK_ACTIVE 20
#define NUM_OF_QUEUE_ELEMENTS 8

//GAP/GATT Specific
#define GAP_EVT 0x06
#define GAP_CMD 0xFE
#define GATT_EVT 0x05
#define GATT_CMD 0xFD


#define UART_TASK_ID 0
#define BLE_TASK_ID  1


//System wide error Flags
//Scheduler
#define SCHEDULER_QUEUE_ERROR 0x01	//Error Adding or removing from the queue
//UART
#define BUFFERFULLERROR 0x02	//UART TX or RX Buffer full
#define BUFFERADDFAILURE 0x03	//Failed to add a byte to the UART Buffer
#define UART_NULL_ERROR 0x04	//Null passed to UART event Processor function
//BLE
#define BLE_FAILURE 0x05	//Overall BLE Critical Failure
#define	BLE_NULL_ERROR 0x06	//NULL Argument passed into BLE Event/Function
#define BLE_CMD_FAILURE 0x07	//Failed to Execute a BLE Command


#define UART_BUFFEROVERFLOW_ERROR 0x08	//UART RXBuffer overflow(Hardware)
#define SCHEDULER_INIT_FAILURE 0x09	//Error in an initialization function wtihin the scheduler


/*********************************************************************
 * TYPEDEFS
 */

typedef struct{
	uint8 head;
	uint8 tail;
	uint8 numOfEl;
	void *pQueue[NUM_OF_QUEUE_ELEMENTS];

}Queue_s;



//What each event handler function will need, each will return a status and take the
//event that caused the function to be called and the task id calling it to reference the global message array
typedef uint8 (*pTaskEventHdrfn)(uint8 taskId,uint8 events);


/*********************************************************************
 * MACROS
 */
/*********************************************************************
 * External Globals
 */
extern uint8 UPDATE_BUFF_AMT;
extern uint8 ERRORFLAG;
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
* Return a pointer to an empty Queue structure.
	*/

	extern Queue_s* initializeQueue();

/*
* Return status of the successful addition of a node to the back of the queue.
	*/

	extern uint8 enqueue(Queue_s *inputQueue,void *data);

/*
* Return a Queue_s structure from the front of the queue.
	*/

	extern void * dequeue(Queue_s *inputQueue);
/*
* Returns the queue length corresponding to a specific tasks' event.
	*/
	extern uint8 getQueueLength(uint8 taskId,uint8 event);

/*
* Returns a pointer to the front of the queue based on a specific task and event.
	*/
	extern void *peekQueue(uint8 taskId,uint8 event);

/*
* Initializes global message array with QueueNode_s 's for each event.
	*/
	extern void initializeMessageArray();




/*********************************************************************
*********************************************************************/

 /* SCHEDULER_H */
#endif

