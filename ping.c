#include "msp430.h"

#define     TXD                   BIT2                      // TXD on P1.2
#define     RXD                   BIT1                      // RXD on P1.1

// wait for time t
void wait(unsigned int t)
{
  volatile int count = t;
  while (count)
    count--;
}

void main(void)
{
  
  WDTCTL = WDTPW + WDTHOLD; 		// Stop WDT
  
  /* next three lines to use internal calibrated 1MHz clock: */
  BCSCTL1 = CALBC1_1MHZ;                    // Set range
  DCOCTL = CALDCO_1MHZ;
  BCSCTL2 &= ~(DIVS_3);                     // SMCLK = DCO = 1MHz

  // initialize P1 direction
  P1DIR = 0x00;                 // clear P1 direction
  P1DIR |= 0x10; 				// P1.4 to output
  P1DIR |= TXD;
  
  // initially set TXD to high for some reason
  P1OUT |= TXD;
  
  /* Configure hardware UART */
  P1SEL = BIT1 + BIT2 ; // P1.1 = RXD, P1.2=TXD
  P1SEL2 = BIT1 + BIT2 ; // P1.1 = RXD, P1.2=TXD
  UCA0CTL1 |= UCSSEL_2; // Use SMCLK
  UCA0BR0 = 104; // Set baud rate to 9600 with 1MHz clock (Data Sheet 15.3.13)
  UCA0BR1 = 0; // Set baud rate to 9600 with 1MHz clock
  UCA0MCTL = UCBRS0; // Modulation UCBRSx = 1
  UCA0CTL1 &= ~UCSWRST; // Initialize USCI state machine
  /* if we were going to receive, we would also:
     IE2 |= UCA0RXIE; // Enable USCI_A0 RX interrupt
  */
  
  unsigned int TXByte = 0;  // we store measurements here before broadcasting them
  unsigned int count;
  /* Main Application Loop */
  while(1) {
    /* read the current distance: */
    count = 0;
    P1OUT |= 0x10;  // start the ping
    wait(50);
    P1OUT &= ~0x10; // stop the ping
    wait(100);
    // wait till echo is low
    while ((P1IN & 0x20) != 0)
        count ++;
    
    
    TXByte = (unsigned char)(count);
    while (! (IFG2 & UCA0TXIFG)); // wait for TX buffer to be ready for new data
    UCA0TXBUF = TXByte;
    
    wait(60000);
  }

}
