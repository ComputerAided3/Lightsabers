#include "sam.h"

#include "clocks.h"

/*
	Function: init_clock_XOSC32K_8M
	
	Preconditions: none
	
	Inputs: none
	
	Output: none
	
	Description: Enables the external 32.786 kHz oscillator and internal 8 MHZ
	oscillator with no prescaler.
	
	GCLK2 operates at 32.768kHz
	GCLK3 operates at 8MHZ
*/
void init_clock_XOSC32K_8M(void) {
	// SYSCTRL Stuff - Enable OSC8M and OSC32K
	SYSCTRL->OSC8M.bit.PRESC = 0;   // Div 1
	SYSCTRL->OSC8M.bit.ENABLE = 1;  // Enable OSC8M
	while (SYSCTRL->PCLKSR.bit.OSC8MRDY == 0); // Wait for clock to stabilize
	
	SYSCTRL->XOSC32K.reg = SYSCTRL_XOSC32K_XTALEN | SYSCTRL_XOSC32K_EN32K | SYSCTRL_XOSC32K_STARTUP(0x5); // External oscillator, enable output, 1 s startup
	SYSCTRL->XOSC32K.bit.ENABLE = 1;
	while (SYSCTRL->PCLKSR.bit.XOSC32KRDY == 0);
	
	// GCLK Stuff - Enable GCLK Generators
	// Note: This does not send any of the clocks out to peripherals
	//       This is done by writing to the GCLK->CLKCTRL register (datasheet p. 122)
	
	// Configure GCLK3 to operate at 8 MHz
	GCLK->GENDIV.reg = GCLK_GENDIV_DIV(1) | GCLK_GENDIV_ID(3);
	GCLK->GENCTRL.reg = GCLK_GENCTRL_GENEN | GCLK_GENCTRL_SRC_OSC8M | GCLK_GENCTRL_ID(3);
	
	while(GCLK->STATUS.bit.SYNCBUSY);

	
	// Configure GCLK2 to operate at 32.768 kHz (DFLL48M Clock)
	GCLK->GENDIV.reg = GCLK_GENDIV_DIV(1) | GCLK_GENDIV_ID(2);
	GCLK->GENCTRL.reg = GCLK_GENCTRL_GENEN | GCLK_GENCTRL_SRC_XOSC32K | GCLK_GENCTRL_ID(2);
	
	while(GCLK->STATUS.bit.SYNCBUSY);
}


/*
	Function: init_clock_DFLL48M
	
	Preconditions: GCLK2 is set up and operates at 32.768kHz
	
	Inputs: none
	
	Output: none
	
	Description: Enables the DFLL48M and sets the CPU and APBx Bus clocks to run at 48MHz. Sets 
	NVM wait states for 48 MHz operation at 3.3V. 
	
	GCLK0 operates at 48 MHz.
*/
void init_clock_DFLL48M(void) {
	// Configure the GCLK multiplexer to send the 32.768 kHz clock from GCLK2 as the source for DFLL48M
	GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID_DFLL48 | GCLK_CLKCTRL_GEN_GCLK2 | GCLK_CLKCTRL_CLKEN;
	
	// Per Table 37-42 of the datasheet, configure NVM Wait States
	NVMCTRL->CTRLB.bit.RWS = 1;		/* 1 wait state required @ 3.3V & 48MHz */
	
	// This works around a quirk in the hardware (errata 1.2.1) -
	// the DFLLCTRL register must be manually reset to this value before configuration.
	while(!SYSCTRL->PCLKSR.bit.DFLLRDY);
	SYSCTRL->DFLLCTRL.reg = SYSCTRL_DFLLCTRL_ENABLE;
	while(!SYSCTRL->PCLKSR.bit.DFLLRDY);
	
	// Set up the multiplier. This tells the DFLL to multiply the 32.768 kHz
	// reference clock to 48 MHz
	SYSCTRL->DFLLMUL.reg =
	SYSCTRL_DFLLMUL_MUL(1465) | // output frequency / reference clock frequency, so 48 MHz / 32.768 kHz
	// The coarse and fine step are used by the DFLL to lock
	// on to the target frequency. These are set to half
	// of the maximum value. Lower values mean less overshoot,
	// whereas higher values typically result in some overshoot but
	// faster locking.
	SYSCTRL_DFLLMUL_FSTEP(511) | // max value: 1023
	SYSCTRL_DFLLMUL_CSTEP(31);  // max value: 63
	while(!SYSCTRL->PCLKSR.bit.DFLLRDY); // Wait for the write to finish

	// Get calibration data
	uint32_t coarse = (*((uint32_t *)FUSES_DFLL48M_COARSE_CAL_ADDR) & FUSES_DFLL48M_COARSE_CAL_Msk) >> FUSES_DFLL48M_COARSE_CAL_Pos;
	SYSCTRL->DFLLVAL.bit.COARSE = coarse;
	while(!SYSCTRL->PCLKSR.bit.DFLLRDY);
	
	// Enable DFLL48M
	SYSCTRL->DFLLCTRL.reg |= SYSCTRL_DFLLCTRL_MODE | SYSCTRL_DFLLCTRL_WAITLOCK | SYSCTRL_DFLLCTRL_ENABLE;

	// Wait for the frequency to lock
	while (!SYSCTRL->PCLKSR.bit.DFLLLCKC || !SYSCTRL->PCLKSR.bit.DFLLLCKF) {}
	
	// Configure GCLK1 to operate at 48 MHz
	GCLK->GENDIV.reg = GCLK_GENDIV_DIV(1) | GCLK_GENDIV_ID(0);
	GCLK->GENCTRL.reg = GCLK_GENCTRL_GENEN | GCLK_GENCTRL_SRC_DFLL48M | GCLK_GENCTRL_ID(0) | GCLK_GENCTRL_IDC;
	
	while(GCLK->STATUS.bit.SYNCBUSY);
	
	// Disable prescalers on CPU and APBx Bus Clocks
	PM->CPUSEL.reg  = PM_CPUSEL_CPUDIV_DIV1 ;
	PM->APBASEL.reg = PM_APBASEL_APBADIV_DIV1_Val ;
	PM->APBBSEL.reg = PM_APBBSEL_APBBDIV_DIV1_Val ;
	PM->APBCSEL.reg = PM_APBCSEL_APBCDIV_DIV1_Val ;
}
