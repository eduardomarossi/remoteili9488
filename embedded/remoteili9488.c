/*
 * remoteili9488.c
 *
 * Created: 22/05/2020 17:36:02
 *  Author: eduardomarossi
 */ 

#include "remoteili9488.h"
#include <string.h>


/** Queue for IRQ -> taskRx */
QueueHandle_t xQueueRx, xQueueMSG;
volatile uint32_t g_touch_x, g_touch_y, g_touch_event;


uint32_t ili9488_init(struct ili9488_opt_t *p_opt)
{
	return 0;
}

void mxt_init(struct mxt_device *device) {
}

int mxt_is_message_pending(struct mxt_device *device) {
	return g_touch_event;
}

void mxt_handler(struct mxt_device *device, uint *x, uint *y) {
	if(x != NULL && y != NULL) {
		*x = g_touch_x;
		*y = g_touch_y;
	}
	g_touch_event = 0;
}


void USART1_Handler(void) {
	uint32_t ret = usart_get_status(USART1);
	char c;

	if(ret & US_IER_RXRDY) {
		usart_serial_getchar(USART1, &c);
		xQueueSendFromISR(xQueueRx, &c, 0);
	}
}

void USART1_init(void) {
	/* Configura opcoes USART */
	const sam_usart_opt_t usart_settings = {
		.baudrate       = 115200,
		.char_length    = US_MR_CHRL_8_BIT,
		.parity_type    = US_MR_PAR_NO,
		.stop_bits   	= US_MR_NBSTOP_1_BIT	,
		.channel_mode   = US_MR_CHMODE_NORMAL
	};

	/* Enable the receiver and transmitter. */
	usart_enable_tx(USART1);
	usart_enable_rx(USART1);

	/* ativando interrupcao */
	NVIC_SetPriority(ID_USART1, 4);
	NVIC_EnableIRQ(ID_USART1);
	usart_enable_interrupt(USART1, US_IER_RXRDY);
	
}

void task_uartRx(void *pvParameters) {
	char rxMSG;
	char s[64];
	uint32_t i = 0;

	xQueueRx = xQueueCreate(32, sizeof(char));
	xQueueMSG = xQueueCreate(32, sizeof(char[64]) );
	
	if(xQueueRx == NULL ){
		printf("Fail to create xQueueRx \n");
		while(1){};
	}
	
	if(xQueueMSG == NULL ){
		printf("Fail to create xQueueMSG \n");
		while(1){};
	}

	while(1){
		if(xQueueReceive(xQueueRx, &rxMSG, ( TickType_t ) 500 )){
			if(rxMSG != '\n'){
				if(rxMSG != '#')
					s[i++] = rxMSG;
			} else{
				s[i++] = '\0';
				i = 0;
				xQueueSend(xQueueMSG, &s, 0);
			}
		}
	}
}

void task_Process(void *pvParameters) {
	char rxMSG[64];
	
	uint32_t x, y, d;
	
	while(1){
		if( xQueueReceive(xQueueMSG, &rxMSG, ( TickType_t ) 500 )){
				char *token = strtok(rxMSG, ",");
				if(token != NULL) {
					x = strtol(token, 0, 10);
					token = strtok(NULL, ",");
					if(token != NULL){
						y = strtol(token, 0, 10);
						token = strtok(NULL, ",");
						if(token != NULL) {
							d = strtol(token, 0, 10);
							g_touch_x = x;
							g_touch_y = y;
							g_touch_event = 1;
						}
					}
				}
		}
	}
}


void ili9488_set_foreground_color(uint32_t color) {
	printf("#ili9488_set_foreground_color:%u#\r\n", color);
}

void ili9488_draw_filled_circle(uint32_t ul_x, uint32_t ul_y, uint32_t ul_r) {
	printf("#ili9488_draw_filled_circle:%d,%d,%d#\r\n", ul_x, ul_y, ul_r);
}

void ili9488_draw_circle(uint32_t ul_x, uint32_t ul_y, uint32_t ul_r) {
	printf("#ili9488_draw_circle:%d,%d,%d#\r\n", ul_x, ul_y, ul_r);
}

void ili9488_draw_pixel(uint32_t ul_x, uint32_t ul_y) {
	printf("#ili9488_draw_pixel:%d,%d#\r\n", ul_x, ul_y);
}

void ili9488_draw_line(uint32_t ul_x, uint32_t ul_y) {
	printf("#ili9488_draw_line:%d,%d#\r\n", ul_x, ul_y);
}

void ili9488_draw_rectangle(uint32_t ul_x1, uint32_t ul_y1, uint32_t ul_x2, uint32_t ul_y2) {
	printf("#ili9488_draw_rectangle:%d,%d,%d,%d#\r\n", ul_x1, ul_y1, ul_x2, ul_y2);
}

void ili9488_draw_filled_rectangle(uint32_t ul_x1, uint32_t ul_y1, uint32_t ul_x2, uint32_t ul_y2) {
	printf("#ili9488_draw_filled_rectangle:%d,%d,%d,%d#\r\n", ul_x1, ul_y1, ul_x2, ul_y2);
}

void font_draw_text1(char *font, const char *text, int x, int y, int spacing) {
	printf("#font_draw_text:%s,%s,%d,%d,%d#\r\n", font, text, x, y, spacing);
}

void ili9488_draw_pixmap1(uint32_t ul_x, uint32_t ul_y, uint32_t ul_width, uint32_t ul_height, const char *p_ul_pixmap) {
	printf("#ili9488_draw_pixmap:%d,%d,%d,%d,%s#\r\n", ul_x, ul_y, ul_width, ul_height, p_ul_pixmap);
}

