#include "msp430.h"


// constant parameters:
#define     TXD                   BIT2                      // TXD on P1.2
#define     RXD                   BIT1                      // RXD on P1.1

#define     ADC_PIN_R             BIT3                      // ADC r      on P1.3
#define     ADC_PIN_T             BIT4                      // ADC theta  on P1.4
#define     ADC_PIN_P             BIT5                      // ADC phi    on P1.5

#define     ADC_R                 INCH_3
#define     ADC_T                 INCH_4
#define     ADC_P                 INCH_5


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


// get the current reading from the ADC for r
int get_r() {
  ADC10CTL1 = ADC_R;
  wait(100);
  ADC10CTL0 |= ENC + ADC10SC;           // Sampling and conversion start
  while (ADC10CTL1 &ADC10BUSY);         // wait until not ADC10BUSY?
  ADC10CTL0 &= ~ENC;
  return ADC10MEM;
}
// get the current reading from the ADC for r
int get_theta() {
  ADC10CTL1 = ADC_T;
  wait(100);
  ADC10CTL0 |= ENC + ADC10SC;           // Sampling and conversion start
  while (ADC10CTL1 &ADC10BUSY);         // wait until not ADC10BUSY?
  ADC10CTL0 &= ~ENC;
  return ADC10MEM;
}
// get the current reading from the ADC for r
int get_phi() {
  ADC10CTL1 = ADC_P;
  wait(100);
  ADC10CTL0 |= ENC + ADC10SC;           // Sampling and conversion start
  while (ADC10CTL1 &ADC10BUSY);         // wait until not ADC10BUSY?
  ADC10CTL0 &= ~ENC;
  return ADC10MEM;
}
// initialization function:
void init_adc() {
  ADC10CTL0 = ADC10SHT_2 | ADC10ON;             // ADC10ON
                                                // V+ = Vcc, V- = Vss (gnd)
  ADC10CTL1 = ADC_R;                            // input A*
  ADC10AE0 |= ADC_PIN_R | ADC_PIN_T | ADC_PIN_P;// PA.* ADC option select
  ADC10CTL0 &= ~ENC;
}


void main() {
  // initialization:
  WDTCTL = WDTPW + WDTHOLD; 		// Stop WDT
  
  P1DIR = 0x00;                 // clear P1 direction
  
  init_output();
  init_adc();
  
  int i;
  for (i = 0; i < 10000; i++) {
    output(get_r());
    wait(10000);
    output(get_theta());
    wait(10000);
  }
}

