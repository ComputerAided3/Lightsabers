#include "sam.h"

#include "timers.h"

void init_TMR4(void) {
	// Set GCLK3 (8 MHz) as clock generator for TMR4
	GCLK->CLKCTRL.reg = GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK3 | GCLK_CLKCTRL_ID_TC4_TC5;
	while (GCLK->STATUS.bit.SYNCBUSY);
	
	// Enable TC4 Bus
	REG_PM_APBCMASK |= PM_APBCMASK_TC4;
	
	// TC4 Setup
	// 16-bit count mode
	// Normal FRQ
	// 8 MHz / 8 = 1 MHz
	REG_TC4_CTRLA = TC_CTRLA_MODE_COUNT16 | TC_CTRLA_WAVEGEN_NFRQ | TC_CTRLA_PRESCALER_DIV8;
	
	// Enable Interrupts
	REG_TC4_INTENSET = TC_INTENSET_MC0;
	
	// Count register
	// 1 MHz / 1000 = 1 kHz
	REG_TC4_COUNT16_CC0 = 1000;
	
	// Enable
	REG_TC4_CTRLA |= TC_CTRLA_ENABLE;
	while (TC4->COUNT16.STATUS.bit.SYNCBUSY == 1);
}
