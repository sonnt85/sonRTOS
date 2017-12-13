#ifndef HIDSerial_h
#define HIDSerial_h
#include <stdint.h>
#include <avr/pgmspace.h>
#define HIDSERIAL_INBUFFER_SIZE 32

unsigned char hidWrite(unsigned char);  // write one character
unsigned char hidWrites(const void *buffer, unsigned char size);
void usbPoll(void);
unsigned char hidAvailable();
unsigned char hidRead(unsigned char *buffer);
void hidBegin();
unsigned char hidWrite8(const unsigned char *buffer, unsigned char size);
unsigned char hidWriteUint16(uint16_t data, uint8_t radix);
unsigned char hidWriteSint16(int16_t data, uint8_t radix);
unsigned char hidWriteUint32(uint32_t data, uint8_t radix);
unsigned char hidWriteSint32(int32_t data, uint8_t radix);
#define hidWritesPGM(s, size) hidWritespgm(PSTR(s), (size))
#define hidWriteStringPGM(b) hidWritespgm(PSTR(b), sizeof(b) - 1)
#endif
