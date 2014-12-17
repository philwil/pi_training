/*
   Copyright (C) 2006 Mauro Carvalho Chehab <mchehab@infradead.org>

   The libv4l2util Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The libv4l2util Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.
  */

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <stdlib.h>

#include "v4l2_driver.h"
#include <linux/videodev2.h>
#include "struct_dump.h"
#include "struct-v4l2.h"

typedef struct flag_def {
    int flag;
    char *str;
}flag_def;

/****************************************************************************
	Auxiliary routines
 ****************************************************************************/
static int xioctl (int fd, unsigned long int request, void *arg)
{
    int r;
  
    do r = ioctl (fd, request, arg);
    while (-1 == r && EINTR == errno);
    
    return r;
}

static void free_list(struct drv_list **list_ptr)
{
    struct drv_list *prev,*cur;
  
    if (list_ptr==NULL)
        return;
    
    prev=*list_ptr;
    if (prev==NULL)
        return;

    do 
      {
        cur=prev->next;
	if (prev->curr)
	  free (prev->curr);	// Free data
	free (prev);			// Free list
	prev=cur;
      } while (prev);
    
    *list_ptr=NULL;
}

/****************************************************************************
	Auxiliary Arrays to aid debug messages
 ****************************************************************************/
char *v4l2_field_names[] = {
  [V4L2_FIELD_ANY]        = "any",
  [V4L2_FIELD_NONE]       = "none",
  [V4L2_FIELD_TOP]        = "top",
  [V4L2_FIELD_BOTTOM]     = "bottom",
  [V4L2_FIELD_INTERLACED] = "interlaced",
  [V4L2_FIELD_SEQ_TB]     = "seq-tb",
  [V4L2_FIELD_SEQ_BT]     = "seq-bt",
  [V4L2_FIELD_ALTERNATE]  = "alternate",
};

char *v4l2_type_names[] = {
  [V4L2_BUF_TYPE_VIDEO_CAPTURE]      = "video-cap",
  [V4L2_BUF_TYPE_VIDEO_OVERLAY]      = "video-over",
  [V4L2_BUF_TYPE_VIDEO_OUTPUT]       = "video-out",
  [V4L2_BUF_TYPE_VBI_CAPTURE]        = "vbi-cap",
  [V4L2_BUF_TYPE_VBI_OUTPUT]         = "vbi-out",
  [V4L2_BUF_TYPE_SLICED_VBI_CAPTURE] = "sliced-vbi-cap",
  [V4L2_BUF_TYPE_SLICED_VBI_OUTPUT]  = "slicec-vbi-out",
};

static char *v4l2_memory_names[] = {
  [V4L2_MEMORY_MMAP]    = "mmap",
  [V4L2_MEMORY_USERPTR] = "userptr",
  [V4L2_MEMORY_OVERLAY] = "overlay",
};

#define ARRAY_SIZE(arr) (sizeof(arr)/sizeof(*arr))
#define prt_names(a,arr) (((a)<ARRAY_SIZE(arr))?arr[a]:"unknown")

static char *prt_caps(uint32_t caps)
{
    static char s[4096]="";
  
    if (V4L2_CAP_VIDEO_CAPTURE & caps)
      strcat (s,"CAPTURE ");
    if (V4L2_CAP_VIDEO_CAPTURE_MPLANE & caps)
      strcat (s,"CAPTURE_MPLANE ");
    if (V4L2_CAP_VIDEO_OUTPUT & caps)
      strcat (s,"OUTPUT ");
    if (V4L2_CAP_VIDEO_OUTPUT_MPLANE & caps)
      strcat (s,"OUTPUT_MPLANE ");
    if (V4L2_CAP_VIDEO_M2M & caps)
      strcat (s,"M2M ");
    if (V4L2_CAP_VIDEO_M2M_MPLANE & caps)
      strcat (s,"M2M_MPLANE ");
    if (V4L2_CAP_VIDEO_OVERLAY & caps)
      strcat (s,"OVERLAY ");
    if (V4L2_CAP_VBI_CAPTURE & caps)
      strcat (s,"VBI_CAPTURE ");
    if (V4L2_CAP_VBI_OUTPUT & caps)
      strcat (s,"VBI_OUTPUT ");
    if (V4L2_CAP_SLICED_VBI_CAPTURE & caps)
      strcat (s,"SLICED_VBI_CAPTURE ");
    if (V4L2_CAP_SLICED_VBI_OUTPUT & caps)
      strcat (s,"SLICED_VBI_OUTPUT ");
    if (V4L2_CAP_RDS_CAPTURE & caps)
      strcat (s,"RDS_CAPTURE ");
    if (V4L2_CAP_RDS_OUTPUT & caps)
      strcat (s,"RDS_OUTPUT ");
    //if (V4L2_CAP_SDR_CAPTURE & caps)
    //	strcat (s,"SDR_CAPTURE ");
    if (V4L2_CAP_TUNER & caps)
      strcat (s,"TUNER ");
    if (V4L2_CAP_HW_FREQ_SEEK & caps)
      strcat (s,"HW_FREQ_SEEK ");
    if (V4L2_CAP_MODULATOR & caps)
      strcat (s,"MODULATOR ");
    if (V4L2_CAP_AUDIO & caps)
      strcat (s,"AUDIO ");
    if (V4L2_CAP_RADIO & caps)
      strcat (s,"RADIO ");
    if (V4L2_CAP_READWRITE & caps)
      strcat (s,"READWRITE ");
    if (V4L2_CAP_ASYNCIO & caps)
      strcat (s,"ASYNCIO ");
    if (V4L2_CAP_STREAMING & caps)
      strcat (s,"STREAMING ");
    //	if (V4L2_CAP_EXT_PIX_FORMAT & caps)
	//	strcat (s,"EXT_PIX_FORMAT ");
    if (V4L2_CAP_DEVICE_CAPS & caps)
      strcat (s,"DEVICE_CAPS ");
    
    return s;
}

static void prt_buf_info(char *name, struct v4l2_buffer *p)
{
    struct v4l2_timecode *tc=&p->timecode;
    
    printf ("%s: %02ld:%02d:%02d.%08ld index=%d, type=%s, "
	    "bytesused=%d, flags=0x%08x, "
	    "field=%s, sequence=%d, memory=%s, offset=0x%08x, length=%d\n",
	    name, (p->timestamp.tv_sec/3600),
	    (int)(p->timestamp.tv_sec/60)%60,
	    (int)(p->timestamp.tv_sec%60),
	    p->timestamp.tv_usec,
	    p->index,
	    prt_names(p->type,v4l2_type_names),
	    p->bytesused,p->flags,
	    prt_names(p->field,v4l2_field_names),
	    p->sequence,
	    prt_names(p->memory,v4l2_memory_names),
	    p->m.offset,
	    p->length);
    tc=&p->timecode;
    printf ("\tTIMECODE: %02d:%02d:%02d type=%d, "
	    "flags=0x%08x, frames=%d, userbits=0x%02x%02x%02x%02x\n",
	    tc->hours,tc->minutes,tc->seconds,
	    tc->type, tc->flags, tc->frames,
	    tc->userbits[0],
	    tc->userbits[1],
	    tc->userbits[2],
	    tc->userbits[3]);
}

static char *num2s(unsigned num);

char *buftype2s(int type)
{
    static char s[128]; 
    switch (type) {
    case 0:
      return "Invalid";
    case V4L2_BUF_TYPE_VIDEO_CAPTURE:
      return "Video Capture";
    case V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE:
      return "Video Capture Multiplanar";
    case V4L2_BUF_TYPE_VIDEO_OUTPUT:
      return "Video Output";
    case V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE:
      return "Video Output Multiplanar";
    case V4L2_BUF_TYPE_VIDEO_OVERLAY:
      return "Video Overlay";
    case V4L2_BUF_TYPE_VBI_CAPTURE:
      return "VBI Capture";
    case V4L2_BUF_TYPE_VBI_OUTPUT:
      return "VBI Output";
    case V4L2_BUF_TYPE_SLICED_VBI_CAPTURE:
      return "Sliced VBI Capture";
    case V4L2_BUF_TYPE_SLICED_VBI_OUTPUT:
      return "Sliced VBI Output";
    case V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY:
      return "Video Output Overlay";
    case V4L2_BUF_TYPE_PRIVATE:
    return "Private";
    default:
      snprintf(s, sizeof(s),"Unknown ( %s )", num2s(type)) ;  
      return s;
    }
}

static char *num2s(unsigned num)
{
    static   char buf[10];

    sprintf(buf, "%08x", num);
    return buf;
}


static char * fcc2s(unsigned int val)
{
    static char s[128];
    char * sp = &s[0];
    s[0]=0;
    *sp++ = val & 0xff;
    *sp++ = (val >> 8) & 0xff;
    *sp++ = (val >> 16) & 0xff;
    *sp++ = (val >> 24) & 0xff;
    return s;
}

//static 
char *field2s(int val)
{
    static char s[128];
    switch (val) {
    case V4L2_FIELD_ANY:
      return "Any";
    case V4L2_FIELD_NONE:
      return "None";
    case V4L2_FIELD_TOP:
      return "Top";
    case V4L2_FIELD_BOTTOM:
      return "Bottom";
    case V4L2_FIELD_INTERLACED:
      return "Interlaced";
    case V4L2_FIELD_SEQ_TB:
      return "Sequential Top-Bottom";
    case V4L2_FIELD_SEQ_BT:
      return "Sequential Bottom-Top";
    case V4L2_FIELD_ALTERNATE:
      return "Alternating";
    case V4L2_FIELD_INTERLACED_TB:
      return "Interlaced Top-Bottom";
    case V4L2_FIELD_INTERLACED_BT:
      return "Interlaced Bottom-Top";
    default:
      snprintf(s, sizeof(s),"Unknown ( %s )", num2s(val)) ;  
      return s;
    }
}

char * colorspace2s(int val)
{
  static char s[128];
  switch (val) {
  case V4L2_COLORSPACE_SMPTE170M:
    return "Broadcast NTSC/PAL (SMPTE170M/ITU601)";
  case V4L2_COLORSPACE_SMPTE240M:
    return "1125-Line (US) HDTV (SMPTE240M)";
  case V4L2_COLORSPACE_REC709:
    return "HDTV and modern devices (ITU709)";
  case V4L2_COLORSPACE_BT878:
    return "Broken Bt878";
  case V4L2_COLORSPACE_470_SYSTEM_M:
    return "NTSC/M (ITU470/ITU601)";
  case V4L2_COLORSPACE_470_SYSTEM_BG:
    return "PAL/SECAM BG (ITU470/ITU601)";
  case V4L2_COLORSPACE_JPEG:
    return "JPEG (JFIF/ITU601)";
  case V4L2_COLORSPACE_SRGB:
    return "SRGB";
  default:
    snprintf(s, sizeof(s),"Unknown ( %s )", num2s(val)) ;  
    return s;
	     
  }
}

char *flags2s(unsigned val, const flag_def *def)
{
    static char s[256];
    s[0]=0;
    char *sp = &s[0];
    while (def->flag) {
      if (val & def->flag) {
	if (strlen(s)) 
	  strcat(s,", ");
	strcat(s,def->str);
	val &= ~def->flag;
      }
      def++;
    }
  if (val) {
    if (strlen(s)) 
      strcat(s,", ");
    strcat(s,num2s(val));
  }
  return s;
}


static char *frmtype2s(unsigned type)
{
  static char *types[] = {
    "Unknown",
    "Discrete",
    "Continuous",
    "Stepwise"
  };
  
  if (type > 3)
    type = 0;
  return types[type];
}

static char *fract2sec(const struct v4l2_fract * f)
{
  static char buf[100];
  
  sprintf(buf, "%.3f s", (1.0 * f->numerator) / f->denominator);
  return buf;
}

static char *fract2fps(const struct v4l2_fract *f)
{
  static char buf[100];

  sprintf(buf, "%.3f fps", (1.0 * f->denominator) / f->numerator);
  return buf;
}


static void print_frmsize(const struct v4l2_frmsizeenum *frmsize, 
			  const char *prefix)
{
  printf("%s\tSize: %s ", prefix, frmtype2s(frmsize->type));
  if (frmsize->type == V4L2_FRMSIZE_TYPE_DISCRETE) {
    printf("%dx%d", frmsize->discrete.width, frmsize->discrete.height);
  } else if (frmsize->type == V4L2_FRMSIZE_TYPE_STEPWISE) {
    printf("%dx%d - %dx%d with step %d/%d",
	   frmsize->stepwise.min_width,
	   frmsize->stepwise.min_height,
	   frmsize->stepwise.max_width,
	   frmsize->stepwise.max_height,
	   frmsize->stepwise.step_width,
	   frmsize->stepwise.step_height);
  }
  printf("\n");
}

static void print_frmival(const struct v4l2_frmivalenum *frmival, const char *prefix)
{
    printf("%s\tInterval: %s ", prefix, frmtype2s(frmival->type));
    if (frmival->type == V4L2_FRMIVAL_TYPE_DISCRETE) {
        printf("%s (%s)\n", fract2sec(&frmival->discrete),
	       fract2fps(&frmival->discrete));
    } else if (frmival->type == V4L2_FRMIVAL_TYPE_STEPWISE) {
        printf("%s - %s with step %s\n",
	       fract2sec(&frmival->stepwise.min),
	       fract2sec(&frmival->stepwise.max),
	       fract2sec(&frmival->stepwise.step));
	printf("%s\t            : ", prefix);
	printf("(%s - %s with step %s)\n",
	       fract2fps(&frmival->stepwise.min),
	       fract2fps(&frmival->stepwise.max),
	       fract2fps(&frmival->stepwise.step));
    }
}

static const flag_def fmtdesc_def[] = {
    { V4L2_FMT_FLAG_COMPRESSED, "compressed" },
    { V4L2_FMT_FLAG_EMULATED, "emulated" },
    { 0, NULL }
};

char *fmtdesc2s(unsigned flags)
{
    return flags2s(flags, fmtdesc_def);
}

//types are:
// V4L2_BUF_TYPE_VIDEO_CAPTURE   
// V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE

typedef struct v4l2_fram
{
    int width;
    int height;
    unsigned int pixel_format;
    unsigned int field;
    float nom;

    float den;
    float fps;

} v4l2_fram_t;

v4l2_fram_t v4l2_fram[1024];

int fram_ix = 0;  

static void print_fram(void)
{

    int ix = 0;
    printf(" Fram options available .. pick one fram_ix %d\n", fram_ix);
    v4l2_fram_t *fram;
    while (ix < fram_ix)
    {
	fram = &v4l2_fram[ix];
	if(ix <400)printf("ix [%2d] %4d X %4d %f %f %f %04x\n"
	       , ix
	       , fram->width
	       , fram->height
	       , fram->nom
	       , fram->den
	       , fram->fps
	       , fram->pixel_format
	       );
	
	ix++;
    }
}

static void print_video_formats_ext(int fd, enum v4l2_buf_type type)
{
    struct v4l2_fmtdesc fmt;
    struct v4l2_frmsizeenum frmsize;
    struct v4l2_frmivalenum frmival;
    struct v4l2_fram *fram;
    
    fram_ix = 0;  
    fmt.index = 0;
    fmt.type = type;
    while (xioctl(fd, VIDIOC_ENUM_FMT, &fmt) >= 0) {
        printf("\tIndex       : %d\n", fmt.index);
	printf("\tType        : %s\n", buftype2s(type));
	printf("\tPixel Format: '%s'", fcc2s(fmt.pixelformat));
	if (fmt.flags)
	  printf(" (%s)", fmtdesc2s(fmt.flags));
	printf("\n");
	printf("\tName        : %s\n", fmt.description);
	frmsize.pixel_format = fmt.pixelformat;
	frmsize.index = 0;
	while (xioctl(fd, VIDIOC_ENUM_FRAMESIZES, &frmsize) >= 0) {
	  print_frmsize(&frmsize, "\t");
	  if (frmsize.type == V4L2_FRMSIZE_TYPE_DISCRETE) {
	    frmival.index = 0;
	    frmival.pixel_format = fmt.pixelformat;
	    frmival.width = frmsize.discrete.width;
	    frmival.height = frmsize.discrete.height;
	    
	    while (xioctl(fd, VIDIOC_ENUM_FRAMEINTERVALS, &frmival) >= 0) {
	      fram = &v4l2_fram[fram_ix];
	      fram->width=frmival.width;
	      fram->height=frmival.height;
	      fram->pixel_format = frmival.pixel_format;
	      fram->nom = frmival.discrete.numerator;
	      fram->den = frmival.discrete.denominator;
	      fram->fps = (1.0 * fram->den) / fram->nom;
	      //fram->field = frmival.field;
	      if (fram_ix < 1024)fram_ix++;
	      //sprintf(buf, "%.3f s", (1.0 * f->numerator) / f->denominator);
	      print_frmival(&frmival, "\t\t");
	      frmival.index++;
	    }
	  }
	  frmsize.index++;
	}
	printf("\n");
	fmt.index++;
    }
    return;
}


/****************************************************************************
	Open V4L2 devices
 ****************************************************************************/
int v4l2_open (char *device, int debug, struct v4l2_driver *drv)
{
  int ret;
  
  memset(drv,0,sizeof(*drv));
  
  drv->debug=debug;
  
  if ((drv->fd = open(device, O_RDWR/* | O_NONBLOCK */)) < 0) {
    return(-errno);
  }
  
  ret=xioctl(drv->fd,VIDIOC_QUERYCAP,(void *) &drv->cap);
  if (!ret && drv->debug) {
    printf ("driver=%s, card=%s, bus=%s, version=%d.%d.%d, "
	    "capabilities=%s, device_caps=%s\n",
	    drv->cap.driver,drv->cap.card,drv->cap.bus_info,
	    (drv->cap.version >> 16) & 0xff,
	    (drv->cap.version >>  8) & 0xff,
	    drv->cap.version         & 0xff,
	    prt_caps(drv->cap.capabilities),
	    (drv->cap.capabilities & V4L2_CAP_DEVICE_CAPS) ?
	    prt_caps(drv->cap.device_caps) : "N/A");
    
    
  }
  return ret;
}


/****************************************************************************
	V4L2 Enumerations
 ****************************************************************************/
int v4l2_enum_stds (struct v4l2_driver *drv)
{
	struct v4l2_standard	*p=NULL;
	struct drv_list		*list;
	int			ok=0,i;

	free_list(&drv->stds);

	list=drv->stds=calloc(1,sizeof(*drv->stds));
	assert (list!=NULL);

	for (i=0; ok==0; i++) {
		p=calloc(1,sizeof(*p));
		assert (p);

		p->index=i;
		ok=xioctl(drv->fd,VIDIOC_ENUMSTD,p);
		if (ok<0) {
			ok=-errno;
			free(p);
			break;
		}
		if (1 || drv->debug) {
			printf ("STANDARD: index=%d, id=0x%08x, name=%s, fps=%.3f, "
				"framelines=%d\n", p->index,
				(unsigned int)p->id, p->name,
				1.*p->frameperiod.denominator/p->frameperiod.numerator,
				p->framelines);
		}
		if (list->curr) {
			list->next=calloc(1,sizeof(*list->next));
			list=list->next;
			assert (list!=NULL);
		}
		list->curr=p;
	}
	if (i>0 && ok==-EINVAL)
		return 0;

	return ok;
}

int v4l2_enum_input (struct v4l2_driver *drv)
{
	struct v4l2_input	*p=NULL;
	struct drv_list		*list;
	int			ok=0,i;

	free_list(&drv->inputs);

	list=drv->inputs=calloc(1,sizeof(*drv->inputs));
	assert (list!=NULL);

	for (i=0; ok==0; i++) {
		p=calloc(1,sizeof(*p));
		assert (p);
		p->index=i;
		ok=xioctl(drv->fd,VIDIOC_ENUMINPUT,p);
		if (ok<0) {
			ok=errno;
			free(p);
			break;
		}
		if (drv->debug) {
			printf ("INPUT: index=%d, name=%s, type=%d, audioset=%d, "
				"tuner=%d, std=%08x, status=%d\n",
				p->index,p->name,p->type,p->audioset, p->tuner,
				(unsigned int)p->std, p->status);
		}
		if (list->curr) {
			list->next=calloc(1,sizeof(*list->next));
			list=list->next;
			assert (list!=NULL);
		}
		list->curr=p;
	}
	if (i>0 && ok==-EINVAL)
		return 0;
	return ok;
}

int v4l2_enum_fmt (struct v4l2_driver *drv, enum v4l2_buf_type type)
{
	struct v4l2_fmtdesc 	*p=NULL;
	struct drv_list		*list;
	int			ok=0,i;

	free_list(&drv->fmt_caps);

	list=drv->fmt_caps=calloc(1,sizeof(*drv->fmt_caps));
	assert (list!=NULL);

	for (i=0; ok==0; i++) {
		p=calloc(1,sizeof(*p));
		assert (p!=NULL);

		p->index=i;
		p->type =type;

		ok=xioctl(drv->fd,VIDIOC_ENUM_FMT,p);
		if (ok<0) {
			ok=errno;
			free(p);
			break;
		}
		if (drv->debug) {
			printf ("FORMAT: index=%d, type=%d, flags=%d, description='%s'\n\t"
				"fourcc=%c%c%c%c\n",
				p->index, p->type, p->flags,p->description,
				p->pixelformat & 0xff,
				(p->pixelformat >>  8) & 0xff,
				(p->pixelformat >> 16) & 0xff,
				(p->pixelformat >> 24) & 0xff
				);
		}
		if (list->curr) {
			list->next=calloc(1,sizeof(*list->next));
			list=list->next;
			assert (list!=NULL);
		}
		list->curr=p;
	}
	if (i>0 && ok==-EINVAL)
		return 0;
	return ok;
}

/****************************************************************************
	Set routines - currently, it also checks results with Get
 ****************************************************************************/
int v4l2_setget_std (struct v4l2_driver *drv, enum v4l2_direction dir, v4l2_std_id *id)
{
	v4l2_std_id		s_id=*id;
	int			ret=0;
	char			s[256];

	if (dir & V4L2_SET) {
		ret=xioctl(drv->fd,VIDIOC_S_STD,&s_id);
		if (ret<0) {
			ret=errno;

			sprintf (s,"while trying to set STD to %08x",
								(unsigned int) *id);
			perror(s);
		}
	}

	if (dir & V4L2_GET) {
		ret=xioctl(drv->fd,VIDIOC_G_STD,&s_id);
		if (ret<0) {
			ret=errno;
			perror ("while trying to get STD id");
		}
	}

	if (dir == V4L2_SET_GET) {
		if (*id & s_id) {
			if (*id != s_id) {
				printf ("Warning: Received a std subset (%08x"
					" std) while trying to adjust to %08x\n",
					(unsigned int) s_id,(unsigned int) *id);
			}
		} else {
			fprintf (stderr,"Error: Received %08x std while trying"
				" to adjust to %08x\n",
				(unsigned int) s_id,(unsigned int) *id);
		}
	}
	return ret;
}

int v4l2_setget_input (struct v4l2_driver *drv, enum v4l2_direction dir, struct v4l2_input *input)
{
	int			ok=0,ret;
	unsigned int		inp=input->index;
	char			s[256];

	if (dir & V4L2_SET) {
		ret=xioctl(drv->fd,VIDIOC_S_INPUT,input);
		if (ret<0) {
			ret=errno;
			sprintf (s,"while trying to set INPUT to %d\n", inp);
			perror(s);
		}
	}

	if (dir & V4L2_GET) {
		ret=xioctl(drv->fd,VIDIOC_G_INPUT,input);
		if (ret<0) {
			perror ("while trying to get INPUT id\n");
		}
	}

	if (dir & V4L2_SET_GET) {
		if (input->index != inp) {
			printf ("Input is different than expected (received %i, set %i)\n",
						inp, input->index);
		}
	}

	return ok;
}

/*
Setting frame rates

use the VIDIOC_G_PARM ioctl and check the 
v4l2_streamparm.parm.capture.capability member to find out whether the 
driver allows V4L2_CAP_TIMEPERFRAME.
if so, use the VIDIOC_ENUM_FRAMEINTERVALS ioctl to get the list of 
possible frame intervals (inverse of framerates), 
in the form of v4l2_fract structures
use these values with the VIDIOC_S_PARM ioctl 
and fill in the v4l2_streamparm.parm.capture.timeperframe member.
*/

int v4l2_get_frame_rate(struct v4l2_driver *drv)
{
    int ret = 0;
    struct v4l2_streamparm streamparm;
    struct v4l2_fract *tpf;
    memset (&streamparm, 0, sizeof (streamparm));
    streamparm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    tpf = &streamparm.parm.capture.timeperframe;
    
    if (xioctl(drv->fd, VIDIOC_G_PARM, &streamparm) < 0) {
       printf(" error getting streamparmm\n");
      // we have an error
      ret = -1;
    }
   
    else
      {
	printf("The V4L2 driver time per frame is %d/%d\n",
	       tpf->numerator, tpf->denominator);
      }
    return ret;
}

int v4l2_set_frame_rate(struct v4l2_driver *drv, int num, int den)
{
    int ret = 0;
    struct v4l2_streamparm streamparm;
    struct v4l2_fract *tpf;
    memset (&streamparm, 0, sizeof (streamparm));
    streamparm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    tpf = &streamparm.parm.capture.timeperframe;
    tpf->numerator = num;
    tpf->denominator = den;
    
    if (xioctl(drv->fd, VIDIOC_S_PARM, &streamparm) < 0) {
      // we have an error
      ret = -1;
    }
   
    if (den != tpf->denominator ||
	num != tpf->numerator) {
      printf("The V4L2 driver changed the time per frame from %d/%d to %d/%d\n",
	     num, den,
	     tpf->numerator, tpf->denominator);
      ret = -1;
    }
    
    //Can the ioctl succeed even if a different frame rate was set?  Weird
    //interface...
    return ret;
}


int v4l2_gettryset_fmt_cap (struct v4l2_driver *drv, enum v4l2_direction dir,
		      struct v4l2_format *fmt,uint32_t width, uint32_t height,
		      uint32_t pixelformat, enum v4l2_field field)
{
    struct v4l2_pix_format  *pix=&(fmt->fmt.pix);
    int			ret=0;

    fmt->type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (dir == V4L2_GET) {
      ret=xioctl(drv->fd,VIDIOC_G_FMT,fmt);
      if (ret < 0) {
	ret=errno;
	perror("VIDIOC_G_FMT failed\n");
      }
      return ret;
    } else if (dir & (~(V4L2_TRY|V4L2_SET)) ) {
      perror ("Invalid direction\n");
      return EINVAL;
    }
    
    if (dir & (V4L2_TRY|V4L2_SET)) {
      pix->width       = width;
      pix->height      = height;
      pix->pixelformat = pixelformat;
      pix->field       = field;
      /*
	enum v4l2_colorspace	colorspace;
      */
      
      if (dir & V4L2_TRY) {
	ret=xioctl(drv->fd,VIDIOC_TRY_FMT, fmt);
	if (ret < 0) {
	  perror("VIDIOC_TRY_FMT failed\n");
	}
	else
	  {
	    drv->sizeimage=pix->sizeimage;
	  }
      }
      
      if (dir & V4L2_SET) {
	ret=xioctl(drv->fd,VIDIOC_S_FMT,fmt);
	if (ret < 0) {
	  perror("VIDIOC_S_FMT failed\n");
	}
	drv->sizeimage=pix->sizeimage;
      }
      
      if (pix->pixelformat != pixelformat) {
	printf("Error: asked for format %d, received %d",pixelformat,
	       pix->pixelformat);
      }
      
      if (pix->width != width) {
	printf("Error: asked for format %d, received %d\n",width,
	       pix->width);
      }
      
      if (pix->height != height) {
	printf("Error: asked for format %d, received %d\n",height,
	       pix->height);
      }
      
      // Not set for compressed formats
      //if (pix->bytesperline == 0 ) {
      //  printf("Error: bytesperline = 0\n");
      //}
      
      if (pix->sizeimage == 0 ) {
	printf("Error: sizeimage = 0\n");
      }
    }
    
    if (drv->debug)
      printf( "FMT SET: %dx%d, fourcc=%c%c%c%c, %d bytes/line,"
	      " %d bytes/frame, colorspace=0x%08x\n",
	      pix->width,pix->height,
	      pix->pixelformat & 0xff,
	      (pix->pixelformat >>  8) & 0xff,
	      (pix->pixelformat >> 16) & 0xff,
	      (pix->pixelformat >> 24) & 0xff,
	      pix->bytesperline,
	      pix->sizeimage,
	      pix->colorspace);
    
    return 0;
}

/****************************************************************************
	Get routines
 ****************************************************************************/
int v4l2_get_parm (struct v4l2_driver *drv)
{
    int ret;
    struct v4l2_captureparm *c;
    
    drv->parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if ((ret=xioctl(drv->fd,VIDIOC_G_PARM,&drv->parm))>=0) 
      {
	c=&drv->parm.parm.capture;
	printf ("PARM: capability=%d, capturemode=%d, %.3f fps "
		"ext=%x, readbuf=%d\n",
		c->capability,
		c->capturemode,
		c->timeperframe.denominator*1./c->timeperframe.numerator,
		c->extendedmode, c->readbuffers);
      }
    else 
      {
	ret=errno;
	
	perror ("VIDIOC_G_PARM");
      }
    
    return ret;
}

/****************************************************************************
	Queue and stream control
 ****************************************************************************/

int v4l2_free_bufs(struct v4l2_driver *drv)
{
    unsigned int i;
    int rc=-1; 
    enum v4l2_buf_type type;

    printf(" free_buffs n_bufs %d\n", drv->n_bufs);

    if (drv->n_bufs == 0)
        return 0;

    memset(&drv->reqbuf, 0 , sizeof (struct v4l2_requestbuffers));
    
    /* Requests the driver to free all buffers */
    drv->reqbuf.count  = 0;
    drv->reqbuf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    drv->reqbuf.memory = V4L2_MEMORY_MMAP;
    type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    
    /* stop capture */
    printf(" before stop capture fd (%d) rc (%d)\n", drv->fd, rc);

    //rc = xioctl(drv->fd, VIDIOC_STREAMOFF, &drv->reqbuf.type);
    rc = xioctl(drv->fd, VIDIOC_STREAMOFF, &type);
    printf(" after stop capture rc (%d)\n", rc);
    if (rc<0)
      return errno;

    sleep (1);	// FIXME: Should check if all buffers are stopped
    
    /* V4L2 API says REQBUFS with count=0 should be used to release buffer.
       However, video-buf.c doesn't implement it.
    */
#if 0
    if (xioctl(drv->fd,VIDIOC_REQBUFS,&drv->reqbuf)<0) {
      printf("reqbufs error while freeing buffers\n");
      perror("reqbufs while freeing buffers");
      return errno;
    }
#endif
    
    if (drv->reqbuf.count != 0) 
      {
	fprintf(stderr,"REQBUFS returned %d buffers while asking for freeing it!\n",
		drv->reqbuf.count);
      }

    for (i = 0; i < drv->n_bufs; i++) 
      {
	if (drv->bufs[i].length)
	  {
	    munmap(drv->bufs[i].start, drv->bufs[i].length);
	  }
	if (drv->v4l2_buffers[i])
	  {
	    free (drv->v4l2_buffers[i]);
	  }
      }
    free(drv->v4l2_buffers);
    free(drv->bufs);

    drv->v4l2_buffers=NULL;
    drv->bufs=NULL;
    drv->n_bufs=0;

    return 0;
} 

//        fmt->fmt.pix.pixelformat=0x47504a4d;  // "MJPG" 
//        fmt->fmt.pix.pixelformat=V4L2_PIX_FMT_JPEG  // "MJPG" 
int v4l2_set_fmt(struct v4l2_driver *drv, struct v4l2_format *fmt)
{
    int rc;
    printf(" ******************open_v4l setting format %08x\n"
	   , fmt->fmt.pix.pixelformat);
        //                            G P J M
        //vd->fmt.fmt.pix.pixelformat=0x47504a4d;  // "MJPG" 
    rc = xioctl (drv->fd, VIDIOC_S_FMT, fmt);
    printf(" VIDEOC_S_FMT fmt fd %d rc_set %d\n", drv->fd, rc);
    rc = xioctl (drv->fd, VIDIOC_G_FMT, fmt);
    printf(" after setting VIDEOC_G_FMT fmt fd %d rc %d\n"
	   , drv->fd, rc);
    printf("              fmt->fmt.pix.width %d\n"
	   , fmt->fmt.pix.width);
    printf("              fmt->fmt.pix.height %d\n"
	   , fmt->fmt.pix.height);
    printf("              fmt->fmt.pix.pixelformat %08x\n"
	   , fmt->fmt.pix.pixelformat);
    return rc;
}

  
//  struct v4l2_format fmt;
int v4l2_get_fmt(struct v4l2_driver *drv, struct v4l2_format *fmt)
{
    int rc;
    memset(fmt, 0 , sizeof(*fmt));   
    fmt->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    rc = xioctl (drv->fd, VIDIOC_G_FMT, fmt);

    printf(" VIDEOC_G_FMT fmt fd %d rc %d\n", drv->fd, rc);
    printf("              fmt.pix.width %d\n", fmt->fmt.pix.width);
    printf("              fmt.fmt.pix.height %d\n", fmt->fmt.pix.height);
    printf("              fmt.fmt.pix.field %d\n", fmt->fmt.pix.field);
    printf("              fmt.fmt.pix.pixelformat %08x (YUYV %08x)\n"
	   , fmt->fmt.pix.pixelformat, V4L2_PIX_FMT_YUYV
	   );
    printf("              fmt.pix.field %04x %d\n"
              , fmt->fmt.pix.field, fmt->fmt.pix.field);
    printf("              fmt.pix.bytesperline %d\n"
              , fmt->fmt.pix.bytesperline);

    printf("              image size %d \n"
	   , fmt->fmt.pix.bytesperline
	   * fmt->fmt.pix.height);
    return rc;
}

int v4l2_mmap_bufs(struct v4l2_driver *drv, unsigned int num_buffers)
{
  /* Frees previous allocations, if required */
  v4l2_free_bufs(drv);
  printf(" drv->sizeimage 1 (%d)\n", drv->sizeimage);

  if (drv->sizeimage==0) 
    {
      printf("Image size is zero! Can't proceed\n");
      return -1;
    }

  /* Requests the specified number of buffers */
  drv->reqbuf.count  = num_buffers;
  drv->reqbuf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  drv->reqbuf.memory = V4L2_MEMORY_MMAP;
  
  if (xioctl(drv->fd, VIDIOC_REQBUFS, &drv->reqbuf)<0) 
    {
      printf("reqbufs failed ..\n");
      perror("reqbufs");
      return -1;
    }
  
  if (drv->debug)
    {
      printf ("REQBUFS: count=%d, type=%s, memory=%s\n",
	      drv->reqbuf.count,
	      prt_names(drv->reqbuf.type,v4l2_type_names),
	      prt_names(drv->reqbuf.memory,v4l2_memory_names));
    }
  /* Allocates the required number of buffers */
  drv->v4l2_buffers=calloc(drv->reqbuf.count, sizeof(*drv->v4l2_buffers));
  assert(drv->v4l2_buffers!=NULL);
  drv->bufs=calloc(drv->reqbuf.count, sizeof(*drv->bufs));
  assert(drv->bufs!=NULL);
  
  printf(" drv->sizeimage 2 (%d)\n", drv->sizeimage);

  for (drv->n_bufs = 0; drv->n_bufs < drv->reqbuf.count; drv->n_bufs++) 
    {
      struct v4l2_buffer *p;
      
      /* Requests kernel buffers to be mmapped */
      p=drv->v4l2_buffers[drv->n_bufs]=calloc(1,sizeof(*p));
      assert (p!=NULL);
      p->index  = drv->n_bufs;
      p->type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      p->memory = V4L2_MEMORY_MMAP;
      if (xioctl(drv->fd,VIDIOC_QUERYBUF,p)<0) 
	{
	  int ret=errno;
	  perror("querybuf");
	  
	  free (drv->v4l2_buffers[drv->n_bufs]);
	  
	  v4l2_free_bufs(drv);
	  return ret;
	}
	  
      if (drv->debug)
	prt_buf_info("QUERYBUF 1", p);
      
      printf(" drv->sizeimage (%d) p->length (%d)  \n"
	     , drv->sizeimage, p->length);
      if (drv->sizeimage != p->length) 
	{
	  if (drv->sizeimage < p->length) 
	    {
	      printf ("QUERYBUF: ERROR: VIDIOC_S_FMT said buffer should have %d size, but received %d from QUERYBUF!\n",
		      drv->sizeimage, p->length);
	    } 
	  else 
	    {
	      printf ("QUERYBUF: Expecting %d size, received %d buff length\n",
		      drv->sizeimage, p->length);
	    }
	}
      
      drv->bufs[drv->n_bufs].length = p->length;
      drv->bufs[drv->n_bufs].start = mmap (NULL,	/* start anywhere */
					   p->length,
					   PROT_READ | PROT_WRITE,		/* required */
					   MAP_SHARED,			/* recommended */
					   drv->fd, p->m.offset);
      
      
      if (MAP_FAILED == drv->bufs[drv->n_bufs].start) 
	{
	  perror("mmap");
	  
	  free (drv->v4l2_buffers[drv->n_bufs]);
	  v4l2_free_bufs(drv);
	  return errno;
	}
    }
  
  return 0;
}

/* Returns <0, if error, 0 if nothing to read and <size>, if something
   read
 */
int v4l2_rcvbuf(struct v4l2_driver *drv, v4l2_recebe_buffer *rec_buf)
{
	int ret;

	struct v4l2_buffer buf;
	memset (&buf, 0, sizeof(buf));

	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;

	if (-1 == xioctl (drv->fd, VIDIOC_DQBUF, &buf)) {
		switch (errno) {

		case EAGAIN:
 		        printf(" try again %m\n");
			return -errno;
		case EBUSY:
 		        printf(" buffer busy %m\n");
			return -errno;

		case EIO:
 		        printf(" buffer problem .. EIO %m\n");
			/* Could ignore EIO, see spec. */
			return -errno;
			/* fall through */

		default:
		  printf(" buffer problem .. %d ?? %m\n", errno);
			//perror ("dqbuf");
			return -errno;
		}
	}
	prt_buf_info("DQBUF",&buf);

	assert (buf.index < drv->n_bufs);

	ret = 0;

        if (rec_buf != NULL)
	  ret = rec_buf (&buf,&drv->bufs[buf.index]);

	if (ret<0) {
		v4l2_free_bufs(drv);
		return ret;
	}

	if (-1 == xioctl (drv->fd, VIDIOC_QBUF, &buf)) {
		perror ("qbuf");
		return -errno;
	}
	return ret;
}

int v4l2_start_streaming(struct v4l2_driver *drv)
{
    uint32_t	i;
    struct v4l2_buffer buf;
  
    if (drv->debug)
        printf("Activating %d queues\n", drv->n_bufs);
    for (i = 0; i < drv->n_bufs; i++) 
    {
	int res;
	
	memset(&buf, 0, sizeof(buf));
	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;
	buf.index  = i;
	
	res = xioctl (drv->fd, VIDIOC_QBUF, &buf);
	
	if (res == 0)
	{
	    prt_buf_info("QBUF",&buf);
	}
	else 
	{
	    perror("qbuf");
	    return errno;
	}
    }
    
    /* Activates stream */
    if (drv->debug)
        printf("Enabling streaming\n");
    
    if (xioctl(drv->fd, VIDIOC_STREAMON, &drv->reqbuf.type)<0)
        return errno;

    if (drv->debug)
        printf("Enabling streaming done\n");
    
    return 0;
}

int v4l2_stop_streaming(struct v4l2_driver *drv)
{
        printf(" stop stream  1.1 ....\n");

	v4l2_free_bufs(drv);

	return 0;
}

/****************************************************************************
	Close V4L2, disallocating all structs
 ****************************************************************************/
int v4l2_close (struct v4l2_driver *drv)
{
	v4l2_free_bufs(drv);

	free_list(&drv->stds);
	free_list(&drv->inputs);
	free_list(&drv->fmt_caps);

	return (close(drv->fd));
}

/****************************************************************************
	Get/Set frequency
 ****************************************************************************/

int v4l2_getset_freq (struct v4l2_driver *drv, enum v4l2_direction dir,
		      double *freq)
{
	struct v4l2_tuner     tun;
	struct v4l2_frequency frq;
	double d = 62500;

	memset(&tun, 0, sizeof(tun));

	if (-1 == xioctl (drv->fd, VIDIOC_G_TUNER, &tun)) {
		perror ("g_tuner");
		printf("Assuming 62.5 kHz step\n");
	} else {
		if (tun.capability & V4L2_TUNER_CAP_LOW)
			d=62.5;
	}

	if (drv->debug) {
		if (tun.capability & V4L2_TUNER_CAP_LOW)
			printf("62.5 Hz step\n");
		else
			printf("62.5 kHz step\n");
	}

	memset(&frq, 0, sizeof(frq));

	frq.type = V4L2_TUNER_ANALOG_TV;

	if (dir & V4L2_GET) {
		if (-1 == xioctl (drv->fd, VIDIOC_G_FREQUENCY, &frq)) {
			perror ("s_frequency");
			return errno;
		}
		*freq = frq.frequency * d;
		if (drv->debug)
			printf("board is at freq %4.3f MHz (%d)\n",
				*freq/1000000, frq.frequency);
	} else {
		frq.frequency = (uint32_t)(((*freq)+d/2) / d);

		if (-1 == xioctl (drv->fd, VIDIOC_S_FREQUENCY, &frq)) {
			perror ("s_frequency");
			return errno;
		}
		if (drv->debug)
			printf("board set to freq %4.3f MHz (%d)\n",
				*freq/1000000, frq.frequency);

	}
	return 0;
}


static int dump_v4l2(int fd, int tab)
{
  struct v4l2_capability  capability;
  struct v4l2_standard    standard;
  struct v4l2_input       input;
  struct v4l2_tuner       tuner;
  struct v4l2_fmtdesc     fmtdesc;
  struct v4l2_format      format;
  struct v4l2_framebuffer fbuf;
  struct v4l2_queryctrl   qctrl;
  int i;
  
  printf("general info\n");
  memset(&capability,0,sizeof(capability));
  if (-1 == ioctl(fd,VIDIOC_QUERYCAP,&capability))
    return -1;
  printf("    VIDIOC_QUERYCAP\n");
  print_struct(stdout,desc_v4l2_capability,&capability,"",tab);
  printf("\n");
  
  printf("standards\n");
  for (i = 0;; i++) {
    memset(&standard,0,sizeof(standard));
    standard.index = i;
    if (-1 == ioctl(fd,VIDIOC_ENUMSTD,&standard))
      break;
    printf("    VIDIOC_ENUMSTD(%d)\n",i);
    print_struct(stdout,desc_v4l2_standard,&standard,"",tab);
  }
  printf("\n");
  
  printf("inputs\n");
  for (i = 0;; i++) {
    memset(&input,0,sizeof(input));
    input.index = i;
    if (-1 == ioctl(fd,VIDIOC_ENUMINPUT,&input))
      break;
    printf("    VIDIOC_ENUMINPUT(%d)\n",i);
    print_struct(stdout,desc_v4l2_input,&input,"",tab);
  }
  printf("\n");
  
  if (capability.capabilities & V4L2_CAP_TUNER) {
    printf("tuners\n");
    for (i = 0;; i++) {
      memset(&tuner,0,sizeof(tuner));
      tuner.index = i;
      if (-1 == ioctl(fd,VIDIOC_G_TUNER,&tuner))
	break;
      printf("    VIDIOC_G_TUNER(%d)\n",i);
      print_struct(stdout,desc_v4l2_tuner,&tuner,"",tab);
    }
    printf("\n");
  }
  
  if (capability.capabilities & V4L2_CAP_VIDEO_CAPTURE) {
    printf("video capture\n");
    for (i = 0;; i++) {
      memset(&fmtdesc,0,sizeof(fmtdesc));
      fmtdesc.index = i;
      fmtdesc.type  = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      if (-1 == ioctl(fd,VIDIOC_ENUM_FMT,&fmtdesc))
	break;
      printf("    VIDIOC_ENUM_FMT(%d,VIDEO_CAPTURE)\n",i);
      print_struct(stdout,desc_v4l2_fmtdesc,&fmtdesc,"",tab);
    }
    memset(&format,0,sizeof(format));
    format.type  = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (-1 == ioctl(fd,VIDIOC_G_FMT,&format)) {
      perror("VIDIOC_G_FMT(VIDEO_CAPTURE)");
    } else {
      printf("    VIDIOC_G_FMT(VIDEO_CAPTURE)\n");
      print_struct(stdout,desc_v4l2_format,&format,"",tab);
    }
    printf("\n");
  }
  
  if (capability.capabilities & V4L2_CAP_VIDEO_OVERLAY) {
    printf("video overlay\n");
    for (i = 0;; i++) {
      memset(&fmtdesc,0,sizeof(fmtdesc));
      fmtdesc.index = i;
      fmtdesc.type  = V4L2_BUF_TYPE_VIDEO_OVERLAY;
      if (-1 == ioctl(fd,VIDIOC_ENUM_FMT,&fmtdesc))
	break;
      printf("    VIDIOC_ENUM_FMT(%d,VIDEO_OVERLAY)\n",i);
      print_struct(stdout,desc_v4l2_fmtdesc,&fmtdesc,"",tab);
    }
    memset(&format,0,sizeof(format));
    format.type  = V4L2_BUF_TYPE_VIDEO_OVERLAY;
    if (-1 == ioctl(fd,VIDIOC_G_FMT,&format)) {
      perror("VIDIOC_G_FMT(VIDEO_OVERLAY)");
    } else {
      printf("    VIDIOC_G_FMT(VIDEO_OVERLAY)\n");
      print_struct(stdout,desc_v4l2_format,&format,"",tab);
    }
    memset(&fbuf,0,sizeof(fbuf));
    if (-1 == ioctl(fd,VIDIOC_G_FBUF,&fbuf)) {
      perror("VIDIOC_G_FBUF");
    } else {
      printf("    VIDIOC_G_FBUF\n");
      print_struct(stdout,desc_v4l2_framebuffer,&fbuf,"",tab);
    }
    printf("\n");
  }
  
  if (capability.capabilities & V4L2_CAP_VBI_CAPTURE) {
    printf("vbi capture\n");
    for (i = 0;; i++) {
      memset(&fmtdesc,0,sizeof(fmtdesc));
      fmtdesc.index = i;
      fmtdesc.type  = V4L2_BUF_TYPE_VBI_CAPTURE;
      if (-1 == ioctl(fd,VIDIOC_ENUM_FMT,&fmtdesc))
	break;
      printf("    VIDIOC_ENUM_FMT(%d,VBI_CAPTURE)\n",i);
      print_struct(stdout,desc_v4l2_fmtdesc,&fmtdesc,"",tab);
    }
    memset(&format,0,sizeof(format));
    format.type  = V4L2_BUF_TYPE_VBI_CAPTURE;
    if (-1 == ioctl(fd,VIDIOC_G_FMT,&format)) {
      perror("VIDIOC_G_FMT(VBI_CAPTURE)");
    } else {
      printf("    VIDIOC_G_FMT(VBI_CAPTURE)\n");
      print_struct(stdout,desc_v4l2_format,&format,"",tab);
    }
    printf("\n");
  }
  
  printf("controls\n");
  for (i = 0;; i++) {
    memset(&qctrl,0,sizeof(qctrl));
    qctrl.id = V4L2_CID_BASE+i;
    if (-1 == ioctl(fd,VIDIOC_QUERYCTRL,&qctrl))
      break;
    if (qctrl.flags & V4L2_CTRL_FLAG_DISABLED)
      continue;
    printf("    VIDIOC_QUERYCTRL(BASE+%d)\n",i);
    print_struct(stdout,desc_v4l2_queryctrl,&qctrl,"",tab);
  }
  for (i = 0;; i++) {
    memset(&qctrl,0,sizeof(qctrl));
    qctrl.id = V4L2_CID_PRIVATE_BASE+i;
    if (-1 == ioctl(fd,VIDIOC_QUERYCTRL,&qctrl))
      break;
    if (qctrl.flags & V4L2_CTRL_FLAG_DISABLED)
      continue;
    printf("    VIDIOC_QUERYCTRL(PRIVATE_BASE+%d)\n",i);
    print_struct(stdout,desc_v4l2_queryctrl,&qctrl,"",tab);
  }
  return 0;
}
 
static int run_v4l2(struct v4l2_driver *drv, int num_buffers, int frames)
{
    int rc;
    int bad;
    int i;
    printf(" map buffers  ....\n");
    rc  = v4l2_mmap_bufs(drv, num_buffers);
    if (rc != 0)
      {
	printf("v4l2_mmap_bufs error rc %d\n", rc);
	v4l2_close (drv);
	return rc;
      }
    //return -1;
    printf(" start stream  1 ....\n");
    bad  =  0;
    v4l2_start_streaming(drv);
    printf(" start stream  2 ....\n");
    for (i=0; bad<100 && i< frames; i++)
      {
	printf(" recv #%d  ....", i);
	rc = -16;
	//rc = v4l2_rcvbuf(drv, NULL /*v4l2_recebe_buffer *rec_buf*/);
	printf(" rc = %d  ....\n", rc);
	if (rc == -16) 
	  {
	    i--;
	    bad++;
	    // TODO sleep for frame interval
	    usleep(1024 *10);
	  }
      }
    printf(" stop stream  1 ....\n");
    v4l2_stop_streaming(drv);
    printf(" stop stream  2 ....\n");
    v4l2_close (drv);
    printf(" stop stream  3 ....\n");
    rc = 0;
    return rc;
}

#define MAX_ARGS 10

int main(int argc, char *argv[])
{
    struct v4l2_driver drv;
    struct v4l2_format fmt;
    uint32_t width;
    uint32_t height;
    uint32_t pixelformat; 
    int rc;
    char device[128];
    int idx;
    int debug = 1;
    int num_buffers;
    int i;
    int bad;
    int fd;
    int ok;
    int tab = 1;
    char dummy[256];
    int cam_ix;
    int argvals[MAX_ARGS];
    int argix = 1;
    int fix = -1;
    int cix = -1;
    int nfram  = 0;

    while(argc > argix)
      {
	argvals[argix] = atoi(argv[argix]);
	argix++;
      }
    drv.stds = NULL;
    drv.inputs =  NULL;
    drv.fmt_caps = NULL;

    // argvals[1] = camera ix
    // argvals[2] = frame ix
    if (argc >1) cix = argvals[1];
    if (argc >2) fix = argvals[2];
    if (argc >3) nfram = argvals[3];
    
    for (idx = 0; idx < 8; idx++)
      {
	snprintf(device, sizeof(device), "/dev/video%d", idx);
        fd = open(device, O_RDWR);
        if (fd < 0) continue;
	
	drv.fd = fd;
	if (-1 != ioctl(fd, VIDIOC_QUERYCAP, dummy)) {
	  memset(&fmt, 0 , sizeof(fmt)); 
	  fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	  int rc_get = xioctl (fd, VIDIOC_G_FMT, &fmt);
	  int rc_set = xioctl (fd, VIDIOC_S_FMT, &fmt);
	  printf("###  [%s] set rc %d get rc %d curr fmt %04x###\n"
		 , device, rc_set, rc_get
		 , fmt.fmt.pix.pixelformat);  
	  //close(fd);
	  //continue;
	  printf("### v4l2 device info [%s] ###\n", device);
	  dump_v4l2(fd, tab);
	  printf("### v4l2 video formats ext [%s] ###\n", device);
	  print_video_formats_ext(fd, V4L2_BUF_TYPE_VIDEO_CAPTURE);
	  printf("## 1 fram index  (%d) fix (%d) cix (%d)###\n"
		 , fram_ix, fix, cix);
	  ok = 1;
	  print_fram();
	  if (idx == cix)
	    { 
	      printf(" using camera %d idx %d\n", cix, idx);
	      printf(" fram_ix %d fix %d\n", fram_ix,fix);
	      struct v4l2_fram *fram=NULL;
	      if (fix < fram_ix)
		{
		  fram = &v4l2_fram[fix];
		}	
	      if (fram)
		{
		  
		  //fmt.fmt.pix.pixelformat=fram->pixel_format;  
		  
		  memset(&fmt, 0 , sizeof(fmt)); 
		  fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		  rc_get = xioctl (fd, VIDIOC_G_FMT, &fmt);
		  unsigned int old_format = fmt.fmt.pix.pixelformat;
		  //fmt.fmt.pix.pixelformat=fram->pixel_format;
		  //rc_set = xioctl (fd, VIDIOC_S_FMT, &fmt);
		  
		  if (0)printf("## 2  [%s] set rc %d get rc %d old fmt %04x req fmt %04x curr fmt %04x  ###\n"
			       , device, rc_set, rc_get
			       , old_format
			       , fram->pixel_format
			       , fmt.fmt.pix.pixelformat);  
		  
		  if(1)rc = v4l2_gettryset_fmt_cap (&drv, V4L2_TRY
						    , &fmt
						    , fram->width
						    , fram->height
						    , fram->pixel_format
						    , V4L2_FIELD_ANY  );
		  printf("## 3  [%s] try rc %d \n", device, rc);
		  printf("## 3,1  [%s] drv.sizeimage (%d) \n"
			 , device, drv.sizeimage);
		  v4l2_get_frame_rate(&drv);
		  v4l2_set_frame_rate(&drv, fram->nom, fram->den );
		  v4l2_get_frame_rate(&drv);
		  printf("## 4  [%s] frames %d \n", device, nfram);
		  if(nfram > 0)
		  {
		      drv.n_bufs = 0;
		      rc = run_v4l2(&drv, 4, nfram);
		      if(rc != 0)v4l2_close (&drv);
		  }
		  //drv.sizeimage=

		}
	    }
	  printf("### close device  [%s] ###\n",device);
	  
	  close(fd);
	}
      }
    return 0;
}
#if 0
    // move past here to try to capture some frames
    rc = v4l2_open(device, debug, &drv);
    if(rc == 0)
      {
	printf(" dev [%s] idx %d open rc = %d\n", device, idx, rc);
	
	printf(" get parm ....\n");
	v4l2_get_parm (&drv);
	
	printf(" enum inputs ....\n");
	v4l2_enum_input (&drv);
	
	printf(" enum stds ....\n");
	v4l2_enum_stds (&drv);
	
	printf(" get fmt  ....\n");
	v4l2_get_fmt(&drv, &fmt);
	
	//V4L2_PIX_FMT_YUYVP
	printf(" set fmt  YUYV....\n");
	fmt.fmt.pix.pixelformat=V4L2_PIX_FMT_YUYV;  // "YUYV" 
	v4l2_set_fmt(&drv, &fmt);
	
	//V4L2_PIX_FMT_YUV422P
	printf(" set fmt  YUYVP....\n");
	fmt.fmt.pix.pixelformat=V4L2_PIX_FMT_YUV422P;  // "YUYV" 
	v4l2_set_fmt(&drv, &fmt);
	
	printf(" set fmt  JPEG....\n");
	fmt.fmt.pix.pixelformat=V4L2_PIX_FMT_JPEG;  // "JPEG" 
	v4l2_set_fmt(&drv, &fmt);
	
	printf(" set fmt  MPEG....\n");
	fmt.fmt.pix.pixelformat=V4L2_PIX_FMT_MPEG;  // "MPEG" 
	v4l2_set_fmt(&drv, &fmt);
	
	printf(" set fmt  MJPEG ??....\n");
	fmt.fmt.pix.pixelformat=0x47504a4d;
	v4l2_set_fmt(&drv, &fmt);
	
	printf(" get fmt 2  ....\n");
	v4l2_get_fmt(&drv, &fmt);
	
	drv.sizeimage =  
	  fmt.fmt.pix.bytesperline
	  * fmt.fmt.pix.height;
	if (drv.sizeimage == 0) {
	  drv.sizeimage=614400;
	}
	height = fmt.fmt.pix.height;
	width = fmt.fmt.pix.width;
	pixelformat = (uint32_t)fmt.fmt.pix.pixelformat;
	
	printf(" get cap  ....\n");
	v4l2_gettryset_fmt_cap (&drv, V4L2_GET
				, &fmt
				, width
				, height
				, pixelformat
				, V4L2_FIELD_ANY  );
	num_buffers  = 4;
	printf(" map buffers  ....\n");
	rc  = v4l2_mmap_bufs(&drv, num_buffers);
	if (rc != 0)
	  {
	    printf("v4l2_mmap_bufs error rc %d\n", rc);
	    v4l2_close (&drv);
	    continue;
	  }
	printf(" start stream  ....\n");
	bad  =  0;
	v4l2_start_streaming(&drv);
	for (i=0; bad<100 && i< 32; i++)
	  {
	    printf(" recv #%d  ....", i);
	    rc = v4l2_rcvbuf(&drv, NULL /*v4l2_recebe_buffer *rec_buf*/);
	    printf(" rc = %d  ....\n", rc);
	    if (rc == -16) 
	      {
		i--;
		bad++;
		usleep(1024 *10);
	      }
	  }
	v4l2_stop_streaming(&drv);
	v4l2_close (&drv);
      }
    return 0;
}
#endif
