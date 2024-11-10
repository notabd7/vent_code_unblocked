/**
 * -------------------------------------------------
 * BOREALIS Valve Control
 * -------------------------------------------------
 * TARGET DEVICE: MSP430G2230
 *
 * Authored by Kristoffer Allick 12/30/2022
 */


#include <msp430.h>
// Configure pins (all in P1)

//pin2 will control the LED
#define ON_LED BIT2
//pin 5
#define RESISTOR BIT5
//pin 6
#define XBEE3  BIT6
//pin7
#define SERVO  BIT7

//servo
#define CLOSED  1
#define OPEN 0
//const unsigned char OPEN = 1;
#define FALSE 0
#define TRUE 1
#define SERVO_ANGLE_BIG 650  //input desired pulse width here from roughly 1700-400 to change angle
#define SERVO_ANGLE_SMALL 500 //input desired pulse width here from roughly 1700-400 to change angle

// ---------------------------------------------------------
// =========================================================


// Function declarations
void initTimer_A(void);
void delay_overflows(int n);
void delay_SEC(int sec);



//Global Variables
unsigned char servo_status = CLOSED;
unsigned int temp;
unsigned long overflow_counter; // Used by delay62MS function & timer
volatile unsigned char control_param = 0;

/**
 * main.c
 */
int main(void){
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer
    servo_status= CLOSED;


    // Configure output pins
    P1DIR |= ON_LED; //P1.2 LED  enable output
    P1OUT |= ON_LED; //LED on
    P1DIR |= SERVO; // P1.SERVO enable output
    P1OUT &=~(SERVO);//Start off
    P1DIR |= RESISTOR; //P1.5 RESISTOR enable output
    P1OUT |=(RESISTOR);//Start on


    // Configure input pin
   // This is like saying:m

// Watch this pin for changes (like watching for the doorbell)
// When the signal changes from low to high (doorbell pressed), stop what you're doing and run this code:
    P1DIR &= ~(XBEE3);//P1.6(XBEE3) enable input
    P1IES &=~(XBEE3); //Start looking for low to high edge
    P1IFG &=~ (XBEE3); //Clear any previous interrupts
    P1IE |=(XBEE3);  // Interrupt Enable
// calls Servo_change
    //MCLK = SMCLK = 1MHz
    DCOCTL =0;
    BCSCTL1 = CALBC1_1MHZ;
    DCOCTL = CALDCO_1MHZ;

    initTimer_A();
    __enable_interrupt();

    while(TRUE){
        delay_SEC(10);
        control_param = 1;
        servo_status = OPEN;
        delay_SEC(10);
        servo_status = CLOSED;
        control_param = 0;
    };

}
//------------------Functions-----------------------------------
void initTimer_A(void){
    //Timer0_A3 Config
    TACCTL0 |=CCIE; //Enable CCRO interrupt
    TACCTL1 |=CCIE; //Enable CCR1 interrupt
    TACCR1 = 20000; //CCR1 intial value (Period) Sets the period to 20 milliseconds
// Servo motors need a signal every 20ms to work properly
    TACCR0 = TACCR1+SERVO_ANGLE_SMALL+(SERVO_ANGLE_BIG*servo_status); // CCR0 initial value (Pulse Width)
    TACTL = TASSEL_2 + ID_0 + MC_2; //Use SMCLK, SMLK/1, Counting Continous Mode
}
//Delays the inputted overflow
void delay_overflows(int n){
    unsigned long desired_overflows= overflow_counter+n; //cool maths for desired delay in 65 ms intervals
    while (overflow_counter < desired_overflows);//do nothing until desired delay in 65 ms intervals
}
void delay_SEC(int sec){
    unsigned long desired_overflows = overflow_counter+15*sec; //cool maths for desired delay in minutes
    while (overflow_counter < desired_overflows);//do nothing until desired delay in minutes
}

void Resist(void){

}

//------------------------Interrupt Service Routines---------------------
//Timer ISR
#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer_A_CCR0_ISR(void) {
   P1OUT &=~(SERVO);        //set SERVO low
   TACCR0 = TACCR1+SERVO_ANGLE_SMALL+(SERVO_ANGLE_BIG*servo_status);    //Add one Period
}
//Servo ISR
#pragma vector = TIMER0_A1_VECTOR
__interrupt void Timer_A_CCR1_ISR(void){
    temp = TAIV;
    if (temp == TAIV_TACCR1){
        P1OUT |=(SERVO);    //set SERVO High
        TACCR1 +=20000;     //add one Period
    }
}

//PWM pin to SERVO pin ISR
#pragma vector = PORT1_VECTOR
__interrupt void Servo_change(void){
    if ((P1IES & XBEE3) == 0){  //rising edge trigger
        P1IES |=(XBEE3);        //start looking for falling edge
        servo_status = OPEN;
    }
    else{                   //falling edge trigger
        P1IES &=~(XBEE3);   //start looking for rising edge
        servo_status = CLOSED;
    }
    P1IFG &=~(XBEE3);
}






