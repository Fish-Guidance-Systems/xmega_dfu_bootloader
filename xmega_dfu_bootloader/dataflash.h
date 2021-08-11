/* dataflash.h
 *
 * Author:  Paul Qureshi
 */


#ifndef DATAFLASH_H_
#define DATAFLASH_H_


#define DF_PAGE_SIZE				264
#define DF_TOTAL_PAGES				4096		// 8Mbit

// memory map
#define DF_MAP_todo					0			// 1 page


extern void	DF_init(void);
extern void	DF_erase_page(uint16_t page);
extern void	DF_erase_sector(uint8_t sector);
extern void	DF_write(void *buffer, size_t size, uint16_t page);
extern void	DF_read(void *buffer, size_t size, uint16_t page, uint16_t byte);
extern void	DF_print_page(uint16_t page);


#endif /* DATAFLASH_H_ */