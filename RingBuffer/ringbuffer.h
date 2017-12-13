/*
 * ringbuffer.h
 *
 *  Created on: Feb 5, 2015
 *      Author: NTS2201
 */

#ifndef RINGBUFFER_H_
#define RINGBUFFER_H_

#include <avr/pgmspace.h>
typedef struct {
   uint8_t * buffer;
   uint8_t isbusy : 1;
   uint8_t length : 7 ;
   uint8_t head;
   uint8_t tail;
   uint8_t count;
} ringBUF;
enum{RB_BUSY = -1,RB_FULL = -3, RB_EMPTY = -2, RB_SUCCESS = 0};
void ringInit(ringBUF * ringbuf, void * data, uint8_t length);
int16_t ringInsertBytes(ringBUF * ringbuf, void * data, uint8_t length);
int16_t ringInsertString(ringBUF * ringbuf, void * data);
int16_t ringRemoveByte(ringBUF * ringbuf);
int16_t ringRemoveBytes(ringBUF * ringbuf, void * data, uint8_t maxbytes);
uint8_t ringReset(ringBUF *ringbuf);

#endif /* RINGBUFFER_H_ */
