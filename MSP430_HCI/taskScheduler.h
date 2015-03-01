/*
 * taskScheduler.h
 *
 *  Created on: Feb 22, 2015
 *      Author: Alexander D'Abreu
 *      Based upon Texas Instruments Operating Systems Abstraction layer
 *      task scheduling software
 */

#ifndef TASKSCHEDULER_H_
#define TASKSCHEDULER_H_

#include "comdef.h"





//Initialization function for all tasks
extern void taskSched_initSystem();
//Begin infinite loop to process and schedule task execution time
extern void taskSched_runSystem();


//Set Task event
extern void taskSched_setTaskEvent(uint8 taskId, uint8 eventFlag);
//Clear Task event
extern void taskSched_clearTaskEvent(uint8 taskId,uint8 eventFlag);



//Message Passing and retrieval functions

//taskId corresponds to task retrieving the message
extern void *getMessage(uint8 taskId,uint8 eventFlag,void *evtMsgArr);

//taskId corresponds to task to send message to
extern void setMessage(uint8 taskId,uint8 eventFlag,void *evtMsgArr);









#endif /* TASKSCHEDULER_H_ */
