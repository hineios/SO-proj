/*
 *  logrw.h
 *  logstruct
 *
 *  Created by Carlos Ribeiro on 11/09/22.
 *  Copyright 2011 IST. All rights reserved.
 *
 */

/* 
 * Log Structure Layer
 * 
 * logblock.h
 *
 * Interface to the LOG storage layer which offers the abstraction
 * of a sequence of blocks of fixed size.
 * 
 */

#ifndef _LOG_BLOCK_H_
#define _LOG_BLOCK_H_

#include "../include/block.h"
#include "sthread.h"

	

#define CLEAN_PRIO 0

/*
 * logrw_struct_t: the storage abstraction of a virtual disk
 * the implementation is hidden
 */
typedef struct logrw_struct_ logrw_struct_t;


/*
 * log_block_new: create a blocks instance
 * - num_blocks: number of blocks
 * - reserve_blocks: boolean 
 *   returns: the blocks instance
 */
logrw_struct_t* logrw_init(unsigned int nregisters, unsigned int nblocks, unsigned int block_size);


/*
 * log_block_free: free the blocks
 * - bks - the blocks to free
 */
void logrw_free(logrw_struct_t* lbks);


/*
 * log_block_num_blocks: get the total number of blocks
 */
unsigned int log_block_num_blocks(logrw_struct_t* lbks);


/*
 * log_block_read: read a whole block
 * - bks: the blocks instance
 * - block_no: the number of the block to read
 * - block: the buffer were to copy the block [out]
 *   returns: 0 if sucessful, -1 if not
 */
int logrw_read(logrw_struct_t* lbks, unsigned register_no, char* block);


/*
 * log_block_write: write a whole block
 * - bks: the blocks instance
 * - block_no: the number of the block to write
 * - block: the data with the data to write
 *   returns: 0 if sucessful, -1 if not
 */
int logrw_write(logrw_struct_t* lbks, unsigned register_no, char* block);


/*
 * log_block_load: load an image of log_blocks from blok structure
 * Should read the low block infrastructure form the file and 
 * - bks: the name blocks structure
 *   returns: the blocks instance
 */
logrw_struct_t* logrw_load(blocks_t *bks);


/*
 * log_block_store: store an image of blocks to its reserved space
 * - lbks - the blocks instance
 *   returns: 0 if sucessful, -1 if not
 */
int logrw_store(logrw_struct_t* lbks);


/*
 * block_dump: dumps the content of blocks
 * - bks - the blocks instance
 */
void logrw_dump(logrw_struct_t* lbks);
void logrw_dump_register(unsigned int i, logrw_struct_t* lbks); 
void logrw_dump_all_registers(logrw_struct_t* lbks);

void *logrw_clean_func(void* _lbks);

#endif
