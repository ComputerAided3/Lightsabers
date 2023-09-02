#include "sam.h"

#include "spi.h"

void spi_init(uint8_t dord, uint8_t cpol, uint8_t cpha) {
	// Reset SERCOM
	REG_SERCOM1_SPI_CTRLA |= SERCOM_SPI_CTRLA_SWRST;
	while (SERCOM1->SPI.SYNCBUSY.bit.SWRST == 1);

	// Configure general clock generator for general clock 3
	REG_GCLK_GENCTRL = GCLK_GENCTRL_GENEN | GCLK_GENCTRL_SRC_OSC8M | GCLK_GENCTRL_ID(3);

	// Configure GCLK3 as SERCOM1 core clock
	// Same as I2C
	REG_GCLK_CLKCTRL = GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK3 | GCLK_CLKCTRL_ID_SERCOM1_CORE;
	REG_GCLK_GENDIV = GCLK_GENDIV_ID(3); // Div 1

	REG_PM_APBCMASK |= PM_APBCMASK_SERCOM1; // Enable TC1 bus clock
		
	// SPI Master
	REG_SERCOM1_SPI_CTRLA |= SERCOM_SPI_CTRLA_MODE_SPI_MASTER;
	
	// Configure SPI
	REG_SERCOM1_SPI_CTRLA |= 
	(cpol ? SERCOM_SPI_CTRLA_CPOL : 0) | // Clock polarity
	(cpha ? SERCOM_SPI_CTRLA_CPHA : 0) | // Clock phase
	(dord ? SERCOM_SPI_CTRLA_DORD : 0) | // Data order
	SERCOM_SPI_CTRLA_DIPO(3) |           // Data in config
	SERCOM_SPI_CTRLA_DOPO(0);            // Data out config
	
	// Set Baud Rate (1MHz)
	REG_SERCOM1_SPI_BAUD = 3;
}
	
void spi_enable() {
	// Enable Pins
	PORT->Group[0].PMUX[MOSI_PIN >> 1].reg &= (MOSI_PIN % 2 ? 0x0f : 0xf0); // Clear PMUX
	PORT->Group[0].PMUX[MOSI_PIN >> 1].reg |= (MOSI_PIN % 2 ? PORT_PMUX_PMUXO_C : PORT_PMUX_PMUXE_C);
	PORT->Group[0].PINCFG[MOSI_PIN].reg |= PORT_PINCFG_PMUXEN;
	
	PORT->Group[0].PMUX[MISO_PIN >> 1].reg &= (MISO_PIN % 2 ? 0x0f : 0xf0); // Clear PMUX
	PORT->Group[0].PMUX[MISO_PIN >> 1].reg |= (MISO_PIN % 2 ? PORT_PMUX_PMUXO_C : PORT_PMUX_PMUXE_C);
	PORT->Group[0].PINCFG[MISO_PIN].reg |= PORT_PINCFG_INEN | PORT_PINCFG_PMUXEN;
	
	PORT->Group[0].PMUX[SCK_PIN >> 1].reg &= (SCK_PIN % 2 ? 0x0f : 0xf0); // Clear PMUX
	PORT->Group[0].PMUX[SCK_PIN >> 1].reg |= (SCK_PIN % 2 ? PORT_PMUX_PMUXO_C : PORT_PMUX_PMUXE_C);
	PORT->Group[0].PINCFG[SCK_PIN].reg |= PORT_PINCFG_PMUXEN;
	
	// Enable SERCOM1
	REG_SERCOM1_SPI_CTRLA |= SERCOM_SPI_CTRLA_ENABLE;
	while(SERCOM0->SPI.SYNCBUSY.bit.ENABLE); // Sync
}

void spi_write_byte(uint8_t data) {
	while (SERCOM1->SPI.INTFLAG.bit.DRE != 1); // Make sure DATA register empty
	REG_SERCOM1_SPI_DATA = data;               // Transmit data
	while (SERCOM1->SPI.INTFLAG.bit.TXC != 1); // Wait until sent
}


uint8_t spi_trade_byte(uint8_t data) {
	REG_SERCOM1_SPI_CTRLB |= SERCOM_SPI_CTRLB_RXEN; // RX enable
	while (SERCOM1->SPI.SYNCBUSY.bit.CTRLB == 1);
	
	while (SERCOM1->SPI.INTFLAG.bit.DRE != 1);  // Make sure DATA register empty
	REG_SERCOM1_SPI_DATA = data;                // Transmit data
	while (SERCOM1->SPI.INTFLAG.bit.TXC != 1 ||
	SERCOM1->SPI.INTFLAG.bit.RXC != 1);              // Wait until sent and received
	
	uint8_t recv = REG_SERCOM1_SPI_DATA;
	
	REG_SERCOM1_SPI_CTRLB &= ~SERCOM_SPI_CTRLB_RXEN; // RX disable
	while (SERCOM1->SPI.SYNCBUSY.bit.CTRLB == 1);
	
	return recv;
}

void spi_write_bytes(uint8_t *data, uint16_t len) {
	for (uint16_t i = 0; i < len; ++i) {
		while (SERCOM1->SPI.INTFLAG.bit.DRE != 1); // Make sure DATA register empty
		REG_SERCOM1_SPI_DATA = data[i];            // Transmit data
		while (SERCOM1->SPI.INTFLAG.bit.TXC != 1); // Wait until sent
	}
}

void spi_trade_bytes(uint8_t *datain, uint8_t *dataout, uint16_t len) {
	REG_SERCOM1_SPI_CTRLB |= SERCOM_SPI_CTRLB_RXEN; // RX enable
	while (SERCOM1->SPI.SYNCBUSY.bit.CTRLB == 1);
	
	for (uint16_t i = 0; i < len; ++i) { 
		while (SERCOM1->SPI.INTFLAG.bit.DRE != 1);    // Make sure DATA register empty
		REG_SERCOM1_SPI_DATA = datain[i];             // Transmit data
		while (SERCOM1->SPI.INTFLAG.bit.TXC != 1 ||
		SERCOM1->SPI.INTFLAG.bit.RXC != 1);           // Wait until sent and received
		
		dataout[i] = REG_SERCOM1_SPI_DATA;            // Add received data
	}
	
	REG_SERCOM1_SPI_CTRLB &= ~SERCOM_SPI_CTRLB_RXEN; // RX disable
	while (SERCOM1->SPI.SYNCBUSY.bit.CTRLB == 1);
}

void spi_disable() {
	// Disable SERCOM
	REG_SERCOM1_SPI_CTRLA &= ~SERCOM_SPI_CTRLA_ENABLE; // Disable
	while(SERCOM1->SPI.SYNCBUSY.bit.ENABLE); // Sync
	
	// Disable MUX
	PORT->Group[0].PINCFG[MOSI_PIN].reg &= ~PORT_PINCFG_PMUXEN;
	PORT->Group[0].PINCFG[MISO_PIN].reg &= ~PORT_PINCFG_PMUXEN;
	PORT->Group[0].PINCFG[SCK_PIN].reg &= ~PORT_PINCFG_PMUXEN;
}
	