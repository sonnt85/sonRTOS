/*
 * ringbuffer.c
 *
 *  Created on: Feb 5, 2015
 *      Author: NTS2201
 */
#include "ringbuffer.h"
#include "string.h"
#include "stdint.h"
void ringInit(ringBUF * ringbuf, void * data, uint8_t length)
{
   ringbuf->buffer = data;
   ringbuf->length = length;
   ringbuf->isbusy = 0;
   ringbuf->head = ringbuf->tail = ringbuf->count = 0;
}
uint8_t ringReset(ringBUF *ringbuf)
{
   if (ringbuf->isbusy) return RB_BUSY;
   ringbuf->isbusy = 1;
   ringbuf->head = ringbuf->tail = ringbuf->count = 0;
   ringbuf->isbusy = 0;
   return RB_SUCCESS;
}

int8_t ringInsertByte(ringBUF * ringbuf, uint8_t data)
{
   int8_t retval;
   if (ringbuf->isbusy) return RB_BUSY;
   ringbuf->isbusy = 1;
   retval = RB_FULL;
   if (ringbuf->count < ringbuf->length)
   {
      ringbuf->buffer[ringbuf->head] = data;
      ringbuf->count++;
      ringbuf->head = (ringbuf->head + 1) % (ringbuf->length);
      retval = RB_SUCCESS;
   }
   ringbuf->isbusy = 0;
   return retval;
}

int16_t ringInsertBytes(ringBUF * ringbuf, void * data, uint8_t length)
{
   uint8_t * ptr = data;
   uint8_t cnt;
   int16_t retval;
   if (ringbuf->isbusy) return RB_BUSY;
   ringbuf->isbusy = 1;
   if(ringbuf->count != ringbuf->length)
   {
      cnt = 0;

      while ((ringbuf->count < ringbuf->length) && (cnt < length))
      {
         ringbuf->buffer[ringbuf->head] = ptr[cnt];
         ringbuf->count++;
         cnt++;
         ringbuf->head = (ringbuf->head + 1) % (ringbuf->length);
      }
      retval = cnt;
   }else retval = RB_FULL;
   ringbuf->isbusy = 0;
   return retval;
}

int16_t ringInsertString(ringBUF * ringbuf, void * data)
{
   return ringInsertBytes(ringbuf, data, strlen(data));
}

int16_t ringRemoveByte(ringBUF * ringbuf)
{
   uint8_t temp;
   int16_t retval;
   if (ringbuf->isbusy) return RB_BUSY;
   ringbuf->isbusy = 1;
   retval = RB_EMPTY;
   if (ringbuf->count > 0)
   {
      temp = ringbuf->buffer[ringbuf->tail];
      ringbuf->count--;
      ringbuf->tail = (ringbuf->tail + 1) % (ringbuf->length);
      retval = temp;
   }
   ringbuf->isbusy = 0;
   return retval;
}

int16_t ringRemoveBytes(ringBUF * ringbuf, void * data, uint8_t maxbytes)
{
   uint8_t cnt;
   uint8_t * ptr = data;
   int16_t retval;
   if (ringbuf->isbusy) return RB_BUSY;
   ringbuf->isbusy = 1;
   cnt = 0;
   if(ringbuf->count)
   {
      while ((ringbuf->count > 0) && (cnt < maxbytes))
      {
         if (ptr) ptr[cnt] = ringbuf->buffer[ringbuf->tail];
         ringbuf->count--;
         cnt++;
         ringbuf->tail = (ringbuf->tail + 1) % (ringbuf->length);
      }
      retval = cnt;
   }else retval = RB_EMPTY;

   ringbuf->isbusy = 0;
   return retval;
}

