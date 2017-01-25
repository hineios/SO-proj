#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <stdlib.h>

#ifndef SLEEP_TIME
#define SLEEP_TIME 1
#define MAX_SLEEP_FACTOR	4
#define MIN(a,b) (a<=b ? a : b)
#define ACCESS_DELAY(a,b) ((((a)==(b+1))||((a)==(b)))?(SLEEP_TIME):(MIN(abs(a-b),MAX_SLEEP_FACTOR)*SLEEP_TIME))
#endif

#ifndef COUNTER_NUM
#define COUNTER_NUM 1000000
#endif


static int Is_off = 1;

void io_delay_on()
{
   printf("---> io_delay is being enabled <--- \n");
   Is_off = 0;
}

unsigned int last_access=0;

void io_delay_simulator(unsigned int block_num)
{
  //static unsigned int last_access=UINT_MAX;
   if (Is_off) {
      return;
   }
 #ifdef SLEEP_ON
   printf("io_delay_simulator ACCESS_DELAY(%d,%d)=%d\n",block_num,last_access,ACCESS_DELAY(block_num,last_access));
 
   sleep(ACCESS_DELAY(block_num,last_access));
#else

   int count = COUNTER_NUM;
   while(count > 0) {
      count--;
   }
#endif
	last_access=block_num;
}

void io_delay_read_block(unsigned int block_num)
{
   io_delay_simulator(block_num);

 // printf("[io_delay] Reading Block Number: %u\n", block_num);
}

void io_delay_write_block(unsigned int block_num)
{
   io_delay_simulator(block_num);

  //printf("[io_delay] Writing in Block Number: %u\n", block_num);
}

