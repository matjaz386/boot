#ifndef		_CAN_H
#define		_CAN_H

#include	"stm32f4xx_hal.h"
#include	"io.h"

typedef enum {    
	_ID_IAP_GO				=0xA0,
	_ID_IAP_ERASE,
	_ID_IAP_ADDRESS,
	_ID_IAP_DWORD,
	_ID_IAP_ACK,
	_ID_IAP_SIGN,
	_ID_IAP_STRING,
	_ID_IAP_PING,
} _StdId;

typedef struct {
	unsigned mask:6;
	unsigned sect:2;
} test;


typedef union {
		__packed struct {
			__packed struct {
				uint8_t	ch:3, count:4, longpulse:1;
			} sect[4];
			uint32_t tref;
		} pulse;
		uint8_t		byte[8];
		uint8_t		bytes[8];
		uint16_t	hword[4];
		uint32_t	word[2];
		uint64_t	dword;
	} payload;

typedef struct {
	CAN_RxHeaderTypeDef hdr;
	payload buf;
} CanRxMsg;

typedef struct {
	CAN_TxHeaderTypeDef hdr;
	payload buf;
} CanTxMsg;

void				Send(int, payload *,int);
void				*canRx(void *),
						*canTx(void *);

#define			_SIGN_PAGE			FLASH_Sector_1
#define			_FLASH_TOP			0x08008000
#define			_BOOT_SECTOR		0x08000000
#define			FATFS_ADDRESS 	0x08040000
						                        
#define			_FW_START				((int *)(_FLASH_TOP-16))
#define			_FW_CRC					((int *)(_FLASH_TOP-20))
#define			_FW_SIZE				((int *)(_FLASH_TOP-24))
#define			_SIGN_CRC				((int *)(_FLASH_TOP-28))
#define			_FLASH_BLANK		((int)-1)
	
HAL_StatusTypeDef	FLASH_Erase(uint32_t, uint32_t),
									FLASH_Program(uint32_t, uint32_t);
#define			SW_version			100


#endif
