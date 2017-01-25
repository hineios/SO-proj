/* 
 * Block Cache Layer
 * 
 * cache.h
 *
 * Interface to a cache of blocks.
 * 
 */

#ifndef _CACHE_H_
#define _CACHE_H_

#ifdef USE_LOG_SOFS
	#define BLOCK_READ(log,register_no,block)			logrw_read(log,register_no,block)
	#define BLOCK_WRITE(log,register_no,block)			logrw_write(log,register_no,block)
	#include "../include/logrw.h"
#else // use simple block allocator
	#define BLOCK_READ(bks,block_no,block)			block_read(bks,block_no,block)
	#define BLOCK_WRITE(bks,block_no,block)			block_write(bks,block_no,block)
	#include "../include/block.h"
#endif

#include "../include/fs.h"

struct content {
	int block_no; /*id*/
	int cache_no; /*position in the array*/
	char* block;
	int type;
	struct content *next;
	struct content *prev;
	UT_hash_handle hh; /* makes this structure hashable */
};

typedef struct content* data;

//Returns true if clean blocks is empty
int clean_blocks_empty();

//Puts data s in the end of FIFO clea blocks
void clean_blocks_put(data);

//Returns the data in the head of the clean blocks FIFO removing it from the FIFO
data clean_blocks_get();

//Returns true if dirty blocks is empty
int dirty_blocks_empty();

//Returns the data in the head of the dirty blocks FIFO removing it from the FIFO
data dirty_blocks_get();

//Puts data s in the end of FIFO dirty blocks
void dirty_blocks_put(data s);

//Removes data s from it's FIFO
void blocks_remove(data s);


//Adds a data to the hashmap using the block_no has the key
void add_block(data s);

//Returns the data that has block_no has index
data find_cache_no(int block_no);


//Deletes the specified data from the hash
void delete_block(data s);

/*
 * cache_init: initialize block/register cache
 * - bks: the blocks/registers instance to be cached
 *
 */
void cache_init(DISK_TYPE* allocator);

void cache_free();
/*
 * cache_read: read a whole block/register
 * - no: the number of the block/register to read
 * - block: the buffer were to copy the block/register 
 *   returns: 0 if sucessful, -1 if not
 */
int cache_read( unsigned no, char* block);


/*
 * cache_write: write a whole block/register
 * - no: the number of the block/register to write
 * - block: the data with the data to write
 *   returns: 0 if sucessful, -1 if not
 */
int cache_write( unsigned no, char* block);


/*
 * cache_block_flush: forces flush of any dirty cache block/register
 */
void cache_flush();


/*
 * cache_dump_stats: display statics of the cache
 */
void cache_dump_stats();
#endif
