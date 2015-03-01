#include <msp430.h>
#include "Mem_Manager.h"
#include "taskScheduler.h"
/*
 * main.c
 */


/*
Events for Tasks(Order of priority of Tasks):
	1)	UART
			- TransmitA Send Command GAP/GATT
			- ReceiveA Byte Processor(Stick into array and set up global vars. for GAP events)
			- TransmitB GPS Send Command
			- ReceiveB GPS Receive
	2)	BLE: External Events
			-GAPDeviceInitDone
			-GAPDeviceDiscoveryRequests
			-GAPDeviceInformation
			-GAP_LinkEstablish
			-GAP_AuthenticationComplete
	3)	SPI Transceiver Events
			-pSend_SPI_Data


*/






int main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

   // osal_mem_init();
    void * voidarr[2][3];
    int i = 0;
    for(;i < 2;i++)
    	voidarr[i][i] = NULL;
		while(1);

	return 0;
}
