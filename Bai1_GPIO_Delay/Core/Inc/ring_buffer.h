/*
 * ring_buffer.h
 *
 *  Created on: Nov 27, 2024
 *      Author: datph
 */

#ifndef INC_RING_BUFFER_H_
#define INC_RING_BUFFER_H_

#define BUFFER_SIZE 10

typedef struct {
  int data[BUFFER_SIZE]; // Mảng dữ liệu
  int head; // Vị trí ghi
  int tail; // Vị trí đọc
} RingBuffer;


void initRingBuffer(RingBuffer *buffer);
int isRingBufferEmpty(RingBuffer *buffer);
int isRingBufferFull(RingBuffer *buffer);
void addToRingBuffer(RingBuffer *buffer, int data);
int getFromRingBuffer(RingBuffer *buffer);

#endif /* INC_RING_BUFFER_H_ */
