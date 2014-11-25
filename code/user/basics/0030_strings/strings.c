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
 * extending this to include the $$commit/$$clear/$$add/$$show and $$del 
 * commands 
 *
 *****************************************************************/
 
/* try to run this with 

./strings " command 1 \n command2 " , " some data", " done \n"

./strings "cmd_2,this ,is, one, for mexx another command xx foo" " and xx one,more time,xx
*/

#include <stdio.h>
#include <string.h>
#include <malloc.h>

#define CMD_TERM "\n"
#define CMD_SEP ","
#define NUM_TAGS 1024
#define NUM_CMDS 1024
int cmd_num = 0;
char cmd_str[1024];
char *cmd_tags[NUM_TAGS];
char *cmd_list[NUM_CMDS];




typedef struct cmd_fcns {
  const char *key;
  int (*handler)(void *data, void *cdata, int argc, char **argv);
  void *cdata;
} cmd_fcns_t;

int run_cmd1(void *data, void *cdata, int cargc, char **cargv);
int run_cmd2(void *data, void *cdata, int cargc, char **cargv);
int run_cmd3(void *data, void *cdata, int cargc, char **cargv);
int run_cmd4(void *data, void *cdata, int cargc, char **cargv);

cmd_fcns_t cmds[] = {
  { "cmd_1", run_cmd1, NULL},
    { "cmd_2", run_cmd2, NULL},
    { "cmd_3", run_cmd3, NULL},
    { "cmd_4", run_cmd4, NULL},
    { NULL, NULL, NULL},
  };

int run_cmd1(void *data, void *cdata, int cargc, char **cargv)
{
  printf(" cmd [%s] running argc (%d)\n", cargv[0], cargc);
  return 0;
}

int run_cmd2(void * data, void *cdata, int cargc, char **cargv)
{
  printf(" cmd [%s] running argc (%d)\n", cargv[0], cargc);
  return 0;
}

int run_cmd3(void *data, void *cdata, int cargc, char **cargv)
{
  printf(" cmd [%s] running argc (%d)\n", cargv[0], cargc);
  return 0;
}

int run_cmd4(void *data, void *cdata, int cargc, char **cargv)
{
  printf(" cmd [%s] running argc (%d)\n", cargv[0], cargc);
  return 0;
}

 

int decode_cmd(char *cmd_sp , void *data)
{
  cmd_fcns_t *cmd;
  int ix = 0;

  char *sep_sp;
  char *sp;
  char *cmd_save = strdup(cmd_sp);
  printf(" running command [%s]\n", cmd_save);
  sp = cmd_save;

  cmd_tags[ix]=sp;
  sep_sp = strstr(sp, CMD_SEP);
  while ((sep_sp != NULL) && ( ix < NUM_TAGS)) 
    {
      *sep_sp=0;
      //printf(" found cmd_tag (%d) [%s]\n", ix, cmd_tags[ix]);
      ix++;
      sp = sep_sp + strlen(CMD_SEP);
      cmd_tags[ix]=sp;
      sep_sp = strstr(sp, CMD_SEP);
    }

  //if ((ix > 0 ) || (ix == 0))
  //    printf(" found cmd_tag (%d) [%s]\n", ix, cmd_tags[ix]);
  cmd = &cmds[0];
  while (cmd->key != NULL)
    {
      if(strcmp(cmd->key, cmd_tags[0])==0) 
	{
	  //printf(" ***found command [%s] \n", cmd->key);
	  cmd->handler(data, (void *)cmd, ix, cmd_tags);
	  break;
	}
      cmd++;
    }
  free(cmd_save);
  return ix;
}

int process_cmd(char *cmd_sp, int len, char *new_sp, void *data)
{
  int rc=0;
  char *term_sp;
  char *cmd_cpy;
  char *cmd_rep;
  int i;
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
      // handle special commands here
      //$$list
      //$$del
      if(strncmp(cmd_cpy, "$$list", strlen("$$list")) == 0 )
	{

	  printf( "command list\n");
	  for(i=0; i < cmd_num; i++)
	    {
	      printf(" command [%d] [%s]\n",i ,cmd_list[i]);
	    }
	  printf( "\n\n");
	}
      else if(strncmp(cmd_cpy, "$$del", strlen("$$del")) == 0 )
	{
	  for(i=0; i < cmd_num; i++)
	    {
	      free(cmd_list[i]);
	      cmd_list[i]=NULL;
	    }
	  cmd_num = 0;
	  printf( "command list deleted\n");
	}
      else if(strncmp(cmd_cpy, "$$run", strlen("$$run")) == 0 )
      {
	  for(i=0; i < cmd_num; i++)
	  {
	      decode_cmd(cmd_list[i], data);
	  }
      }
      else
      {

	  cmd_list[cmd_num] = cmd_cpy;
	  cmd_num++;
      }
      term_sp+=strlen(CMD_TERM);
      cmd_rep = cmd_sp;
      while(*term_sp) 
      {
	  *cmd_rep++ = *term_sp++;
      }
      *cmd_rep = 0;
      //printf(" command found [%s] string left [%s] \n", cmd_cpy, cmd_sp);
    
      //decode_cmd(cmd_cpy, data);
      //free(cmd_cpy);
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
  char buffer[1024];
  sp = &cmd_str[0];
  for (i = 1 ; i < argc; i++)
    {
      snprintf (&cmd_str[strlen(cmd_str)]
		, sizeof(cmd_str) - strlen(cmd_str), "%s", argv[i]);
    }
  //printf(" working with cmd_str [%s] \n", cmd_str);

  cmd_str[0]=0;
  for (i = 1 ; i < argc; i++)
    {
      rc = 1;
      sp = argv[i];
      while (rc > 0)
	{ 
	  rc = process_cmd(cmd_str, sizeof(cmd_str), sp, NULL);
	  sp = NULL;
	}
    }
  //printf(" test all done \n");
  while (fgets(buffer, sizeof (buffer), stdin))
    {
      //printf(" You entered [%s] \n" ,buffer);
      rc = process_cmd(cmd_str, sizeof(cmd_str), buffer, NULL);
      
    } 
  return 0;
}


