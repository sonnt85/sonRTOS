// ******************************************************************
//
// File Name:     main.c
// Author:        based on work done by Glen Worstell
// Created:       Nguyen Thanh Son
// Revised:       thanhson.rf@gmail.com
// Date revised:  3/1/2015
// Version:       1.0
// Target MCU:    All AVR
//
// The only changes required for your own task setup are:
// - the task number enum (in sonRTOS.h)
// - the semaphore enum (in sonRTOS.h) and
// - the const u16 startAdr[] array below
// If wanting up to 16 tasks see notes at beginning of csRTOS.c
// ******************************************************************

#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <string.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include "ringbuffer.h"

#include "sonRTOS.h"
#include "HIDserial.h"
#include <stdlib.h>
#include "main.h"
#include "gpio.h"

#define MAX_ARGS 5
#define WHITE_SPACE "\t\n\r "
#define LED0 PINC00
#define LED1 PINC01
u08 usbRXBuffer[HIDSERIAL_INBUFFER_SIZE], uartRXBuffer[64] ;
u08 usbTXBuffer[32], uartTXBuffer[64];
ringBUF usbtx, usbrx, uarttx, uartrx;
struct {
	u08 AUTOSHOWTIME : 1;
	u08 UART2HID : 1;
	u08 BLINKLED : 1;
}sys_status;

DATE_T EEMEM date_e;

void blinkTask(void);
void clockTask(void);
void adcTask(void);
void usbTask(void);

const u16 ten2nArray[] PROGMEM = { 0, 1, 10, 100, 1000, 10000 };

#if   defined(__AVR_3_BYTE_PC__) && __AVR_3_BYTE_PC__
typedef u32 typedefPC;
#else
typedef u16 typedefPC;
#endif

const u08 dayspermonth[12] PROGMEM = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
u16 nextAddTask;

void main(void) __attribute__ ((naked));
void main(void)
{
   SP = RAMEND - sizeof(port_jmp_buf[0]) - 20;
   Init_SystemClock1MS();

   pinMode(PIND00,INPUT_PULLUP);
   pinMode(PIND01,OUTPUT_LOW);
   pinMode(LED0, OUTPUT_HIGH);
   pinMode(LED1, OUTPUT_HIGH);


   UBRRL = (F_CPU / (16UL * 9600)) - 1; // (fCPUosc / 16Baud) - 1, for 12MHz crystal
   UBRRH = ((F_CPU / (16UL * 9600)) - 1) >> 8;
   UCSRB |= (1 << TXCIE) | (1 << TXEN) | (1 << RXCIE) | (1 << RXEN);

   // ADC initialisation
   ADMUX |= (1 << REFS1) | (1 << REFS0) | 2; // select on chip 2.56V ref, chanel 2 PC2
   // The internal voltage reference options may not be used if an external
   // reference voltage is being applied to the AREF pin.

   // enable ADC & ints, clock / 64 = 62.5 KHz
   //  ADCSRA |= (1<<ADEN) | (1<<ADIE) | (1<<ADPS2) | (1<<ADPS1);//turn on interrupt
   ADCSRA |= (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1);   //off interrupt
   osINIT_TASK((u16) blinkTask, 100, blinkTaskNumber);
   osINIT_TASK((u16) clockTask, 100, clockTaskNumber);
   osINIT_TASK((u16) adcTask, 100, adcTaskNumber);
   osINIT_TASK((u16) usbTask, 250, usbTaskNumber);
   // set some tasks ready to run
   osSetRTR(usbTaskNumber);
   osSetRTR(blinkTaskNumber);
   osSetRTR(adcTaskNumber);
   osSetRTR(clockTaskNumber);
   eeprom_read_block(&date_r, &date_e, sizeof(date_r));
   if((secs > 59) || (mins > 59) || (hrs > 23))
   {
      memset(&date_r, 0, sizeof(date_r));
      eeprom_write_block(&date_r, &date_e, sizeof(date_r));
   }
   ringInit(&uartrx, uartRXBuffer, sizeof(uartRXBuffer));
   ringInit(&uarttx, uartTXBuffer, sizeof(uartTXBuffer));
   ringInit(&usbtx, usbTXBuffer, sizeof(usbTXBuffer));
   ringInit(&usbrx, usbRXBuffer, sizeof(usbRXBuffer));
   sys_status.AUTOSHOWTIME = 1;
   sys_status.BLINKLED = 1;
   sys_status.UART2HID = 1;
   hidBegin();
   asm("sei");
   __schedule(); // start multitasking.
}

u08 execCMD(char * c, char ** args)
{
//   hidWriteString(c);
   if (!strcasecmp_P(c, PSTR("reset")))
   {
      wdt_enable(1);
   } else if (!strcasecmp_P(c, PSTR("blinkled")))
   {
      if (!strcasecmp_P(args[0], PSTR("on"))) sys_status.BLINKLED = 1;
      else if (!strcasecmp_P(args[0], PSTR("off"))) sys_status.BLINKLED = 0;
   } else if (!strcasecmp_P(c, PSTR("showtime")))
   {
      if (!strcasecmp_P(args[0], PSTR("on"))) sys_status.AUTOSHOWTIME = 1;
      else if (!strcasecmp_P(args[0], PSTR("off"))) sys_status.AUTOSHOWTIME = 0;
   }
   return 0;
}

u08 processInput(const char * b)
{
   // make a quick copy in case the ring buffer gets more data
   char buff[HIDSERIAL_INBUFFER_SIZE];
   int idx = 0;
   memcpy(buff, b, HIDSERIAL_INBUFFER_SIZE);
   char *cmd, *ptr, *args[MAX_ARGS];
   if (strtok(buff, WHITE_SPACE) == NULL) return 1;
   cmd = buff;

   while ((ptr = strtok(NULL, WHITE_SPACE)) != NULL)
   {
      args[idx] = ptr;
      if (++idx == (MAX_ARGS - 1)) break;
   }
   args[idx] = NULL;

   return execCMD(cmd, args);
}

void blinkTask(void)
{
   while (1)
   {
      if (sys_status.BLINKLED)
      {
        digitalToggle(LED0);
      } else digitalWrite(LED0, HIGH);
      osWaitMs(500); // Wait about 1s
   }
}

void clockTask(void)
{

   while (1)
   {
      u08 flag_leap_year, max_day_in_current_month, temp_var1, temp_var2;
//      secs += 1;

      if (secs > 59)
      {
         mins += (secs / 60);
         secs %= 60;
         eeprom_write_block(&date_r, &date_e, sizeof(date_r));
         if (mins > 59)
         {
            hrs += 1;
            mins -= 60;
            if (hrs > 23)
            {
               hrs -= 24;
               flag_leap_year = ((month == 2)
                     && ((((year) % 400) == 0)
                           || ((((year) % 4) == 0) && ((year) % 100 != 0))));
               if (flag_leap_year)
               {
                  max_day_in_current_month = 29;
               } else
               {
                  max_day_in_current_month = pgm_read_word((char* )&(dayspermonth[month - 1]));
               }
               if (day > max_day_in_current_month)
               {
                  day = 1;
                  month++;
                  if (month == 13)
                  {
                     year++;
                     month = 1;
                  }
               }
               //caculate day
               temp_var2 = 0;
               for (temp_var1 = 0; temp_var1 < (month - 1); temp_var1++)
               {
                  temp_var2 = temp_var2 + pgm_read_word((char* )&(dayspermonth[temp_var1]));
               }
               if (flag_leap_year)
               {
                  temp_var2++;
               };
               day = ((year - 1) + (year - 1) / 4 - (year - 1) / 100
                     + (year - 1) / 400 + day + temp_var2) % 7 + 1;
            }
         }
      }
     if(!sys_status.AUTOSHOWTIME)
     {
         hidWriteStringPGM("Time System - ");
         hidWriteUint16(hrs, 10);
         hidWrite(':');

         hidWriteUint16(mins, 10);
         hidWrite(':');

         hidWriteUint16(secs, 10);
         hidWrite('\n');
     }
     digitalToggle(LED1);
     osWaitMs(1000);
   }
}

void adcTask(void)
{
   u16 result, pre_result; // variables used across os calls need to be static
   result = pre_result = 0;
   while (1)
   {
      osWaitTicks(10); // wait approx 100ms, ie no ADC conversions
      result = ADCW;
      ADCSRA |= (1 << ADSC); // start ADC conversion
      if (pre_result != result)
      {
         pre_result = result;
      }
   }
}

void usbTask()
{
   u08 size;

   while (1)
   {
      if (hidAvailable())
      {
         size = hidRead(usbRXBuffer);
         if (size != 0)
         {
//            hidWrites((const uint8_t*) usbRXBuffer, size);
//            hidWrite('\n');
            ringInsertBytes(&uarttx, usbRXBuffer, size);
            ringInsertByte(&uarttx, '\n');
            UDR = ringRemoveByte(&uarttx);//transmit fist byte
            processInput(usbRXBuffer);
         }
      }
      if (uartrx.count)
      {

         while (uartrx.count)
         hidWrites(uartrx.buffer + uartrx.tail,ringRemoveBytes(&uartrx, NULL, uartrx.tail >= uartrx.head ? uartrx.length - uartrx.tail : uartrx.count));
      }

      osWaitMs(30);
      usbPoll();
   }
}

ISR (ADC_vect) // this ISR just clears the ADC interrupt flag.
{
   osSetRTR(adcTaskNumber);
}

ISR (USART_TXC_vect)
{
   if(uarttx.count)UDR = ringRemoveByte(&uarttx);
}

ISR (USART_RXC_vect)
{
   ringInsertByte(&uartrx, UDR);
}
