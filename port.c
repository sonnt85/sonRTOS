#if defined(__AVR__)

extern u16 nextAddTask;
void port_OS_TASK_INIT(u16 pfunction, u16 sizeStack, u08 priorityTask)
{
   u08 * p08;
   u16 temp;
   temp = nextAddTask;
   nextAddTask -= ((sizeStack) + sizeof(port_jmp_buf[0]) + 1);
   //if(nextAddTask <  __heap_start)while(1);
   os_pointer[priorityTask] = (u08 *)nextAddTask + 2;
   p08 = (u08 *)(os_pointer[priorityTask]);
   p08[17] = p08[19] = temp >> 8;//YH  = SPH
   p08[16] = p08[18] = temp & 0xFF;//YL = SPL
   p08[20] = 0x80;//SSEG
   p08[21] = (pfunction) & 0XFF; //PCL
   p08[22] = (pfunction) >> 8;   //PCH
   p08 = (u08 *)((u08 *)os_pointer[priorityTask]  - 1);
   *p08 = MASK_STACK;
}

void port_InitOSTimer()
{
   nextAddTask = RAMEND;
   rtr = 0;
   for (thisTask = 0; thisTask < nMutex; thisTask++) // using thisTask for a different purpose here
   {
      wantMutex[thisTask] = 0; // Nobody wants the semaphore. NOTE: not required as
      mutexOwner[thisTask] = -1; // c initializes variables to zero.
   }
  // SP =  RAMEND - sizeof(env[0][0]) - 10;
   TCCR1A=0x00;
   TCCR1B=0x09;
   TCNT1 = 0;
   //ICR1=12001;
   OCR1A = 12000*MSPERTICKS;//MSPERTICKSms
   //OCR1BH=0x00;
   //OCR1BL=0x00;
   TIMSK |= (1 << OCIE1A);
  // sei();
} 
#elif defined(__CODEVISIONAVR__)

#define port_OS_TASK_INIT(sizeStack) if(__saveContex(env[thisTask]) == 0)\
                                                    {\
                                                         env[thisTask][3] = nextAddTask >> 8;\
                                                         env[thisTask][2] = nextAddTask & 0xFF;\
                                                         nextAddTask -= sizeStack;\
                                                         env[thisTask][1] = nextAddTask >> 8;\
                                                         env[thisTask][0] = nextAddTask & 0xFF;\
                                                         return;\
                                                     }
#endif
