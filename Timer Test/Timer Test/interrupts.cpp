#include "sam.h"

#include "interrupts.h"

void enable_interrupts(void) {
	// Enable Handler for TC4
	NVIC_EnableIRQ(TC4_IRQn);
}

void TC4_Handler () {
	// Compare match 0 interrupt triggered
	if (TC4->COUNT16.INTFLAG.bit.MC0 == 1 )
	{
		REG_TC4_INTFLAG = TC_INTFLAG_MC0;
		
		REG_TC4_COUNT16_COUNT = 0;

		PORT->Group[1].OUTTGL.reg = PORT_PB30;
	}
} 