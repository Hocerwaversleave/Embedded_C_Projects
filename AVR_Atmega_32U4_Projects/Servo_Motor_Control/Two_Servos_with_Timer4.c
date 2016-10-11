/*
 * REMOTOROBO.c
 *
 * Created: 16.04.2016 21:43:27
 * Author : 1
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <inttypes.h>
#include <avr/sfr_defs.h>

//Stop is Key p
//Forward Speed 1 is Key q
//Forward Speed 2 is Key a
//Forward Speed 3 is Key y
//Backward Speed 1 is Key e
//Backward Speed 2 is Key d
//Backward Speed 3 is Key c
//Rotate Left Speed 1 is Key z
//Rotate Left Speed 2 is Key h
//Rotate Left Speed 3 is Key n
//Rotate Right Speed 1 is Key i
//Rotate Right Speed 2 is Key k
//Rotate Right Speed 3 is Key l

#define STOPDRIVE 'P'
#define FWDSMALLDRIVE 'q'
#define FWDMEDIUMDRIVE 'a'
#define FWDLARGEDRIVE 'y'
#define BWDSMALLDRIVE 'e'
#define BWDMEDIUMDRIVE 'd'
#define BWDLARGEDRIVE 'c'
#define ROTATELEFTSMALLDRIVE 'z'
#define ROTATELEFTMEDIUMDRIVE 'h'
#define ROTATELEFTLARGEDRIVE 'n'
#define ROTATERIGHTSMALLDRIVE 'i'
#define ROTATERIGHTMEDIUMDRIVE 'k'
#define ROTATERIGHTLARGEDRIVE 'l'

//#define MILISECSTOGO 1198
//#define MILISECMX 32767
#define STOPTIME 100 //in ms
#define F_CPU 8000000

volatile uint16_t milisecs;

uint8_t speedstop = 95;

uint8_t speedfwdsmall = 89;
uint8_t speedfwdmedium = 85;
uint8_t speedfwdlarge = 79;

uint8_t speedbwdsmall = 101;
uint8_t speedbwdmedium = 105;
uint8_t speedbwdlarge = 111;

//Global variables
//GPIOR0 for UART RX

//Reset MCUCSR early during startup.
void handle_mcucsr(void)
__attribute__((section(".init3")))
__attribute__((naked));
void handle_mcucsr(void){
	MCUSR = 0;
}

static
void
uart_38400(void)
{
	#undef BAUD  // avoid compiler warning
	#define BAUD 38400UL
	#include <util/setbaud.h>
	UBRR1H = UBRRH_VALUE;
	UBRR1L = UBRRL_VALUE;
	#if USE_2X
	UCSR1A |= (1 << U2X1);
	#else
	UCSR1A &= ~(1 << U2X1);
	#endif
}

//Minimum on functions
static
void
setFuses(void){
	MCUCR |= _BV(JTD);//Within four cycles to disable the JTAG interface
	MCUCR |= _BV(JTD);
	CLKPR = _BV(7);
	CLKPR = _BV(0);// Clock Division Factor 16/2
	
	//All power reduction possibilities
	PRR0 = _BV(PRTIM1) | _BV(PRSPI) | _BV(PRADC);
	PRR1 = _BV(PRUSB) | _BV(PRTIM3);
	//Analog Comparator disable:
	ACSR |= _BV(ACD);
}

static
void
iocon(void){
	//Timing Variable
	//tick
	GPIOR1 = 0;
	//Ticks for second
	GPIOR2 = 62;
	//pulsev = 0;
	//Minimum on Timing Register Settings
	TIMSK0 = _BV(TOIE0);
	TCCR0B = _BV(CS00);

	
	//Alles für Timer4
	//Für Motor Left und Right:
	TCCR4C = _BV(COM4D1) | _BV(COM4B1S) | _BV(PWM4D);
	TCCR4D = _BV(WGM40);//Phase and Frequency Correct PWM
	//Für Motor Right:
	TCCR4A |= _BV(PWM4B);
	
	//PD7 aka Motor_Left und PB6 aka Motor_Right als Output deklariern.
	DDRD = _BV(DDD7);
	DDRB = _BV(DDB6);
	
	//Preset für Timer4 Compare and Count Register:
	TCCR4E = _BV(TLOCK4);
	// 10-bit operation
	TC4H = 0x03;
	OCR4C = 0xFF;
	
	TC4H = 0x00;

	OCR4B = speedstop;
	
	OCR4D = speedstop;

	TCCR4E &= ~_BV(TLOCK4);
	//
	//Prescaler Timer4 Activation:
	TCCR4B = _BV(CS42) | _BV(CS41) | _BV(CS40); //Divide by 64
	
	//Setup the UART Interface:
	//Enable Receive Mode
	UCSR1B = _BV(RXEN1);
	//Set frame format 8 0 1
	UCSR1C = _BV(UCSZ10) | _BV(UCSZ11); // 8 bit
	
	milisecs=0;
	//sei();
}

static
void
stopelrobo(){
	TCCR4E = _BV(TLOCK4);

	OCR4B = speedstop;
	
	OCR4D = speedstop;

	TCCR4E &= ~_BV(TLOCK4);
}

static
void
forwardsmallelrobo(){
	TCCR4E = _BV(TLOCK4);

	OCR4B = speedfwdsmall;
	
	OCR4D = speedbwdsmall;

	TCCR4E &= ~_BV(TLOCK4);
}
static
void
forwardmediumelrobo(){
	TCCR4E = _BV(TLOCK4);

	OCR4B = speedfwdmedium;
	
	OCR4D = speedbwdmedium;

	TCCR4E &= ~_BV(TLOCK4);
}
static
void
forwardlargeelrobo(){
	TCCR4E = _BV(TLOCK4);

	OCR4B = speedfwdlarge;
	
	OCR4D = speedbwdlarge;

	TCCR4E &= ~_BV(TLOCK4);
}


static
void
backwardsmallelrobo(){
	TCCR4E = _BV(TLOCK4);

	OCR4B = speedbwdsmall;
	
	OCR4D = speedfwdsmall;

	TCCR4E &= ~_BV(TLOCK4);
}
static
void
backwardmediumelrobo(){
	TCCR4E = _BV(TLOCK4);

	OCR4B = speedbwdmedium;
	
	OCR4D = speedfwdmedium;

	TCCR4E &= ~_BV(TLOCK4);
}
static
void
backwardlargeelrobo(){
	TCCR4E = _BV(TLOCK4);

	OCR4B = speedbwdlarge;
	
	OCR4D = speedfwdlarge;

	TCCR4E &= ~_BV(TLOCK4);
}


static
void
leftturnsmallelrobo(){
	TCCR4E = _BV(TLOCK4);
	
	OCR4B = speedfwdsmall;
	
	OCR4D = speedfwdsmall;

	TCCR4E &= ~_BV(TLOCK4);
}
static
void
leftturnmediumelrobo(){
	TCCR4E = _BV(TLOCK4);

	OCR4B = speedfwdmedium;
	
	OCR4D = speedfwdmedium;

	TCCR4E &= ~_BV(TLOCK4);
}
static
void
leftturnlargeelrobo(){
	TCCR4E = _BV(TLOCK4);

	OCR4B = speedfwdlarge;
	
	OCR4D = speedfwdlarge;

	TCCR4E &= ~_BV(TLOCK4);
}

static
void
rightturnsmallelrobo(){
	TCCR4E = _BV(TLOCK4);

	OCR4B = speedbwdsmall;
	
	OCR4D = speedbwdsmall;

	TCCR4E &= ~_BV(TLOCK4);
}
static
void
rightturnmediumelrobo(){
	TCCR4E = _BV(TLOCK4);

	OCR4B = speedbwdmedium;
	
	OCR4D = speedbwdmedium;

	TCCR4E &= ~_BV(TLOCK4);
}
static
void
rightturnlargeelrobo(){
	TCCR4E = _BV(TLOCK4);

	OCR4B = speedbwdlarge;
	
	OCR4D = speedbwdlarge;

	TCCR4E &= ~_BV(TLOCK4);
}

//Minimum on ISR`s for timing purposes
ISR(TIMER0_OVF_vect){
	GPIOR1++;
	if(GPIOR1 > GPIOR2){
		GPIOR1 = 0;
		milisecs++;
	}
	//if(milisecs > MILISECMX)
}

int 
main(void){

	//Local Flags
	uint8_t stopflag;
	//Stop is Key p
	//Forward Speed 1 is Key q
	//Forward Speed 2 is Key a
	//Forward Speed 3 is Key y
	//Backward Speed 1 is Key e
	//Backward Speed 2 is Key d
	//Backward Speed 3 is Key c
	//Rotate Left Speed 1 is Key z
	//Rotate Left Speed 2 is Key h
	//Rotate Left Speed 3 is Key n
	//Rotate Right Speed 1 is Key i
	//Rotate Right Speed 2 is Key k
	//Rotate Right Speed 3 is Key l
	
	setFuses();
	uart_38400();
	iocon();
    
	for(;;){
	    
		loop_until_bit_is_set(UCSR1A,RXC1);
		GPIOR0 = UDR1;
		if(stopflag){
			sei();
			stopflag = 0;
			stopelrobo();
			while(milisecs < STOPTIME);
			milisecs=0;
			cli();
		}
		switch(GPIOR0){
			case STOPDRIVE:
				stopelrobo();
			break;
			case FWDSMALLDRIVE:
				forwardsmallelrobo();
				stopflag = 1;
			break;
			case FWDMEDIUMDRIVE:
				forwardmediumelrobo();
				stopflag = 1;
			break;
			case FWDLARGEDRIVE:
				forwardlargeelrobo();
				stopflag = 1;
			break;

			case BWDSMALLDRIVE:
				backwardsmallelrobo();
				stopflag = 1;
			break;
			case BWDMEDIUMDRIVE:
				backwardmediumelrobo();
				stopflag = 1;
			break;
			case BWDLARGEDRIVE:
				backwardlargeelrobo();
				stopflag = 1;
			break;
			case ROTATELEFTSMALLDRIVE:
				leftturnsmallelrobo();
				stopflag = 1;
			break;
			case ROTATELEFTMEDIUMDRIVE:
				leftturnmediumelrobo();
				stopflag = 1;
			break;
			case ROTATELEFTLARGEDRIVE:
				leftturnlargeelrobo();
				stopflag = 1;
			break;
		
			case ROTATERIGHTSMALLDRIVE:
				rightturnsmallelrobo();
				stopflag = 1;
			break;
			case ROTATERIGHTMEDIUMDRIVE:
				rightturnmediumelrobo();
				stopflag = 1;
			break;
			case ROTATERIGHTLARGEDRIVE:
				rightturnlargeelrobo();
				stopflag = 1;
				break;
			default:
				stopelrobo();
		}
	}
}
