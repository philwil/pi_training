/*****************************************************************
 *
 * This is the starting point for all our user code
 * This simple example is often used to make sure that we can build,
 * deploy and run code on our target system
 * 
 * This code also checks out our embedded compiler and associated basic 
 * libraries.
 *   
 * this code can be tested out on an x86 system.
 *
 * Just to make it interesting we'll add a few options
 *****************************************************************/
 
/* try to run this with 

FOO="a sample env variable" ./hello these are "some args"
Note how we can get to the env vars.
*/
  

#include <stdio.h>

/* first the basic one that we normally use */
int old_main(int argc, char * argv[])
{
  int i;
  printf( "hello from the raspberry pi argc = %d\n", argc);
  for (i = 0 ; i < argc; i++)
    {
      printf ( " arg [%d] is [%s]\n", i, argv[i]);
    }
  return 0;
}

/* next the extended main that adds the environment vars */
int main(int argc, char *argv[], char *env[])
{
  int i;
  char * envp;
  printf( "hello from the raspberry pi argc = %d\n", argc);
  printf(" args ....\n");
  for (i = 0 ; i < argc; i++)
    {
      printf ( " arg [%d] is [%s]\n", i, argv[i]);
    }
  i = 0;
  envp = env[i];
  printf(" environment variables ....\n");
  while(envp )
    {
      printf ( " env [%d] is [%s]\n", i, envp);
      i++;
      envp = env[i];
    }

  return 0;
}


