/*
  FUSE: Filesystem in Userspace
  ACV - IST -  Taguspark, October 2011	
*/

#define FUSE_USE_VERSION 28

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <fuse.h>
#include <libgen.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/xattr.h>

#include "../include/fs.h"

#ifdef USE_CACHE
#include "../include/cache.h"
#endif


#ifdef SIMULATE_IO_DELAY
void io_delay_on();
#endif

static fs_t* FS;

///////////////////////////////////////////////////////////
//
// Prototypes for all these functions, and the C-style comments,
// come indirectly from /usr/include/fuse.h
////////////////////////////////////////////////////////////

/**
 * Initialize filesystem
 */
void *sofs_init(struct fuse_conn_info *conn)
{

	sthread_init();
	FS = fs_new(NUM_BLOCKS);
        fs_format(FS);
	#ifdef SIMULATE_IO_DELAY
	  #ifdef NOT_FS_INITIALIZER   
   		io_delay_on();
	 #endif  
	#endif  
}

/**
 * Clean up filesystem
 *
 * Called on filesystem exit.
 *
 */
void sofs_destroy(void *userdata)
{
	#ifdef USE_CACHE
	struct timeval start, end;

	cache_dump_stats();
	gettimeofday(&start, NULL);
	printf("...cache flushing started...\n");
	fs_flush(FS);
	printf("DONE!\n");

	cache_dump_stats();

	gettimeofday(&end, NULL);

	printf("Time elapsed to flush the cache: %ld\n", ((end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec))/1000);

	#endif
}



/**
 * Create and open a file
 *
 * If the file does not exist, first create it with the specified
 * mode, and then open it.
 *
 * If this method is not implemented or under Linux kernel
 * versions earlier than 2.6.15, the mknod() and open() methods
 * will be called instead.
 *
 * Introduced in version 2.5
 */
int sofs_create(const char *path, mode_t mp, struct fuse_file_info *fi) 
{
   int res = -1;
   inodeid_t dir = 1 ; // All files are created in root directory
   inodeid_t fileid;

   char *filename=(char *)malloc(MAX_FILE_NAME_SIZE*sizeof(char));
   memcpy(filename,path,strlen(path));	
   filename[strlen(path)] = '\0';

   if (!fs_create(FS,dir,filename,&fileid)) {
         fi->fh = fileid;
         res = 0;
   }
    
   free(filename);

   return res;
}


/** Get file attributes.
 *
*/
int sofs_getattr(const char *path, struct stat *stbuf)
{

   int res = -ENOENT;
   inodeid_t fileid;

   char* filename; 
   strcpy(filename,path);
   memset(stbuf, 0, sizeof(struct stat));
   
   // Root Directory Attributes
   if ( strcmp(filename,"/")== 0 ) {
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
		stbuf->st_size = 512;
		return 0;		
   }
  
   if (fs_lookup(FS,filename,&fileid) == 1) {
    	//printf("[sofs_getattr] filename: %s fileinode: %d\n", filename, fileid);
      	fs_file_attrs_t attrs;
      	if (fs_get_attrs(FS,fileid,&attrs) == 0) {
	      if (attrs.type == 2) {
			stbuf->st_mode = S_IFREG | 0777;
			stbuf->st_nlink = 1;
			stbuf->st_size = attrs.size;
			res = 0;
              } else if (attrs.type == 1) {
				stbuf->st_mode = S_IFDIR | 0777;
				stbuf->st_nlink = 2;
				stbuf->st_size = 512;
				res = 0;
	             }			
      	}
   } 
      
   return res;   
}

/** Read directory
 *
*/ 
int sofs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
		       off_t offset, struct fuse_file_info *fi)
{
    int res = -1;	
    (void) offset;
    (void) fi;

    if(strcmp(path, "/") != 0) //There is a single directory! The sofs root directory!!!
        return -ENOENT;
    //printf("[sofs_readdir] get attrs. from file: '%s'\n", path);
    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);
	
	inodeid_t fileid = 1;
    int maxentries;	
	fs_file_attrs_t attrs;
    if (fs_get_attrs(FS,fileid,&attrs) == 0) {
		//printf("[sofs_readdir] get attrs. from file type: '%d'\n", attrs.type);	
		if (attrs.type == 1) {
			maxentries = attrs.num_entries;
			//printf("[sofs_readdir] Entries N: '%d'\n", attrs.num_entries);			
			res = 0;
        }
		
		if (maxentries) {
			// Read the directory
			fs_file_name_t entries[MAX_READDIR_ENTRIES];
			int numentries;
			if (!fs_readdir(FS,fileid,entries,maxentries,&numentries)) {
				  int i;
				  for (i = 0; i < numentries; i++) {
						//printf("[sofs_readdir] Directory: '%s' File: %s\n", path, entries[i].name);
						filler(buf, entries[i].name, NULL, 0);  
				  }
				res = 0;	  
			}
		}	
	}
    return res;	
}


/** File open operation
 *
 * No creation, or truncation flags (O_CREAT, O_EXCL, O_TRUNC)
 * will be passed to open().  Open should check if the operation
 * is permitted for the given flags.  Optionally open may also
 * return an arbitrary filehandle in the fuse_file_info structure,
 * which will be passed to all file operations.
 *
 */
int sofs_open(const char *path, struct fuse_file_info *fi)
{
   int res = -ENOENT;
   inodeid_t fileid;

   char *filename=(char *)malloc(MAX_FILE_NAME_SIZE*sizeof(char));
   memcpy(filename,path,strlen(path));	
   filename[strlen(path)] = '\0';
  
   if (fs_lookup(FS,filename,&fileid) == 1) {
	fi->fh= fileid;
	res = 0;
   } 

   free(filename);
   return res;	
}

/** Read data from an open file
 *
 * Read should return exactly the number of bytes requested except
 * on EOF or error, otherwise the rest of the data will be
 * substituted with zeroes (check fuse documentation for exceptions).
 *
 */
int sofs_read(const char *path, char *buf, size_t size, off_t offset,
		    struct fuse_file_info *fi)
{
   int res = -1;
   int fileid = fi->fh;
   int nread = 0; 
   
   if (fs_read(FS,fileid,offset,size,buf,&nread) == 0){
	offset += nread;
	res = nread;
   }   
   
   return res;	

}

/** Write data to an open file
 *
 * Write should return exactly the number of bytes requested
 * except on error/** Read data from an open file
 *
 * Read should return exactly the number of bytes requested except
 * on EOF or error, otherwise the rest of the data will be
 * substituted with zeroes (check fuse documentation for exceptions).
 *
 */
int sofs_write(const char *path, const char *buf, size_t size,
		     off_t offset, struct fuse_file_info *fi)
{
   int res=-1;
   int fileid = fi->fh; 


   char *filedata=(char *)malloc(size*sizeof(char));
   memcpy(filedata,buf,size);

   if (!fs_write(FS,fileid,offset,size,filedata)){
		res=(int) size;
   }   

   free(filedata);

   return res;	
}

/** Create a file node
 *
*/ 
// All I know is that mknod() will be called, instead of create(),
// for many shell commands performing file creation, directory 
// listing, etc ..
int sofs_mknod(const char *path, mode_t mode, dev_t dev)
{
   return 0;
}


/*
* Flush is called on each close() of a file descriptor.  So if a
 * filesystem wants to return write errors in close() and the file
 * has cached dirty data, this is a good place to write back data
 * and return any errors.
 */
int sofs_flush(const char *path, struct fuse_file_info *fi)
{   
   return 0;
}

/** Change the access and/or modification times of a file */
int sofs_utime(const char *path, struct utimbuf *ubuf)
{
    return 0;
}

/** Change the owner and group of a file */
int sofs_chown(const char *path, uid_t uid, gid_t gid)  
{ 
    return 0;
}

/** Change the permission bits of a file */
int sofs_chmod(const char *path, mode_t mode)
{  
    return 0;
}

/** Change the size of a file */
int sofs_truncate(const char *path, off_t newsize)
{
   int res = -ENOENT;
   inodeid_t fileid;

   char *filename=(char *)malloc(MAX_FILE_NAME_SIZE*sizeof(char));
   memcpy(filename,path,strlen(path));	
   filename[strlen(path)] = '\0';
   
   if (fs_lookup(FS,filename,&fileid) == 1) {
        if (fs_truncate(FS, fileid) == 0)
	   res = 0;
   } 

   free(filename);	
	
   return res;
}

/** Release an open file
 *
 * Release is called when there are no more references to an open
 * file: all file descriptors are closed and all memory mappings
 * are unmapped.
 *
 * For every open() call there will be exactly one release() call
 * with the same flags and file descriptor.  It is possible to
 * have a file opened more than once, in which case only the last
 * release will mean, that no more reads/writes will happen on the
 * file.  The return value of release is ignored.
 *
 */
int sofs_release(const char *path, struct fuse_file_info *fi)
{
   return 0;
}

/** Get extended attributes */
// Is usefull when executing "ls" shell command
int sofs_getxattr(const char *path, const char *name, char *value, size_t size)
{  
   return 0;
}

static struct fuse_operations sofs_oper = {
	.getattr	= sofs_getattr,
	.readdir	= sofs_readdir,
	.open		= sofs_open,
	.create		= sofs_create,
	.mknod		= sofs_mknod,
	.read		= sofs_read,
	.write		= sofs_write,
	.init		= sofs_init,
	.flush		= sofs_flush,
	.destroy	= sofs_destroy,
	.release	= sofs_release,
	.utime		= sofs_utime,
	.chown		= sofs_chown,
	.chmod		= sofs_chmod,
	.truncate	= sofs_truncate,
	.getxattr 	= sofs_getxattr,	
	
};

int main(int argc, char *argv[])
{
    return fuse_main(argc, argv, &sofs_oper, NULL);	
}

