#include "../../proj/tl_common.h"
#include "ftl.h"
//unsigned char cache_buffer[128] = {0};
//unsigned short ftl_rpt = 0;
//unsigned short ftl_wpt = 0;
//----------------------------------
// metadata structure
//----------------------------------


#define		PAGE_STATE_FULL_MASK		BIT(0)
#define		SECTOR_STATE_FULL_MASK		BIT(1)

#define		get_lpn(p)					(p / SECTORS_PER_PAGE)
#define		get_sector(p)					(p % SECTORS_PER_PAGE)

/*Flag to mask which section info need to printf*/
/*Used for FLASH_PRINTF_MODE*/
extern u32 flash_printf_ll_phy_mask;
extern u32 flash_printf_driver_mask;
extern u32 flash_printf_app_mask;

unsigned int lbn_pbn_map[PAGE_NUM] = {0};


unsigned short cur_write_lbn = 0;	// < PAGE_NUM*SECTORS_PER_PAGE
unsigned char current_sector = 0;
unsigned char version_no = 0;
u16	write_pointer_offset = 0;

volatile u16 record_page_info_index = 0;


ftl_status ftl_update_record_info(u32 addr) ;
static ftl_status reload_ftl_ib(void) {
	unsigned char i = 0, j = 0;
	unsigned int pv = 0;

	while(1) {
		pv = *((unsigned int *)lbn_pbn_map[i]);

		if(pv & PAGE_STATE_FULL_MASK) {

			unsigned int current_page_addr = lbn_pbn_map[i];

			//u8 sector_tem = 0;
			for(j = 0; j < SECTORS_PER_PAGE; j++) {
				pv = *((unsigned int *)((current_page_addr + j * FLASH_SECTOR_SIZE)));

				if(pv & SECTOR_STATE_FULL_MASK) {
					current_sector = j;
					break;
				}
				else {
					version_no = pv >> 8;
				}
			}

			if(j < SECTORS_PER_PAGE) {
				cur_write_lbn = i * SECTORS_PER_PAGE + j;
				return TFL_SUCCESS;
			}
		}

		i++;

		if(i == PAGE_NUM) {
			return TFL_NO_SPACE;
		}
	}
}

static u8 ftl_select_erase_page(void) {
	u8 value_f;
	u8 version_temp = 0xff, index = 0;

	for(u8 i = 0; i < PAGE_NUM; i++) {
		value_f = ((*((unsigned int *)lbn_pbn_map[i])) >> 8);

		if(value_f < version_temp) {
			version_temp = value_f;
			index = i;
		}
	}

	return index;
}

static void ftl_erase_4k(u32 addr) {
	flash_erase_sector(addr);
}
static void set_sector_state(u8 state, u16 lbn) {
	u8 new_value = 0;
	u8 offset_true = 1;

	if(state & PAGE_STATE_FULL_MASK) {
		new_value = 0xfe;
		offset_true = 0;
	}

	else if(state & SECTOR_STATE_FULL_MASK) {
		new_value = 0xfD;
	}

	flash_write_page(lbn_pbn_map[get_lpn(lbn)] + get_sector(lbn)*FLASH_SECTOR_SIZE * offset_true, 1, &new_value);
}
static void ftl_add_header(u16 lbn) {
	sector_header sh;
	sh.lbn = lbn;
	sh.sector_offset = get_sector(lbn);
	sh.sesctor_state = (~SECTOR_STATE_FULL_MASK) & 0xff;
	sh.version_num = ++version_no;
	flash_write_page(lbn_pbn_map[get_lpn(lbn)] + sh.sector_offset * FLASH_SECTOR_SIZE, sizeof(sector_header), (u8 *)(&sh));
}

void ftl_init(void) {

	for(u8 i = 0; i < PAGE_NUM; i++) {
		lbn_pbn_map[i] = PAGE_BASE_ADDR + i * PAGE_SIZE;
	}

	write_pointer_offset = sizeof(sector_header);

	if(reload_ftl_ib() == TFL_NO_SPACE) {
		u8 i = ftl_select_erase_page();
		ftl_erase_4k(lbn_pbn_map[i]);
		cur_write_lbn = i * SECTORS_PER_PAGE;
		current_sector = 0;
	}

	ftl_add_header(cur_write_lbn);

	/*Record current page info*/
	//test Eason
	u32 page_index = 0xFFFFFFFF;
	u32 page_info_read = 0xFFFFFFFF;
#if 1

	for(int i = 0; i < PAGE_SIZE; i += 4) {
		flash_read_page(PAGE_BASE_ADDR - PAGE_SIZE + i, 4, &page_index);

		if(page_index == 0xFFFFFFFF) {
			record_page_info_index = i;
			break;
		}

	}

#else

	for(int i = 0; i < PAGE_SIZE; i += 4) {
		flash_read_page(PAGE_BASE_ADDR - PAGE_SIZE + i, 4, &page_index);

		if(page_index == 0xFFFFFFFF) {
			record_page_info_index = i;

			if(i > 0) {
				flash_read_page(PAGE_BASE_ADDR - PAGE_SIZE + i - 4, 4, &page_info_read);
				ftl_erase_4k(PAGE_BASE_ADDR - PAGE_SIZE);
				flash_write_page(PAGE_BASE_ADDR - PAGE_SIZE, 4, &page_info_read);
				record_page_info_index = 4;

			}

			else {
				ftl_erase_4k(PAGE_BASE_ADDR - PAGE_SIZE);
				flash_write_page(PAGE_BASE_ADDR - PAGE_SIZE, 4, &page_info_read);
				record_page_info_index = 0;

			}

			break;

		}



	}

#endif

	/*Used for FLASH_PRINTF_MODE*/
	/*For customer, this value can be read from flash, so that product can turn to debug mode
	 * just by writing one byte in flash*/
#if 0
	flash_printf_ll_phy_mask = 0;
	flash_printf_driver_mask = 0;
	flash_printf_app_mask = 0;
#else
	flash_printf_ll_phy_mask = (
	                               // FP_LL_SEND_DATA |
	                               //FP_LL_SEND_CMD |
	                               //FP_LL_DROP_CMD_DATA |
	                               //FP_LL_CONN_REQ |
	                               //  FP_LL_RECEIVE_DATA |
	                               // FP_LL_RECEIVE_CMD |
	                               //FP_LL_RECEIVE_DATA_CMD_DECRY_FAIL |
	                               //FP_LL_BLE_CONN_RX |
	                               //FP_LL_TERMINATE |
	                               //FP_LL_CONN_PARA_UPDATE |
	                               //FP_LL_CHANNEL_MAP_UPDATE |
	                               //FP_LL_CHANNEL_DEBUG |
	                               FP_PHY_PACKET_TX_RX |0
	                               //FP_PHY_PACKET_TX_RX_IRQ |
	                               //FP_LL_CONN_COUNTER
	                           );

	//flash_printf_driver_mask = FP_DRIVER_SUSPEND;
	//flash_printf_app_mask =FP_APP_STATE_CHANGE;
#endif


}

//u8 cache_fifo[128] = {0};
//u8 fifo_wpt = 0;
//u8 fifo_rpt = 0;
void flash_print_init(void) {
/*For debug mode*/
#if FLASH_PRINTF_MODE
	ftl_init();

/*For production mode*/
#else

#endif
}

u8 flash_putc(unsigned char pdata) {
	//	cache_fifo[fifo_wpt++ & 0x7f] = pdata;
	//	return pdata;
}
/*
u32 tick0 = 0;
void ftl_task(void ){
	if((fifo_wpt == fifo_rpt)||(clock_time() - tick0 < 500*CLOCK_SYS_CLOCK_1MS)){
		return;
	}
	tick0 = clock_time();
	u8 c1 = fifo_wpt&0x7f;
	u8 c2 = fifo_rpt&0x7f;

	if(c1>c2){
		ftl_write(c1-c2,&cache_fifo[c2]);
	}
	else if(c2>c1){
		ftl_write(127-c2,&cache_fifo[c2]);
		ftl_write(c1,&cache_fifo[0]);
	}
	fifo_rpt = fifo_wpt;
}
/****************************************************************************
 * @brief	FTL write API
 *
 * @param	lbn: logic block number
 *			len: write byte length
 *			data: data address
 * @return	none
 */
ftl_status ftl_write(char *data, int len) {
/*For debug mode*/
#if FLASH_PRINTF_MODE
	unsigned char send_len;
	unsigned char lpn, sect_offset;


	lpn          = get_lpn(cur_write_lbn);	//get logic page number
	sect_offset  = get_sector(cur_write_lbn);	//get sector offset

	u32 addr = lbn_pbn_map[lpn] + sect_offset * FLASH_SECTOR_SIZE + write_pointer_offset;	//write pointer start address

	if(write_pointer_offset + len < FLASH_SECTOR_SIZE) {
		write_pointer_offset += len;
		flash_write_page(addr, len, data);
		/*Record current page info, don't need to record the same sector info*/
		//ftl_update_record_info(addr);

		//printf("flash write success1,len %d, write pointer offset %d\n",len,write_pointer_offset);
		return TFL_SUCCESS;
	}

	send_len = FLASH_SECTOR_SIZE - write_pointer_offset;
	// single page write individually
	flash_write_page(addr, send_len, data);
	set_sector_state(SECTOR_STATE_FULL_MASK, cur_write_lbn);
	//printf("flash write success2,len %d, write pointer offset %d\n",send_len,write_pointer_offset);
	current_sector++;

	if(current_sector < SECTORS_PER_PAGE) {
		write_pointer_offset = sizeof(sector_header);
		addr = lbn_pbn_map[lpn] + (sect_offset + 1) * FLASH_SECTOR_SIZE + write_pointer_offset;
		cur_write_lbn++;
		ftl_add_header(cur_write_lbn);

		write_pointer_offset += (len - send_len);
		//printf("flash write3,len %d, write pointer offset %d\n",len-send_len,write_pointer_offset);
		flash_write_page(addr, len - send_len, data + send_len);

		/*Record current page info, don't need to record the same PAGE info*/
		//ftl_update_record_info(addr);


		return TFL_SUCCESS;
	}
	//different page
	else {
		current_sector = 0;
		set_sector_state(PAGE_STATE_FULL_MASK, cur_write_lbn);
		cur_write_lbn++;
		lpn++;

		if(lpn == PAGE_NUM) {
			lpn = 0;
			cur_write_lbn = 0;
		}

		//lpn = (lpn==PAGE_NUM)?0:(lpn+1);
		//	cur_write_lbn = (cur_write_lbn==PAGE_NUM*SECTORS_PER_PAGE)?0:(cur_write_lbn+1);
		unsigned int pv	 = *((unsigned int *)lbn_pbn_map[lpn]);

		if((pv & PAGE_STATE_FULL_MASK) == 0) {
			ftl_erase_4k(lbn_pbn_map[lpn]);
		}

		write_pointer_offset = sizeof(sector_header);
		addr = lbn_pbn_map[lpn] + write_pointer_offset;
		ftl_add_header(cur_write_lbn);
		flash_write_page(addr, len - send_len, data + send_len);
		write_pointer_offset += (len - send_len);

		/*Record current page info*/
		ftl_update_record_info(addr);
		return TFL_SUCCESS;
	}

	/*For production mode*/
#else
	return TFL_SUCCESS;
#endif
}


/****************************************************************************
 * @brief	FTL read API
 *
 * @param	lbn: logic block numcur_write_lbn
 * 			offset: read pointer offset value
 *			len: write byte length
 *			buff: buffer address to hold read data
 * @return	none
 */
ftl_status ftl_read(unsigned char offset, unsigned char len, unsigned char *buff) {
	u32 addr = lbn_pbn_map[get_lpn(cur_write_lbn)] + get_sector(cur_write_lbn) * FLASH_SECTOR_SIZE + offset;
	flash_read_page(addr, len, buff);
	return TFL_SUCCESS;
}

/****************************************************************************
 * @brief	FTL read API
 *
 * @param	addr: current write process flash addr
 *
 * @return	none
 */
ftl_status ftl_update_record_info(u32 addr) {

	/*Record current page info*/
	if((PAGE_BASE_ADDR - PAGE_SIZE + record_page_info_index + 4) > PAGE_BASE_ADDR) {
		ftl_erase_4k(PAGE_BASE_ADDR - PAGE_SIZE);
		record_page_info_index = 0;
	}

	flash_write_page(PAGE_BASE_ADDR - PAGE_SIZE + record_page_info_index, 4, &addr);
	record_page_info_index += 4;

	return TFL_SUCCESS;
}
#if 0
volatile u8 AAtest1, AAtest2, AAtest3;
u8 test_write[32] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 0x0a, 0x0e};
void ftl_test_write(void) {
	//printf("ftl test start\n");
	//
	//printf("ftl init done\n");
	u8 cache_buffer[32] = {0};

	for(u8 i = 0; i < 12; i++) {
		AAtest1++;

		for(u8 j = 0; j < 8; j++) {
			AAtest2++;
			ftl_write(cur_write_lbn, 32, test_write);

			if(write_pointer_offset > 32) {
				ftl_read(cur_write_lbn, write_pointer_offset - 32, 32, cache_buffer);
			}
			else {
				ftl_read(cur_write_lbn - 1, FLASH_SECTOR_SIZE - 32 + write_pointer_offset, 32, cache_buffer);
			}

			printf("read data:");

			for(u8 j = 0; j < 32; j++) {
				printf(" %d/", cache_buffer[j]);
			}

			printf("\n*****************************\n");
			sleep_us(100000);
		}
	}

	/*
	printf("loop success\n");
	u8 cache_buffer[32] = {0};
	for(u8 i=0;i<64;i++){
		AAtest3++;
		ftl_read(i,4,32,cache_buffer);
		printf("read data:");
		for(u8 j=0;j<32;j++){
			printf(" %d/",cache_buffer[j]);
		}
		printf("\n*****************************\n");
	}*/
}

#endif
