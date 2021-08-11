/*
 * main.c
 *
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include "usb.h"
#include "dfu_config.h"
#include "hw.h"
#include "usb/xmega.h"

typedef void (*AppPtr)(void) __attribute__ ((noreturn));

extern void CCPWrite(volatile uint8_t *address, uint8_t value);

volatile bool reset_flag = false;

int main(void)
{
	PORTE.OUT = SPI_nSS_PIN_bm;					// hold SS high for SPI peripheral
	PORTE.DIR = SPI_nSS_PIN_bm | SPI_MOSI_PIN_bm | SPI_SCK_PIN_bm;
	ENABLE_PULLUP(SPI_MISO_PINCTRL);

	PORTF.OUT = FLASH_nCS_PIN_bm | LED1_PIN_bm;	// Flash RST and WP enabled
	PORTF.DIR = FLASH_nCS_PIN_bm;

	PORTJ.OUT = 0xFF;
	PORTJ.DIR = 0xFF;

	PORTK.OUT = 0;
	PORTK.DIR = ~WD_WAKE_PIN_bm;
	ENABLE_PULLUP(WD_WAKE_PINCTRL);

	if (!CheckStartConditions())
	{
		// exit bootloader
		AppPtr application_vector = (AppPtr)0x000000;
		CCP = CCP_IOREG_gc;		// unlock IVSEL
		PMIC.CTRL = 0;			// disable interrupts, set vector table to app section
		EIND = 0;				// indirect jumps go to lower 128k of app section
		RAMPZ = 0;				// LPM uses lower 64k of flash
		application_vector();
	}

	CCPWrite(&PMIC.CTRL, PMIC_IVSEL_bm);	// bootloader interrupt vectors

	usb_configure_clock();
	usb_init();
	PMIC.CTRL |= PMIC_LOLVLEN_bm | PMIC_MEDLVLEN_bm | PMIC_HILVLEN_bm;
	sei();
	usb_attach();

	while (!reset_flag);
	_delay_ms(25);
	usb_detach();
	_delay_ms(100);
	for(;;)
		CCPWrite(&RST.CTRL, RST_SWRST_bm);
}
