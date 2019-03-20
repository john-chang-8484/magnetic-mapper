#include "msp430.h"


// constant parameters:
#define     TXD                   BIT2                      // TXD on P1.2
#define     RXD                   BIT1                      // RXD on P1.1


/********** START MATH STUFF **********/

// defines the denominator of the fraction (must be square):
#define FRAC 65536
#define FRACRT 256
#define NUM(n) (n * FRAC)

// in hexadecimal, pi = 3.243F6
#define PI 0x3243F

// degree of the taylor series
#define NUM_TERMS 12


// define the "num" datatype
typedef long num;


// returns 0 if math does not work
int check_math() {
  return sizeof(num) == 4;
}

int num2int(num x) {
    return x / FRAC;
}

num int2num(int x) {
    return x * FRAC;
}

num mul(num a, num b) {
    return ((a / FRACRT) * (b / FRACRT));
}

num div(num a, num b) {
    return ((a * FRACRT) / (b / FRACRT));
}

num sine(num x) {
    num terms[NUM_TERMS];
    int i;
    
    // compute terms of exponential series
    terms[0] = NUM(1);
    for (i = 1; i < NUM_TERMS; i++) {
        terms[i] = div(mul(x, terms[i-1]), NUM(i));
    }
    return terms[1] + terms[5] + terms[9] - (terms[3] + terms[7] + terms[11]);
}
/********** END MATH STUFF **********/




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
  P1SEL = BIT1 + BIT2 ; // P1.1 = RXD, P1.2=TXD
  P1SEL2 = BIT1 + BIT2 ; // P1.1 = RXD, P1.2=TXD
  UCA0CTL1 |= UCSSEL_2; // Use SMCLK
  UCA0BR0 = 104; // Set baud rate to 9600 with 1MHz clock (Data Sheet 15.3.13)
  UCA0BR1 = 0;   // Set baud rate to 9600 with 1MHz clock
  UCA0MCTL = UCBRS0; // Modulation UCBRSx = 1
  UCA0CTL1 &= ~UCSWRST; // Initialize USCI state machine
}


void main() {
  // initialization:
  WDTCTL = WDTPW + WDTHOLD; 		// Stop WDT
  
  P1DIR = 0x00;                 // clear P1 direction
  
  init_output();
  
  if (!check_math()) {
    output(-1);
    return;
  }
  
  num i = NUM(0);
  num step = NUM(1) / 100;
  for (i = 0; ; i += step) {
    output(num2int(1000 * sine(i)));
    wait(1000);
  }
}

