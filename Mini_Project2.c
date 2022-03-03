/*
 * Mini_Project2.c
 *
 *  Created on: Jan 13, 2022
 *      Author: Menna Saeed
 *      Diploma 62
 */


#include <avr/io.h>
#include<avr/interrupt.h>
#include <util/delay.h>

/* Global variables declaration*/
volatile unsigned char second=0;
volatile unsigned char minute=0;
volatile unsigned char hour=0;

/*Timer 1 code- Compare mode*/

void Timer1_CTC(void)
{
	TCNT1=0;
	TCCR1A=(1<<FOC1A); //For non-PWM mode
	TCCR1B=(1<<WGM12)|(1<<CS11)|(1<<CS10); //CTC mode-> WGM12=1 & WGM11=0 & WGM10=0 & WGM13=0
	//Choosing prescaler N=64
	/*Compare value calculating:
	 * let N=64
	 * F_OC1A=F_CPU/N*(1+Top)
	 * If we want the interrupt to be executed every 1 second, then the Timer Frequency would be 1Hz
	 * Hence, The top value is 15625
	 */
	OCR1A=15625;
	SREG|=(1<<7); //Enable I-bit
	TIMSK|=(1<<OCIE1A); //Output Compare A match Interrupt Enable
}

/*ISR Timer 1 -> Increment the 7-segment*/
ISR (TIMER1_COMPA_vect)
{
	second++;
	if (second==60)
	{
		second=0;
		minute++;
	}
	if (minute==60)
	{
		second=0;
		minute=0;
		hour++;
	}
	if (hour==12)
	{
		second=0;
		minute=0;
		hour=0;
	}

}

/*External INT0: RESET
 * Falling edge
 * Push button-> Internal pull-up resistor
 * If falling edge is detected, RESET
 */
void INT0_Init_RESET (void)
{
 	DDRD &=~(1<<PD2); //INT0 input
	PORTD|=(1<<PD2); //Enable internal pull-up resistor
	GICR|=(1<<INT0); //Enable INT0
	MCUCR|=(1<<ISC01); //Falling edge
	MCUCR&=(1<<ISC00); //Falling edge
}

ISR (INT0_vect)
{
	second=0;
	minute=0;
	hour=0;
}

/*External INT1: PAUSE
 * Raising edge
 * Push button -> External pull-down
 * When raising edge is detected, PAUSE
 */

void INT1_Init_PAUSE (void)
{
	DDRD &=~(1<<PD3); //INT1 input
	GICR|=(1<<INT1); //Enable INT1
	MCUCR|=(1<<ISC10)|(1<<ISC11); //Raising edge
}

ISR (INT1_vect)
{
	TIMSK&=~(1<<OCIE1A);
}

/*External INT2: RESUME
 * Falling edge
 * Push button-> Internal pull-up
 * When falling edge is detected, RESUME
 */

void INT2_Init_RESUME (void)
{
	DDRB &=~(1<<PB2); //INT2 input
	PORTB|=(1<<PB2); //Enable internal pull-up resistor
	GICR|=(1<<INT2); //Enable INT2
	MCUCSR&=~(1<<ISC2); //Falling edge
}

ISR (INT2_vect)
{
	TIMSK|=(1<<OCIE1A);
}


/*Main Program*/

int main (void)
{
	DDRC=0x0F;
	PORTC=0x00;
	DDRA=0x3F;
	PORTA=0xFF;
	SREG|=(1<<7); //Enable I-bit

	INT0_Init_RESET();
	INT1_Init_PAUSE();
	INT2_Init_RESUME();
	Timer1_CTC();
	/*SUPER LOOP*/
	while(1)
	{
		PORTA=(1<<5); //6th 7-segment
		PORTC=(second%10); //1st digit
		_delay_ms(5);
		PORTA=(1<<4); //5th 7-segment
		PORTC=(second/10); //2nd digit
		_delay_ms(5);
		PORTA=(1<<3); //4th 7-segment
		PORTC=(minute%10); //1st digit
		_delay_ms(5);
		PORTA=(1<<2); //3rd 7-segment
		PORTC=(minute/10); //2nd digit
		_delay_ms(5);
		PORTA=(1<<1); //2nd 7-segment
		PORTC=(hour%10);  //1st digit
		_delay_ms(5);
		PORTA=(1<<0); //1st 7-segment
		PORTC=(hour/10);  //2nd digit
		_delay_ms(5);
	}
}
