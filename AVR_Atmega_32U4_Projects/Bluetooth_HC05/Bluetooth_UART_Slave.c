/*
 * BlueSlaveSimp.c
 *
 * Created: 16.01.2016 10:54:07
 *  Author: 1
 */ 


#include <avr/io.h>
#include <avr/interrupt.h>
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

	DDRD |= _BV(DDD6);
	//Set frame format 8 0 1
	UCSR1B = _BV(RXEN1) | _BV(RXCIE1);
	UCSR1C = _BV(UCSZ10) | _BV(UCSZ11); // 8 bit
	sei();
}

volatile char rxBuffer;//stores incoming byte
volatile uint8_t rxFlag;//tells main if new byte in rxBuffer

ISR(USART1_RX_vect){
	
	rxBuffer = UDR1;
	rxFlag = 1;
}

int main(void){
	
	char symbgo = 'G';
	setFuses();
	uart_38400();
	iocon();
	
    for(;;){
        
		 while(!rxFlag);
		 
		 rxFlag = 0;
		 
		 if(rxBuffer == symbgo)
			PORTD ^= _BV(PORTD6);
    }
}
