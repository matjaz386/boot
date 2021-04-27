#include	"can.h"

_io				*_CAN;
uint32_t	filter_count,flash_address;
payload		py;

extern		CAN_HandleTypeDef hcan2;
extern		CRC_HandleTypeDef hcrc;
extern		uint32_t					boot_timeout;
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
HAL_StatusTypeDef	FLASH_Program(uint32_t Address, uint32_t Data) {
			HAL_StatusTypeDef status=HAL_OK;
			__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_OPERR  | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR );
			if(*(uint32_t *)Address !=  Data) {
				HAL_FLASH_Unlock();
				do
					status=HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,Address,Data);
				while(status == HAL_BUSY);
				HAL_FLASH_Lock();
			}	
			return status;
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:FLASH_Erase
* Return				:
*******************************************************************************/
HAL_StatusTypeDef	FLASH_Erase(uint32_t sector, uint32_t n) {
FLASH_EraseInitTypeDef EraseInitStruct;
HAL_StatusTypeDef ret=HAL_OK;
uint32_t	SectorError;
			HAL_FLASH_Unlock();
			EraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
			EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;
			EraseInitStruct.Sector = sector;
			EraseInitStruct.NbSectors = n;
			ret=HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError);
			HAL_FLASH_Lock(); 
			return ret;
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
void	Send(int id,  payload *buf, int len) {
	CanTxMsg tx={{0,0,CAN_ID_STD,CAN_RTR_DATA,0,DISABLE},{0,0}};
	uint32_t mailbox;
	tx.hdr.StdId=id;
	tx.hdr.DLC=len;
	if(buf && len)
		memcpy(&tx.buf,buf,len);
	if(HAL_CAN_AddTxMessage(&hcan2, &tx.hdr, (uint8_t *)&tx.buf, &mailbox) != HAL_OK)
		_buffer_push(_CAN->tx,&tx,sizeof(CanTxMsg));
}
/*******************************************************************************
* Function Name  : CAN_Initialize
* Description    : Configures the CAN, transmit and receive using interrupt.
* Input          : None
* Output         : None
* Return         : PASSED if the reception is well done, FAILED in other case
*******************************************************************************/
void	canFilterCfg(int id1, int mask1, int id2, int mask2) {
CAN_FilterTypeDef  sFilterConfig;

  sFilterConfig.FilterBank = 14 + filter_count++;
  sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
  sFilterConfig.FilterScale = CAN_FILTERSCALE_16BIT;
	sFilterConfig.FilterIdHigh = id1<<5;
	sFilterConfig.FilterMaskIdHigh = mask1<<5;	
	sFilterConfig.FilterIdLow =  id2<<5;
	sFilterConfig.FilterMaskIdLow = mask2<<5;
  sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;
  sFilterConfig.FilterActivation = ENABLE;
  sFilterConfig.SlaveStartFilterBank = 0;
	HAL_CAN_ConfigFilter(&hcan2, &sFilterConfig) ;
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef* hcan) {
	CanRxMsg msg;
	HAL_CAN_GetRxMessage(hcan,CAN_RX_FIFO0,&msg.hdr,(uint8_t *)&msg.buf);
	_buffer_push(_CAN->rx,&msg,sizeof(CanRxMsg));
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
void HAL_CAN_RxFifo1MsgPendingCallback(CAN_HandleTypeDef* hcan) {
	HAL_CAN_RxFifo0MsgPendingCallback(hcan);
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
void HAL_CAN_TxMailbox2CompleteCallback(CAN_HandleTypeDef* hcan) {
	CanTxMsg	tx;
	uint32_t	mailbox;
	if(_buffer_pull(_CAN->tx,&tx,sizeof(CanTxMsg)))
		HAL_CAN_AddTxMessage(&hcan2, &tx.hdr, (uint8_t *)&tx.buf, &mailbox);
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
void HAL_CAN_TxMailbox0CompleteCallback(CAN_HandleTypeDef* hcan) {
	HAL_CAN_TxMailbox2CompleteCallback(hcan); 
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
void HAL_CAN_TxMailbox1CompleteCallback(CAN_HandleTypeDef* hcan) {
	HAL_CAN_TxMailbox2CompleteCallback(hcan); 
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
void	*canTx(void *v) {
	
	return canTx;
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
void	*canRx(void *v) {
	if(!_CAN) {
		_CAN	=	_io_init(100*sizeof(CanTxMsg), 100*sizeof(CanTxMsg));

		canFilterCfg(_ID_IAP_GO, 0x7f0, _ID_IAP_GO, 0x7f0);
		HAL_CAN_ActivateNotification(&hcan2,CAN_IT_RX_FIFO0_MSG_PENDING);
		HAL_CAN_ActivateNotification(&hcan2,CAN_IT_TX_MAILBOX_EMPTY);
		HAL_CAN_Start(&hcan2);
		
		py.word[1]=(uint32_t)HAL_CRC_Calculate(&hcrc,(uint32_t *)0x1FFF7A10,3);
	} else {
		CanRxMsg	rx;

		if(_buffer_pull(_CAN->rx,&rx,sizeof(CanRxMsg))) {
			boot_timeout=0;
			switch(rx.hdr.StdId) {
				
				case _ID_IAP_GO:
					HAL_NVIC_SystemReset();
					break;

				case _ID_IAP_ERASE:
					py.word[0]=FLASH_Erase(rx.buf.word[0],1);
					Send(_ID_IAP_ACK,&py,sizeof(payload));
					break;

				case _ID_IAP_ADDRESS:
					flash_address=rx.buf.word[0];
					break;

				case _ID_IAP_DWORD:
					if(py.word[0]==HAL_OK)
						py.word[0]=FLASH_Program(flash_address, rx.buf.word[0]);
					flash_address += sizeof(uint32_t);
					if(py.word[0]==HAL_OK)
						py.word[0]=FLASH_Program(flash_address, rx.buf.word[1]);
					flash_address += sizeof(uint32_t);
					Send(_ID_IAP_ACK,&py,sizeof(payload));
					break;

				case _ID_IAP_PING:
					Send(_ID_IAP_ACK,&py,sizeof(payload));
					break;
				
				default:
					break;
			}
		}
	}
	return canRx;
}
