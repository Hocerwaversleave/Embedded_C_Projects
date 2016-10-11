/*
 * BlueMasterSimp.c
 *
 * Created: 16.01.2016 10:40:01
 *  Author: 1
 */ 


#include <avr/io.h>
#include <inttypes.h>

static
void
setFuses(void){
	
	MCUCR |= (1<<JTD);//Within four cycles to disable the JTAG interface
	MCUCR |= (1<<JTD);
	CLKPR = 0x80;
	CLKPR = 0x01;// Clock Division Factor 16/2
}

#define F_CPU 8000000
//USART Baudrate einstellen und initilisieren fuer Transmit und Transmit Ineterrupt

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
static
void
iocon(void){

	DDRB &= ~_BV(DDB0);
	PORTB |= _BV(PORTB0);
	
	//Set frame format 8 0 1
	UCSR1B = _BV(TXEN1);
	UCSR1C = _BV(UCSZ10) | _BV(UCSZ11); // 8 bit
}
static 
void
putchr(char *c)
{
	
	while(!(UCSR1A & _BV(UDRE1)));
	UDR1 = *c;
}
static
uint8_t
PollingGoKey(void){
	
	static uint8_t stateKey;
	register uint8_t valueReturn;
	
	if( (stateKey == 0) && !(PINB & (1 << PINB0) ) ){
		
		stateKey = 1;
		valueReturn = 1;
	}
	
	else if( (stateKey == 1) && !(PINB & (1 << PINB0) ) ){
		
		stateKey = 2;
		valueReturn = 0;
	}
	
	else if( (stateKey == 2) && (PINB & (1 << PINB0) ) ){
		
		stateKey = 3;
		valueReturn = 0;
	}
	else if( (stateKey == 3) && (PINB & (1 << PINB0) ) ){
		
		stateKey = 0;
		valueReturn = 0;
	}
	else
		valueReturn = 0;
	
	return valueReturn;
}

int main(void){
	
	//symb Liste
	char symbgo = 'G';
	
	setFuses();
	uart_38400();
	iocon();
	
    while(1){
		
		if(PollingGoKey())
			putchr(&symbgo);
		
    }
}
