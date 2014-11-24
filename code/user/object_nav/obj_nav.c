#include <stdio.h>
#include <stddef.h>
#include <math.h>
// basic test code for spacial navigation / placement
//  part 1
//  place some objects in a 3 d space
// give them mass and some motion
// calc gravitationasl forces to provide acceleration
// plot their movements over time.
//
// part 2 
// Shoot an object straight up and watch it fall
//

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


  double mass;


}base_obj;

#define NUM_OBJS 64
base_obj bos[NUM_OBJS];
float g_const = 6.67384E-11;

int set_obj(base_obj *obj, double mass, double xpos, double ypos, double zpos)
{
  obj->mass = mass*1000.0;

  obj->xpos = xpos;
  obj->ypos = ypos;
  obj->zpos = zpos;

  obj->xvel = 0;
  obj->yvel = 0;
  obj->zvel = 0;
}

int move_obj(base_obj *obj,  double timep, double xforce, double yforce, double zforce)
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
int calc_accels(double timep, double gconst)
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

	      distx = obj2->xpos - obj1->xpos;
	      disty = obj2->ypos - obj1->ypos;
	      distz = obj2->zpos - obj1->zpos;
              dist2 = (distx*distx)+(disty*disty)+(distz*distz);
              dist = sqrt(dist2);
	      g_force = gconst * (obj2->mass * obj1->mass) / dist2;

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
  printf( " First test simple movement\n");
  set_obj(&bos[0], 100, 0, 0, 0);
  bos[0].xvel=1;
  bos[0].yvel=1;
  set_obj(&bos[1], 100, 200, 200, 200);
  set_obj(&bos[2], 100, 0, 200, 200);
  print_objs(-1);
  for (i = 0; i < 16 ; i++)
    {
      calc_accels(10, g_const);
      print_objs(i);
    }

  printf( "\n\n Second watch an object rise and fall\n"
	  "Not quite working yet !\n");
  set_obj(&bos[0], 100000000000, 0, 0, 0);
  set_obj(&bos[1], 100, 10, 0, 0);
  set_obj(&bos[2], 0, 0, 0, 0);
  bos[0].xvel=0;bos[0].yvel=0;bos[0].zvel=0;
  bos[1].xvel=10;bos[1].yvel=0;bos[1].zvel=0;
  for (i = 0; i < 128 ; i++)
    {
      calc_accels(0.1, g_const);
      print_objs(i);
    }

  return 0;
}
