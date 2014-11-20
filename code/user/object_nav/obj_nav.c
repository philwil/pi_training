#include <stdio.h>
#include <stddef.h>
#include <math.h>

#include <unistd.h>
// some test fuunctions

typedef struct base_obj {

  double xpos;
  double ypos;
  double zpos;

  double xvel;
  double yvel;
  double zvel;

  double xacc;
  double yacc;
  double zacc;


  int mass;


}base_obj;

#define NUM_OBJS 64
base_obj bos[NUM_OBJS];
float g_const = 100;

int set_obj(base_obj *obj, int mass, double xpos, double ypos, double zpos)
{
  obj->mass = mass;

  obj->xpos = xpos;
  obj->ypos = ypos;
  obj->zpos = zpos;

  obj->xvel = 0;
  obj->yvel = 0;
  obj->zvel = 0;
}

int move_obj(base_obj *obj, double timep, double xforce, double yforce, double zforce)
{
  obj->xacc = 0;
  obj->yacc = 0;
  obj->zacc = 0;


  obj->xacc = xforce / obj->mass ;
  obj->yacc = yforce / obj->mass;
  obj->zacc = zforce / obj->mass;
  
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
  double xforce;
  double yforce;
  double zforce;
  for (i = 0 ; i < NUM_OBJS; i++)
    {
      obj1 = &bos[i];
      if (obj1->mass == 0) continue;
      xforce = 0;
      yforce = 0;
      zforce = 0;
      
      for (j = 0 ; j < NUM_OBJS; j++)
	{

	  if ((j != i) && (bos[j].mass > 0))
	    {
	      obj2 = &bos[j];
	      double dist, dist2, distx, disty, distz;
              double g_force;
              int xf;
	      distx = obj2->xpos - obj1->xpos;
	      disty = obj2->ypos - obj1->ypos;
	      distz = obj2->zpos - obj1->zpos;
              dist2 = (distx*distx)+(disty*disty)+(distz*distz);
              dist = sqrt(dist2);
	      g_force = (obj2->mass * g_const) / dist2;

	      xforce += (1 *  g_force * distx/dist);
	      yforce += (1 *  g_force * disty/dist);
	      zforce += (1 *  g_force * distz/dist);
	      
	    }
	}
      if(obj1->mass > 0)
	{
	  printf(" Moving %d xf= %f yf=%f zf=%f\n", i ,xforce, yforce, zforce);
	  move_obj(obj1, timep, xforce, yforce, zforce);
	}

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
	  printf(" item [%d] x=%f, y=%f, z=%f\n"
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
  set_obj(&bos[0], 100, 0, 0, 0);
  bos[0].xvel=1;
  bos[0].yvel=1;
  set_obj(&bos[1], 100, 200, 200, 200);
  set_obj(&bos[2], 100, 0, 200, 200);
  print_objs(-1);
  for (i = 0; i < 16 ; i++)
    {
      calc_accels(10);
      print_objs(i);
    }
  return 0;
}
