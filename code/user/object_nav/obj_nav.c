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
float g_const = 100;

int set_obj(base_obj *obj, int mass, int xpos, int ypos, int zpos)
{
  obj->mass = mass;

  obj->xpos = xpos;
  obj->ypos = ypos;
  obj->zpos = zpos;

  obj->xvel = 0;
  obj->yvel = 0;
  obj->zvel = 0;
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

int init_objs(void)
{
  int i;
  base_obj *obj;

  for (i = 0 ; i < NUM_OBJS; i++)
    {
      obj = &bos[i];
      set_obj(obj, 0, 0, 0, 0);
    }
  return 0;
}
// gravity / distance ** 2
int calc_accels(int timep)
{
  int i,j;
  base_obj *obj1;
  base_obj *obj2;
  int xforce;
  int yforce;
  int zforce;
  for (i = 0 ; i < NUM_OBJS; i++)
    {
      obj1 = &bos[i];
      xforce = 0;
      yforce = 0;
      zforce = 0;
      
      for (j = 0 ; j < NUM_OBJS; j++)
	{

	  if ((j != i) && (bos[j].mass > 0))
	    {
	      obj2 = &bos[j];
	      int dist;
              int xf;
	      dist = obj2->xpos - obj1->xpos;
	      xf = 1;if (dist < 0 ) xf = -1; 
	      xforce += xf * (obj2->mass * g_const) / (dist * dist);
	      dist = obj2->ypos - obj1->ypos;
	      xf = 1;if (dist < 0 ) xf = -1; 
	      yforce += xf * (obj2->mass * g_const) / (dist * dist);
	      dist = obj2->zpos - obj1->zpos;
	      xf = 1;if (dist < 0 ) xf = -1; 
	      zforce += xf * (obj2->mass * g_const) / (dist * dist);
	      
	    }
	}
      if(obj1->mass > 0)printf(" Moving %d xf= %d yf=%d zf=%d\n", i ,xforce, yforce, zforce);
      move_obj(obj1, timep, xforce, yforce, zforce);

    }
  return 0;
}
 
int print_objs(int idx)
{
  int i;
  base_obj *obj;
  printf(" Positions at [%d] \n", idx);
  for (i = 0 ; i < NUM_OBJS; i++)
    {
      obj = &bos[i];
      if( obj->mass > 0)
	{
	  printf(" item [%d] x=%d, y=%d, z=%d\n"
		 , i
		 , obj->xpos
		 , obj->ypos
		 , obj->zpos
		 );

	}
    }
}
int main( int argc, char * argv[])
{
  int i;
  init_objs();
  set_obj(&bos[0], 100, 100, 100, 20);
  set_obj(&bos[1], 100, 200, 200, 40);
  print_objs(-1);
  for (i = 0; i < 16 ; i++)
    {
      calc_accels(1);
      print_objs(i);
    }
  return 0;
}
