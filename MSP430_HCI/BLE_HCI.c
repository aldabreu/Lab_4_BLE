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
 * @fn          errorStatusHdr
 *
 * @brief
 * 		 This is the error Code Handler to recover from a bad packet
 * 		 when the BLE Device(CC2540) sends and HCI error code.
 *
 * @params 	uint8 errorCode
 * @return void
 *
 *  */
void errorStatusHdr(uint8 errorCode)
{
	switch(errorCode)
	{

	case bleNotReady:break;

	case bleNotConnected:break;

	case bleTimeout:break;

	case bleGAPUserCanceled:break;
	}

}
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

		if(tempMsg->pData[1] != 0x06)
		{
			while(1);
		}


		if(tempMsg != NULL)
		{
			event = tempMsg->pData[0];
			status = tempMsg->pData[2];
		}
		else
		{
			ERRORFLAG = BLE_NULL_ERROR;
		}

		//For an error condition see what needs to be done to fix the error
		if(status != SUCCESS)
			errorStatusHdr(status);

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
			bleMaster.deviceState = GAP_CONNECTEDDEVICE;

//---------------------------------------------------------------------------------------------------------
			//Test send message
			//Send MSG OPCODE FOR transmission
			cmdPkt_Gen_s *tempMsgx = osal_mem_alloc(sizeof(cmdPkt_Gen_s));
			tempMsgx->opCode[0] = 0xFE;
			tempMsgx->opCode[1] = 0x04;

			//tempMsgx->pData = osal_mem_alloc(1);
			//Peer Address index from Device DB
			tempMsgx->pData = NULL;

			scheduler_send_Msg(BLE_TASK_ID,GAP_CMD_EVT,(void*)tempMsgx);
			//Set Event
			scheduler_set_Evt(BLE_TASK_ID,GAP_CMD_EVT);
//---------------------------------------------------------------------------------------------------------


		}break;
		case GAP_DeviceDiscovery:
		{
			//Obtain number of devices found
			bleMaster.numOfConnectedDevices = tempMsg->pData[3];

			//Obtain specific Device information from Device information event

			//Set event to start Pairing process(if Devices found)
//---------------------------------------------------------------------------------------------------------
			//Test send message
			//Send MSG OPCODE FOR transmission
			cmdPkt_Gen_s *tempMsgx = osal_mem_alloc(sizeof(cmdPkt_Gen_s));
			tempMsgx->opCode[0] = 0xFE;
			tempMsgx->opCode[1] = 0x09;

			int index;
			for(index = 0; index < NUMOFDEVICES;index++)
			{
				if(bleDeviceDB[index]->GAPState == GAP_DISCOVEREDDEVICE)	//Set Events to begin authentication for all sensortags
				{
					tempMsgx->pData = osal_mem_alloc(1);
					//Peer Address index from Device DB
					tempMsgx->pData[0] = index;

					scheduler_send_Msg(BLE_TASK_ID,GAP_CMD_EVT,(void*)tempMsgx);
					//Set Event
					scheduler_set_Evt(BLE_TASK_ID,GAP_CMD_EVT);
				}
			}



//---------------------------------------------------------------------------------------------------------








		}break;

		case GAP_LinkEstablished:
		{
			//Event representing a device's connection parameters(Conn. Time, Interval Timeout..)

			uint8 *devAddr = (uint8*)osal_mem_alloc(6);
			uint8 *connHandle = (uint8*)osal_mem_alloc(2);


			//Copy Data to new memory locations
			copyArr(tempMsg->pData,devAddr,4,10,0);
			copyArr(tempMsg->pData,connHandle,10,12,0);

			//Find BLE device to database
			updateDeviceDB(GAP_CONNECTEDDEVICE,SUCCESS,devAddr,connHandle,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL);

				//Begin Bonding Process
//---------------------------------------------------------------------------------------------------------
			//Test send message
			//Send MSG OPCODE FOR transmission
			cmdPkt_Gen_s *tempMsgx = osal_mem_alloc(sizeof(cmdPkt_Gen_s));
			tempMsgx->opCode[0] = 0xFE;
			tempMsgx->opCode[1] = 0x0B;

			int index;
			for(index = 0; index < NUMOFDEVICES;index++)
			{
				if(bleDeviceDB[index]->GAPState == GAP_CONNECTEDDEVICE)
					break;
			}

			tempMsgx->pData = osal_mem_alloc(1);
			//Peer Address index from Device DB
			tempMsgx->pData[0] = index;

			scheduler_send_Msg(BLE_TASK_ID,GAP_CMD_EVT,(void*)tempMsgx);
			//Set Event
			scheduler_set_Evt(BLE_TASK_ID,GAP_CMD_EVT);
//---------------------------------------------------------------------------------------------------------











		}break;

		case GAP_LinkTerminated:
		{
			//Event representing a device's connection parameters(Conn Time, Interval Timeout..)
			uint8 *connHandle = (uint8*)osal_mem_alloc(2);

			//Copy Data to new memory locations
			copyArr(tempMsg->pData,connHandle,3,5,0);

			//Find BLE device to database
			updateDeviceDB(GAP_DISCONNECTEDDEVICE,SUCCESS,NULL,connHandle,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL);



		}break;
		case GAP_SignatureUpdated:
		{

		}break;
		case GAP_DeviceInformation:
		{
			//Event representing a device's advertising data OR scan response packet
			uint8 eventType = tempMsg->pData[3];
			if(eventType == ScanResponse)	//Only use data from scan response
			{
				uint8 addrType = tempMsg->pData[4];
				uint8 rssi = tempMsg->pData[11];
				uint8 dataLen = tempMsg->pData[12];
				uint8 *advData = (uint8*)osal_mem_alloc(dataLen);
				static uint8 TI_SENSORTAG_SCANRSP_NAME[9] = {0x53,0x65,0x6E,0x73,0x6F,0x72,0x54,0x61,0x67};

				//Copy Data to new memory locations
				copyArr(tempMsg->pData,advData,15,dataLen + 11,0);

				//Make sure a the CC2540 only add's SensorTags
				if( compareArray(advData,TI_SENSORTAG_SCANRSP_NAME,9) == SUCCESS)
				{
					uint8 *devAddr = (uint8*)osal_mem_alloc(6);
					copyArr(tempMsg->pData,devAddr,5,11,0);

					//Find Or add new BLE device to database
					updateDeviceDB(GAP_DISCOVEREDDEVICE,SUCCESS,devAddr,0,NULL,dataLen,advData,rssi,eventType,addrType,NULL,NULL);



				}

				else
				{
					//Not a SensorTag found
					osal_mem_free(advData);

				}




			}
//-----------------------------------------------TESTCODE----------------------------------------------------
/*			//Test send message
			//Send MSG OPCODE FOR transmission
			cmdPkt_Gen_s *tempMsgx = osal_mem_alloc(sizeof(cmdPkt_Gen_s));
			tempMsgx->opCode[0] = 0xFE;
			tempMsgx->opCode[1] = 0x0B;

			tempMsgx->pData = osal_mem_alloc(1);
			//Peer Address index from Device DB
			tempMsgx->pData[0] = 0x00;

			scheduler_send_Msg(BLE_TASK_ID,GAP_CMD_EVT,(void*)tempMsgx);
			//Set Event
			scheduler_set_Evt(BLE_TASK_ID,GAP_CMD_EVT);
*/
//-----------------------------------------------TESTCODE----------------------------------------------------




		}break;
		case GAP_PassKeyNeeded:
		{
			//Sent after GAP_EstablishLink Command

			//Store connection handle and send in message
			//Send MSG OPCODE FOR transmission
			cmdPkt_Gen_s *tempCmdMsg = osal_mem_alloc(sizeof(cmdPkt_Gen_s));
			tempCmdMsg->pData = osal_mem_alloc(2);

			tempCmdMsg->opCode[0] = 0xFE;
			tempCmdMsg->opCode[1] = GAP_PasskeyUpdate;


			//Connection Handle to send PassKey data
			tempCmdMsg ->pData[0] = tempMsg->pData[9];
			tempCmdMsg->pData[1] =	tempMsg->pData[10];


			scheduler_send_Msg(BLE_TASK_ID,GAP_CMD_EVT,(void*)tempCmdMsg);
			//Set Event
			scheduler_set_Evt(BLE_TASK_ID,GAP_CMD_EVT);

		}break;
		case GAP_BondComplete:
		{




		}break;
		case GAP_AuthenticationComplete:
		{
			//Event with data corresponding to bonding key information

			uint8 *devAddr = (uint8*)osal_mem_alloc(6);
			uint8 *connHandle = (uint8*)osal_mem_alloc(2);
			uint8 *LTK = (uint8*)osal_mem_alloc(16);
			uint8 *LTK_DIV = (uint8*)osal_mem_alloc(2);
			uint8 *LTK_RAND = (uint8*)osal_mem_alloc(8);



			//Copy Data to new memory locations
			copyArr(tempMsg->pData,connHandle,3,5,0);
			copyArr(tempMsg->pData,LTK,36,52,0);
			copyArr(tempMsg->pData,LTK_DIV,52,54,0);
			copyArr(tempMsg->pData,LTK_RAND,54,62,0);
			copyArr(tempMsg->pData,devAddr,79,85,0);
			//Update device database with specific bond key information
			updateDeviceDB(GAP_BONDEDDEVICE,SUCCESS,devAddr,connHandle,
					LTK,0,NULL,NULL,NULL,NULL,LTK_DIV,LTK_RAND);




		}break;
		case GAP_CommandStatus:
		{
			errorStatusHdr(status);
			static cmdstatuscnt = 0;
			cmdstatuscnt++;
			if(cmdstatuscnt == 4)
				{};//while(1);
		}break;


		};


		 //Deallocate memory for received message
		 osal_mem_free((void*)tempMsg->pData);
		 osal_mem_free((void*)tempMsg);

		 //Verify another event was not triggered
		 if(getQueueLength(taskId,events) == 0)
			 events &= ~GAP_EVT_EVT;	//Mask out completed events



	}break;
	case GAP_CMD_EVT:{
		//Retrieve Message
		cmdPkt_Gen_s *tempMsg = (cmdPkt_Gen_s *)scheduler_receive_Msg(taskId,GAP_CMD_EVT);
		LinearBuffer_s *cmdData;

		uint8 opCode[2];	//LSB First*
		opCode[0] = tempMsg->opCode[0];
		opCode[1] = tempMsg->opCode[1];

		const uint8 type = 0x01;//Command
		//Variables for formatting UART TX packet
		uint8 dst_start = 0;
		uint8 src_end = 0;
		uint8 prevLen = 0;


		//Build commands and set UART TX Event
		switch(opCode[1])
		{

		case GAP_DeviceInit:{

		//Return Value - CMD Status EVT

			uint8 dataLen = sizeof(GAP_DeviceInitCMD_s);
			uint8 totalPktLen = dataLen + CMDHDRLEN;

			uint8 *pCmd = osal_mem_alloc(totalPktLen);	//Allocate memory for actual command data
			cmdData = osal_mem_alloc(sizeof(LinearBuffer_s));	//Allocate memory for message to next task

			cmdData->dataEnd = totalPktLen;
			cmdData->isInUse = INUSE;
			cmdData->linBuffer = pCmd;


			//Type(1 byte), opcode(2 bytes), data length(1 Byte) , Actual Data *
			uint8 profileRole = 0x08; //Central role
			uint8 maxScanResponse =  0x05;	//Take info from at most 5 devices
			uint8 IRK[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
			uint8 CSRK[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
			uint8 signCounter[4] = {1,0,0,0};



			src_end = prevLen = sizeof(type);
			pCmd[dst_start] = type;


			dst_start += prevLen;
			src_end = prevLen = sizeof(opCode);
			pCmd[dst_start] = opCode[1]; 	//Swap bytes
			pCmd[dst_start + 1] = opCode[0];

			dst_start += prevLen;
			src_end = prevLen = sizeof(dataLen);
			pCmd[dst_start] = dataLen;

			dst_start += prevLen;
			src_end = prevLen = sizeof(profileRole);
			pCmd[dst_start] = profileRole;

			dst_start += prevLen;
			src_end = prevLen = sizeof(maxScanResponse);
			pCmd[dst_start] = maxScanResponse;




			dst_start += prevLen;
			src_end = prevLen = sizeof(IRK);
			copyArr(IRK,pCmd,0,src_end,dst_start);

			dst_start += prevLen;
			src_end = sizeof(CSRK);
			copyArr(CSRK,pCmd,0,src_end,dst_start);

			dst_start += prevLen;
			src_end = sizeof(signCounter);
			copyArr(signCounter,pCmd,0,src_end,dst_start);



		}break;

		case GAP_DeviceDiscoveryRequest:{
		//OpCode - 0xFE04
		//Return Value - CMD Status
		//GAP Device Information Events
		//GAP Device Discovery Event

			uint8 dataLen = sizeof(GAP_DeviceDiscoveryRequestCMD_s);
			uint8 totalPktLen = dataLen + CMDHDRLEN;

			uint8 *pCmd = osal_mem_alloc(totalPktLen);	//Allocate memory for actual command data
			cmdData = osal_mem_alloc(sizeof(LinearBuffer_s));	//Allocate memory for message to next task

			cmdData->dataEnd = totalPktLen;
			cmdData->isInUse = INUSE;
			cmdData->linBuffer = pCmd;

			uint8 mode = 0x03;
			uint8 activeScan = 0x01;
			uint8 whiteList = 0x00;


			src_end = prevLen = sizeof(type);
			pCmd[dst_start] = type;


			dst_start += prevLen;
			src_end = prevLen = sizeof(opCode);
			pCmd[dst_start] = opCode[1]; 	//Swap bytes
			pCmd[dst_start + 1] = opCode[0];

			dst_start += prevLen;
			src_end = prevLen = sizeof(dataLen);
			pCmd[dst_start] = dataLen;

			dst_start += prevLen;
			src_end = prevLen = sizeof(mode);
			pCmd[dst_start] = mode;

			dst_start += prevLen;
			src_end = prevLen = sizeof(activeScan);
			pCmd[dst_start] = activeScan;

			dst_start += prevLen;
			src_end = prevLen = sizeof(whiteList);
			pCmd[dst_start] = whiteList;



		}break;
		case GAP_DeviceDiscoveryCancel:break;
		case GAP_EstablishLinkRequest:{
		//MSG Data Contains Peer Address Index in Device DB pData[0]
			//OpCode - 0xFE09
			//Return Value - CMD Status
			//GAP Link Established Event

			uint8 dataLen = sizeof(GAP_EstablishLinkRequestCMD_s);
			uint8 totalPktLen = dataLen + CMDHDRLEN;

			uint8 *pCmd = osal_mem_alloc(totalPktLen);	//Allocate memory for actual command data
			cmdData = osal_mem_alloc(sizeof(LinearBuffer_s));	//Allocate memory for message to next task

			cmdData->dataEnd = totalPktLen;
			cmdData->isInUse = INUSE;
			cmdData->linBuffer = pCmd;

			uint8 highDutyCycle = 0x00;
			uint8 whiteList = 0x00;
			uint8 addrTypePeer = 0x00;
			uint8 peerAddrIndex = tempMsg->pData[0];
			uint8 *peerAddr = bleDeviceDB[peerAddrIndex]->devAddr;	//LSB FIRST


			//Append data to UART TX Packet Array
			src_end = prevLen = sizeof(type);
			pCmd[dst_start] = type;


			dst_start += prevLen;
			src_end = prevLen = sizeof(opCode);
			pCmd[dst_start] = opCode[1]; 	//Swap bytes
			pCmd[dst_start + 1] = opCode[0];

			dst_start += prevLen;
			src_end = prevLen = sizeof(dataLen);
			pCmd[dst_start] = dataLen;

			//CMD Specific Data Start
			dst_start += prevLen;
			src_end = prevLen = sizeof(highDutyCycle);
			pCmd[dst_start] = highDutyCycle;

			dst_start += prevLen;
			src_end = prevLen = sizeof(whiteList);
			pCmd[dst_start] = whiteList;

			dst_start += prevLen;
			src_end = prevLen = sizeof(addrTypePeer);
			pCmd[dst_start] = addrTypePeer;


			dst_start += prevLen;
			src_end = prevLen = 0x06;
			copyArr(peerAddr,pCmd,0,src_end,dst_start);



		}break;
		case GAP_TerminateLinkRequest:break;
		case GAP_Authenticate:{
	//MSG Data Contains Peer Address Index in Device DB pData[0]
			//OpCode - 0xFE0B
			//Return Value - CMD Status
			//GAP Authentication Complete Event
			//*GAP PasskeyNeeded Event

			const uint8 dataLen = sizeof(GAP_AuthenticateCMD_s);
			uint8 totalPktLen = dataLen + CMDHDRLEN;
			uint8 peerAddrIndex = tempMsg->pData[0];

			uint8 *pCmd = osal_mem_alloc(totalPktLen);	//Allocate memory for actual command data
			cmdData = osal_mem_alloc(sizeof(LinearBuffer_s));	//Allocate memory for message to next task

			cmdData->dataEnd = totalPktLen;
			cmdData->isInUse = INUSE;
			cmdData->linBuffer = pCmd;

			uint8 connHandle[2];

			connHandle[0] = bleDeviceDB[peerAddrIndex]->connHandle[0]; // LSB First
			connHandle[1] = bleDeviceDB[peerAddrIndex]->connHandle[1];

			const uint8 secReq_ioCaps = 0x03;	//0x03 = No I/O or Passcode needed , 0x04 Passcode required
			const uint8 secReq_oobAvailable = 0x00;
				  uint8 secReq_oob[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
			const uint8 secReq_authReq = 0x01; //Bonding exchange and save key information
			const uint8 secReq_maxEntKeySize = 0x10;
			const uint8 secReq_keyDist = 0x3F;

			const uint8 pariReq_Enable = 0x00;
			const uint8 pariReq_ioCaps = 0x03;
			const uint8 pariReq_oobDataFlag = 0x00;
			const uint8 pariReq_authReq = 0x01;
			const uint8 pariReq_maxEncKeySize = 0x10;
			const uint8 pariReq_keyDist = 0x3F;


			//Append data to UART TX Packet Array
			src_end = prevLen = sizeof(type);
			pCmd[dst_start] = type;


			dst_start += prevLen;
			src_end = prevLen = sizeof(opCode);
			pCmd[dst_start] = opCode[1]; 	//Swap bytes
			pCmd[dst_start + 1] = opCode[0];

			dst_start += prevLen;
			src_end = prevLen = sizeof(dataLen);
			pCmd[dst_start] = dataLen;

			//CMD Specific Data Start
			dst_start += prevLen;
			src_end = prevLen = sizeof(connHandle);
			pCmd[dst_start] = connHandle[1]; 	//Swap bytes
			pCmd[dst_start + 1] = connHandle[0];


			dst_start += prevLen;
			src_end = prevLen = sizeof(secReq_ioCaps);
			pCmd[dst_start] = secReq_ioCaps;

			dst_start += prevLen;
			src_end = prevLen = sizeof(secReq_oobAvailable);
			pCmd[dst_start] = secReq_oobAvailable;


			dst_start += prevLen;
			src_end = prevLen = 16;
			copyArr(secReq_oob,pCmd,0,src_end,dst_start);



			dst_start += prevLen;
			src_end = prevLen = sizeof(secReq_authReq);
			pCmd[dst_start] = secReq_authReq;

			dst_start += prevLen;
			src_end = prevLen = sizeof(secReq_maxEntKeySize);
			pCmd[dst_start] = secReq_maxEntKeySize;

			dst_start += prevLen;
			src_end = prevLen = sizeof(secReq_keyDist);
			pCmd[dst_start] = secReq_keyDist;

			dst_start += prevLen;
			src_end = prevLen = sizeof(pariReq_Enable);
			pCmd[dst_start] = pariReq_Enable;

			dst_start += prevLen;
			src_end = prevLen = sizeof(pariReq_ioCaps);
			pCmd[dst_start] = pariReq_ioCaps;

			dst_start += prevLen;
			src_end = prevLen = sizeof(pariReq_oobDataFlag);
			pCmd[dst_start] = pariReq_oobDataFlag;

			dst_start += prevLen;
			src_end = prevLen = sizeof(pariReq_authReq);
			pCmd[dst_start] = pariReq_authReq;

			dst_start += prevLen;
			src_end = prevLen = sizeof(pariReq_maxEncKeySize);
			pCmd[dst_start] = pariReq_maxEncKeySize;

			dst_start += prevLen;
			src_end = prevLen = sizeof(pariReq_keyDist);
			pCmd[dst_start] = pariReq_keyDist;





		}break;
		case GAP_PasskeyUpdate:{
			//OpCode - 0xFE0C
			//Return Value - CMD Status
			uint8 dataLen = sizeof(GAP_PasskeyUpdateCMD_s);
			uint8 totalPktLen = dataLen + CMDHDRLEN;

			uint8 *pCmd = osal_mem_alloc(totalPktLen);	//Allocate memory for actual command data
			cmdData = osal_mem_alloc(sizeof(LinearBuffer_s));	//Allocate memory for message to next task

			cmdData->dataEnd = totalPktLen;
			cmdData->isInUse = INUSE;
			cmdData->linBuffer = pCmd;


			uint8 connHandle[2];
			connHandle[0] = tempMsg->pData[0];
			connHandle[1] = tempMsg->pData[1];
			uint8 passkey[6] = {0x30,0x30,0x30,0x30,0x30,0x30};	//0x30 = 0 	ASCII

			//Append data to UART TX Packet Array
			src_end = prevLen = sizeof(type);
			pCmd[dst_start] = type;


			dst_start += prevLen;
			src_end = prevLen = sizeof(opCode);
			pCmd[dst_start] = opCode[1]; 	//Swap bytes
			pCmd[dst_start + 1] = opCode[0];

			dst_start += prevLen;
			src_end = prevLen = sizeof(dataLen);
			pCmd[dst_start] = dataLen;

			//CMD Specific Data Start
			dst_start += prevLen;
			src_end = prevLen = sizeof(connHandle);
			pCmd[dst_start] = connHandle[1]; 	//Swap bytes
			pCmd[dst_start + 1] = connHandle[0];

			dst_start += prevLen;
			src_end = prevLen = 6;
			copyArr(passkey,pCmd,0,src_end,dst_start);

		}break;
		case GAP_Bond:{
			//OpCode - 0xFE0F
			//Return Value - CMD Status
			//GAP Bond complete Event

			//Acquire specific sensorTag from device database
			PBLEDevice_s *tmpBLEDevice = findDevice(tempMsg->pData,NULL);
			if(tmpBLEDevice == NULL)
			{
				//Set Error state as a bond cannot occur
				ERRORFLAG = BLE_CMD_FAILURE;
				break;
			}


	//MUST SEND IN Device ADDRESS in packet Data
			uint8 dataLen = sizeof(GAP_BondCMD_s);
			uint8 totalPktLen = dataLen + CMDHDRLEN;

			uint8 *pCmd = osal_mem_alloc(totalPktLen);	//Allocate memory for actual command data
			cmdData = osal_mem_alloc(sizeof(LinearBuffer_s));	//Allocate memory for message to next task

			cmdData->dataEnd = totalPktLen;
			cmdData->isInUse = INUSE;
			cmdData->linBuffer = pCmd;



			//Append data to UART TX Packet Array
			src_end = prevLen = sizeof(type);
			pCmd[dst_start] = type;


			dst_start += prevLen;
			src_end = prevLen = sizeof(opCode);
			pCmd[dst_start] = opCode[1]; 	//Swap bytes
			pCmd[dst_start + 1] = opCode[0];

			dst_start += prevLen;
			src_end = prevLen = sizeof(dataLen);
			pCmd[dst_start] = dataLen;


			//CMD Specific Data Start
			dst_start += prevLen;
			src_end = prevLen = 1; //2 Bytes for connection Handle size
			pCmd[dst_start] = tmpBLEDevice->connHandle[1] ;
			pCmd[dst_start + 1] = tmpBLEDevice->connHandle[0];


			dst_start += prevLen;
			src_end = prevLen = 1;
			pCmd[dst_start] = 0x01;	//Authenticated

			dst_start += prevLen;
			src_end = prevLen = 16;
			copyArr(tmpBLEDevice->LTK,pCmd,0,src_end,dst_start);	//16 Byte LTK

			dst_start += prevLen;
			src_end = prevLen = 2;
			copyArr(tmpBLEDevice->LTK_DIV,pCmd,0,src_end,dst_start);	//2 Byte LTK Div

			dst_start += prevLen;
			src_end = prevLen = 8;
			copyArr(tmpBLEDevice->LTK_RAND,pCmd,0,src_end,dst_start);	//8 Byte LTK Rand

			dst_start += prevLen;
			src_end = prevLen = 1;
			pCmd[dst_start] = 0xA;	//LTK Size , 16 Bytes






		}break;
		case GAP_GetParam:break;



		}

		//Deallocate memory from calling function/event, Not all functions send data
		if(tempMsg->pData != NULL)
			osal_mem_free((void*)tempMsg->pData);

		osal_mem_free((void*)tempMsg);

		if(ERRORFLAG == SUCCESS)
		{
			scheduler_send_Msg(UART_TASK_ID,UART_A_TX_EVT,(void*)cmdData);	//Set UART TX Event
			scheduler_set_Evt(UART_TASK_ID,UART_A_TX_EVT);
		}

		//Mask out completed events
		 if(getQueueLength(taskId,events) == 0)
			events &= ~GAP_CMD_EVT;
	}break;
	case GATT_EVT_EVT:{
		//Retrieve Message from Global Queue
			evtPkt_Gen_s  *tempMsg = (evtPkt_Gen_s *)scheduler_receive_Msg(taskId,GATT_EVT_EVT);

			//Decode GAP Event
			switch(tempMsg->eventCode)
			{



			};

			//Mask out completed events
		 if(getQueueLength(taskId,events) == 0)
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
		bleDeviceDB[i]->connHandle[0] = 0xFF;
		bleDeviceDB[i]->connHandle[1] = 0xFF;
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
 * 		 Finds the device by address OR connection handle.
 *
 * @params 	uint8 *devAddr - Device Address
 * 			uint8 *connHandle - Connection Handle for specific device
 *
 * @return PBLEDevice_s * - if Device found, NULL - otherwise
 *
 *  */
PBLEDevice_s *findDevice(uint8 * devAddr, uint8 *connHandle)
{
	uint8 i;

	for(i = 0; i < NUMOFDEVICES;i++)
	{
		if(devAddr != NULL)	//Search by Address
		{
			if(compareArray(bleDeviceDB[i]->devAddr,devAddr,6) == SUCCESS)
				return bleDeviceDB[i];
		}
		else //Search by Handle
		{
			//See if there is a corresponding connection handle in the device database
			 if(compareArray(bleDeviceDB[i]->connHandle,connHandle,2) == SUCCESS)
				return bleDeviceDB[i];
		}
	}
		return NULL;	//Device not found
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
 * 			uint8 GAPState - Connection state for a specific device
 * 			uint8 CMDStatus - Current Command for the specific BLE Device status
 * 			uint8 *devAddr - device Address
 * 			uint8 *connHandle - Specific 2 Byte handle for a specific connection
 * 			uint8 *LTK	- 16 byte security key
 *			uint8 *LTK_DIV
 *			uint8 *LTK_RAND - Random Number for security key
 *
 *
 * @return VOID
 *
 *  */
void updateDeviceDB(uint8 GAPState,uint8 CMDStatus,uint8 *devAddr,uint8 *connHandle,uint8 *LTK,
					uint8 dataLen, uint8 *data,uint8 rssi,uint8 advEvtType,uint8 addrType,uint8 *LTK_DIV, uint8 *LTK_RAND)
{
	PBLEDevice_s *currDevice = NULL;

	//Search for Device
	currDevice = findDevice(devAddr,connHandle);

	//Device not found, add new device
	if(currDevice == NULL)
	{
		currDevice = addNewDevice(devAddr);
	}
	else if(devAddr != NULL)
	{
		//Deallocate address sent in(Already in DB)
		osal_mem_free(devAddr);
	}


	if(GAPState != NULL)
		currDevice->GAPState = GAPState;
	if(CMDStatus != NULL)
		currDevice->CMDStatus = CMDStatus;
	if(connHandle != NULL)
	{
		if(currDevice->connHandle != NULL)
			osal_mem_free((void*)currDevice->connHandle);

		currDevice->connHandle = connHandle;

	}
	if(LTK != NULL)
	{
		//Replace new key and free memory
		if(currDevice->LTK != NULL)
			osal_mem_free((void*)currDevice->LTK);

		currDevice->LTK = LTK;
	}

	if(LTK_DIV != NULL)
	{
		//Replace new key and free memory
		if(currDevice->LTK_DIV != NULL)
			osal_mem_free((void*)currDevice->LTK_DIV);

		currDevice->LTK_DIV = LTK_DIV;
	}
	if(LTK_RAND != NULL)
	{
		//Replace new key and free memory
		if(currDevice->LTK_RAND != NULL)
			osal_mem_free((void*)currDevice->LTK_RAND);

		currDevice->LTK_RAND = LTK_RAND;
	}

	if((dataLen != 0) && (data != NULL))
	{
		//Free memory
		if(currDevice->dataField != NULL)
			osal_mem_free((void*)currDevice->dataField);

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
