
#pragma once

#ifndef FTL_H
#define FTL_H


#define			FLASH_SECTOR_SIZE				256
#define			SECTORS_PER_PAGE				16
#define			PAGE_SIZE						(FLASH_SECTOR_SIZE*SECTORS_PER_PAGE)	//4k

#if 0
#define			PAGE_BASE_ADDR				(4 * FLASH_SECTOR_SIZE*176)	//176K ~ 192K,0x2c000~0x30000, 172K, 0x2b000
#define			PAGE_NUM						50
#else
/*By default, 
 * 120k~124k, store flash log info, 0x1E000
 * 124k~252k, store debug info, 0x1F000~0x3F000*/
#define			PAGE_BASE_ADDR				(4 * FLASH_SECTOR_SIZE*124)	//124K ~ 252K,0x1F000~0x3F000
#define			PAGE_NUM						32

#endif

#define                 FTL_CONVERT_U32_U8(x)                (x)&0xFF, ((x)>>8)&0xFF, ((x)>>16)&0xFF, ((x)>>24)&0xFF  
#define                 FTL_CONVERT_U16_U8(x)                (x)&0xFF, ((x)>>8)&0xFF 

typedef struct
{
	unsigned char sesctor_state;
	unsigned char version_num;
	unsigned char lbn;
	unsigned char sector_offset;
}sector_header;

typedef enum{
	TFL_SUCCESS,
	TFL_REQUIRE_NEW_PAGE,
	TFL_NO_SPACE,
}ftl_status;

/*Enum to mask which section info need to printf*/
typedef enum {
	FP_APP_STATE_CHANGE  = BIT(0),
} flash_printf_app_e;


typedef enum {
	FP_LL_SEND_DATA = BIT(0),
	FP_LL_SEND_CMD = BIT(1),
	FP_LL_DROP_CMD_DATA = BIT(2),
	FP_LL_CONN_REQ = BIT(3),
	FP_LL_RECEIVE_DATA = BIT(4),
	FP_LL_RECEIVE_CMD = BIT(5),
	FP_LL_RECEIVE_DATA_CMD_DECRY_FAIL = BIT(6),
	FP_LL_BLE_CONN_RX = BIT(7),
	FP_LL_TERMINATE = BIT(8),
	FP_LL_CONN_PARA_UPDATE = BIT(9),
	FP_LL_CHANNEL_MAP_UPDATE = BIT(10),
	FP_LL_CHANNEL_DEBUG = BIT(11),
	FP_PHY_PACKET_TX_RX = BIT(12),
	FP_PHY_PACKET_TX_RX_IRQ = BIT(13),
	FP_LL_CONN_COUNTER = BIT(14),
	
} flash_printf_ll_phy_e;

typedef enum {
	FP_DRIVER_SUSPEND  = BIT(0),
} flash_printf_driver_e;

/*Printf Log Flag*/
#define PFLAG_APP_STATE_ADV                              0x20, 0x20
#define PFLAG_APP_STATE_CONNECT                     0x2a, 0x30
#define PFLAG_APP_STATE_TERMINATE                  0x3a, 0x40

#define PFLAG_LL_SEND_DATA                             0x36, 0x30
#define PFLAG_LL_SEND_CMD                               0x38, 0x30
#define PFLAG_LL_TX_BUF_FULL_DATA_LOST                         0x37, 0x30

#define PFLAG_LL_CONN_REQ                      0x29, 0x30
#define PFLAG_LL_CONN_REQ_END            0x30, 0x30 

#define PFLAG_LL_RECEIVE_DATA                      0x35, 0x30
#define PFLAG_LL_RECEIVE_CMD                       0x41, 0x30
#define PFLAG_LL_RECEIVE_DECRY_FAIL          0x04, 0x30

#define PFLAG_LL_CONN_RX_TIME_1                     0x3B, 0x30
#define PFLAG_LL_CONN_RX_TIME_2                     0x3C, 0x30

#define 	PFLAG_LL_SLAVE_END    	                0x40, 0x40
#define 	PFLAG_LL_SEND_TERMINATE    	        0x39, 0x40
#define 	PFLAG_LL_RECEIVE_TERMINATE    	0x38, 0x40

#define 	PFLAG_LL_CONN_PARA_UPDATE_SUCCESS    	       0x31, 0x30
#define 	PFLAG_LL_RECEIVE_CONN_PARA_UPDATE_CMD    	0x01, 0x30

#define 	PFLAG_LL_CHANNEL_UPDATE_UPDATE_SUCCESS    	      0x30, 0x32
#define 	PFLAG_LL_RECEIVE_CHANNEL_MAP_UPDATE_CMD             0x02, 0x30	

#define  PFLAG_LL_PACKET_CRC_ERROR           0x38,0x30 
#define PFLAG_LL_PACKET_LEN_ERROR             0x39, 0x30 
#define PFLAG_LL_INTERVAL_END_FIRST_TIMEOUT    	         0x56, 0x30 
#define  PFLAG_LL_INTERVAL_END_TIMEOUT                           0x57, 0x30
#define  PFLAG_LL_INTERVAL_END_CRC2_ERROR                  0x58, 0x30

#define PFLAG_LL_IRQ_FIRST_TIMEOUT    	         0x53, 0x30 
#define  PFLAG_LL_IRQ_TIMEOUT                           0x54, 0x30
#define  PFLAG_LL_IRQ_CRC2_ERROR                  0x55, 0x30

#define PFLAG_LL_ON_INTERVAL_END_COUNTER  0x34, 0x30

#define PFLAG_DRIVER_REJECT_ENTER_SUSPEND_BY_APP  0x03, 0x30
#define PFLAG_DRIVER_EXIT_SUSPEND                                       0x3A, 0x30

///////////////////////////////
// FTL public functions
///////////////////////////////
ftl_status ftl_write(char *data, int len);
u8 flash_putc(unsigned char pdata);
#endif //FTL_H
