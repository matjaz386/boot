#include "stm32f4xx_hal.h"
#include "fatfs.h"
#include "can.h"
#include "proc.h"

extern		CRC_HandleTypeDef hcrc;

void			binFileProg(void) {
FATFS			fs;
FIL				f;
uint32_t	i,j,a;
	
					if(f_mount(&fs,"0:",1)==FR_OK) {
						if(f_open(&f,"L405.bin",FA_READ)==FR_OK) {
							for(uint32_t k=SIGN_SECTOR; k<FATFS_SECTOR; ++k) {
								Watchdog();
								FLASH_Erase(k,1);
							}
							a=FLASH_TOP;
							while(!f_eof(&f)) {
								Watchdog();
								if(f_read(&f, &i, sizeof(int), (UINT *)&j) != FR_OK)
									return;
								FLASH_Program(a,i);
								a+=sizeof(int);
							}
							f_close(&f);
							FLASH_Erase(SIGN_SECTOR,1);
							FLASH_Program(SIGN_TOP,FLASH_TOP);
							FLASH_Program(SIGN_TOP+4,(FATFS_TOP-FLASH_TOP)/sizeof(int));
							FLASH_Program(SIGN_TOP+8,HAL_CRC_Calculate(&hcrc,(uint32_t *)FLASH_TOP,  (FATFS_TOP-FLASH_TOP)/sizeof(int)));
							FLASH_Program(SIGN_TOP+12,HAL_CRC_Calculate(&hcrc,(uint32_t *)SIGN_TOP, 3));
						}
						f_mount(NULL,"0:",9);
					}
}
