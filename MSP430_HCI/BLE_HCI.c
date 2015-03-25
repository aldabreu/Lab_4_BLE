/**************************************************************************************************
  Filename:       peripheral.c
  Revised:        $Date: 2013-09-23 08:15:44 -0700 (Mon, 23 Sep 2013) $
  Revision:       $Revision: 35416 $

  Description:    GAP Peripheral Role

**************************************************************************************************/

/*********************************************************************
 * INCLUDES
 */
#include "BLE_HCI.h"


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

static MBLEDevice_s bleMaster;
static PBLEDevice_s *bleDeviceDB[NUMOFDEVICES];

static uint8 numOfDevices_InRange = 0;
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
 * @fn          BLE_ProcessEvent
 *
 * @brief
 * 		 This is the event handler function for all BLE processes for GAP and
 *  	 GATT commands and events. It calls message processing functions to
 *  	 decode HCI packets from the BLE Central device.
 *  	 interrupt is not called.
 *
 * @params 	uint8 taskId
 * 			uint8 events
 *
 * @return uint8 events - Tasks that still need processing
 *
 *  */
uint8 BLE_ProcessEvent(uint8 taskId,uint8 events){

	switch(events)
	{
	case GAP_EVT_EVT:{

		//Retrieve Message from Global Queue
		evtPkt_Gen_s  *tempMsg = (evtPkt_Gen_s *)scheduler_receive_Msg(taskId,GAP_EVT_EVT);
		uint8 event,status;

		if(tempMsg != NULL)
		{
			event = tempMsg->pData[0];
			status = tempMsg->pData[2];
		}
		else
		{
			ERRORFLAG = BLE_NULL_ERROR;
		}
		if(status != SUCCESS)
			ERRORFLAG = BLE_FAILURE;

		//Decode GAP Event
		switch(event)
		{
		case GAP_DeviceInitDone:
		{
			uint8 *IRK = (uint8*)osal_mem_alloc(16);
			uint8 *CSRK = (uint8*)osal_mem_alloc(16);
			uint8 *devAddr = (uint8*)osal_mem_alloc(6);



		uint16 heapBlockCnt = osal_heap_block_free();
		uint16 maxMem = osal_heap_mem_used();



			//Copy Data to new memory locations
			copyArr(tempMsg->pData,devAddr,3,9,0);
			copyArr(tempMsg->pData,IRK,12,28,0);
			copyArr(tempMsg->pData,CSRK,28,44,0);

			//Add Device Information to MASTER BLE Device
			bleMaster.devAddr = devAddr;
			bleMaster.IRK = IRK;
			bleMaster.CSRK = CSRK;


		}break;
		case GAP_DeviceDiscovery:
		{
			//Obtain number of devices found
			numOfDevices_InRange = tempMsg->pData[3];

			//Obtain specific Device information from Device information event

			//Set event to start Pairing process(if Devices found)

		}break;

		case GAP_LinkEstablished:
		{

		}break;

		case GAP_LinkTerminated:
		{

		}break;
		case GAP_SignatureUpdated:
		{

		}break;
		case GAP_DeviceInformation:
		{
			//Event representing a device's advertising data OR scan response packet
						uint8 eventType = tempMsg->pData[3];
						uint8 addrType = tempMsg->pData[4];
						uint8 rssi = tempMsg->pData[11];
						uint8 dataLen = tempMsg->pData[12];


						uint8 *devAddr = (uint8*)osal_mem_alloc(6);
						uint8 *advData = (uint8*)osal_mem_alloc(dataLen);


						//Copy Data to new memory locations
						copyArr(tempMsg->pData,devAddr,5,11,0);
						copyArr(tempMsg->pData,advData,13,dataLen + 13,0);

			//Find Or add new BLE device to database

			updateDeviceDB(GAP_DISCOVEREDDEVICE,SUCCESS,devAddr,0,NULL,dataLen,advData,rssi,eventType,addrType);






		}break;
		case GAP_PassKeyNeeded:
		{

		}break;
		case GAP_BondComplete:
		{

		}break;
		case GAP_AuthenticationComplete:
		{

		}break;
		case GAP_CommandStatus:
		{

		}break;










		};


		 //Deallocate memory for received message
		 osal_mem_free((void*)tempMsg->pData);
		 osal_mem_free((void*)tempMsg);

		//Mask out completed events
		events &= ~GAP_EVT_EVT;


	}break;
	case GAP_CMD_EVT:break;
	case GATT_EVT_EVT:{
		//Retrieve Message from Global Queue
			evtPkt_Gen_s  *tempMsg = (evtPkt_Gen_s *)scheduler_receive_Msg(taskId,GATT_EVT_EVT);

			//Decode GAP Event
			switch(tempMsg->eventCode)
			{



			};

			//Mask out completed events
			events &= ~GATT_EVT;


	}break;
	case GATT_CMD_EVT:break;


	};


	return events;
}
/*********************************************************************
 * @fn         BLE_Init
 *
 * @brief
 * 		 This function initializes the BLE Functions such as the device
 * 		 Database.
 *
 * @params 	void
 *
 * @return VOID
 *
 *  */
void BLE_Init(void)
{
	uint8 i = 0;
	//Set default state to all devices in Database
 	for(i = 0; i < NUMOFDEVICES;i++)
	{
 		//Allocate new memory
 		bleDeviceDB[i] = osal_mem_alloc(sizeof(PBLEDevice_s));

 		bleDeviceDB[i]->GAPState = GAP_FREEDEVICE;
		bleDeviceDB[i]->CMDStatus = SUCCESS;
		bleDeviceDB[i]->connHandle = 0;
		bleDeviceDB[i]->LTK = NULL;
		bleDeviceDB[i]->addrType = 0;
		bleDeviceDB[i]->dataLen = 0;
		bleDeviceDB[i]->dataField = NULL;
		bleDeviceDB[i]->rssi = 0;
		bleDeviceDB[i]->advEvtType = 0;
	}


 	bleMaster.CSRK = NULL;
 	bleMaster.IRK = NULL;
 	bleMaster.connHandle = 0;
 	bleMaster.devAddr = NULL;
 	bleMaster.numOfConnectedDevices = 0;


}
/*********************************************************************
 * @fn         addNewDevice
 *
 * @brief
 * 		 This function finds an open slot in the device DB and adds the
 * 		 address
 *
 * @params 	uint8 *devAddr - Device Address
 *
 *
 * @return PBLEDevice_s * - if open slot, NULL - DB Full
 *
 *  */
PBLEDevice_s *addNewDevice(uint8 * devAddr)
{
	uint8 i;

	//Valid input address
	if(devAddr == NULL)
		return NULL;

	for(i = 0; i < NUMOFDEVICES;i++)
	{
		//Check DB for open slot by status
		if(bleDeviceDB[i]->GAPState == GAP_FREEDEVICE)
		{
			bleDeviceDB[i]->devAddr = devAddr;
			bleDeviceDB[i]->GAPState = GAP_DISCONNECTEDDEVICE;
			return bleDeviceDB[i];

		}
	}
	//If NULL Returned, the device DB is full -- ERROR
	return NULL;
}

/*********************************************************************
 * @fn         findDevice
 *
 * @brief
 * 		 This function finds a specific Bluetooth device and returns
 * 		 a pointer to the structure containing its information.
 * 		 Finds the device by address.
 *
 * @params 	uint8 *devAddr - Device Address
 *
 * @return PBLEDevice_s * - if Device found, NULL - otherwise
 *
 *  */
PBLEDevice_s *findDevice(uint8 * devAddr)
{

	uint8 i;
	if(devAddr == NULL)
		return NULL;

	//Attempt to find device by address
	for(i = 0; i < NUMOFDEVICES;i++)
	{
			//Check if in database
			if(compareArray(bleDeviceDB[i]->devAddr,devAddr,6) == SUCCESS)
				return bleDeviceDB[i];
	}
		return NULL;
}

/*********************************************************************
 * @fn         compareArray
 *
 * @brief
 * 		 This function compares two arrays to see if they are the equal.
 *
 * @params 	uint8 *arr1
 * 			uint8 *arr2
 *
 *
 * @return uint8 - SUCCESS or FAILURE
 *  */
uint8 compareArray(uint8 *arr1,uint8 *arr2,uint8 length)
{
	int i;

	for(i = 0; i < length;i++)
	{
		if(arr1[i] != arr2[i])
			return FAILURE;
	}
	return SUCCESS;
}

/*********************************************************************
 * @fn         updateDeviceDB
 *
 * @brief
 * 		 This function adds new information such as a new device
 * 		 or updated security parameters to the device Database.
 * 		 NULL Device Address = Add a new Device to the DB
 *
 * @params
 * 			uint8 GAPState
 * 			uint8 CMDStatus - Current Command for the specific BLE Device status
 * 			uint8 *devAddr - device Address
 * 			uint8 connHandle
 * 			uint8 *IRK
 *			uint8 *CSRK
 *
 *
 * @return VOID
 *
 *  */
void updateDeviceDB(uint8 GAPState,uint8 CMDStatus,uint8 *devAddr,uint8 *connHandle,uint8 *LTK,
					uint8 dataLen, uint8 *data,uint8 rssi,uint8 advEvtType,uint8 addrType)
{
	PBLEDevice_s *currDevice = NULL;

	//Search for Device
	currDevice = findDevice(devAddr);

	//Device not found, add new device
	if(currDevice == NULL)
		currDevice = addNewDevice(devAddr);


	if(GAPState != NULL)
		currDevice->GAPState = GAPState;
	if(CMDStatus != NULL)
		currDevice->CMDStatus = CMDStatus;
	if(connHandle != NULL)
		currDevice->connHandle = connHandle;

	if(LTK != NULL)
	{
		//Replace new key and free memory
		if(currDevice->LTK != NULL)
			osal_mem_free(currDevice->LTK);

		currDevice->LTK = LTK;
	}
	if((dataLen != 0) && (data != NULL))
	{
		//Free memory
		if(currDevice->dataField != NULL)
			osal_mem_free(currDevice->dataField);

		currDevice->dataLen = dataLen;
		currDevice->dataField = data;

	}
	if(rssi != 0)
	{
		currDevice->rssi = rssi;
		currDevice->advEvtType = advEvtType;
		currDevice->addrType = addrType;
	}
}
