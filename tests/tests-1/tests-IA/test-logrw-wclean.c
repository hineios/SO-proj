#include <stdio.h>
#include <stdlib.h>
#include <sthread.h>
#include <block.h>
#include <logrw.h>
#include <time.h>

#define BLKSIZE 8
#define NUMBLK 12
#define NUMREGS 3

// Fills a block with "number" and "version" and "thread index" on its first three integers 
// positions and then writes it on position "number"
#define WRITEREGISTER(number, version, index) (block[0]=(char)number, block[1]=(char)version, block[2]=(char)index,logrw_write(lbks,number, (char *)block))
// Reads the block indexed by "number"
#define READREGISTER(number) (logrw_read(lbks, number, block))
// Dumps the logblock and block memories: the logblock is just a index:number structure while the
// block memory is a triple index:first position:second position
#define DUMP() (logrw_dump(lbks), logrw_dump_all_registers(lbks))

typedef struct {
 logrw_struct_t* lbks;
 unsigned int* index_t;
} logrw_t;

static unsigned int nticks;
static unsigned int n_cycle;

void *thread_BLKW(void * _lbks_s) {
    logrw_t* lbks_s = (logrw_t *)_lbks_s;
	logrw_struct_t *lbks = lbks_s->lbks;
	unsigned int* index=lbks_s->index_t;
	
	int i,j,t;

	char *block;

	if( (block = (char *)malloc(BLKSIZE)) == NULL)
	{
	 printf("error in block malloc\n");
	 return NULL;
    }

	t = time(NULL);
	
	for (j=0;j<n_cycle;j++)	
	 for (i=0;i<NUMREGS;i++)
	{
     WRITEREGISTER(i,j,*index);
    }

    t = time(NULL) - t;
    printf("time: %i sec at write thread %i\n", t,*index);
	
	free(block);
	sthread_exit(NULL);
	return NULL;
}

void *thread_BLKR(void * _lbks_s) {
    logrw_t* lbks_s = (logrw_t *)_lbks_s;
	logrw_struct_t *lbks = lbks_s->lbks;
	unsigned int* index=lbks_s->index_t;
	int i,t,t1;
	char *block;

	if( (block = (char *)malloc(BLKSIZE)) == NULL)
	{
	 printf("error in block malloc\n");
	 return NULL;
    }
	char version[NUMREGS][2];
	for (i=0;i<NUMREGS;i++)
	   {
          READREGISTER(i);
          version[i][0]=block[1];
          version[i][1]=block[2];
	   }
	DUMP();
	t=time(NULL);

	while(1)	
	 {
	   for (i=0;i<NUMREGS;i++)
	   {
        READREGISTER(i);
        if (block[1]!= version[i][0] || block[2]!= version[i][1])
	     {
          version[i][0]=block[1];
          version[i][1]=block[2];
                DUMP();
	     }
	   }
	   t1=time(NULL);
       if (t1>t+0.000001*n_cycle*NUMREGS*(nticks+50000)*10)
	      break;
     }

    free(block);
	sthread_exit(NULL);
	return NULL;
}

void *thread_dummy(void * _lbks_s) {
while(1);
}

int main (int argc, const char * argv[]) {
    void *ret;
	int i, t;
	
    int n_r = argc > 1 ? atoi(argv[1]) : 1;
    int n_w = argc > 2 ? atoi(argv[2]) : 1;
    nticks = argc > 3 ? atoi(argv[3]) : 0;

    n_cycle = argc > 4 ? atoi(argv[4]) : 4;

  	io_delay_on(nticks*10);
	 
    sthread_t tR[n_r];
    sthread_t tW[n_w];
	
	unsigned int indexR[n_r];
	unsigned int indexW[n_w];	
	
    sthread_init();
	
	logrw_t lbks_sR[n_r];
	logrw_t lbks_sW[n_w];
    logrw_struct_t *lbks = logrw_init(NUMREGS, NUMBLK, BLKSIZE);
    sthread_create(thread_dummy, NULL, 1,1,0);

	//char* block = (char *)malloc(BLKSIZE);
    //WRITEREGISTER(0,0,0);
    //WRITEREGISTER(1,0,0);
    //WRITEREGISTER(2,0,0);
	//free(block);

    t = time(NULL);

    for (i=0;i<n_w;i++)
	{
     lbks_sW[i].lbks = lbks;
	 indexW[i]=i;
     lbks_sW[i].index_t=&indexW[i];
 	 if ((tW[i] = sthread_create(thread_BLKW, &lbks_sW[i], 1,1,0)) == NULL){
	   printf("thread creation failed\n");
	   exit(-1);
	  }
	}
    for (i=0;i<n_r;i++)
	{
     lbks_sR[i].lbks = lbks;
	 indexR[i]=i;
     lbks_sR[i].index_t=&indexR[i];
     if ((tR[i] = sthread_create(thread_BLKR, &lbks_sR[i], 1,1,0)) == NULL){
	   printf("thread creation failed\n");
	   exit(-1);
	 } 
	} 

    for (i=0;i<n_w;i++)	 
	 sthread_join(tW[i], &ret);	
    for (i=0;i<n_r;i++)	 
	 sthread_join(tR[i], &ret);		 
	
    t = time(NULL) - t;
    printf("time: %i sec\n", t);
	
    DUMP();
    logrw_free(lbks);
    return 0;
}
