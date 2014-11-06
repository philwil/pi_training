/*****************************************************************
 *
 * This is a simple exercise in file handling
 *
 * It uses basic files
 *   
 * Just to make it interesting we'll add a few options
 *****************************************************************/
 
/* try to run this with 

./file_101 my_test_file [read|write|mmap]

*/
  
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <sys/mman.h>

//int open(const char *pathname, int flags);
//int open(const char *pathname, int flags, mode_t mode);

#define BUF_SIZE 4096 * 4

int action_write(const char * fname)
{
  int rc=-1;
  int fd;
  int i;
  int buf[BUF_SIZE];  
  // set up the buffer
  for (i = 0 ; i < BUF_SIZE; i++)
    {
      buf[i]=i;
    } 

  fd=open(fname, O_WRONLY|O_CREAT, 0644);
  if (fd >=0)
    {
      rc =  write(fd, buf, sizeof(buf));
      printf(" wrote (%d) bytes to [%s]\n", rc, fname);
      close(fd);
    }
  return rc;
}

int action_read(const char * fname)
{
  int rc=-1;
  int fd;
  int i;
  int buf[BUF_SIZE];  
  int rbuf[BUF_SIZE];  
  // set up the buffer
  for (i = 0 ; i < BUF_SIZE; i++)
    {
      buf[i]=i;
    } 

  fd=open(fname, O_RDONLY);
  if (fd >=0)
    {
      rc =  read(fd, rbuf, sizeof(rbuf));
      printf(" read (%d) bytes from [%s]\n", rc, fname);
      close(fd);
    }

  for (i = 0 ; i < BUF_SIZE; i++)
    {
      if(buf[i] != rbuf[i]) 
	{
	  printf(" buf mismatch at %d read (%d) expected (%d)\n",
		 i, rbuf[i], buf[i]);
	}
    } 

  return rc;
}

int action_mmap(const char * fname)
{
  int rc=-1;
  int fd;
  int i;
  int buf[BUF_SIZE];  
  int *rbuf;  
  // set up the buffer
  for (i = 0 ; i < BUF_SIZE; i++)
    {
      buf[i]=i;
    } 

  fd=open(fname, O_RDWR);
  if (fd >=0)
    {
      rc =  sizeof(buf);
      // MAP_SHARED MAP_PRIVATE MAP_ANONYMOUS 
      rbuf = mmap(NULL, sizeof(buf), PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
      if( rbuf == MAP_FAILED )
	{
	  printf(" unable to read (%d) bytes from [%s]\n", rc, fname);
	  
	  close(fd);
	  return 0;
	}
      
    }

  for (i = 0 ; i < BUF_SIZE; i++)
    {
      if(buf[i] != rbuf[i]) 
	{
	  printf(" buf mismatch at %d read (%d) expected (%d)\n",
		 i, rbuf[i], buf[i]);
	}
    } 

  rbuf[1]++;
  
  msync(rbuf, sizeof(buf), MS_SYNC);
  close(fd);
  return rc;
}

/* next the extended main that adds the environment vars */
int main(int argc, char *argv[])
{
  int i;
  char *fname;
  char *faction=NULL;

  printf( "hello from the raspberry pi argc = %d\n", argc);
  printf(" args ....\n");
  for (i = 0 ; i < argc; i++)
    {
      printf ( " arg [%d] is [%s]\n", i, argv[i]);
    }
  if (argc < 2 )
    {
      printf(" Please supply a file name as arg 1 "
	     "and some optional actions as arg 2\n");
      printf(" Example %s <file_name> [read|write|mmap]\n", argv[0]);
      return 0;
    }

  fname = argv[1];
  if (argc > 2 )
    {
    faction = argv[2];
    }

  if(!faction) faction = "write";

  if (strcmp(faction,"write") == 0)
    {
      printf( " Performing write action on file [%s]\n", fname);
      action_write(fname);

    }
  else if (strcmp(faction,"read") == 0)
    {
      printf( " Performing read action on file [%s]\n", fname);
      action_read(fname);
    }
  else if (strcmp(faction,"mmap") == 0)
    {
      printf( " Performing mmap action on file [%s]\n", fname);
      action_mmap(fname);
    }


  return 0;
}


