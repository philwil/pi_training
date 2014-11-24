/*****************************************************************
 *
 * This code is the basis of a command string decoder.
 * we add data recieved ( say from a socket) to a buffer 
 * until we detect a terminating string sequence.
 * we then parse the string into comma seperated args and then decode 
 * the first arg against a table of commands.
 * the command buffer is reset to remove the initial command but leave any 
 * other commands that may have started.
 *   
 * this code can be tested out on an x86 system.
 *
 *****************************************************************/
 
/* try to run this with 

./strings " command 1 \n command2 " , " some data", " done \n"
*/
  

#include <stdio.h>
#include <string.h>
#include <malloc.h>

#define CMD_TERM "xx"
#define CMD_SEP ","
#define NUM_TAGS 1024
char cmd_str[1024];
char *cmd_tags[NUM_TAGS];

int decode_cmd(char *cmd_sp)
{
  int ix = 0;
  char *sp = cmd_sp;
  char *sep_sp;
  cmd_tags[ix]=sp;
  sep_sp = strstr(sp, CMD_SEP);
  while ((sep_sp != NULL) && ( ix < NUM_TAGS)) 
    {

      *sep_sp=0;
      printf(" found cmd_tag (%d) [%s]\n", ix, cmd_tags[ix]);
      ix++;
      sp = sep_sp + strlen(CMD_SEP);
      cmd_tags[ix]=sp;
      sep_sp = strstr(sp, CMD_SEP);
    }

  if ((ix > 0 ) || (ix == 0))
      printf(" found cmd_tag (%d) [%s]\n", ix, cmd_tags[ix]);

  return ix;
}

int process_cmd(char *cmd_sp, int len, char *new_sp)
{
  int rc=0;
  char *term_sp;
  char *cmd_cpy;
  char *cmd_rep;
  if(new_sp)
    {
      snprintf (&cmd_sp[strlen(cmd_sp)]
		, len - strlen(cmd_sp), "%s", new_sp);  
    }
  term_sp = strstr(cmd_sp, CMD_TERM);
  if (term_sp != NULL)
    {
      cmd_cpy=strdup(cmd_sp);
      cmd_cpy[term_sp-cmd_sp]=0;
      term_sp+=strlen(CMD_TERM);
      cmd_rep = cmd_sp;
      while(*term_sp) 
	{
	  *cmd_rep++ = *term_sp++;
	}
      *cmd_rep = 0;
      printf(" command found [%s] string left [%s] \n", cmd_cpy, cmd_sp);
      decode_cmd(cmd_cpy);
      free(cmd_cpy);
      rc = 1;
    }
  return rc;
}

/* next the extended main that adds the environment vars */
int main(int argc, char *argv[])
{
  int i;
  int rc;
  memset (cmd_str, 0, sizeof(cmd_str));
  char *sp;
  sp = &cmd_str[0];
  for (i = 1 ; i < argc; i++)
    {
      snprintf (&cmd_str[strlen(cmd_str)]
		, sizeof(cmd_str) - strlen(cmd_str), "%s", argv[i]);
    }
  printf(" working with cmd_str [%s] \n", cmd_str);

  cmd_str[0]=0;
  for (i = 1 ; i < argc; i++)
    {
      rc = 1;
      sp = argv[i];
      while (rc > 0)
	{ 
	  rc = process_cmd(cmd_str, sizeof(cmd_str), sp);
	  sp = NULL;
	}
    }
  printf(" test all done \n");
  return 0;
}

