/* 
 * Storage Layer
 * 
 * block.c
 *
 * Storage layer which offers the abstraction of a sequence of 
 * blocks of fixed size. Blocks are kept in memory.
 * 
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>

#include "block.h"



/*
 * to simulate delays in the io accesses, define 
 * SIMULATE_IO_DELAY and link with the appropriate implementation 
 * of funcions io_delay_*_block()
 */
#ifdef SIMULATE_IO_DELAY
void io_delay_read_block(unsigned int block_num);
void io_delay_write_block(unsigned int block_num);
#endif


// internal implementation of 'blocks_t' 
struct blocks_ {
   unsigned int block_size;
   unsigned int num_blocks;
   char blocks[0];
};


blocks_t* block_new(unsigned num_blocks, unsigned block_sz)
{
   if (num_blocks * block_sz == 0) {
      return NULL;
   }
   blocks_t* bks = (blocks_t*) 
   malloc(sizeof(blocks_t) + num_blocks * block_sz);
   bks->block_size = block_sz;
   bks->num_blocks = num_blocks;
   memset(&bks->blocks[0], 0, num_blocks * block_sz);
   return bks;
}


void block_free(blocks_t* bks)
{
   free(bks);
}


unsigned block_size(blocks_t* bks)
{
   return bks->block_size;
}


unsigned block_num_blocks(blocks_t* bks)
{
   return bks->num_blocks;
}


int block_read(blocks_t* bks, unsigned block_no, char* block)
{
   #ifdef DEBUG
	printf("[block_read] invoked\n");
   #endif
   if (block_no >= bks->num_blocks) {
	  return -1;
   }

#ifdef SIMULATE_IO_DELAY
 #ifdef NOT_FS_INITIALIZER   
   io_delay_read_block(block_no);
 #endif  
#endif
   char* ptr = &bks->blocks[block_no * bks->block_size]; 
   memcpy(block,ptr,bks->block_size);
   return 0;
}


int block_write(blocks_t* bks, unsigned block_no, char* block)
{
   #ifdef DEBUG
	printf("[block_write] invoked\n");
   #endif
   if (block_no >= bks->num_blocks) {
	  return -1;
   }

#ifdef SIMULATE_IO_DELAY
 #ifdef NOT_FS_INITIALIZER
   io_delay_write_block(block_no);
 #endif  
#endif

   char* ptr = &bks->blocks[block_no * bks->block_size]; 
   memcpy(ptr,block,bks->block_size);
   return 0;
}


blocks_t* block_load(char* file)
{
   if (file == NULL) {
      return NULL;
   }

   int fd = open(file, O_RDONLY);
   if (fd < 0) {
      return NULL;
   }

   int status = 0;

   unsigned block_size;
   status = read(fd,&block_size,sizeof(block_size));
   if (status != sizeof(block_size)) {
      close(fd);
      return NULL;
   }

   unsigned num_blocks;
   status = read(fd,&num_blocks,sizeof(num_blocks));
   if (status != sizeof(num_blocks)) {
      close(fd);
      return NULL;
   }

   blocks_t* bks = (blocks_t*)
      malloc(sizeof(blocks_t) + num_blocks * block_size);
   status = read(fd, bks->blocks, num_blocks * block_size);
   if (status != num_blocks * block_size) {
      close(fd);
      free(bks);
      return NULL;
   }
   bks->block_size = block_size;
   bks->num_blocks = num_blocks;
   return bks;
}


int block_store(blocks_t* bks, char* file)
{
   if (bks == NULL && file == NULL) {
      return -1;
   }

   int fd = open(file, O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR);
   if (fd < 0) {
      return -1;
   }

   unsigned size = sizeof(blocks_t) + bks->block_size * bks->num_blocks;

   int status = write(fd, (char*)bks, size);
   if (status != size) {
      close(fd);
      return -1;
   }

   close(fd);
   return 0;
}


void dump_all_blocks(blocks_t* bks)
{
	int i;
	
	for(i=0;i<bks->num_blocks;i++) {
		block_dump(bks, i);
//		unsigned *block = (unsigned *)&bks->blocks[ i * bks->block_size ];
//		printf("%.3u:%.2u:%.2u ",i,block[0],block[1]);
	}
	printf("\n");
}

void block_dump(blocks_t* bks, unsigned int block) {
	dump_block(&(bks->blocks[block * bks->block_size]));
}
void dump_block(char * block){
	int i;
	for(i=0; i<8 ; i++)
		printf("%.2X:",block[i]);
	printf("\t");
	for(i=0; i<8 ; i++) {
		char c =  block[i];
		printf("%c",isprint(c)?c:'.');
	}
	printf("\n");
}

