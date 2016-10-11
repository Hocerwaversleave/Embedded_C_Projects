/*
 * BluetoothCON.c
 *
 * Created: 16.01.2016 02:51:54
 *  Author: 1
 */ 


#include <avr/io.h>
#include <avr/interrupt.h>
#include <inttypes.h>
#include <avr/pgmspace.h>

#define F_CPU 8000000//Prescaller settings needed +++

static
void
set_fuses(void){
	
	MCUCR |= (1<<JTD);
	MCUCR |= (1<<JTD);
	CLKPR = 0x80;
	CLKPR = 0x01;//16/2
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

static
void
iocon(void){
	
	//Key PB0 Pull up
	PORTB |= _BV(PORTB0);
	
	//LED all done PD6 Output
	DDRD |= _BV(DDD6);
	
	UCSR1B = _BV(TXEN1);
	UCSR1C = _BV(UCSZ10) | _BV(UCSZ11); // 8 bit
	
	//Timer 1
	TIMSK1 |= _BV(TOIE1); //Timer_0 Overflow Interrupt freigeben
	TCCR1B = TCCR1B | (1<<CS11)|(1<<CS10);//:64 Teiler => dt=64/8MHz=8us
	sei(); //Halbe Sekunde
	
}

static void
putchr(char c)
{
	
	while(!(UCSR1A & _BV(UDRE1)));
	UDR1 = c;
}

static void
printstr_p(const char *s)
{
	char c;

	for (c = pgm_read_byte(s); c; ++s, c = pgm_read_byte(s))
	{
		putchr(c);
	}
}
volatile uint8_t timerint;

ISR (TIMER1_OVF_vect){

	timerint = 1;		
}

int 
main(void){
	
	enum
	{
		ATORGL,
		ATNAME,
		ATROLE,
		ATCMODE,
		ATDONE
	} __attribute__((packed)) mode = ATORGL;
	
	set_fuses();
	uart_38400();
	iocon();
	
    for(;;)
    {
		
		while(!timerint);
		
			timerint = 0; //about 0,5s each
				
			switch (mode)
			{
				case ATORGL:
					mode = ATNAME;
					printstr_p(PSTR("AT+ORGL\r\n"));
				break;
			
				case ATNAME:
					mode = ATROLE;
					printstr_p(PSTR("AT+NAME=Slave\r\n"));
				break;
			
				case ATROLE:
					mode = ATCMODE;
					printstr_p(PSTR("AT+ROLE=0\r\n"));//Master
				break;
			
				case ATCMODE:
					mode = ATDONE;
					printstr_p(PSTR("AT+CMODE=1\r\n"));//Any Address
						
				break;
				
				case ATDONE:
					PORTD ^= _BV(PORTD6);
					break;
			}
	}
}