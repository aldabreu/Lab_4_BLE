#include <msp430.h>
#include "Scheduler.h"

#define UART_TASK_ID 0

//Static Global Definitions for Main ISR's
   //--------------------------------------------------------------
	static RingBuffer_s *UART_A_RXCircBuf;
	static RingBuffer_s *UART_A_TXCircBuf;
	static RingBuffer_s *UART_B_RXCircBuf;
	static RingBuffer_s *UART_B_TXCircBuf;

	static uint8 ERRORFLAG = 0;
//---------------------------------------------------------------

	void SetVcoreUp (unsigned int level)
	{
	  // Open PMM registers for write
	  PMMCTL0_H = PMMPW_H;
	  // Set SVS/SVM high side new level
	  SVSMHCTL = SVSHE + SVSHRVL0 * level + SVMHE + SVSMHRRL0 * level;
	  // Set SVM low side to new level
	  SVSMLCTL = SVSLE + SVMLE + SVSMLRRL0 * level;
	  // Wait till SVM is settled
	  while ((PMMIFG & SVSMLDLYIFG) == 0);
	  // Clear already set flags
	  PMMIFG &= ~(SVMLVLRIFG + SVMLIFG);
	  // Set VCore to new level
	  PMMCTL0_L = PMMCOREV0 * level;
	  // Wait till new level reached
	  if ((PMMIFG & SVMLIFG))
	    while ((PMMIFG & SVMLVLRIFG) == 0);
	  // Set SVS/SVM low side to new level
	  SVSMLCTL = SVSLE + SVSLRVL0 * level + SVMLE + SVSMLRRL0 * level;
	  // Lock PMM registers for write access
	  PMMCTL0_H = 0x00;
	}
//-------------------------------------------------------------------------




void main(void) {
	WDTCTL = WDTPW | WDTHOLD;	// Stop Watchdog timer

	// Initialize the Memory Allocation System
	osal_mem_init();

	//Initialize UART RX TX Buffers
	UART_A_RXCircBuf = initializeBuffer(RXBUFFER_INIT);
	//UART_A_TXCircBuf = initializeBuffer(TXBUFFER_INIT);
	//UART_B_RXCircBuf = initializeBuffer(RXBUFFER_INIT);
	//UART_B_TXCircBuf = initializeBuffer(TXBUFFER_INIT);

	//Start the HCI system
	//scheduler_start_system();
	/*---------------------------------------------------------------------
	Test Code Block
	*/

	  volatile unsigned int i;

	  WDTCTL = WDTPW+WDTHOLD;                   // Stop WDT


	  P2DIR |= BIT2;                            // SMCLK set out to pins
	  P2SEL |= BIT2;
	  P7DIR |= BIT7;                            // MCLK set out to pins
	  P7SEL |= BIT7;

	  // Increase Vcore setting to level3 to support fsystem=25MHz
	  // NOTE: Change core voltage one level at a time..
	  SetVcoreUp (0x01);
	  SetVcoreUp (0x02);
	  SetVcoreUp (0x03);

	  UCSCTL3 = SELREF_2;                       // Set DCO FLL reference = REFO
	  UCSCTL4 |= SELA_2;                        // Set ACLK = REFO

	  __bis_SR_register(SCG0);                  // Disable the FLL control loop
	  UCSCTL0 = 0x0000;                         // Set lowest possible DCOx, MODx
	  UCSCTL1 = DCORSEL_7;                      // Select DCO range 50MHz operation
	  UCSCTL2 = FLLD_0 + 762;                   // Set DCO Multiplier for 25MHz  						SET TO 6 for 20 MHZ
	                                            // (N + 1) * FLLRef = Fdco
	                                            // (762 + 1) * 32768 = 25MHz
	                                            // Set FLL Div = fDCOCLK/2
	  __bic_SR_register(SCG0);                  // Enable the FLL control loop

	  // Worst-case settling time for the DCO when the DCO range bits have been
	  // changed is n x 32 x 32 x f_MCLK / f_FLL_reference. See UCS chapter in 5xx
	  // UG for optimization.
	  // 32 x 32 x 25 MHz / 32,768 Hz ~ 780k MCLK cycles for DCO to settle
	  __delay_cycles(782000);

	  // Loop until XT1,XT2 & DCO stabilizes - In this case only DCO has to stabilize
	  do
	  {
	    UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + DCOFFG);
	                                            // Clear XT2,XT1,DCO fault flags
	    SFRIFG1 &= ~OFIFG;                      // Clear fault flags
	  }while (SFRIFG1&OFIFG);                   // Test oscillator fault flag


	//REMOVEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE

	P4SEL |= BIT4+BIT5;                       // P3.3,4 = USCI_A0 TXD/RXD
	UCA1CTL1 |= UCSWRST;                      // **Put state machine in reset**
	UCA1CTL1 |= UCSSEL_2;                     // SMCLK
	UCA1BR0 = 0xD9;                              // 1MHz 115200 (see User's Guide)
	UCA1BR1 = 0;                              // 1MHz 115200
	UCA1MCTL = 0;           // Modulation UCBRSx=1, UCBRFx=0
	UCA1CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**



	UCA1IE |= UCRXIE;                         // Enable USCI_A1 RX interrupt


	_bis_SR_register(GIE);







		while(1);
/*
-----------------------------------------------------------------------
*/
}









// Vector 0 - no interrupt
// Vector 2 - RXIFG
// Vector 4 - TXIFG

#pragma vector=USCI_A1_VECTOR
__interrupt void USCI_A1_ISR(void)
{
	//Flag to skip current RX Packet if all buffers are full
	static uint8 SkipPacket = FALSE;
	static uint8 pktType = 0;


	//Number of bytes either received or transmitted
		 static uint8 TxByteCtr = 0;
		 static uint8 RxByteCtr = 0;

	//Index of Buffer currently being used
		 static uint8 currTxBuf = 0;
		 static uint8 currRxBuf = 0;
	//Index of end of data
		// static uint8 RxPktEnd = BUFFERSIZE;
		 static uint8 RxPktEnd = 44;
		 static uint8 TxPktEnd = 0;

	//Status of adding to a buffer
		 uint8 addStatus = SUCCESS;
		 volatile uint8 rxByte = 0;









	switch(__even_in_range(UCA1IV,4))
	{
	case UARTTXINT:
			{
			 //UCA1TXBUF = 'a';

			}break;
	case UARTRXINT:
	{




//----------------------------------------------
		 static count = 0;
		count++;
		__delay_cycles(8900);

		 if(UCA1STAT & UCOE)
			 while(1);
//----------------------------------------------


		 //Read Received Byte into Circular RX Buffer
		 rxByte = UCA1RXBUF;
		 RxByteCtr++;
		 /*Buffer writing procedure
		  * 0)Update NumOfBufInUse
			1)Test to see if the overall Circular Buffer is fully in use - STOP ERROR
			2)Find open Buffer,Set in use flag,increment Total buffers in use
			3)Write to same open buffer until : Full or End transmission
			4)If Full and Transmission not ended:Send MSG with Full Buffer->DO NOT reset rxByteCtr,read into new buffer for next rxByte->Send MSG & Set Event when Complete

			Cases: 	1) End of incoming data packet
					2)Buffer is full BUT Data not complete
					3)Buffer Not full AND Data not complete
						3A)RxByte == 3, parse data packet length
						3B) DO nothing




				*/


		 //Save Packet type
		 if(RxByteCtr == 1)
			 pktType = rxByte;

		 //Buffer's not full and NOT skipping a previous Packet
		 if((!isCircBufFull(UART_A_RXCircBuf)) && (!SkipPacket))
		 {
			 //Find a new Buffer if the one is currently being used
			 if(UART_A_RXCircBuf->circBuffer[currRxBuf]->isInUse)
			 {
				 currRxBuf = findOpenBuffer(UART_A_RXCircBuf,currRxBuf);
			 }

			 //Add Data to circular Buffer
			 addStatus =  addToBuffer(UART_A_RXCircBuf,currRxBuf,rxByte);

			  if(addStatus == SUCCESS)
			  {
				  //End of data Reached
				  if(RxByteCtr == RxPktEnd)
				  {
					  //Reset Byte Counter
					  RxByteCtr = 0;
					  RxPktEnd = BUFFERSIZE;

					  //Set in use flag so it doesen't get overwritten
					  UART_A_RXCircBuf->circBuffer[currRxBuf]->isInUse = 1;
					  UART_A_RXCircBuf->numOfBufInUse++;

					  //Send MSG
					  scheduler_send_Msg(UART_TASK_ID,UART_A_RX_EVT,(void*)UART_A_RXCircBuf->circBuffer[currRxBuf]);
					  //Set Event
					  //scheduler_set_Evt(UART_TASK_ID,UART_A_RX_EVT);
				  }

				  //Check for full buffer && Not finished with incoming Packet data
				  else if((isBuffFull((*(UART_A_RXCircBuf->circBuffer[currRxBuf])))))
				  {

					  UART_A_RXCircBuf->numOfBufInUse++;

					  //Make sure there's enough space for the rest of the packet
					  if((!isCircBufFull(UART_A_RXCircBuf)))
					  {
						  //Just don't RESET RX Byte Counter
						  UART_A_RXCircBuf->circBuffer[currRxBuf]->isInUse = 1;

						  //SEND SCHED MSG
						  //scheduler_send_Msg(UART_TASK_ID,UART_A_RX_EVT,UART_A_RXCircBuf->circBuffer[currRxBuf]);
					  }
					  else
					  {
						  UART_A_RXCircBuf->numOfBufInUse--;
						  //Not enough room, skip rest of packet and set error Flag
						  SkipPacket  = 1;
						  ERRORFLAG = 1;
					  }
				  }
				  else if(RxByteCtr == 0x04)
				  {


					 //End of data transmission
					 RxPktEnd = UART_A_RXCircBuf->circBuffer[currRxBuf]->linBuffer[EVTDATALENINDEX] + RXOFFSET;
				  }

			  }
			  //ADD STATUS FAILURE
			  else
			  {
				//SET SOME KIND OF FLAG
				  ERRORFLAG = 1;
			  }

		 }
		 else
		 {
			 //Circular Buffer is Full so skip the current packet and try again later
			 SkipPacket = TRUE;
			 //Continue to count bytes BUT just to make sure to start at next valid buffer

			 if((RxByteCtr == 0x03) && (pktType == EVTPKT))
				 RxPktEnd = rxByte + RXOFFSET;
			 else if((RxByteCtr == 0x04) && (pktType == (CMDPKT || DATAPKT)))
				 RxPktEnd = rxByte + RXOFFSET;


			 if(RxByteCtr == RxPktEnd)
			 {
				 //Reset Byte Counter
				 RxByteCtr = 0;
				 RxPktEnd = BUFFERSIZE;
				 //Packet Skip Completed
				 SkipPacket = FALSE;
				 pktType = 0;
			 }
		 }

	}break;


	default: break;
	}

}











/*

#pragma vector=TIMER0_A0_VECTOR
__interrupt void TIMER0_A0_ISR(void)
{
  P1OUT ^= 0x01;                            // Toggle P1.0
  volatile static long int timer = 0;
  timer++;


  P1DIR |= 0x01;                            // P1.0 output
  TA0CCTL0 &= ~CCIE;                          // CCR0 interrupt enabled
  TA0CCR0 = 50000;
  TA0CTL = TASSEL_2 + MC_1 + TACLR;         // SMCLK, upmode, clear TAR
}

*/
