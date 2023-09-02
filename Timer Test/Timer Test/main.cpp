/*
 * Timer Test.cpp
 *
 * Created: 3/14/2023 3:50:19 PM
 * Author : Kenny Hora
 */ 


#include "sam.h"

#include "clocks.h"
#include "interrupts.h"
#include "timers.h"

// Onboard LED stuff
// Yellow LED is PB30
#define ONBOARD_LED_PIN 30
#define ONBOARD_LED     PORT_PB30


int main(void) {
	init_clock_XOSC32K_8M();
	init_clock_DFLL48M();
	
	init_TMR4();
	
	enable_interrupts();
	
	
	
	// PORT Stuff - Set up all GPIO stuff appropriately
	PORT->Group[1].DIRSET.reg = ONBOARD_LED;
	
    while (1) 
    {
		
	}
}