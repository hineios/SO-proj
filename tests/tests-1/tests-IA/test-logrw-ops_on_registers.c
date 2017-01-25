#include <stdio.h>
#include <stdlib.h>
#include <sthread.h>
#include <block.h>
#include <logrw.h>
#include <time.h>

#define BLKSIZE 8
#define NUMBLK 12
#define NUMREGS 3

// Fills a block with "number" and "version" on its first two integers 
// positions and then writes it on position "number"
#define WRITEREGISTER(number, version) (block[0]=(char)number, block[1]=(char)version, logrw_write(lbks,number, (char *)block))
// Reads the block indexed by "number"
#define READREGISTER(number) (logrw_read(lbks, number, block))
// Dumps the logblock and block memories: the logblock is just a index:number structure while the
// block memory is a triple index:first position:second position
#define DUMP() (logrw_dump(lbks), logrw_dump_all_registers(lbks))

typedef struct{
 logrw_struct_t* lbks;
 unsigned int* index_t;
 int registerN;
 unsigned int cycle_n;
}logrw_t;

static unsigned int nticks;

void *thread_BLKW(void * _lbks_s) {
    logrw_t* lbks_s = (logrw_t *)_lbks_s;
	logrw_struct_t *lbks = lbks_s->lbks;
	unsigned int* index=lbks_s->index_t;
	
	int j,t;

	char *block;

	if( (block = (char *)malloc(BLKSIZE)) == NULL)
	{
	 printf("error in block malloc\n");
	 return NULL;
    }

	t = time(NULL);
	
	int ind = lbks_s->registerN > -1 ? lbks_s->registerN : *index;
		
	for (j=*index*lbks_s->cycle_n;j<(*index+1)*lbks_s->cycle_n;j++)	
	{
      WRITEREGISTER(ind,j);
    }
    t = time(NULL) - t;
    printf("time: %i sec at write thread %i\n", t,*index);

	free(block);
	sthread_exit(NULL);
	return NULL;
}

void *thread_dummy(void * _lbks_s) {
while(1);
}

void *thread_BLKR(void * _lbks_s) {
    logrw_t* lbks_s = (logrw_t *)_lbks_s;
    logrw_struct_t *lbks = lbks_s->lbks;
	unsigned int* index=lbks_s->index_t;
	int j,t;
	char *block;

	if( (block = (char *)malloc(BLKSIZE)) == NULL)
	{
	 printf("error in block malloc\n");
	 return NULL;
    }
	
	t = time(NULL);

	int ind = lbks_s->registerN > -1 ? lbks_s->registerN : *index;
	for (j=0;j<lbks_s->cycle_n;j++)				
	{
      READREGISTER(ind);
    }
	t = time(NULL) - t;
    printf("time: %i sec at read thread %i\n", t,*index);
	
    free(block);
	sthread_exit(NULL);
	return NULL;
}

int main (int argc, const char * argv[]) {
    void *ret;
	int i, t;

	char typeOp = argc > 1 ? argv[1][0] : 'r';
	char typeReg = argc > 2 ? argv[2][0] : '=';
    nticks = argc > 3 ? atoi(argv[3]) : 1;
	io_delay_on(nticks*10);

    int n = argc > 4 ? atoi(argv[4]) : 2;
    unsigned int cycle_n = argc > 5 ? atoi(argv[5]) : NUMBLK;
    int register_n;

	if(typeReg=='=')
	  register_n = argc > 6 ? atoi(argv[6]) : 0;
	else
	  register_n=-1;

    printf("options: typeOp %c, typeReg %c, n %i, cycle_n %i, register_n %i\n", typeOp,typeReg,n,cycle_n,register_n);
	
    sthread_t tRW[n];
	unsigned int index[n];	
	
    sthread_init();
	
	logrw_t lbks_s[n];
    logrw_struct_t *lbks = logrw_init(NUMREGS, NUMBLK, BLKSIZE);

    sthread_create(thread_dummy, NULL, 1,1,0);

	//char* block = (char *)malloc(BLKSIZE);
    //WRITEREGISTER(0,0);
    //WRITEREGISTER(1,0);
    //WRITEREGISTER(2,0);
	//free(block);

    t = time(NULL);
   
    for (i=0;i<n;i++)
	{
	  index[i]=i;
      lbks_s[i].index_t=&index[i];
    lbks_s[i].lbks = lbks;
	lbks_s[i].registerN=register_n;
	lbks_s[i].cycle_n=cycle_n;

	  if(typeOp=='r')
	   tRW[i] = sthread_create(thread_BLKR, &lbks_s[i], 1,1,0);
      else
	   tRW[i] = sthread_create(thread_BLKW, &lbks_s[i], 1,1,0);
      if (tRW[i] == NULL){
	   printf("thread creation failed\n");
	   exit(-1);
	 } 
	}

    for (i=0;i<n;i++)	 
	 sthread_join(tRW[i], &ret);		 
	
    t = time(NULL) - t;

    printf("time: %i sec\n", t);
	
    DUMP();

    logrw_free(lbks);
    return 0;
}

