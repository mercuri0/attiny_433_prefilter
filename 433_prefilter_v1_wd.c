/* attiny45 - 433 MHz prefilter - V 1.0 - written by mercuri0 - 24.12.2013*/
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>

/* ------- handy macro section ------- */
#define bool  _Bool
#define true  1
#define false  0

#define CAT(a, ...) PRIMITIVE_CAT(a, __VA_ARGS__)
#define PRIMITIVE_CAT(a, ...) a ## __VA_ARGS__

//(a ... PORT, b ... PIN)
#define SET(a,b)		a |=  _BV(b)
#define CLEAR(a,b)		a &= ~_BV(b)
#define GET(a,b)			a & _BV(b)
#define TOGGLE(a,b)	a ^= _BV(b)

//(a ... DDR, b ... PIN)
#define SET_OUTPUT(a,b)		a |= _BV(b)
#define SET_INPUT(a,b)		a &= ~_BV(b)

#define D_PORT DDRB
#define D_PIN(a) PRIMITIVE_CAT(DDB,a)

#define V_PORT PORTB
#define V_PIN(a) PRIMITIVE_CAT(PORTB,a)

/* ------- definitions and volatiles ------- */
#define REC_OUT 4
#define PI_IN 3

//counter counts in 10us steps => pulselength/10 !!, be aware of that we don't use a crystal => add some tolerance (±10%)
//MIN_PULSELENGTH =  ( shortest expected pulse - 10% ) / 10
//MAX_PULSELENGTH =  ( longest expected pulse + 10% ) / 10
//
//the shortest expected pulse is the minimal base pulse-width of all your protocols
//the longest expected pulse is the maximal base pulse-width of all your protocols multiplied by 34

#define MIN_PULSELENGTH 10			//tested to work down to 30us pulsewidth (=2)
#define MAX_PULSELENGTH 1600

volatile uint16_t ten_us_counter = 0;		//counts in 10us steps
volatile uint8_t valid_buffer = 0x00;		//if 8 bit in a row are valid, the pulsestream is accepted as valid


/* ------- system initialization ------- */
void init_system(void){
	cli();
	
	//init watchdog
	wdt_reset();						// reset watchdog timer
	MCUSR = 0x00; 
	WDTCR |= ( _BV(WDCE) | _BV(WDE) );	// write logical one to WDCE and WDE
	WDTCR = ( _BV(WDP3) | _BV(WDP0) );	//set watchdog timeout to 8s
	
	//init timer
	SET(TCCR1, CS12);	//timer1 16MHz/8
	SET(TCCR1, CTC1);	//clear timer1 on compare register c match
	OCR1A = OCR1C = 0x14;	//set output compare(a) to output comparec (autoclear at interrupt!) 0x14 => one interrupt each 10us
	SET(TIMSK, OCIE1A);	//enable compare(a) timer1 interrupt
	
	//init input
	SET(PCMSK, PCINT4);	//PB4 pin change interrupt
	SET(MCUCR, ISC00);	//enamle pin interrupt on any change
	SET(GIMSK, PCIE);	//enable pin change interrupt
	
	//init output
	SET_OUTPUT(D_PORT, D_PIN(PI_IN));
	CLEAR(V_PORT, V_PIN(PI_IN));
	
	sei();
}

/* ------- empty main loop ------- */
int main (void)
{
	init_system();
	
	while(1){
		asm volatile("NOP");	//to prevent problems due to optimization
	}
}

/* ------- timer isr ------- */
ISR(TIMER1_COMPA_vect){
	//this isr has to be interruptible, so never block the interrupt (not by cli() and not by CLEAR(TIMSK,OCIE1A);) !!!
	ten_us_counter++;
}

/* ------- pin isr ------- */
ISR(PCINT0_vect){
	//while we calculate the pulse length we don't want any interruption. the timer is resetted anyway!
	cli();
	valid_buffer <<= 1;
    if (ten_us_counter > MIN_PULSELENGTH)
    {
       if (ten_us_counter < MAX_PULSELENGTH)
       {
		   valid_buffer |= 0x01;
		   if (valid_buffer == 0xFF)
		   {
             TOGGLE(V_PORT, V_PIN(PI_IN));
		 }
       }
    }
	
	wdt_reset();		//reset watchdog timer
	ten_us_counter = 0;	//begin new pulselength measurement => reset pulselength counter
	TCNT1 = 0;			//also reset timer1counter
	sei();
}
