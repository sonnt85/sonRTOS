#include "HIDSerial.h"

#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>  /* for sei() */
#include <util/delay.h>     /* for _delay_ms() */
#include <avr/eeprom.h>
#include <avr/pgmspace.h>   /* required by usbdrv.h */
#include "usbdrv.h"

#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <stdlib.h>

#define HIDSERIAL_OTBUFFER_SIZE 8
static uchar received = 0;
static uchar outBuffer[HIDSERIAL_OTBUFFER_SIZE];
static uchar inBuffer[HIDSERIAL_INBUFFER_SIZE];
static uchar reportId = 0;
static uchar bytesRemaining;
static uchar* pos;

PROGMEM const char usbHidReportDescriptor[USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH] =
      { /* USB report descriptor */
      0x06, 0x00, 0xff,              // USAGE_PAGE (Generic Desktop)
            0x09, 0x01,                    // USAGE (Vendor Usage 1)
            0xa1, 0x01,                    // COLLECTION (Application)
            0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
            0x26, 0xff, 0x00,              //   LOGICAL_MAXIMUM (255)
            0x75, 0x08,                    //   REPORT_SIZE (8)
            0x95, 0x08,                    //   REPORT_COUNT (8)
            0x09, 0x00,                    //   USAGE (Undefined)
            0x82, 0x02, 0x01,              //   INPUT (Data,Var,Abs,Buf)
            0x95, HIDSERIAL_INBUFFER_SIZE, //   REPORT_COUNT (32)
            0x09, 0x00,                    //   USAGE (Undefined)
            0xb2, 0x02, 0x01,              //   FEATURE (Data,Var,Abs,Buf)
            0xc0                           // END_COLLECTION
      };

/* usbFunctionRead() is called when the host requests a chunk of data from
 * the device. For more information see the documentation in usbdrv/usbdrv.h.
 */
uchar usbFunctionRead(uchar *data, uchar len)
{
   return 0;
}

/* usbFunctionWrite() is called when the host sends a chunk of data to the
 * device. For more information see the documentation in usbdrv/usbdrv.h.
 */
uchar usbFunctionWrite(uchar *data, uchar len)
{
   if (reportId == 0)
   {
      int i;
      if (len > bytesRemaining) len = bytesRemaining;
      bytesRemaining -= len;
      //int start = (pos==inBuffer)?1:0;
      for (i = 0; i < len; i++)
      {
         if (data[i] != 0)
         {
            *pos++ = data[i];
         }
      }
      if (bytesRemaining == 0)
      {
         received = 1;
         *pos++ = 0;
         return 1;
      } else
      {
         return 0;
      }
   } else
   {
      return 1;
   }
}

usbMsgLen_t usbFunctionSetup(uchar data[8])
{
   usbRequest_t *rq = (usbRequest_t *) data;
   reportId = rq->wValue.bytes[0];
   if ((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS)
   { /* HID class request */
      if (rq->bRequest == USBRQ_HID_GET_REPORT)
      {
         /* wValue: ReportType (highbyte), ReportID (lowbyte) */
         /* since we have only one report type, we can ignore the report-ID */
         return USB_NO_MSG; /* use usbFunctionRead() to obtain data */
      } else if (rq->bRequest == USBRQ_HID_SET_REPORT)
      {
         /* since we have only one report type, we can ignore the report-ID */
         pos = inBuffer;
         bytesRemaining = rq->wLength.word;
         if (bytesRemaining > sizeof(inBuffer)) bytesRemaining =
               sizeof(inBuffer);
         return USB_NO_MSG; /* use usbFunctionWrite() to receive data from host */
      }
   } else
   {
      /* ignore vendor type requests, we don't use any */
   }
   return 0;
}

int8_t waitUsbInterruptReady(uint16_t times)
{
   while (!usbInterruptIsReady())
   {
      times--;
      if(!times)return -1;
      usbPoll();
   }
   return 0;
}

uchar hidAvailable()
{
   return received;
}

unsigned char hidRead(unsigned char *buffer)
{
   if (received == 0) return 0;
   int i;
   for (i = 0; inBuffer[i] != 0 && i < HIDSERIAL_INBUFFER_SIZE; i++)
   {
      buffer[i] = inBuffer[i];
   }
   inBuffer[0] = 0;
   buffer[i] = 0;
   received = 0;
   return i;
}

// hidWrite one character
unsigned char hidWrite(unsigned char data)
{
   if(waitUsbInterruptReady(1000)) return 0;
   memset(outBuffer, 0, sizeof(outBuffer));
   outBuffer[0] = data;
   usbSetInterrupt(outBuffer, sizeof(outBuffer));
   return 1;
}

// hidWrite up to 8 characters
unsigned char write8(const unsigned char *buffer, unsigned char size)
{
   unsigned char i;
   if(waitUsbInterruptReady(1000)) return 0;
   memset(outBuffer, 0, sizeof(outBuffer));
   for (i = 0; i < size && i < sizeof(outBuffer); i++)
   {
      outBuffer[i] = buffer[i];
   }
   usbSetInterrupt(outBuffer, sizeof(outBuffer));
   return (i);
}

// hidWrite a string
unsigned char hidWrites(const void *buffer, unsigned char size)
{
   size_t count = 0;
   unsigned char i;
   for (i = 0; i < (size / (unsigned char)sizeof(outBuffer)) + 1; i++)
   {
      count += write8((uint8_t *)buffer + i * (unsigned char)sizeof(outBuffer),(size < (count + (unsigned char)sizeof(outBuffer))) ? (size - count) : (unsigned char)sizeof(outBuffer));
   }
   return count;
}

unsigned char write8PGM(const void *address, unsigned char size)
{
   unsigned char i;
   if(waitUsbInterruptReady(1000)) return 0;
   memset(outBuffer, 0, sizeof(outBuffer));
   for (i = 0; i < size && i < sizeof(outBuffer); i++)
   {
      outBuffer[i] = pgm_read_byte((uint8_t * )address + i);
   }
   usbSetInterrupt(outBuffer, sizeof(outBuffer));
   return (i);
}

unsigned char hidWritespgm(void *address, unsigned char size)
{
   size_t count = 0;
   unsigned char i;
   for (i = 0; i < (size / (unsigned char)sizeof(outBuffer)) + 1; i++)
   {
      count += write8PGM((uint8_t *) address + i * (unsigned char)sizeof(outBuffer),
            (size < (count + (unsigned char)sizeof(outBuffer))) ? (size - count) : (unsigned char)sizeof(outBuffer));
   }
   return count;
}

unsigned char hidWriteString(void * buff)
{
   return hidWrites(buff, strlen(buff));
}

unsigned char hidWriteUint16(uint16_t data, uint8_t radix)
{
   char arr[17];
   return hidWriteString(utoa(data, arr, radix));
}

//unsigned char hidWriteSint16(int16_t data, uint8_t radix)
//{
//   char arr[17];
//   return hidWriteString(itoa(data, arr, radix));
//}

//unsigned char hidWriteUint32(uint32_t data, uint8_t radix)
//{
//   char arr[33];
//   return hidWriteString(ultoa(data, arr, radix));
//}
//
//unsigned char hidWriteSint32(int32_t data, uint8_t radix)
//{
//   char arr[33];
//   return hidWriteString(ltoa(data, arr, radix));
//}

void hidBegin()
{
   uchar i;
   cli();
   usbDeviceDisconnect();
   i = 0;
   while (--i)
   { /* fake USB disconnect for > 250 ms */
      _delay_ms(1);
   }
   usbDeviceConnect();
   usbInit();
   sei();

   received = 0;
}
