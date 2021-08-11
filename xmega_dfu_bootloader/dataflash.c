/* dataflash.c
 *
 * Author:  Paul Qureshi
 */

/* Timeouts: The longest common operation used here is the page erase which takes up to 100ms.
 * Page write with erase can take 40ms and page write without erase can take 6ms. Erasing the
 * entire memory takes several seconds at least. Keep in mind that some functions will block
 * while the DF is busy.
 */

#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "hw.h"
#include "dataflash.h"

#define	FLASH_SPI					SPIE		// SPI port to use

// Dataflash commands
#define DF_CMD_RD_DEVID				0x9F		// Manufacturer and Device ID Information
#define DF_CMD_STATUS				0xD7		// Read status register
#define DF_CMD_PAGE_ERASE			0x81		// Erase entire page
#define DF_CMD_SECTOR_ERASE			0x7C		// Erase entire sector
#define DF_CMD_CHIP_ERASE			0xC7		// Erase entire chip
#define DF_CMD_ARRAY_READ			0x0B		// continuous array read
#define DF_CMD_BUF1_WRITE			0x84
#define DF_CMD_BUF2_WRITE			0x87
#define DF_CMD_WRITE_BUF1_W_ERASE	0x83		// write buffer 1 to flash memory with erase
#define DF_CMD_WRITE_BUF2_W_ERASE	0x86		// write buffer 2 to flash memory with erase
#define DF_CMD_WRITE_BUF1_NO_ERASE	0x88		// write buffer 1 to flash memory without erase
#define DF_CMD_WRITE_BUF2_NO_ERASE	0x89		// write buffer 2 to flash memory without erase
#define DF_CMD_READ_PAGE_TO_BUF1	0x53
#define DF_CMD_READ_PAGE_TO_BUF2	0x55
#define DF_CMD_POWER_DOWN			0xB9
#define DF_CMD_RESUME				0xAB
#define DF_CMD_READ_SECURITY_REG	0x77


#pragma region Low Level

/**************************************************************************************************
** Send/receive one byte. Blocks until transaction is complete.
*/
uint8_t df_spi_byte(uint8_t byte)
{
	FLASH_SPI.DATA = byte;							// start transaction
	while ((FLASH_SPI.STATUS & SPI_IF_bm) == 0)		// wait for completion
		;
	return (FLASH_SPI.DATA);
}

/**************************************************************************************************
** Send a page/byte address. Does not validate the page/byte.
*/
void df_spi_addr(uint16_t page, uint16_t byte)
{
	// 264 byte pages
	df_spi_byte((page >> 7) & 0xFF);
	df_spi_byte(((page << 1) | (byte >> 8)) & 0xFF);
	df_spi_byte(byte & 0xFF);
}

/**************************************************************************************************
** Check if Dataflash is ready for next command by examining the status register.
*/
bool df_ready(void)
{
	uint8_t status;

	FLASH_ENABLE;
	df_spi_byte(DF_CMD_STATUS);
	status = df_spi_byte(0);
	FLASH_DISABLE;

	return (status & (1<<7));	// bit 7 is RDY/BUSY
}

/**************************************************************************************************
** Check status register to see if DF is ready to execute further commands.
*/
void df_wait_for_ready(void)
{
	while (!df_ready())	// TODO timeout?
		;
}

#pragma endregion

#pragma region Read / Write / Erase

/**************************************************************************************************
** Erase an entire page.
*/
void DF_erase_page(uint16_t page)
{
	df_wait_for_ready();

	FLASH_ENABLE;
	df_spi_byte(DF_CMD_PAGE_ERASE);
	df_spi_addr(page, 0);
	FLASH_DISABLE;
}

/**************************************************************************************************
** Erase an entire sector.
*/
void DF_erase_sector(uint8_t sector)
{
	if (sector == 0)
		return;

	df_wait_for_ready();

	FLASH_ENABLE;
	df_spi_byte(DF_CMD_SECTOR_ERASE);
	df_spi_addr((uint16_t)sector * 256, 0);
	FLASH_DISABLE;
}

/**************************************************************************************************
** Start / end a continuous array read.
**
** Once started bytes can simply be clocked out with DF_spi_byte(0x00) and the address is
** auto incremented.
*/
void df_start_array_read(uint16_t page, uint16_t byte)
{
	df_wait_for_ready();

	FLASH_ENABLE;
	df_spi_byte(DF_CMD_ARRAY_READ);
	df_spi_addr(page, byte);
	df_spi_byte(0x00);				// dummy byte, see datasheet
}

inline void df_end_array_read(void)
{
	FLASH_DISABLE;
}

/**************************************************************************************************
** Start / end writing Dataflash internal buffer. Always uses buffer 1.
*/
void df_start_buffer_write(uint16_t byte)
{
	df_wait_for_ready();

	FLASH_ENABLE;
	df_spi_byte(DF_CMD_BUF1_WRITE);
	df_spi_addr(0, byte);
}

/**************************************************************************************************
** End writing to a buffer.
*/
void df_end_buffer_write(void)
{
	FLASH_DISABLE;
}

/**************************************************************************************************
** Write buffer 1 to flash memory page. Page auto-erased.
*/
void df_write_buffer(uint16_t page)
{
	df_wait_for_ready();

	FLASH_ENABLE;
	df_spi_byte(DF_CMD_WRITE_BUF1_W_ERASE);
	df_spi_addr(page, 0);
	FLASH_DISABLE;
}

/**************************************************************************************************
** Clear buffer 1. Debug only, code should be robust enough to handle arbitrary data in unused
** parts of DF pages when reading.
*/
void df_clear_buffer(void)
{
	uint8_t		i;

	df_start_buffer_write(0);

	for (i = 0; i < (DF_PAGE_SIZE / 8); i++)
	{
		FLASH_SPI.DATA = 0xFF;							// unrolled a bit for performance
		while ((FLASH_SPI.STATUS & SPI_IF_bm) == 0)
			;
		FLASH_SPI.DATA = 0xFF;
		while ((FLASH_SPI.STATUS & SPI_IF_bm) == 0)
			;
		FLASH_SPI.DATA = 0xFF;
		while ((FLASH_SPI.STATUS & SPI_IF_bm) == 0)
			;
		FLASH_SPI.DATA = 0xFF;
		while ((FLASH_SPI.STATUS & SPI_IF_bm) == 0)
			;
		FLASH_SPI.DATA = 0xFF;
		while ((FLASH_SPI.STATUS & SPI_IF_bm) == 0)
			;
		FLASH_SPI.DATA = 0xFF;
		while ((FLASH_SPI.STATUS & SPI_IF_bm) == 0)
			;
		FLASH_SPI.DATA = 0xFF;
		while ((FLASH_SPI.STATUS & SPI_IF_bm) == 0)
			;
		FLASH_SPI.DATA = 0xFF;
		while ((FLASH_SPI.STATUS & SPI_IF_bm) == 0)
			;
	}
	df_end_buffer_write();
}

/**************************************************************************************************
** Write data from RAM to Dataflash. Whole pages are written at once. If size is not an exact
** multiple of DF_PAGE_SIZE then unused parts of pages will contain random data.
*/
void DF_write(void *buffer, size_t size, uint16_t page)
{
	uint8_t *ptr = (uint8_t *)buffer;
	FLASH_WP_DISABLE;
	while (size)
	{
		uint16_t bytes_to_write = size;
		if (bytes_to_write > DF_PAGE_SIZE)
			bytes_to_write = DF_PAGE_SIZE;
		df_start_buffer_write(0);
		while (bytes_to_write--)
			df_spi_byte(*ptr++);
		df_end_buffer_write();
		df_write_buffer(page++);
		size -= bytes_to_write;
	}
	df_wait_for_ready();
	FLASH_WP_ENABLE;
}

/**************************************************************************************************
** Read data from Dataflash memory.
*/
void DF_read(void *buffer, size_t size, uint16_t page, uint16_t byte)
{
	uint8_t *ptr = (uint8_t *)buffer;
	df_start_array_read(page, byte);
	while (size--)
		*ptr++ = df_spi_byte(0);
	df_end_array_read();
}

#pragma endregion

#pragma region Diagnostics / development

/**************************************************************************************************
** Self-diagnostics. Returns true if Dataflash is OK, otherwise false. If id_store is not 0 it will
** be filled with the DF's ID bytes.
*/
bool DF_diag(uint8_t *id_store)
{
	bool		df_okay = true;
	uint8_t		temp[4];
	uint8_t		i;

	// check device ID and size
	FLASH_ENABLE;
	df_spi_byte(DF_CMD_RD_DEVID);

	for (i = 0; i < 4; i++)
		temp[i] = df_spi_byte(0x00);
	FLASH_DISABLE;

	if ((temp[0] != 0x1F) ||			// manufacturer == Adesto
		(temp[1] != 0x25) ||			// family code, 8mbit
		(temp[2] != 0x00))				// sub-code, variant
	{
		df_okay = false;
	}

	if (id_store != NULL)					// can be null if not used
	{
		id_store[0] = temp[0];
		id_store[1] = temp[1];
		id_store[2] = temp[2];
		id_store[3] = temp[3];
	}

	// check status register bits
	FLASH_ENABLE;
	df_spi_byte(DF_CMD_STATUS);
	temp[0] = df_spi_byte(0x00);
	FLASH_DISABLE;

	if ((temp[0] & 0b00111111) != 0b00100100)		// 0b1001 signature, protect and page size bits should be zero, busy/comp ignored
		df_okay = false;

	return df_okay;
}

/**************************************************************************************************
** Dumps a page from Dataflash to terminal. Debug only. Doesn't validate parameters or check for
** errors.
*/
void DF_print_page(uint16_t page)
{
	uint16_t	byte = 0;
	uint8_t		i, j;

	df_start_array_read(page, 0);
	for (j = 0; j < 8; j++)		// 32 x 8 = 256 bytes
	{
		printf_P(PSTR("0x%02X:\t"), byte);	// print address

		for (i = 0; i < 32; i++)							// print one line of data
			printf_P(PSTR("%02X"), df_spi_byte(0x00));
		putchar('\n');

		byte += 32;
	}

	// last 8 odd bytes
	printf_P(PSTR("0xFF:\t"));
	for (i = 0; i < 8; i++)
		printf_P(PSTR("%02X"), df_spi_byte(0x00));

	df_end_array_read();
	putchar('\n');
}

#pragma endregion

/**************************************************************************************************
** Set up Dataflash, enter low power state
*/
void DF_init(void)
{
	FLASH_WP_ENABLE;
	// reset
	FLASH_RST_ENABLE;
	_delay_ms(1);
	FLASH_RST_DISABLE;
}
