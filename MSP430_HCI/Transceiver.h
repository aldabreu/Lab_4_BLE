/**
  Headerfile:      Transceiver.h


  Revised:        $Date: 2015-4-10   (Thur, 10 Apr 2015) $
  Revision:       $Revision: 1 $
  Author: Alexander D'Abreu
  Description:

**************************************************************************************************/
#ifndef TRANSCEIVER_H_
#define TRANSCEIVER_H_

/*********************************************************************
 * INCLUDES
 */
#include "Scheduler.h"


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
extern uint8 Transceiver_ProcessEvent(uint8 taskId,uint8 events);

/*
* SensorTag Initialization function to set up the empty device database.
	*/
extern void Transceiver_Init(void);





/*********************************************************************
*********************************************************************/
#endif /* TRANSCEIVER_H_ */
