#ifndef SPI_H
#define SPI_H

// The SPI interface will be run on SERCOM1

#define MOSI_PIN 30
#define MISO_PIN 25
#define SCK_PIN  31

#define SPI_DORD_MSBFIRST 0
#define SPI_DORD_LSBFIRST 1

void spi_init(uint8_t dord, uint8_t cpol, uint8_t cpha);   // Configures SPI
void spi_enable(); // Enables SPI

void spi_write_byte(uint8_t data);
uint8_t spi_trade_byte(uint8_t data);

void spi_write_bytes(uint8_t *data, uint16_t len);
void spi_trade_bytes(uint8_t *datain, uint8_t *dataout, uint16_t len);

void spi_disable(); // Disable SPI

#endif