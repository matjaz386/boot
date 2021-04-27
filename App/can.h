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

HAL_StatusTypeDef	
						FLASH_Erase(uint32_t, uint32_t),
						FLASH_Program(uint32_t, uint32_t);


#define			SW_version			100


#endif
