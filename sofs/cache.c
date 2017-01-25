/* 
 * Block Cache Layer
 * 
 * cache.h
 *
 * Cache of blocks.
 * 
 */

#include "../uthash/uthash.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "../include/sthread.h"
#include "../include/block.h"
#include "../include/cache.h"
#define CACHE_SIZE 2000
#define CACHE_BLOCK_SIZE 512
#define DIRTY_BLOCK 0
#define CLEAN_BLOCK 1

sthread_mutex_t MUTEX;

DISK_TYPE* disk;

data map = NULL; //hash com os indexes na cache para cada bloco

int hit, miss, flush;
static data dirty_blocks_head, dirty_blocks_tail;
static data clean_blocks_head, clean_blocks_tail;

int clean_blocks_empty() {
  return clean_blocks_head == NULL;
}

void clean_blocks_put(data s) {
  if (clean_blocks_head == NULL) {
    s->type = CLEAN_BLOCK;
    clean_blocks_head = s;
    clean_blocks_tail = s;
    return;
  }
  s->type = CLEAN_BLOCK;
  clean_blocks_tail->next = s;
  clean_blocks_tail = clean_blocks_tail->next;
}

data clean_blocks_get() {
  data t = clean_blocks_head;
  clean_blocks_head = t->next;
  return t;
}

int dirty_blocks_empty() {
  return dirty_blocks_head == NULL;
}

void dirty_blocks_put(data s) {
  if (dirty_blocks_head == NULL) {
    s->type = DIRTY_BLOCK;
    dirty_blocks_head = s;
    dirty_blocks_tail = s;
    return;
  }
  s->type = DIRTY_BLOCK;
  dirty_blocks_tail->next = s;
  dirty_blocks_tail = dirty_blocks_tail->next;
}

data dirty_blocks_get() {
  data s = dirty_blocks_head;
  dirty_blocks_head = s->next;
  return s;
}  

void blocks_remove(data s){
  if( (s->block_no == dirty_blocks_head->block_no) ){
    dirty_blocks_head = dirty_blocks_head->next;
    return;
  }
  else if(s->block_no == clean_blocks_head->block_no) {
    clean_blocks_head = clean_blocks_head->next;
    return;
  }
  else if(s->block_no == dirty_blocks_tail->block_no){
    dirty_blocks_tail = dirty_blocks_tail->prev;
    return;
  }
  else if(s->block_no == clean_blocks_tail->block_no){
    clean_blocks_tail = clean_blocks_tail->prev;
    return;
  }
  data prev, next;
  prev = s->prev;
  next = s->next;
  prev->next = next;
  next->prev = prev;
}

//Adds the index of the block in the cache array to the map with the block_no has the id
void add_block(data s) {
  HASH_ADD_INT( map, block_no, s );  /* block_no: name of key field */
}

//Returns the index of the block in the cache array, if not present returns -1
data find_cache_no(int block_no) {
  data s;
  HASH_FIND_INT(map, &block_no, s );  /* s: output pointer */
  return s;
}

//Deletes the specified data
void delete_block(data s) {
  HASH_DEL(map, s);  /* s: pointer to deletee */
}

/*
 * cache_init: initialize block/register cache
 * - allocator: the blocks/registers instance to be cached
 *
 */
void cache_init(DISK_TYPE* allocator) {
  int i;
  MUTEX = sthread_mutex_init();
  sthread_mutex_lock(MUTEX);
  clean_blocks_head = NULL;
  clean_blocks_tail = NULL;
  dirty_blocks_head = NULL;
  dirty_blocks_tail = NULL;
  hit = 0;
  miss = 0;
  flush = 0;
  disk = allocator;
  for(i = 0; i < CACHE_SIZE ; i++){ //Alocates all CACHE_SIZE blocks needed for the cache, and classifies them as CLEAN_BLOCK
    data s = malloc( sizeof( struct content));
    s->cache_no = i;
    s->type = CLEAN_BLOCK;
    clean_blocks_put(s);
  }
  sthread_mutex_unlock(MUTEX);
  return ;
}

void cache_free(){
/******
 ******VERY IMPORTANT!!!!!!!!!!
 ******ALWAYS FLUSH CACHE BEFORE FREEING IT!!!!!
 ******/
  data s;
  sthread_mutex_lock(MUTEX);
  while (!clean_blocks_empty()){ //Frees all CACHE_SIZE blocks allocated for the cache 
    s = clean_blocks_get();
    free(s);
  }
  sthread_mutex_unlock(MUTEX);
  sthread_mutex_free(MUTEX);
}

/*
 * cache_read: read a whole block/register
 * - no: the number of the block/register to read
 * - block: the buffer were to copy the block/register 
 *   returns: 0 if sucessful, -1 if not
 */
int cache_read(unsigned block_no,	char* block) {
  sthread_mutex_lock(MUTEX);
  data s = find_cache_no(block_no);
  if(s == NULL){ //It's not loaded to the cache, so we have to load it
    if(clean_blocks_empty()){ //there are no clean blocks to use
      s = dirty_blocks_get();//we get the dirty block to use
      delete_block(s);//we remove it from the hash
      BLOCK_WRITE(disk, s->block_no, s->block);//and we write it back tthe disk
      flush += 1;
    }
    else{//we still have clean blocks to use
      s = clean_blocks_get();//we fetch the clean block
    }
    BLOCK_READ(disk, block_no, block);//we load the info from the disk
    s->block_no = block_no;//we copy the info to the actual block
    s->block = block;
    s->type = CLEAN_BLOCK;
    add_block(s);//we add it to the hash
    clean_blocks_put(s);//and put it in the clean blocks
    miss += 1;
    sthread_mutex_unlock(MUTEX);
    return 0;
  }
  else{//It's already loaded into the cache
    blocks_remove(s);//We remove it from it's FIFO
    block = s->block;
    if(s->type == CLEAN_BLOCK) //and put it back in in the corresponding one
      clean_blocks_put(s);
    else
      dirty_blocks_put(s);
    hit +=1;
    sthread_mutex_unlock(MUTEX);
    return 0;
  }  
}
  

/*
 * cache_write: write a whole block/register
 * - no: the number of the block/register to write
 * - block: the data with the data to write
 *   returns: 0 if sucessful, -1 if not
 */
int cache_write(unsigned block_no, char* block) {
  sthread_mutex_lock(MUTEX);
  data s = find_cache_no(block_no);
  if(s == NULL){//It's not loaded in the cash, so we have to load it
    if(clean_blocks_empty()){//We do not have clean blocks to use
      s = dirty_blocks_get();//get the dirty block to use, we need to save it
      delete_block(s);//remove it from the hash
      blocks_remove(s);//remove it from it's FIFO
      BLOCK_WRITE(disk, s->block_no, s->block);//write it's info to the disk
      flush += 1;
    }
    else{//we have clean blocks to use
      s = clean_blocks_get(); //get the block to use
      blocks_remove(s);//remove it from it's FIFO
      delete_block(s);//remove it from the hash
    }
    BLOCK_READ(disk, block_no, s->block);//read the new block (outdated because of the write we're about to do) into the block
    s->block_no = block_no;//update it's info with the new info
    s->block = block;
    s->type = DIRTY_BLOCK;
    add_block(s);//add it to the hash
    dirty_blocks_put(s);//put it back in the dirty blocks
    miss += 1 ;
    sthread_mutex_unlock(MUTEX);
    return 0;
  }
  else{
    blocks_remove(s);//we remove it from the corresponding list (doesn't matter if is clean or dirty)
    s->block = block;//we put the new info into it
    s->type = DIRTY_BLOCK;
    dirty_blocks_put(s);//and put it in the dirty blocks
    hit += 1;
    sthread_mutex_unlock(MUTEX);
    return 0;
  }
}


/*
 * cache_block_flush: forces flush of any dirty cache block/register
 */
void cache_flush() {
  data s;
  sthread_mutex_lock(MUTEX);
  while(!dirty_blocks_empty()){ //while there are still dirty blocks, we need to flush them
    s = dirty_blocks_get(); //we fetch the dirty block
    delete_block(s);//we remove it from the hash
    blocks_remove(s);//remove it from the corresponding FIFO
    BLOCK_WRITE(disk, s->block_no, s->block);//Write it back to the disk
    flush += 1;
    s->type = CLEAN_BLOCK;
    clean_blocks_put(s);
  }
  sthread_mutex_unlock(MUTEX);
}

/*
 * cache_dump_stats: display statics of the cache
 */
void cache_dump_stats() {
  int counter_clean, counter_dirty;
  sthread_mutex_lock(MUTEX);
  data t = clean_blocks_head;
  while(t->cache_no != clean_blocks_tail->cache_no){
    counter_clean += 1;
  }
  t = dirty_blocks_head;
  while(t->cache_no != dirty_blocks_tail->cache_no){
    counter_dirty += 1;
  }
  printf("Capacidade : %d", CACHE_SIZE);
  printf("Clean blocks/registers: %d", counter_clean);
  printf("Dirty blocks/registers: %d", counter_dirty);
  printf("Hit Ratio: %f", ((float)hit / ((float)hit+(float)miss)) );
  printf("Misses: %d", miss);
  printf("Flushed: %d", flush);
  sthread_mutex_unlock(MUTEX);
  return ;
}


