#include <stdio.h>
#include <stddef.h>
#include <unistd.h>
// some test fuunctions

typedef struct base_obj {

  int xpos;
  int ypos;
  int zpos;

  int xvel;
  int yvel;
  int zvel;

  int xacc;
  int yacc;
  int zacc;


  int mass;


}base_obj;

#define NUM_OBJS 64
base_obj bos[NUM_OBJS];

int set_obj(base_obj *obj, int mass, int xpos, int ypos, int zpos)
{
  obj->mass = mass;
  obj->xpos = xpos;
  obj->ypos = ypos;
  obj->zacc = zpos;
}

int move_obj(base_obj *obj, int timep, int xforce, int yforce, int zforce)
{
  obj->xacc = 0;
  obj->yacc = 0;
  obj->zacc = 0;

  if (xforce != 0)
    {
      obj->xacc = obj->mass / xforce;
    }
  if (yforce != 0)
    {
      obj->yacc = obj->mass / yforce;
    }
  if (zforce != 0)
    {
      obj->zacc = obj->mass / zforce;
    }

  obj->xvel += obj->xacc * timep;
  obj->yvel += obj->yacc * timep;
  obj->zvel += obj->zacc * timep;


  obj->xpos += obj->xvel * timep;
  obj->ypos += obj->yvel * timep;
  obj->zpos += obj->zvel * timep;
  return 0;
}


 
int main( int argc, char * argv[])
{
  return 0;
}
