/*
 *  gpio.c
 *
 *  Created on: Feb 6, 2015
 *      Author: NTS2201
 */
#include "gpio.h"
#include <avr/io.h>


#define NOT_IS_ADD_REGPIN 0XFFFF

const uint8_t * PROGMEM index_to_pinreg_PGM[] = {
#ifdef PINA
      (uint8_t *)&PINA,
#else
      (uint8_t *)NOT_IS_ADD_REGPIN,
#endif

#ifdef PINB
      (uint8_t *)&PINB,
#else
      (uint8_t *)NOT_IS_ADD_REGPIN,
#endif

#ifdef PINC
      (uint8_t *)&PINC,
#else
      (uint8_t *)NOT_IS_ADD_REGPIN,
#endif

#ifdef PIND
      (uint8_t *)&PIND,
#else
      (uint8_t *)NOT_IS_ADD_REGPIN,
#endif

#ifdef PINE
      (uint8_t *)&PINE,
#else
      (uint8_t *)NOT_IS_ADD_REGPIN,
#endif

#ifdef PINF
      (uint8_t *)&PINF,
#else
      (uint8_t *)NOT_IS_ADD_REGPIN,
#endif
};

const uint8_t PROGMEM tow2n8bits[] = {1, 2, 4, 8, 16, 32, 64, 128};
#define digitalPinToPort(P) ( pgm_read_byte( digital_pin_to_port_PGM + (P) ) )
#define digitalPinToBitMask(P) ( pgm_read_byte( digital_pin_to_bit_mask_PGM + (P) ) )

void pinMode(uint8_t pin, uint8_t mode)
{
   uint8_t *pinreg = (uint8_t *)pgm_read_word(index_to_pinreg_PGM + (uint8_t)(pin >> 3));
   uint8_t pinmask = pgm_read_byte(tow2n8bits + pin % 8);
   if(pinreg == NOT_IS_ADD_REGPIN)return;
   if(mode == INPUT)
   {
      *(pinreg + 1) &= (~pinmask);
   }
   else if (mode == OUTPUT)
   {
      *(pinreg + 1) |= pinmask;
   }
   else if (mode == OUTPUT_HIGH)
   {
      *(pinreg + 1) |= pinmask;
      *(pinreg + 2) |= pinmask;
   }
   else if (mode == OUTPUT_LOW)
   {
      *(pinreg + 1) |= pinmask;
      *(pinreg + 2) &= ~pinmask;
   }
   else if (mode == INPUT_PULLUP)
   {
      *(pinreg + 1) &= (~pinmask);
      *(pinreg + 2) |= pinmask;
   }
}

void digitalWrite(uint8_t pin, uint8_t val)
{
   uint8_t *pinreg = (uint8_t *)pgm_read_word(index_to_pinreg_PGM + (uint8_t)(pin >> 3));
   uint8_t pinmask = pgm_read_byte(tow2n8bits + pin % 8);
   if(pinreg == NOT_IS_ADD_REGPIN)return;
   if (val == LOW) {
      *(pinreg + 2) &= (~pinmask);
   }else{
      *(pinreg + 2) |= pinmask;
   }
}

void digitalToggle(uint8_t pin)
{
   uint8_t *pinreg = (uint8_t *)pgm_read_word(index_to_pinreg_PGM + (uint8_t)(pin >> 3));
   uint8_t pinmask = pgm_read_byte(tow2n8bits + pin % 8);
   if(pinreg == NOT_IS_ADD_REGPIN)return;
   *(pinreg + 2) ^= pinmask;
}

void digitalRead(uint8_t pin)
{
   uint8_t *pinreg = (uint8_t *)pgm_read_word(index_to_pinreg_PGM + (uint8_t)(pin >> 3));
   uint8_t pinmask = pgm_read_byte(tow2n8bits + pin % 8);
   if(pinreg == 0XFFFF)return;
   if(pinmask & *pinreg)return HIGH;else return LOW;
}
