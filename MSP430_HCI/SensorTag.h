/**
  Headerfile:       SensorTag.h


  Revised:        $Date: 2015-4-10   (Thur, 10 Apr 2015) $
  Revision:       $Revision: 1 $
  Author: Alexander D'Abreu
  Description:    SensorTag application to manage device connections and Sensor Readings.

**************************************************************************************************/
#ifndef SENSORTAG_H_
#define SENSORTAG_H_

/*********************************************************************
 * INCLUDES
 */
#include "Scheduler.h"
#include "BLE_HCI.h"

/*********************************************************************
 * CONSTANTS
 */



/*********************************************************************
 * TYPEDEFS
 */


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


/*********************************************************************
 * FUNCTIONS
 */

/*
* SensotTag Event processing function.
	*/
extern uint8 SensorTag_ProcessEvent(uint8 taskId,uint8 events);

/*
* SensorTag Initialization function to set up the empty device database.
	*/
extern void SensorTag_Init(void);

extern void masterDeviceInit(void);



extern uint8 commandStatus;






/*
* Helper function to compare two array's.
	*/
extern uint8 compareArray(uint8 *arr1,uint8 *arr2,uint8 length);

/*
* Searches the device database for a specific device by the device address.
	*/
//extern PBLEDevice_s *findDevice(uint8 * devAddr, uint8 *connHandle);


/*
* Copies bytes from one memory location to another.
	*/
extern void copyArr(uint8 *src,uint8 *dst,uint8 src_start,uint8 src_end,uint8 dst_start);
/*********************************************************************
*********************************************************************/



#endif /* SENSORTAG_H_ */

