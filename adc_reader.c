#include "msp430.h"


// constant parameters:
#define     TXD                   BIT2                      // TXD on P1.2
#define     RXD                   BIT1                      // RXD on P1.1

#define     ADC_PIN               BIT3                      // ADC on P1.3


// wait for time t
// (approximate)
void wait(unsigned long t)
{
  volatile unsigned long count = t;
  while (count)
    count--;
}


// given an int x, output x
void output(int x) {
  unsigned char TXByte;
  TXByte = (unsigned char)((x >> 8) & 0xFF);
  while (! (IFG2 & UCA0TXIFG)); // wait for TX buffer to be ready for new data
  UCA0TXBUF = TXByte;
  TXByte = (unsigned char)(x & 0xFF);
  while (! (IFG2 & UCA0TXIFG)); // wait for TX buffer to be ready for new data
  UCA0TXBUF = TXByte;
}
// initialization function:
void init_output() {
  P1DIR |= TXD;
  P1OUT |= TXD;   // initially set TXD to high for some reason

  /* Configure hardware UART */
  P1SEL  |= RXD | TXD ; // P1.1 = RXD, P1.2=TXD
  P1SEL2 |= RXD | TXD ; // P1.1 = RXD, P1.2=TXD
  UCA0CTL1 |= UCSSEL_2; // Use SMCLK
  UCA0BR0 = 104; // Set baud rate to 9600 with 1MHz clock (Data Sheet 15.3.13)
  UCA0BR1 = 0;   // Set baud rate to 9600 with 1MHz clock
  UCA0MCTL = UCBRS0; // Modulation UCBRSx = 1
  UCA0CTL1 &= ~UCSWRST; // Initialize USCI state machine
}


// get the current reading from the ADC
void get_adc() {
  ADC10CTL0 |= ENC + ADC10SC;           // Sampling and conversion start
  while (ADC10CTL1 &ADC10BUSY);         // wait until not ADC10BUSY?
  return ADC10MEM;
}
// initialization function:
void init_adc() {
  ADC10CTL0 = ADC10SHT_2 + ADC10ON; 	    // ADC10ON
  ADC10CTL1 = INCH_1;                       // input A1
  ADC10AE0 |= ADC_PIN;                      // PA.1 ADC option select
}


void main() {
  // initialization:
  WDTCTL = WDTPW + WDTHOLD; 		// Stop WDT
  
  P1DIR = 0x00;                 // clear P1 direction
  
  init_output();
  
  int i;
  for (i = 0; i < 10000; i++) {
    output(i);
    wait(1000);
  }
}

