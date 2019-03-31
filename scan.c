#include "msp430.h"

// constant parameters:
#define MOTOR_PIN_1         BIT1                      // Motor pin on P2.1 : azm
#define MOTOR_PIN_2         BIT5                      // Motor pin on P2.5 : alt
#define TXD                 BIT2                      // TXD on P1.2
#define RXD                 BIT1                      // RXD on P1.1

// ADC fields:
#define     ADC_PIN_R             BIT3                      // ADC r      on P1.3
#define     ADC_PIN_T             BIT4                      // ADC theta  on P1.4
#define     ADC_PIN_P             BIT5                      // ADC phi    on P1.5

#define     ADC_R                 INCH_3
#define     ADC_T                 INCH_4
#define     ADC_P                 INCH_5

// macro definitions:
// usage: e is an expression
#define azmto(e) old = azm; azm = (e); glide_servo(old, azm, 1);
#define altto(e) old = alt; alt = (e); glide_servo(old, alt, 2);
// do a brief warmup of the servos
#define servo_warmup() azmto(1200); altto(1200); azmto(1000); altto(1000);



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



// set the servo PWM to a given value
void set_servo(int pwm, int n) {
  switch (n) {
    case 1:
      TA1CCR1 = pwm;
      break;
    case 2:
      TA1CCR2 = pwm;
      break;
  }
}

// initialization function for servo stuff
void init_servo() {
  P2DIR |= MOTOR_PIN_1;             // set motor pins to output
  P2SEL |= MOTOR_PIN_1;             // set motor pins to TA output 1
  
  P2DIR |= MOTOR_PIN_2;             // set motor pins to output
  P2SEL |= MOTOR_PIN_2;             // set motor pins to TA output 2
  
  /* next three lines to use internal calibrated 1MHz clock: */
  BCSCTL1 = CALBC1_1MHZ;                    // Set range
  DCOCTL = CALDCO_1MHZ;
  BCSCTL2 &= ~(DIVS_3);                     // SMCLK = DCO = 1MHz
  
  // reset capture bit (so that we are always comparing)
  TA1CCTL1 &= ~CAP;
  TA1CCTL2 &= ~CAP;
  
  // set timer period
  TA1CCR0 = 10000;
  
  TA1CCTL1 |= OUTMOD_7;			// CCR1 reset/set
  TA1CCR1 = 1000; 				  // CCR1 PWM duty cycle
  
  TA1CCTL2 |= OUTMOD_7;     // CCR1 reset/set
  TA1CCR2 = 1000;          // CCR1 PWM duty cycle
  
  TA1CTL = TASSEL_2 + MC_1; 		// SMCLK, up mode
}

// gently glide a servo motor from one value to another:
void glide_servo(int start, int end, int n) {
    int step;
    int i;
    if (start > end)
        step = -1;
    else
        step = 1;
    for (i = start; i != end; i += step) {
        set_servo(i, n);
        wait(100);
    }
    set_servo(end, n);
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
// get the current reading from the ADC for theta
int get_theta() {
  ADC10CTL1 = ADC_T;
  wait(100);
  ADC10CTL0 |= ENC + ADC10SC;           // Sampling and conversion start
  while (ADC10CTL1 &ADC10BUSY);         // wait until not ADC10BUSY?
  ADC10CTL0 &= ~ENC;
  return ADC10MEM;
}
// get the current reading from the ADC for phi
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



void main(void)
{
  /* initialization: */
  WDTCTL = WDTPW + WDTHOLD; 		// Stop WDT
  // initialize P1 direction
  P1DIR = 0x00;                 // clear P1 direction
  init_output();    // we put this one first because it kills the servo init if we put it after for some reason
  init_servo();
  init_adc();
  
  int old; // a variable used to temporarily store old values
  // intialize altitue and azimuth
  int alt = 1000; //altto(alt);
  int azm = 1000; //azmto(azm);

  servo_warmup();

  // scanning procedure:
  
  // wait for user to start python program
  wait(100000);
  
  output(1); // begin header
  
  azmto(550);
  altto(900);
  for (; azm < 1170; ) {
    
    for(; alt < 1325; ) {
      
      output(0);
      output(azm);
      output(alt);
      output(get_r());
      output(get_phi());
      output(get_theta());
      
      wait(1000);
      
      altto(alt + 50);
    }
    
    altto(900);
    azmto(azm + 50);
  }
  
  for (old = 0; old < 100; old++) {
    output(2);
    output(get_r());
    output(get_phi());
    output(get_theta());
  }
  
  while(1); // loop forever
}

