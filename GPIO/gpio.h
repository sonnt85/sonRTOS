/*
 * gpio.h
 *
 *  Created on: Feb 6, 2015
 *      Author: NTS2201
 */

#ifndef LIBRARIES_IO_GPIO_H_
#define LIBRARIES_IO_GPIO_H_
#include <stdint.h>
#include <avr/pgmspace.h>
extern const uint8_t PROGMEM tow2n8bits[];
enum {PINA00 = 0, PINA01, PINA02, PINA03, PINA04, PINA05, PINA06, PINA07,
      PINB00, PINB01, PINB02, PINB03, PINB04, PINB05, PINB06, PINB07,
      PINC00, PINC01, PINC02, PINC03, PINC04, PINC05, PINC06, PINC07,
      PIND00, PIND01, PIND02, PIND03, PIND04, PIND05, PIND06, PIND07,
      PINE00, PINE01, PINE02, PINE03, PINE04, PINE05, PINE06, PINE07,
      PINF00, PINF01, PINF02, PINF03, PINF04, PINF05, PINF06, PINF07
};
#define HIGH 0x1
#define LOW  0x0

#define INPUT 0x0
#define OUTPUT 0x1
#define INPUT_PULLUP 0x2
#define OUTPUT_HIGH  0x3
#define OUTPUT_LOW   0x4

void pinMode(uint8_t pin, uint8_t mode);
void digitalWrite(uint8_t pin, uint8_t val);
void digitalRead(uint8_t pin);
void digitalToggle(uint8_t pin);


#endif /* LIBRARIES_IO_GPIO_H_ */
