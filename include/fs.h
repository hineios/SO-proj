/* 
 * File System Layer
 * 
 * fs.h
 *
 * Interface to the file system layer implemented at the server side.
 * The following interface functions are provided.
 * 
 * Manages the internal organization of files and directories in a 'virtual
 * memory disk' and provides the following interface functions to programmers.
 * 
 */

#ifndef _FS_H_
#define _FS_H_


#include "sthread.h"


#ifdef USE_CACHE
	#define DISK_READ(allocator,block_no,block)  cache_read(block_no,block)
	#define DISK_WRITE(allocator,block_no,block)  cache_write(block_no,block)
#else
	#ifdef USE_LOG_SOFS
		#define DISK_READ(log,register_no,block)			logrw_read(log,register_no,block)
		#define DISK_WRITE(log,register_no,block)			logrw_write(log,register_no,block)
	#else // use simple block allocator
		#define DISK_READ(bks,block_no,block)			block_read(bks,block_no,block)
		#define DISK_WRITE(bks,block_no,block)			block_write(bks,block_no,block)
	#endif
#endif

#ifdef USE_LOG_SOFS
	#define DISK_TYPE				logrw_struct_t
	#define DIM_ACTIVE_ARRAY(num_blocks)            num_blocks*10
	#define DISK_INIT(num_blocks,block_size)        logrw_init(num_blocks,DIM_ACTIVE_ARRAY(num_blocks), block_size)
	#include "../include/logrw.h"
#else // use simple block allocator
	#define DISK_TYPE			blocks_t
	#define DISK_INIT(num_blocks,block_size)                         block_new(num_blocks,block_size)
	#include "../include/block.h"
#endif   


#ifdef USE_CACHE
#include "../include/cache.h"
#endif

#ifndef BLOCK_SIZE
#define BLOCK_SIZE		512 
#endif

#ifndef NUM_BLOCKS
// default storage of 8 MB (8*1024*2 blocks * 512 bytes/block)
#define NUM_BLOCKS (8*1024*2)
#endif

#ifndef MAX_READDIR_ENTRIES
// maximum amount of directory entries 
#define MAX_READDIR_ENTRIES 64
#endif

#ifndef MAX_FILE_NAME_SIZE
// maximum size of a file name used in sofs
#define MAX_FILE_NAME_SIZE 14
#endif

// maximum space for the file name (13 chars + '\0')
#define FS_MAX_FNAME_SZ 14

// maximum size of a file name used in messages
#define MAX_PATH_NAME_SIZE 200

// type of the inode: directory or file
typedef enum {FS_DIR = 1, FS_FILE = 2} fs_itype_t;


// type of inode identifier
typedef unsigned short inodeid_t;


// attributes of a file
typedef struct {
   inodeid_t inodeid;  // inode number
   fs_itype_t type;    // directory or file
   unsigned size;      // total size in bytes
   int num_entries;    // number of entries if it is a directory
} fs_file_attrs_t;


// identify the name and the type of a file
typedef struct {
   char name[FS_MAX_FNAME_SZ];
   fs_itype_t type;
} fs_file_name_t;


// file system structure (the implementation is hidden)
typedef struct fs_ fs_t;


/*
 * fs_new: allocates storage - blocks - and memory for the fs structure
 * - num_blocks - number of blocks
 *   returns: the fs structure
 */
fs_t* fs_new(unsigned num_blocks);


/*
 * fs_format: formats the file system
 * - fs: reference to file system
 *   returns: 0 if successful, -1 otherwise
 */
int fs_format(fs_t* fs);


/*
 * fs_lookup: gets the inode id of an object (file/directory)
 * - fs: reference to file system
 * - file: the name of the object
 * - fileid: the inode id of the object [out]
 *   returns: 0 if successful,  -1 otherwise
 */
int fs_lookup(fs_t* fs,  char* file, inodeid_t* fileid);


/*
 * fs_get_attrs: gets the attributes of an object (file/directory)
 * - fs: reference to file system
 * - file: node id of the object
 * - attrs: the attributes of the object [out]
 *   returns: 0 if successful, -1 otherwise
 */
int fs_get_attrs(fs_t* fs, inodeid_t file, fs_file_attrs_t* attrs);


/*
 * fs_read: read the contents of a file
 * - fs: reference to file system
 * - file: node id of the file
 * - offset: starting position for reading
 * - count: number of bytes to read
 * - buffer: where to put the data [out]
 * - nread: number of bytes effectively read [out]
 *   returns: 0 if successful, -1 otherwise
 */
int fs_read(fs_t* fs, inodeid_t file, unsigned offset, unsigned count, 
   char* buffer, int* nread);


/*
 * fs_write: write data to file
 * - fs: reference to file system
 * - file: node id of the file
 * - offset: starting position for writing
 * - count: number of bytes to write
 * - buffer: the data to write
 *   returns: 0 if successful, -1 otherwise (the write operation is atomic)
 */
int fs_write(fs_t* fs, inodeid_t file, unsigned offset, unsigned count,
   char* buffer);


/*
 * fs_create: create a file in a specified directory
 * - fs: reference to file system
 * - dir: the directory where to create the file
 * - file: the name of the file
 * - fileid: the inode id of the file [out]
 *   returns: 0 if successful, -1 otherwise
 */
int fs_create(fs_t* fs, inodeid_t dir, char* file, inodeid_t* fileid);

/*
 * fs_flush: flush any cached dirty blocks to disk (used only in case
caching is enabled)
 *
 */
void fs_flush(fs_t* fs);


/*
 * fs_mkdir: create a subdirectory in a specified directory
 * - fs: reference to file system
 * - dir: the directory where to create the file
 * - newdir: the name of the new subdirectory
 * - newdirid: the inode id of the subdirectory [out]
 *   returns: 0 if successful, -1 otherwise
 */
int fs_mkdir(fs_t* fs, inodeid_t dir, char* newdir, inodeid_t* newdirid);


/*
 * fs_readdir: read the contents of a directory
 * - fs: reference to file system
 * - dir: the directory
 * - entries: where to write the entries of the directory [out]
 * - maxentries: maximum number of entries to write in 'entries'
 * - numentries: number of entries written [out]
 *   returns: 0 if successful, -1 otherwise
 */
int fs_readdir(fs_t* fs, inodeid_t dir, fs_file_name_t* entries, int maxentries,
   int* numentries);


/*
 * fs_truncate: truncate the content of a file by setting its size to 0. ( Created by ACV, IST - Taguspark, October 2011)
 * - fs: reference to file system
 * - fileid: the inode id of the file 
 *   returns: 0 if successful, -1 otherwise
 */
int fs_truncate(fs_t* fs, inodeid_t file);

/*
 * fd_dump: dump the contents of a file system
 */
void fs_dump(fs_t* fs);


#ifdef USE_CACHE
/*

 * fd_flush: force the flushing of any dirty cache content to disk.
 */
void fs_flush(fs_t* fs);
#endif

#endif

