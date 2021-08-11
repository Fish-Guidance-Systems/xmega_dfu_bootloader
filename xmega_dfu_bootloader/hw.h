/*
 * hw.h
 *
 * Created: 26/04/2021 11:29:27
 *  Author: Paul Qureshi
 */ 


#ifndef HW_H_
#define HW_H_


/**************************************************************************************************
** GPIOs
*/
// port E
#define	SPI_nSS_PIN_bm					PIN4_bm
#define	SPI_MOSI_PIN_bm					PIN5_bm
#define	SPI_MISO_PIN_bm					PIN6_bm
#define	SPI_MISO_PINCTRL				PORTE.PIN6CTRL
#define	SPI_SCK_PIN_bm					PIN7_bm
// port F
#define	FLASH_PORT						PORTF
#define	FLASH_nCS_PIN_bm				PIN2_bm
#define	FLASH_ENABLE					do { PORTF.OUTCLR = FLASH_nCS_PIN_bm; } while(0)
#define	FLASH_DISABLE					do { PORTF.OUTSET = FLASH_nCS_PIN_bm; } while(0)
#define	FLASH_nWP_PIN_bm				PIN3_bm
#define	FLASH_WP_ENABLE					do { PORTF.OUTCLR = FLASH_nWP_PIN_bm; } while(0)
#define	FLASH_WP_DISABLE				do { PORTF.OUTSET = FLASH_nWP_PIN_bm; } while(0)
#define	FLASH_nRST_PIN_bm				PIN4_bm
#define	FLASH_RST_ENABLE				do { PORTF.OUTCLR = FLASH_nRST_PIN_bm; } while(0)
#define	FLASH_RST_DISABLE				do { PORTF.OUTSET = FLASH_nRST_PIN_bm; } while(0)
#define	LED1_PIN_bm						PIN5_bm
#define	LED1_ENABLE						do { PORTF.OUTCLR = LED1_PIN_bm; } while(0)
#define	LED1_DISABLE					do { PORTF.OUTSET = LED1_PIN_bm; } while(0)
#define	LED1_TOGGLE						do { PORTF.OUTTGL = LED1_PIN_bm; } while(0)
// port J
#define	LED3_PIN_bm						PIN0_bm
#define	LED3_ENABLE						do { PORTJ.OUTCLR = LED3_PIN_bm; } while(0)
#define	LED3_DISABLE					do { PORTJ.OUTSET = LED3_PIN_bm; } while(0)
#define	LED3_TOGGLE						do { PORTJ.OUTTGL = LED3_PIN_bm; } while(0)
#define	LED2_PIN_bm						PIN7_bm
#define	LED2_ENABLE						do { PORTJ.OUTCLR = LED2_PIN_bm; } while(0)
#define	LED2_DISABLE					do { PORTJ.OUTSET = LED2_PIN_bm; } while(0)
#define	LED2_TOGGLE						do { PORTJ.OUTTGL = LED2_PIN_bm; } while(0)
// port K
#define	WD_PORT							PORTK
#define	WD_DONE_PIN_bm					PIN1_bm
#define	WD_DONE_ENABLE					do { PORTK.OUTSET = WD_DONE_PIN_bm; } while(0)
#define	WD_DONE_DISABLE					do { PORTK.OUTCLR = WD_DONE_PIN_bm; } while(0)
#define	WD_WAKE_PIN_bm					PIN2_bm
#define	WD_WAKE_PINCTRL					PORTK.PIN2CTRL


#endif /* HW_H_ */
