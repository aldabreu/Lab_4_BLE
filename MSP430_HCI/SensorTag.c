/**************************************************************************************************
  Filename:       SensorTag.c
  Revised:        $Date: 2015-4-10   (Thur, 10 Apr 2015) $
  Revision:       $Revision: 1 $

  Description:    GAP Peripheral Role

**************************************************************************************************/

/*********************************************************************
 * INCLUDES
 */
#include "SensorTag.h"


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
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES Static
 */





void masterDeviceInit(void)
{
	cmdPkt_Gen_s *tempMsg = osal_mem_alloc(sizeof(cmdPkt_Gen_s));
	//Add OpCode for BLE Device Init command
	tempMsg->opCode[0] = 0xFE;
	tempMsg->opCode[1] = 0x00;

	//No Extra data necessary
	tempMsg->pData = NULL;

	//Send Message
	scheduler_send_Msg(BLE_TASK_ID,GAP_CMD_EVT,(void*)tempMsg);
	//Set Event
	scheduler_set_Evt(BLE_TASK_ID,GAP_CMD_EVT);

}
void startDeviceDiscovery(void)
{
	cmdPkt_Gen_s *tempMsg = osal_mem_alloc(sizeof(cmdPkt_Gen_s));
	tempMsg->opCode[0] = 0xFE;
	tempMsg->opCode[1] = 0x04;


	tempMsg->pData = NULL;

	scheduler_send_Msg(BLE_TASK_ID,GAP_CMD_EVT,(void*)tempMsg);
	//Set Event
	scheduler_set_Evt(BLE_TASK_ID,GAP_CMD_EVT);

}
//Begins the Pairing process for a specific sensorTag
void establishDeviceLink(uint8 devIndex)
{

	cmdPkt_Gen_s *tempMsg = osal_mem_alloc(sizeof(cmdPkt_Gen_s));
	tempMsg->opCode[0] = 0xFE;
	tempMsg->opCode[1] = 0x09;

	tempMsg->pData = osal_mem_alloc(1);
	//Peer Address index from Device DB
	tempMsg->pData[0] = devIndex;

	scheduler_send_Msg(BLE_TASK_ID,GAP_CMD_EVT,(void*)tempMsg);
	//Set Event
	scheduler_set_Evt(BLE_TASK_ID,GAP_CMD_EVT);
}

void authenticateDevice(uint8 connectedDeviceIndex)
{
	cmdPkt_Gen_s *tempMsg = osal_mem_alloc(sizeof(cmdPkt_Gen_s));
	tempMsg->opCode[0] = 0xFE;
	tempMsg->opCode[1] = 0x0B;

	tempMsg->pData = osal_mem_alloc(1);
	//Peer Address index from Device DB
	tempMsg->pData[0] = connectedDeviceIndex;

	scheduler_send_Msg(BLE_TASK_ID,GAP_CMD_EVT,(void*)tempMsg);
	//Set Event
	scheduler_set_Evt(BLE_TASK_ID,GAP_CMD_EVT);
}





/*********************************************************************
 * @fn          SensorTag_ProcessEvent
 *
 * @brief
 * 		 This is the event handler function for checking the connection status to the sensorTags.
 *
 * @params 	uint8 taskId
 * 			uint8 events
 *
 * @return uint8 events - Tasks that still need processing
 *
 *  */
uint8 SensorTag_ProcessEvent(uint8 taskId,uint8 events){

	findDevice(NULL,NULL);

	return events;
}
/*********************************************************************
 * @fn         SensorTag_Init
 *
 * @brief
 * 		 This function initializes the SensorTag's by setting up bonding events.
 *
 * @params 	void
 *
 * @return VOID
 *
 *  */
void SensorTag_Init(void)
{

/*	To be included in function to manage Connection status
	int index;
	for(index = 0; index < NUMOFDEVICES;index++)
	{
		if(bleDeviceDB[index]->GAPState == GAP_DISCOVEREDDEVICE)
			establishDeviceLink(index);

	}
*/



}

