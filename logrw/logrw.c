/* 
 * Log Structure Layer Implementation
 * 
 * logrw.c
 *
 * logrw.c (to be implemented by the student) implements this API.
 * block.c implements a storage layer which offers the abstraction 
 * of a sequence of blocks of fixed size. Blocks are kept in memory.
 * Contains functions for creating, destroying, writing, reading and 
 * dumping blocks, among other. 
 * 
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../include/logrw.h"
#include "../include/block.h"
#include "../include/sthread.h"





struct logrw_struct_ {
  long unsigned int* registo; 
  unsigned livre;
  int array_activo;
  unsigned int num_reg;            
  blocks_t* array_1;
  blocks_t* array_2;
  int tempo;
  
  sthread_rwlock_t PERMISSOES;
};
 
 
 
/*
 * logrw_init: create a logrw instance
 * should allocate space for all registers
 * should allocate space for block arrays
 * - num_reg: number of registers
 * - array_dim: size of each of the two arrays of blocks
 * - bloco_dim: size of each block/register
 *   returns: pointer to the logrw instance
 */
logrw_struct_t* logrw_init(unsigned int num_reg, unsigned int array_dim, unsigned int bloco_dim) {
  int i;

  
  if (num_reg >= array_dim){
    logrw_struct_t* log = (logrw_struct_t*) NULL;
    return log;
  }
  

  logrw_struct_t* log = (logrw_struct_t*) malloc(sizeof(struct logrw_struct_));
  log->PERMISSOES = sthread_rwlock_init();
  
  log->registo = (long int*) malloc(num_reg*sizeof(long int));
  sthread_rwlock_wrlock(log->PERMISSOES);
  for(i=0; i < num_reg;i++){
    log->registo[i] = 0;
  }
  log->livre = 0;
  log->array_activo = 0;
  log->num_reg = num_reg;
  log->array_1 = block_new(bloco_dim, array_dim);
  log->array_2 = block_new(bloco_dim, array_dim);
  log->tempo = 0;
  sthread_rwlock_unlock(log->PERMISSOES);
  return log;
}


/*

sthread_rwlock_t sthread_rwlock_init();
void sthread_rwlock_destroy(sthread_rwlock_t rwlock);
void sthread_rwlock_rdlock(sthread_rwlock_t rwlock);
void sthread_rwlock_wrlock(sthread_rwlock_t rwlock);
void sthread_rwlock_unlock(sthread_rwlock_t rwlock);


 * logrw_free: free the logrw
 * - log - the logrw instance
 */
void logrw_free(logrw_struct_t* log)  {

  free(log->registo);
  block_free(log->array_1);
  block_free(log->array_2);
  sthread_rwlock_destroy(log->PERMISSOES);
  free(log); 
}


/*
 * logrw_num_registers: gets the number of registers
 * - log - the logrw instance
 *   returns: number of the num_reg parameter that was provided when
 *   the logrw was first initialized; -1 if error
 */
int logrw_num_registers(logrw_struct_t* log)  {
  unsigned int num_reg = log->num_reg;
  return num_reg;
}


/*
 * logrw_read: reads a single register, reading the whole block with the 
 * most up-to-date content for that register
 * - log - the logrw instance
 * - register_no: the number of the register to read
 * - block: the buffer were to copy the block containing that register content [out]
 *   returns: 0 if sucessful, -1 if not
 */
int logrw_read(logrw_struct_t* log, unsigned register_no, char* block)  {
  sthread_rwlock_wrlock(log->PERMISSOES);
  unsigned indice = log->registo[register_no];

  if(indice < block_num_blocks(log->array_1)){
    if( block_read(log->array_1, indice, block) == 0){
      sthread_rwlock_unlock(log->PERMISSOES);
      return 0;
    }
    sthread_rwlock_unlock(log->PERMISSOES);
    return -1;
  }
  else {
    if( block_read(log->array_2, 
                   indice - block_num_blocks(log->array_2) , 
                   block)
        == 0){
      sthread_rwlock_unlock(log->PERMISSOES);
      return 0;
    }
    sthread_rwlock_unlock(log->PERMISSOES);
    return -1;
  }
}


/*
 * logrw_write: writes a register into the next free array block, writing to the whole block
 * - log: the logrw instance
 * - register_no: the number of the register to write
 * - block: the buffer with the data to write
 *   returns: 0 if sucessful, -1 if not
 */
int logrw_write(logrw_struct_t* log, unsigned register_no, char* block)  {
  int i;
  char *block_aux = (char *)malloc(block_size(log->array_1));


  //if (register_no < logrw_num_registers(log)) {
    //sthread_rwlock_unlock(log->PERMISSOES);
    sthread_rwlock_rdlock(log->PERMISSOES);
    
    // flip do array 1 para o 2
//*********************************************************************
    if (log->livre == block_num_blocks(log->array_1)) {
      log->array_activo = 1;
      for (i = 0; i < logrw_num_registers(log) ; i++){
        if (block_read(log->array_1, log->registo[i] , block_aux) == -1)
          continue;
        if (block_write(log->array_2, (log->livre - block_num_blocks(log->array_1)), block_aux) == 0){
          log->registo[i] = log->livre;
          log->livre += 1;
        }
      }
//escrita do bloco inicialmente pedido
      if( block_write(log->array_2, (log->livre - block_num_blocks(log->array_1)),block) == 0){
        log->registo[register_no] = log->livre;
        log->livre += 1;
        sthread_rwlock_unlock(log->PERMISSOES);
        return 0;
      }
      else{
        sthread_rwlock_unlock(log->PERMISSOES);
        return-1;
      }
      free(block_aux);
      sthread_rwlock_unlock(log->PERMISSOES);
      return 0;
    }
      
      
      /* flip do array 2 para o 1*/
//*******************************************************************
    if(log->livre == (2*block_num_blocks(log->array_1))) {
      log->array_activo = 0;
      log->livre = 0;
      for (i = 0; i < logrw_num_registers(log) ; i++){
        if (block_read(log->array_2, (log->registo[i] - block_num_blocks(log->array_1)) , block_aux) == -1)
          continue;
        if (block_write(log->array_1, log->livre, block_aux) == 0){
          log->registo[i] = log->livre;
          log->livre += 1;
        } 
      }
//escrita do bloco inicialmente pedido
      if( block_write(log->array_1,log->livre,block) == 0){
        log->registo[register_no] = log->livre;
        log->livre += 1;
        sthread_rwlock_unlock(log->PERMISSOES);
        return 0;
      }
      else{
        sthread_rwlock_unlock(log->PERMISSOES);
        return -1;
      }
      free(block_aux);
      sthread_rwlock_unlock(log->PERMISSOES);
      return 0;
    }
    free(block_aux);


//escrita no array 1
//******************************************************************
    if(log->livre < block_num_blocks(log->array_1)){
      if( block_write(log->array_1,log->livre,block) == 0){
        log->registo[register_no] = log->livre;
        log->livre += 1;
        sthread_rwlock_unlock(log->PERMISSOES);
        return 0;
      }
      sthread_rwlock_unlock(log->PERMISSOES);
      return -1;
    }


//escrita no array 2
//******************************************************************
    if(log->livre > block_num_blocks(log->array_1)){
      if( block_write(log->array_2, (log->livre - block_num_blocks(log->array_1)),block) == 0){
        log->registo[register_no] = log->livre;
        log->livre += 1;
        sthread_rwlock_unlock(log->PERMISSOES);
        return 0;
      }
      sthread_rwlock_unlock(log->PERMISSOES);
      return -1;
    }
  free(block_aux);
  sthread_rwlock_unlock(log->PERMISSOES);
  return -1;
}


/*
 * logrw_dump: prints general debugging information, including:
 * - which array is active ("first array"/"second array")
 * - index for next free block in the active array
 * - for each register, the index and array ("active array"/"inactive array") 
 * of the block that currently holds the most up-to-date contents of the register
 * Input:
 * - log - the logrw instance
 */
void logrw_dump(logrw_struct_t* log)  {
  int i;
  
 /* if( log == NULL){
    printf("no valid logrw instance\n");
    return;
  }*/
  
  printf("\nGeneral Info:\n");
  sthread_rwlock_rdlock(log->PERMISSOES);
  if(log->livre <= (block_num_blocks(log->array_1))) { //ESTA NO ARRAY 1
    printf("Array: %d NextWrBlk: %ld ",  log->array_activo , log->livre);
    printf("ThreadWaiting=%d\n", log->tempo);  //TODO
    printf("REG_INDX: \n");
    for(i=0 ; i < log->num_reg ; i++ ){
      printf("R[%d]=%ld ", i, log->registo[i]);
      if(log->array_activo == 1)
        printf("in active array\n");
      else
        printf("in inactive array\n");
    }
    sthread_rwlock_unlock(log->PERMISSOES);
  }
  else {  																							//SE ESTA NO ARRAY 2
    printf("Array: %d NextWrBlk: %ld " , log->array_activo , (log->livre - block_num_blocks(log->array_1)));
    printf("ThreadWaiting=%d",log->tempo);  //TODO
   	printf("REG_INDX:\n ");
    for(i=0 ; i < log->num_reg ; i++ ){
      printf("\tR[%d]=%ld ", i, (log->registo[i] - block_num_blocks(log->array_1)));
      if(log->array_activo == 2)
        printf("in active array\n");
      else
        printf("in inactive array\n");
    }
    sthread_rwlock_unlock(log->PERMISSOES);
  }
}


/*
 * logrw_dump_register: prints general information about a register 
 * If a valid register number is provided, the printed information must include:
 * - register number, 
 * - the index and array ("active array"/"inactive array") of the block 
 * that currently holds the most up-to-date contents of the register
 * - the first 8 bytes of the current contents of the register, in 
 * hexadecimal representation.
 * Input:
 * - i - the number of the register
 * - log - the logrw instance
 */
void logrw_dump_register(unsigned int i, logrw_struct_t* log) {
  sthread_rwlock_rdlock(log->PERMISSOES);
  unsigned indice = log->registo[i];
  char* block = (char*)malloc(block_size(log->array_1)); 
         
  if( indice <= block_num_blocks(log->array_1)) {
    if (block_read (log->array_1, indice, block) == 0){                 //Se LEU 
      dump_block(block);
      sthread_rwlock_unlock(log->PERMISSOES);
      return ;  
    }
    else {
      printf("\tdata: impossible to read\n");
      sthread_rwlock_unlock(log->PERMISSOES);
      return;
    }
  }
  else {
    if (block_read (log->array_2, indice - block_num_blocks(log->array_1) , block) == 0){                 /*Se LEU*/ 
      dump_block(block);
      sthread_rwlock_unlock(log->PERMISSOES);
      return;
    }
    else {
      printf("\tdata: impossible to read\n");
      sthread_rwlock_unlock(log->PERMISSOES);
      return;
    }
  }
}

/*
 * logrw_dump_all_registers: prints general information about all registers
 * For each register, the printed information should be the same as printed
 * by logrw_dump_register.
 * - log - the logrw instance
 */
void logrw_dump_all_registers(logrw_struct_t* log) {
  int i;

  for(i = 0; i < log->num_reg;i++){
    logrw_dump_register(i,log);  
  }
}


/*
 * log_block_load: load an image of log_blocks from blok structure
 * Should read the low block infrastructure form the file and 
 * - bks: the name blocks structure
 *   returns: the blocks instance
 */  


logrw_struct_t* logrw_load(blocks_t *bks){
	if(bks == NULL)
		return NULL;
	else {
		int i;
		char* block;
		logrw_struct_t* log = logrw_init ( block_num_blocks(bks), (block_num_blocks(bks) +10),block_size(bks));
		//Garantidamente a estrutura logrw_struct vai efectuar escritas no array activo, array 1.  
		//vai ter os mesmo numero de registos que tamanho de blocos.
		
		for(i=0; i <= log->num_reg ; i++){
			
			if(block_read(bks, i, block) == 0)
			  logrw_write(log, i, block);
			else 
				continue;
		}
		return log; 
	}
}


/*
 * log_block_store: store an image of blocks to its reserved space
 * - lbks - the blocks instance
 *   returns: 0 if sucessful, -1 if not
 */



int logrw_store(logrw_struct_t* lbks) {
	if(lbks == NULL)
		return -1;
	else {
		blocks_t* blk = block_new (block_size(lbks->array_1), block_num_blocks(lbks->array_1));
		char* block;
		int i=0;
		for (; i<= logrw_num_registers(lbks) ; i++) {
		  if (logrw_read (lbks, i,  block) == 0){
		  	if (block_write(blk, i , block) ==  0)
					 block_store(blk, block);
			}
		  else
		  	continue;
		}	
		return 0;	    			
	}
}




