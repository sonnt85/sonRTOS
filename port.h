#ifndef PORT_H
#define PORT_H

#include <setjmp.h>
#include <avr/interrupt.h>

typedef  unsigned char       u08;
typedef  unsigned int        u16;
typedef  unsigned long int   u32;
typedef  unsigned long long  u64;

typedef  signed char         s08;
typedef  signed int          s16;
typedef  signed long int     s32;
typedef  signed long long    s64;

#define MSPERTICKS   5
#define __saveContex(env) setjmp((env))
#define __restoreContex(env, rt ) longjmp((env), (rt))
#define port_jmp_buf jmp_buf
void port_OS_TASK_INIT(u16 pfunction, u16 sizeStack, u08 priorityTask);
void port_InitOSTimer();
#define ISR_TIMER ISR (TIMER1_COMPA_vect) // compare match every 1ms.

#endif

