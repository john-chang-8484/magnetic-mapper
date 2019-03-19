#include "msp430.h"


// this program is to determine what PWM a given physical servo angle corresponds to
// (saves measuring angle and computing the corresponding PWM)


// constant parameters:
#define MOTOR_PIN           BIT6
#define TXD                 BIT2                      // TXD on P1.2
#define RXD                 BIT1                      // RXD on P1.1

// movement parameters:
#define START               1325
#define STEP                -5
#define MINIM               550
#define MAXIM               2330



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
  P1SEL = BIT1 + BIT2 ; // P1.1 = RXD, P1.2=TXD
  P1SEL2 = BIT1 + BIT2 ; // P1.1 = RXD, P1.2=TXD
  UCA0CTL1 |= UCSSEL_2; // Use SMCLK
  UCA0BR0 = 104; // Set baud rate to 9600 with 1MHz clock (Data Sheet 15.3.13)
  UCA0BR1 = 0;   // Set baud rate to 9600 with 1MHz clock
  UCA0MCTL = UCBRS0; // Modulation UCBRSx = 1
  UCA0CTL1 &= ~UCSWRST; // Initialize USCI state machine
}



// set the servo PWM to a given value
void set_servo(int pwm) {
  CCR1 = pwm;
}

// initialization function for servo stuff
void init_servo() {
  P1DIR |= MOTOR_PIN; 				// set motor pin to output
  P1SEL |= MOTOR_PIN; 				// set motor pin to TA0.1
  
  /* next three lines to use internal calibrated 1MHz clock: */
  BCSCTL1 = CALBC1_1MHZ;                    // Set range
  DCOCTL = CALDCO_1MHZ;
  BCSCTL2 &= ~(DIVS_3);                     // SMCLK = DCO = 1MHz
  
  // set timer period
  CCR0 = 10000-1; 				// PWM Period is 3000
  
  CCTL1 = OUTMOD_7;			// CCR1 reset/set
  CCR1 = 600; 				// CCR1 PWM duty cycle
  
  TACTL = TASSEL_2 + MC_1; 		// SMCLK, up mode
}



// wait for time t
// (approximate)
void wait(unsigned long t)
{
  volatile unsigned long count = t;
  while (count)
    count--;
}




void main(void)
{
  /* initialization: */
  WDTCTL = WDTPW + WDTHOLD; 		// Stop WDT
  // initialize P1 direction
  P1DIR = 0x00;                 // clear P1 direction
  init_output();    // we put this one first because it kills the servo init if we put it after for some reason
  init_servo();
  
  int pulse_width = START;
  
  while (pulse_width < MAXIM && pulse_width > MINIM) {
    pulse_width += STEP;
    wait(10000);
    set_servo(pulse_width);
    output(pulse_width);
  }
}
  
  
  
  
  
