/*
 * uart.c
 *
 *  Created on: Nov 27, 2024
 *      Author: datph
 */

#include "uart.h"
#include "ring_buffer.h"

uint8_t receive_buffer1 = 0;
uint8_t msg[100];
RingBuffer buffer;
/* Functions */
uint8_t temp;
uint32_t mypow_2(uint8_t m, uint8_t n) {
	uint32_t result = 1;
	while (n--)
		result *= m;
	return result;
}

void uart_init_rs232() {
	HAL_UART_Receive_IT(&huart1, &receive_buffer1, 1);
}

void uart_Rs232SendString(uint8_t *str) {
	HAL_UART_Transmit(&huart1, (void*) msg, sprintf((void*) msg, "%s", str),
			10);
}

void uart_Rs232SendBytes(uint8_t *bytes, uint16_t size) {
	HAL_UART_Transmit(&huart1, bytes, size, 10);
}

void uart_Rs232SendNum(uint32_t num) {
	if (num == 0) {
		uart_Rs232SendString((void*)"0");
		return;
	}
	uint8_t num_flag = 0;
	int i;
	if (num < 0)
		uart_Rs232SendString((void*)"-");
	for (i = 10; i > 0; i--) {
		if ((num / mypow_2(10, i - 1)) != 0) {
			num_flag = 1;
			sprintf((void*) msg, "%ld", num / mypow_2(10, i - 1));
			uart_Rs232SendString(msg);
		} else {
			if (num_flag != 0)
				uart_Rs232SendString((void*)"0");
		}
		num %= mypow_2(10, i - 1);
	}
}

void uart_Rs232SendNumPercent(uint32_t num) {
	sprintf((void*) msg, "%ld", num / 100);
	uart_Rs232SendString(msg);
	uart_Rs232SendString((void*)".");
	sprintf((void*) msg, "%ld", num % 100);
	uart_Rs232SendString(msg);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
	if(huart->Instance == USART1){
		// rs232 isr
		// can be modified
//		HAL_UART_Transmit(&huart1, &receive_buffer1, 1, 10);


		// turn on the receice interrupt
		temp = receive_buffer1;
		addToRingBuffer(&buffer, receive_buffer1);
		HAL_UART_Receive_IT(&huart1, &receive_buffer1, 1);
	}
}
