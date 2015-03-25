/**
  Headerfile:       GAP_HCI.h


  Revised:        $Date: 2015-3-21   (Sun, 1 Mar 2015) $
  Revision:       $Revision: 1 $
  Author: Alexander D'Abreu
  Description:    GAP and GATT BLE commands and event processing functions.

**************************************************************************************************/
#ifndef BLE_HCI_H_
#define BLE_HCI_H_

/*********************************************************************
 * INCLUDES
 */
#include "Scheduler.h"


/*********************************************************************
 * CONSTANTS
 */

//GAP OPCodes(Low Byte) - 0x06XX
#define GAP_DeviceInitDone 0x00
#define GAP_DeviceDiscovery 0x01
#define GAP_LinkEstablished 0x05
#define GAP_LinkTerminated 0x06
#define GAP_SignatureUpdated 0x09
#define GAP_DeviceInformation 0x0D
#define	GAP_PassKeyNeeded 0x0B
#define	GAP_BondComplete 0x0E
#define GAP_AuthenticationComplete 0x0A
#define GAP_CommandStatus 0x7F

//GATT OpCodes(LowByte) - 0x





//Event Flags
#define GAP_EVT_EVT BIT0
#define GAP_CMD_EVT BIT1
#define GATT_EVT_EVT BIT2
#define GATT_CMD_EVT BIT3


#define NUMOFDEVICES 3




//GAP Connection States
#define GAP_FREEDEVICE 100
#define GAP_CONNECTEDDEVICE 101
#define GAP_DISCONNECTEDDEVICE	102
#define GAP_PAIREDDEVICE	103
#define GAP_BONDEDDEVICE	104
#define GAP_DISCOVEREDDEVICE	105


//Command Error Status Codes
#define INVALIDPARAMETER 0x02
#define INVALID_TASK 0x03
#define MSG_BUFFER_NOT_AVAIL 0x04
#define INVALID_MSG_POINTER 0x05
#define INVALID_EVENT_ID 0x06
#define INVALID_INTERRUPT_ID 0x07
#define NO_TIMER_AVAIL 0x08
#define NV_ITEM_UNINIT 0x09
#define NV_OPER_FAILED 0x0A
#define INVALID_MEM_SIZE 0x0B
#define NV_BAD_ITEM_LEN 0x0C
#define bleNotReady 0x10
#define bleAlreadyInRequestedMode 0x11
#define bleIncorrectMode 0x12
#define bleMemAllocError 0x13
#define bleNotConnected 0x14
#define bleNoResources 0x15
#define blePending 0x16
#define bleTimeout 0x17
#define bleInvalidRange 0x18
#define bleLinkEncrypted 0x19
#define bleProcedureComplete 0x1A
#define bleGAPUserCanceled 0x30
#define bleGAPConnNotAcceptable 0x31
#define bleGAPBondRejected 0x32
#define bleInvalidPDU 0x40
#define bleInsufficientAuthen 0x41
#define bleInsufficientEncrypt 0x42
#define bleInsufficientKeySize 0x43
#define INVALID_TASK_ID 0xFF




//GAP Event Types
#define ConnectableUndirectedAdv 0
#define ConnectableDirectedAdv 1
#define DiscoverableUndirectedAdv 2
#define NONConnectableUndirectedAdc 3
#define ScanResponse 4

//GAP Address Types
#define ADDRTYPE_PUBLIC 0
#define ADDRTYPE_STATIC 1
#define ADDRTYPE_PRIVATE_NONRESOLVE 2
#define ADDRTYPE_PRIVATE_RESOLVE 3


/*********************************************************************
 * TYPEDEFS
 */

//Master BLE device data
typedef struct{
	uint8 numOfConnectedDevices;
	uint8 *devAddr;
	uint8 *connHandle;
    uint8 *IRK;
    uint8 *CSRK;
}MBLEDevice_s;

//Peripheral BLE Device data
typedef struct{
	uint8 GAPState;
	uint8 CMDStatus;
	uint8 *devAddr;
	uint8 *connHandle;

	uint8 addrType;
	uint8 advEvtType;
	uint8 rssi;
	uint8 dataLen;
	uint8 *dataField;	//From Scan response

	uint8 *LTK;
}PBLEDevice_s;



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

extern uint8 BLE_ProcessEvent(uint8 taskId,uint8 events);

extern void updateDeviceDB(uint8 GAPState,uint8 CMDStatus,uint8 *devAddr,uint8 *connHandle,uint8 *LTK,uint8 dataLen, uint8 *data,uint8 rssi,uint8 advEvtType,uint8 addrType);
extern uint8 compareArray(uint8 *arr1,uint8 *arr2,uint8 length);
extern PBLEDevice_s *findDevice(uint8 * devAddr);
extern PBLEDevice_s *addNewDevice(uint8 * devAddr);
extern void BLE_Init(void);

/*********************************************************************
*********************************************************************/



#endif /* BLE_HCI_H_ */


