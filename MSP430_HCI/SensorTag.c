/**************************************************************************************************
  Filename:       SensorTag.c
  Revised:        $Date: 2015-4-10   (Thur, 10 Apr 2015) $
  Revision:       $Revision: 1 $

  Description:

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
extern PBLEDevice_s *bleDeviceDB[NUMOFDEVICES];


/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES Static
 */





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
	//scheduler_set_Evt(BLE_TASK_ID,GAP_CMD_EVT);

}
void startDeviceDiscovery(void)
{
	cmdPkt_Gen_s *tempMsg = osal_mem_alloc(sizeof(cmdPkt_Gen_s));
	tempMsg->opCode[0] = 0xFE;
	tempMsg->opCode[1] = 0x04;


	tempMsg->pData = NULL;

	scheduler_send_Msg(BLE_TASK_ID,GAP_CMD_EVT,(void*)tempMsg);
	//Set Event
	//scheduler_set_Evt(BLE_TASK_ID,GAP_CMD_EVT);

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
	//scheduler_set_Evt(BLE_TASK_ID,GAP_CMD_EVT);
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
	//scheduler_set_Evt(BLE_TASK_ID,GAP_CMD_EVT);
}
void writeCharValue(uint8 deviceIndex,uint8 *attrHandle,uint8 dataLen,uint8 *data)
{

	//0x0031 = Accelerometer Configuration Handle, 0x00 Disable, 0x01 Enable Accelerometer
	//MSG Data contains the address index to the bleDeviceDB pData[0]
	//MSG Data contains 2 Byte attribute Handle to write to	pData[1:2]
	//MSG Data contains 1 Byte Attribute write length pData[3]
	//MSG Data contains N Byte notification data value to write to the device pData[4:N]
		cmdPkt_Gen_s *tempMsg = osal_mem_alloc(sizeof(cmdPkt_Gen_s));
		tempMsg->opCode[0] = 0xFD;
		tempMsg->opCode[1] = 0x92;




		tempMsg->pData = osal_mem_alloc(4 + dataLen);
		//Peer Address index from Device DB & UUID for Command
		tempMsg->pData[0] = deviceIndex;	//BLE Device Index

		tempMsg->pData[1] = attrHandle[1];	//Attribute Handle
		tempMsg->pData[2] = attrHandle[0];

		tempMsg->pData[3] = dataLen;//Attribute Data Length
		copyArr(data,tempMsg->pData,0,dataLen,4);//N Bytes for att



		scheduler_send_Msg(BLE_TASK_ID,GATT_CMD_EVT,(void*)tempMsg);
		//Set Event
		//scheduler_set_Evt(BLE_TASK_ID,GATT_CMD_EVT);
}

//Takes Bonded Device's index in DB,UUID LSB First
void readCharByUUID(uint8 deviceIndex,uint8 *UUID)
{
	//TODO: Remove Finding Device index from function

	int index;
	for(index = 0; index < NUMOFDEVICES;index++)
	{
		if(bleDeviceDB[index]->GAPState == GAP_BONDEDDEVICE)
			break;
	}





	cmdPkt_Gen_s *tempMsg = osal_mem_alloc(sizeof(cmdPkt_Gen_s));
	tempMsg->opCode[0] = 0xFD;
	tempMsg->opCode[1] = 0xB4;


	tempMsg->pData = osal_mem_alloc(3);
	//Peer Address index from Device DB & UUID for Command
	tempMsg->pData[0] = index;
	tempMsg->pData[2] = UUID[0];	//UUID[0:1]
	tempMsg->pData[1] = UUID[1];

	scheduler_send_Msg(BLE_TASK_ID,GATT_CMD_EVT,(void*)tempMsg);
	//Set Event
	//scheduler_set_Evt(BLE_TASK_ID,GATT_CMD_EVT);
}
