#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <asm/types.h>    /* for videodev2.h */
#include <linux/videodev2.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <string.h>
/* for select */
#include <sys/select.h>


#define ENC_INFO printf
#define ENC_DEBUG printf

#define NB_USB_BUFFER 4
#define NUM_OMX_BUFFERS 4

#define DHT_SIZE 420

#define RAW_BUFF_TYPE 0
#define MJ_BUFF_TYPE 1
#define NUM_BUFF_TYPES 2

#define BUFF_CLEAR 0
#define BUFF_READY 10
#define BUFF_FULL 20
#define BUFF_EMPTIED 30

// This is for mJPEG !!!
#define HEADERFRAME1 0xaf

static unsigned char dht_data[DHT_SIZE] = {
  0xff, 0xc4, 0x01, 0xa2, 0x00, 0x00, 0x01, 0x05, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02,
  0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x01, 0x00, 0x03,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
  0x0a, 0x0b, 0x10, 0x00, 0x02, 0x01, 0x03, 0x03, 0x02, 0x04, 0x03, 0x05,
  0x05, 0x04, 0x04, 0x00, 0x00, 0x01, 0x7d, 0x01, 0x02, 0x03, 0x00, 0x04,
  0x11, 0x05, 0x12, 0x21, 0x31, 0x41, 0x06, 0x13, 0x51, 0x61, 0x07, 0x22,
  0x71, 0x14, 0x32, 0x81, 0x91, 0xa1, 0x08, 0x23, 0x42, 0xb1, 0xc1, 0x15,
  0x52, 0xd1, 0xf0, 0x24, 0x33, 0x62, 0x72, 0x82, 0x09, 0x0a, 0x16, 0x17,
  0x18, 0x19, 0x1a, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x34, 0x35, 0x36,
  0x37, 0x38, 0x39, 0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a,
  0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x63, 0x64, 0x65, 0x66,
  0x67, 0x68, 0x69, 0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a,
  0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x92, 0x93, 0x94, 0x95,
  0x96, 0x97, 0x98, 0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8,
  0xa9, 0xaa, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xc2,
  0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2, 0xd3, 0xd4, 0xd5,
  0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7,
  0xe8, 0xe9, 0xea, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9,
  0xfa, 0x11, 0x00, 0x02, 0x01, 0x02, 0x04, 0x04, 0x03, 0x04, 0x07, 0x05,
  0x04, 0x04, 0x00, 0x01, 0x02, 0x77, 0x00, 0x01, 0x02, 0x03, 0x11, 0x04,
  0x05, 0x21, 0x31, 0x06, 0x12, 0x41, 0x51, 0x07, 0x61, 0x71, 0x13, 0x22,
  0x32, 0x81, 0x08, 0x14, 0x42, 0x91, 0xa1, 0xb1, 0xc1, 0x09, 0x23, 0x33,
  0x52, 0xf0, 0x15, 0x62, 0x72, 0xd1, 0x0a, 0x16, 0x24, 0x34, 0xe1, 0x25,
  0xf1, 0x17, 0x18, 0x19, 0x1a, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x35, 0x36,
  0x37, 0x38, 0x39, 0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a,
  0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x63, 0x64, 0x65, 0x66,
  0x67, 0x68, 0x69, 0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a,
  0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x92, 0x93, 0x94,
  0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7,
  0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba,
  0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2, 0xd3, 0xd4,
  0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7,
  0xe8, 0xe9, 0xea, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa
};

typedef struct OMX_BUFFERHEADER
{
  int nAllocLen;
  int nFilledLen;
  void * pBuffer; 
} OMX_BUFFERHEADERTYPE;

struct usb_omx_buffer
{
  int port;
  int state;      //0 unallocated; 1=allocated; 2 = please fill me
  int type;       // mj_ip = 1 // mj_op = 2
  int usb_index;  // may not be used
  void *buff;
  void *obj;
  int buff_index;
  void *cord;
  //OMX_HANDLETYPE handle;
};

#define NUM_BTYPES 3

// one of these for each video input device
struct usb_Ip {
  int fd;
  int old_fd;
  int port_no;
  int num_buffs;
  char *videodevice;
  char *status;
  char *pictName;
  int framecnt[NUM_BUFF_TYPES];
  FILE *cf_id[NUM_BUFF_TYPES];
  FILE *cf_info[NUM_BUFF_TYPES];
  char  cfname[NUM_BUFF_TYPES][128];
  char  cfinfo[NUM_BUFF_TYPES][128];
  struct v4l2_capability cap;
  struct v4l2_format fmt;
  struct v4l2_buffer buf;
  struct v4l2_requestbuffers req;
  struct v4l2_streamparm stp;
  void *mem[NB_USB_BUFFER];
  unsigned long mem_len[NB_USB_BUFFER];
  unsigned char *tmpbuffer;
  int tmpsize;
  unsigned char *framebuffer;
  int isstreaming;
  long streamcnt;
  long mjpegcnt;
  int usestream;
  int usemjpeg;

  long grabcnt;
  long buffcnt;
  long errcnt;
  int grabmethod;
  int width;
  int height;
  int formatIn;
  int formatOut;
  int framesizeIn;
  int signalquit;
  int toggleAvi;
  int getPict;
  struct usb_omx_buffer usb_buffers[NUM_OMX_BUFFERS];
  int num_buffers;
  //  OMX_HANDLETYPE mjpeg_handle;
};

int mj_dec_buf_emptied=0;
int mj_dec_buf_found=0;
int mj_dec_buf_type=0;
int max_mjpeg = 0;


FILE *capture_file=NULL;
char *capture_filename = (char *)"/var/internal/capture.mjpeg";        
FILE *capture_file_info=NULL;
char *capture_filename_info = (char *)"/var/internal/capture.info";        
int run_grab = 1;


OMX_BUFFERHEADERTYPE *creat_buff(int len)
{
  OMX_BUFFERHEADERTYPE *buff;
  buff = (OMX_BUFFERHEADERTYPE *)malloc(sizeof (*buff));
  if(buff)
  {
      buff->pBuffer=malloc(len);
      if(buff->pBuffer)
      {
          buff->nAllocLen =len;
          buff->nFilledLen = 0;
      }

  }
  return buff;
}


void *usb_memcpy(void *dst, const void *src, size_t n, int fd)
{
    void *src_phys_p=NULL;
    void *dst_phys_p=NULL;
    int rc;

    static int first=10;
#if 0
    if (first > 0)
    {
      //src_phys_p = (OMX_PTR) DomxCore_mapUsrVirtualAddr2phy ((uint32_t)src);
        dst_phys_p = (OMX_PTR) DomxCore_mapUsrVirtualAddr2phy ((uint32_t)dst);
        first--;
        ENC_DEBUG(" USB_MEMCPY  src %p (phys %p) dest %p (phys %p)\n"
                  , src, src_phys_p
                  , dst, dst_phys_p
                  );
    }
#endif

    if (fd == -1)
    {
        return memcpy(dst, src, n);
    }
    else
    {
#if 0
        // DMA if we can
        src_phys_p = (OMX_PTR) DomxCore_mapUsrVirtualAddr2phy ((uint32_t)src);
        dst_phys_p = (OMX_PTR) DomxCore_mapUsrVirtualAddr2phy ((uint32_t)dst);
        
        struct mem_copy_struct data;
        data.src = src_phys_p;
        data.dst = dst_phys_p;
        data.line_len = n;
        data.in_stride = n;
        data.num_lines = 1;
        data.out_stride = n;
        
        rc = ioctl(fd, IOCTL_MEM_COPY, &data);
        if(rc == 0)
        {
            return dst;
        }
        else
        {
            return NULL;
        }
#endif
    }
}

static int xioctl(int fd, int req, void *arg)
{
        int rc;

        do {
          rc = ioctl(fd, req, arg);
        } while (rc == -1 && errno == EINTR);

        return rc;
}

int usb_open(void)
{
    char viddev[128];
    int o_flags = O_RDWR;// 
    int fd;
    int i;
    o_flags |= O_NONBLOCK;

    for (i = 0 ; i < 16; ++i)
    {
        snprintf(viddev, sizeof (viddev), "/dev/video%d", i);
        printf(" VIDEO testing device [%s] \n", viddev);
        fd = open(viddev, o_flags);
        if (fd > 0) 
        {
            printf(" VIDEO device [%s] opened fd %d  \n", viddev, fd);
            break;
        }
    }
    return fd;
}

#if 0
    if (fd > 0)
    {
        struct usb_Ip *vd=&usb_in;
        vd->fd = fd;
        usb_close_v4l(vd);
    }
#endif


//int mj_type = 0;
static int do_get_grab(char *buf, int *len, int *cnt, int *wid, int *hi)
{
    FILE *grab_file=NULL;
    char *grab_filename = (char *)"/var/internal/grab_frame.vid";        
    FILE *grab_file_info=NULL;
    char *grab_filename_info = (char *)"/var/internal/grab_frame.info";        
    int  local_len = 0;
    
    grab_file = fopen(grab_filename, "rb");
    grab_file_info = fopen(grab_filename_info, "r");
    
    
    if(grab_file_info)
    {
        fscanf(grab_file_info,"frame:%d size:%d width:%d height:%d\n", cnt, &local_len, wid, hi);
        if(grab_file)
        {
          if (buf && len)
          {
              if (local_len <= *len)
              {
                  fread(buf, 1, local_len, grab_file);
                  *len = local_len;
              }
              else
              {
                  if(buf)fread(buf, 1, *len, grab_file);
              }
          }
        }
        fclose(grab_file);
        fclose(grab_file_info);
    }
    else
    {
        if (len)
        {
            *len = local_len;
        }
    }

    return local_len;
}

static int do_run_grab(char *buf, int len, int cnt, int wid, int hi)
{
    FILE *grab_file=NULL;
    char *grab_filename = (char *)"/var/internal/grab.vid";        
    FILE *grab_file_info=NULL;
    char *grab_filename_info = (char *)"/var/internal/grab.info";        
    if(run_grab)
    {
        grab_file = fopen(grab_filename, "wb");
        grab_file_info = fopen(grab_filename_info, "wb");
        if(grab_file)
            fwrite(buf, 1, len, grab_file);
        if(grab_file_info)
          fprintf(grab_file_info,"frame:%d size:%d width:%d height:%d\n", cnt, len, wid, hi);

        fclose(grab_file);
        grab_file = NULL;
        fclose(grab_file_info);
        grab_file_info = NULL;
        run_grab = 0;
    }
    return 0;
}

int fill_buff_mj(struct usb_Ip *vd,int btype, OMX_BUFFERHEADERTYPE *buff
                 , int copy_fd)
{
  // Dummy if the bytes in are too many for the buffer
  if ((vd->buf.bytesused + DHT_SIZE) > buff->nAllocLen)
    {
      usb_memcpy (buff->pBuffer, vd->mem[vd->buf.index],
                  (size_t) buff->nAllocLen, copy_fd);
      buff->nFilledLen = buff->nAllocLen;
    }
  else
    {
      usb_memcpy (buff->pBuffer, vd->mem[vd->buf.index], HEADERFRAME1, copy_fd);
      usb_memcpy (buff->pBuffer + HEADERFRAME1, dht_data, DHT_SIZE, copy_fd);
      usb_memcpy (buff->pBuffer + HEADERFRAME1 + DHT_SIZE,
                  (void *) ((char *)vd->mem[vd->buf.index] + HEADERFRAME1),
                  (vd->buf.bytesused - HEADERFRAME1), copy_fd);
      buff->nFilledLen = vd->buf.bytesused + DHT_SIZE;
      
    }
  
  return buff->nFilledLen;
}

int fill_buff(struct usb_Ip *vd,int btype, OMX_BUFFERHEADERTYPE *buff, int copy_fd)
{
  // Dummy if the bytes in are too many for the buffer
  //ENC_DEBUG("%s fill_buff vd->buf.index %d\n", __FUNCTION__, vd->buf.index);
  //ENC_DEBUG("%s fill_buff pBuffer %p len %d\n", __FUNCTION__, buff->pBuffer, buff->nAllocLen);
  copy_fd = -1;
  if (vd->buf.bytesused > buff->nAllocLen)
  {
      usb_memcpy (buff->pBuffer, vd->mem[vd->buf.index],
                  (size_t) buff->nAllocLen, copy_fd);
      buff->nFilledLen = buff->nAllocLen;
  }
  else
  {
      usb_memcpy (buff->pBuffer, vd->mem[vd->buf.index],
                  (size_t) vd->buf.bytesused, copy_fd);
      buff->nFilledLen = vd->buf.bytesused;
  }
  //ENC_DEBUG("%s done FilledLen %d\n",__FUNCTION__, buff->nFilledLen);
  return buff->nFilledLen;
}

int empty_buff(struct usb_Ip *vd, int btype, OMX_BUFFERHEADERTYPE *buff, int buff_ix)
{
    struct usb_omx_buffer *usb_buff;
    usb_buff = vd->usb_buffers;

    // when we have set up the usb_empty_buf object
    if ((buff_ix >=0) && (usb_buff[buff_ix].obj!=NULL))
      {
        mj_dec_buf_emptied++;

        //ENC_DEBUG("%s running  mjpeg usb_empty_buff %p at %d \n", name(), buff, buff_ix);
        //void mjpeg_decoder::usb_empty_buf(OMX_BUFFERHEADERTYPE *buff, int port, int buff_index)
        //              usb_capture *obj = (usb_capture *)usb_buff[buff_ix].obj;
        //obj->usb_empty_buf(buff, usb_buff[buff_ix].port, usb_buff[buff_ix].buff_index);
        //ENC_DEBUG("%s DONE running  mjpeg usb_empty_buff %p at %d \n", name(), buff, buff_ix);
      }
    
    if(buff_ix >= 0) usb_buff[buff_ix].state = BUFF_EMPTIED; 
    return 0;
}

int find_buff_type(struct usb_Ip *vd, int btype)
{
    int j;
    int buff_ix=-1;
    struct usb_omx_buffer *usb_buff;
    usb_buff = vd->usb_buffers;
    ENC_DEBUG(" %s usb_buff (%p) num_buffs %d\n", __FUNCTION__, usb_buff, vd->num_buffs);

    for (j=0; j<vd->num_buffs; j++)
      {
        if ((usb_buff[j].type == btype) && (usb_buff[j].state == BUFF_READY)) 
          {
            vd->buffcnt++;
            usb_buff[j].state = BUFF_FULL;
            
            buff_ix = j;
            break;
          }
      }
    return buff_ix;
}



int file_buff(struct usb_Ip *vd, int btype, OMX_BUFFERHEADERTYPE *buff)
{

    if(vd->framecnt[btype]==0)
      {
        vd->cf_id[btype] = fopen(vd->cfname[btype], "wb");
        vd->cf_info[btype] = fopen(vd->cfinfo[btype], "wb");
      }  
    
    if(vd->framecnt[btype]>60)
      {
        if(vd->cf_id[btype])
          {
            fclose(vd->cf_id[btype]);
            vd->cf_id[btype] = NULL;
          }
        if(vd->cf_info[btype])
          {
            fclose(vd->cf_info[btype]);
            vd->cf_info[btype] = NULL;
          }
      }
    
    if(vd->cf_id[btype])
      {
        fwrite(buff->pBuffer, 1, buff->nFilledLen, vd->cf_id[btype]);
      }
    if(vd->cf_info[btype])
      {
        fprintf(vd->cf_info[btype]," frame (%d) size (%d)\n"
                , vd->framecnt[btype], buff->nFilledLen);
      }
    vd->framecnt[btype]++;
    return vd->framecnt[btype];
    
}

int usb_grab_v4l(struct usb_Ip *vd)
{

  int rc;
  int j;
  int buff_ix = -1;
  // TODO use Dma once we get the phys address
  //copy_fd = dma_copy_fd;
  int copy_fd = -1;
  int mj_type = 0;
  OMX_BUFFERHEADERTYPE *buff=NULL;
  //void *buff = NULL;
  struct usb_omx_buffer *usb_buff;
  usb_buff = vd->usb_buffers;
  vd->grabcnt++;
  // flush buffers if we are not connected
  if (vd->fd < 0) 
  {

      for (j=0; j < vd->num_buffs; j++)
      {
        // raw_type
          if ((usb_buff[j].type == RAW_BUFF_TYPE)  && (usb_buff[j].state == BUFF_READY))
          {
              //buff = (OMX_BUFFERHEADERTYPE *)usb_buff[j].buff;
              if(buff)
              {
                  //fill_buffer_done_callback(buff);
                  usb_buff[j].state = BUFF_CLEAR;
              } 
              else 
              {
                usb_buff[j].state = -2; //??
              }
          }
      }
      
      return -1;
  }
  // if we are not streaming then return an error
  if (vd->isstreaming == 0) return -2;

  // try to dequeue a buffer 
  usb_buff = vd->usb_buffers;
  memset (&vd->buf, 0, sizeof (struct v4l2_buffer));
  vd->buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  vd->buf.memory = V4L2_MEMORY_MMAP;
  rc = xioctl (vd->fd, VIDIOC_DQBUF, &vd->buf);
  if (rc == -1)
  {
      if(errno == EAGAIN)
      {
          ENC_DEBUG("USB !!! dequeue usb buffer  errno (%d) %m\n",errno);
          return rc;
      }
      
      if (vd->errcnt++ < 50)
      {
          ENC_DEBUG("USB !!! dequeue usb buffer failed errno (%d) %m\n",errno);
      }
      vd->fd = -1;
      return rc;
  }
  // now capture the buffer
  ENC_DEBUG("found data\n");
  int btype=-1;
  buff = NULL;

  switch (vd->formatIn) 
  {

  case V4L2_PIX_FMT_MJPEG:
    ENC_DEBUG("USB !!! dequeue mjpeg\n");
    vd->mjpegcnt++;
    btype = MJ_BUFF_TYPE;
    if (vd->buf.bytesused > max_mjpeg)
      {
        max_mjpeg=vd->buf.bytesused;
      }
    break;
    
  case V4L2_PIX_FMT_YUYV:
    vd->streamcnt++;
    btype = RAW_BUFF_TYPE;
    if(0)do_run_grab((char *)vd->mem[vd->buf.index], vd->buf.bytesused
                , vd->streamcnt 
                , vd->fmt.fmt.pix.width, vd->fmt.fmt.pix.height);
    break;
      
  default:
    goto err;
    break;

  }
  ENC_DEBUG("found data btype %d\n", btype);

  if(btype != -1)
    {
      // find a ready buff type
      buff_ix = find_buff_type(vd, btype);
      ENC_DEBUG("found  btype %d buff_ix %d\n", btype, buff_ix);
      if(buff_ix >= 0)
        {
          buff = (OMX_BUFFERHEADERTYPE *)usb_buff[buff_ix].buff;
          ENC_DEBUG("%s found usb buffer %p at %d \n", "test", buff, buff_ix);
        }
      if(buff != NULL)
        {
          fill_buff(vd, btype, buff, -1);
          empty_buff(vd, btype, buff, buff_ix);
          //file_buff(vd, btype, buff);
        }
    }
  // Now requeue the usb buffer
  rc = xioctl (vd->fd, VIDIOC_QBUF, &vd->buf);
  if (rc < 0) 
    {
      vd->fd = -1;
      if (vd->errcnt < 50)
        {
          ENC_DEBUG("USB !!! enqueue usb buffer failed errno (%d) %m\n",errno);
        }
      //fprintf (stderr, "Unable to requeue buffer (%d).\n", errno);
      goto err;
    }
  return 0;
 err:
  vd->errcnt++;
  vd->signalquit = 0;
  return -1;
}


int usb_open_v4l(struct usb_Ip *vd, int use_mj)
{
    int rc;
    int rc_set = 0;
    int i;

    memset(&vd->fmt, 0 , sizeof(vd->fmt));   
    memset(&vd->stp, 0 , sizeof(vd->stp));   
    memset(&vd->req, 0 , sizeof(vd->req));   

    vd->fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    rc = xioctl (vd->fd, VIDIOC_G_FMT, &vd->fmt);
    ENC_DEBUG(" VIDEOC_G_FMT fmt fd %d rc %d\n", vd->fd, rc);
    ENC_DEBUG(" VIDEOC_G_FMT fmt.fmt.pix.width %d\n", vd->fmt.fmt.pix.width);
    ENC_DEBUG(" VIDEOC_G_FMT fmt.fmt.pix.height %d\n", vd->fmt.fmt.pix.height);
    ENC_DEBUG(" VIDEOC_G_FMT fmt.fmt.pix.field %d\n", vd->fmt.fmt.pix.field);
    ENC_DEBUG(" VIDEOC_G_FMT fmt.fmt.pix.pixelformat %08x YUYV %08x\n"
              , vd->fmt.fmt.pix.pixelformat, V4L2_PIX_FMT_YUYV
              );
    ENC_DEBUG(" VIDEOC_G_FMT fmt.fmt.pix.field %04x %d\n"
              , vd->fmt.fmt.pix.field, vd->fmt.fmt.pix.field);
    ENC_DEBUG(" VIDEOC_G_FMT fmt.fmt.pix.bytesperline %d\n"
              , vd->fmt.fmt.pix.bytesperline);

    if(use_mj)
    {
        ENC_DEBUG(" ******************open_v4l  forcing mjpeg\n");
        //                            G P J M
        vd->fmt.fmt.pix.pixelformat=0x47504a4d;  // "MJPG" 
        rc_set = xioctl (vd->fd, VIDIOC_S_FMT, &vd->fmt);
        ENC_DEBUG(" VIDEOC_S_FMT fmt fd %d rc_set %d\n", vd->fd, rc_set);
        rc = xioctl (vd->fd, VIDIOC_G_FMT, &vd->fmt);
        ENC_DEBUG(" after setting mjpeg  VIDEOC_G_FMT fmt fd %d rc %d\n"
                  , vd->fd, rc);
        ENC_DEBUG(" VIDEOC_G_FMT fmt.fmt.pix.width %d\n"
                  , vd->fmt.fmt.pix.width);
        ENC_DEBUG(" VIDEOC_G_FMT fmt.fmt.pix.height %d\n"
                  , vd->fmt.fmt.pix.height);
        ENC_DEBUG(" VIDEOC_G_FMT fmt.fmt.pix.pixelformat %08x YUYV %08x\n"
                  , vd->fmt.fmt.pix.pixelformat, V4L2_PIX_FMT_YUYV);
    }
    
    vd->formatIn = vd->fmt.fmt.pix.pixelformat;

    vd->stp.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    rc = xioctl (vd->fd, VIDIOC_G_PARM, &vd->stp);
    ENC_DEBUG(" VIDEOC_G_PARM stp fd %d rc %d\n", vd->fd, rc);
    ENC_DEBUG(" VIDEOC_G_PARM stp.parm.capture.capability %08x\n"
              , vd->stp.parm.capture.capability);
    ENC_DEBUG(" VIDEOC_G_PARM stp.parm.capture.timeperframe %d/%d\n"
              , vd->stp.parm.capture.timeperframe.numerator
              , vd->stp.parm.capture.timeperframe.denominator);

#if 0
    memset(&config_usb, 0, sizeof(usb_capture_config));

    config_usb.in_width  = vd->fmt.fmt.pix.width;
    config_usb.in_height = vd->fmt.fmt.pix.height;
    config_usb.width     = vd->fmt.fmt.pix.width;
    config_usb.height    = vd->fmt.fmt.pix.height;
    // hack for now
    config_usb.framerate = vd->stp.parm.capture.timeperframe.denominator;

    config_usb.in_color_space   = OMX_COLOR_FormatYCbYCr;
    config_usb.deinterlace      = false;
    config_usb.standard         = OMX_VIDEO_DECODER_STD_AUTO_DETECT;
#endif

    memset(&vd->req, 0 , sizeof(vd->req));   
	vd->req.count               = NB_USB_BUFFER;
	vd->req.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	vd->req.memory              = V4L2_MEMORY_MMAP;
    //vd->req.memory              = V4L2_MEMORY_DMABUF;
	//req.memory              = V4L2_MEMORY_USERPTR;

	rc = xioctl (vd->fd, VIDIOC_REQBUFS, &vd->req);
    ENC_DEBUG(" VIDEOC_REQBUFS req fd %d rc %d count %d\n"
              , vd->fd, rc, vd->req.count);
    vd->num_buffers = (int)vd->req.count;
    unsigned long mem_start = 0;
    unsigned long mem_size = 0;

    if(rc == 0)
    {
        for (i = 0; i < vd->num_buffers; i++)
        {
            //struct v4l2_buffer buf;
            memset(&vd->buf, 0 , sizeof(vd->buf));   

            vd->buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            vd->buf.memory      = V4L2_MEMORY_MMAP;
            //buf.memory      = V4L2_MEMORY_USERPTR;
            vd->buf.index       = i;

            rc = xioctl (vd->fd, VIDIOC_QUERYBUF, &vd->buf);
            ENC_DEBUG(" VIDEOC_QUERYBUF fd %d rc %d i %d\n", vd->fd, rc, i);
            if (rc == 0)
            {

                ENC_DEBUG("buf[%d]mmap*******\n", i);
                ENC_DEBUG("buf[%d].length %d\n", i, vd->buf.length);
                ENC_DEBUG("buf[%d].memory %p\n", i, (void *)vd->buf.memory);
                ENC_DEBUG("buf[%d].offset %p\n", i, (void *)vd->buf.m.offset);
                ENC_DEBUG("buf[%d].userptr %p\n", i, (void *)vd->buf.m.userptr);
                ENC_DEBUG("buf[%d].flags %08x\n", i, vd->buf.flags);
                mem_size += vd->buf.length;
                vd->mem_len[i]= vd->buf.length;
                vd->mem[i] = mmap (0 /* start anywhere */ ,
                                   vd->buf.length, PROT_READ, MAP_SHARED, vd->fd,
                                   vd->buf.m.offset);
                if(vd->mem[i] == MAP_FAILED)
                {
                    ENC_DEBUG("buf[%d].offset  %d mmap failed\n"
                              , i, vd->buf.m.offset);
                    rc = -1;
                }
                else
                {
                    memset(&vd->buf, 0 , sizeof(vd->buf));   

                    vd->buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                    vd->buf.memory      = V4L2_MEMORY_MMAP;
                    //buf.memory      = V4L2_MEMORY_USERPTR;
                    vd->buf.index       = i;

                    rc = xioctl (vd->fd, VIDIOC_QBUF, &vd->buf);
                    ENC_DEBUG(" VIDEOC_QBUF fd %d rc %d i %d\n", vd->fd, rc, i);
                }
            }
        }
    }
    return rc;
}

int small_pause(int usecs)
{
  // Nothing avaible currently, rest a bit and than continue
  struct timeval interval;
  
  interval.tv_sec = 0;
  interval.tv_usec = usecs;
  select(1, NULL, NULL, NULL, &interval);
  return;
}


// We have a buffer give it to the usb_ip system
// return the index of the buffer
// note that the user still has to set up the buffer type
int usb_set_buf(void *vd_in, OMX_BUFFERHEADERTYPE *buff, int port
                , void *cord, int buff_index, int usb_set)
{
    struct usb_Ip *vd = (struct usb_Ip *)vd_in;
    struct usb_omx_buffer *usb_buff;
    int usb_ix=-1;
    int j;
    //    if(vd == NULL) vd = &usb_in;
    //if(vd->fd < 0) return usb_ix;
    if (usb_set < 0) return usb_ix;
               
    if(vd->port_no == -1) vd->port_no = port;       

    usb_buff = vd->usb_buffers;
    // find the usb buffer for this OMX guy
    // else set one up
    for (j = 0; j < NUM_OMX_BUFFERS; j++)
      {
        if (j >= vd->num_buffs) 
          //if (usb_buff[j].state < 0) 
          {
            // Set up this as a new buffer
            usb_buff[j].state = 0;
            usb_buff[j].buff = buff;
            usb_buff[j].port = port;
            usb_buff[j].type = 0;
            usb_buff[j].cord = cord;
            usb_buff[j].buff_index = buff_index;

            usb_ix = j;
            vd->num_buffs = j+1;
            ENC_DEBUG("USB !!! allocated usb buffer %p (addr %p) at %d:%d type %d\n"
                      , buff, buff->pBuffer, port, j, usb_set);
            break;
          }
        //if ((usb_buff[j].cord == cord) && (usb_buff[j].buff_index == buff_index) )
          if (usb_buff[j].buff == buff)
          {
            usb_ix = j;
            break;
          }
      }
    return usb_ix;
}

OMX_BUFFERHEADERTYPE *usb_use_buf(void *vd_in, OMX_BUFFERHEADERTYPE *buff,
                                  void * handle, void *mcbd, void *xconfig, 
                                  int port, int usb_ix, int btype, void * obj, 
                                  int buff_index)
{
    int rc = -1;
    struct usb_Ip *vd = (struct usb_Ip *)vd_in;
    struct usb_omx_buffer *usb_buff;
    OMX_BUFFERHEADERTYPE *buffx = NULL;
    //if(vd == NULL) vd = &usb_in;
    usb_buff = vd->usb_buffers;
    // The same sort of code is to be found in the mjpeg_decoder
    if((usb_ix >=0) && (btype == RAW_BUFF_TYPE))
      {
        if(xconfig)
          {
            //xconfig->in_color_space  = OMX_COLOR_FormatYCbYCr;
            //int myrc = _set_media_cb_data(mcbd, port, xconfig);
            //if(myrc)ENC_DEBUG("usb_use_buf Med 42 use_buf set 422 color_space for stream ix %d", port);
          }
        //usb_buff[usb_ix].handle = handle; // use this handle
        usb_buff[usb_ix].state = BUFF_READY; // please fill me then use FILL THIS BUFFER
        usb_buff[usb_ix].obj = NULL; // not mjpeg decode
        usb_buff[usb_ix].buff_index = buff_index; // use this handle
        usb_buff[usb_ix].port = port; // use this handle
        buffx = usb_buff[usb_ix].buff;//cord->buf_headers[buff_index];
        rc = 0; 
      }
    else if((usb_ix >= 0) && (btype == MJ_BUFF_TYPE))   // fill buffer from mjpeg decoder
      {
        if(xconfig)
          {
            //xconfig->in_color_space  = OMX_COLOR_FormatYUV420SemiPlanar;
            //int myrc = _set_media_cb_data(mcbd, port, xconfig);
            //if(myrc)ENC_DEBUG("usb_use_buf Med 42 use_buf set 420 color_space for mjpeg ix %d", port);
          }
        //usb_buff[usb_ix].handle = handle; // use this handle
        usb_buff[usb_ix].state = BUFF_READY; // please fill me then use  EMPTY THIS BUFFER via the fill _complete
        usb_buff[usb_ix].obj = obj; // is mjpeg_decode
        usb_buff[usb_ix].buff_index = buff_index; // use this handle
        usb_buff[usb_ix].port = port; // use this handle
        buffx = usb_buff[usb_ix].buff;//cord->buf_headers[buff_index];
        rc = 0; 
        //TODO copy USB DATA to buffer rc = OMX_EmptyThisBuffer (vd->mjpeg_handle, buff);
      }
    return buffx;
}

// type is MJ_BUFF_TYPE or RAW_BUFF_TYPE
// State is BUFF_READY or BUFF_FULL or BUFF_CLEAR
#define NUM_BASE_BUFFS 30
int base_ix = 0;
OMX_BUFFERHEADERTYPE *base_buffs[NUM_BASE_BUFFS];

int setup_buffs(struct usb_Ip *vd, int num, int len, int btype)
{
  int i;
  int buff_index;
  int usb_ix;
  struct usb_omx_buffer *usb_buffp;
  void * handle = NULL;
  void * mcbd = NULL;
  void * config_data = NULL;
  
  OMX_BUFFERHEADERTYPE *buff;
  OMX_BUFFERHEADERTYPE *buffx;
  usb_buffp = vd->usb_buffers;
  for (i=0; i < num; i++)
  {
      // create a buffer
      buff = creat_buff(len);
      base_buffs[base_ix]= buff;

      buff_index = base_ix;
      if(base_ix <(NUM_BASE_BUFFS-1))base_ix++;

      // Add the buffer to the USB buffer queue
      // TYPE will be assigned to 0
      usb_ix = usb_set_buf(vd, buff, i, buff, buff_index, 1);

      if(usb_ix >= 0)
      {
          // Allocate type if needed
          if (usb_buffp[usb_ix].type == 0)
          {
              usb_buffp[usb_ix].type = btype;
              ENC_DEBUG("[%s] set  INPUT buffer %p type %d\n"
                        , "test", buff, btype);
          }
          // Set this buffer to be clear
          if (usb_buffp[usb_ix].type == btype) // TODO check state
          {
              usb_buffp[usb_ix].state = BUFF_CLEAR; // queue for buffer empty
              // Now pend it for the usb capture system
              buffx = usb_use_buf(vd, buff, handle, mcbd, config_data, i, usb_ix, btype, (void *)NULL, buff_index);
          }
      }
  }
  return num;
}

int usb_ip_enable(struct usb_Ip *vd)
{

    int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    int rc=-1;
    if (vd->isstreaming == 0)
      {
        rc = xioctl (vd->fd, VIDIOC_STREAMON, &type);
        if (rc < 0) 
          {
            rc = -errno;
            //vd->isstreaming = -1;
            ENC_DEBUG(" VIDEOC_STREAMON unable to start capture fd %d rc %d\n", vd->fd, rc);
            return rc;
          }
      }
    vd->isstreaming = 1;
    vd->streamcnt = 0;
    vd->usestream = 0;
    vd->mjpegcnt = 0;
    vd->usemjpeg = 0;
    //vd->mjpeg_handle = NULL;

    vd->grabcnt = 0;
    vd->errcnt = 0;
    vd->buffcnt = 0;
    ENC_DEBUG(" VIDEOC_STREAMON capture started fd %d rc %d\n", vd->fd, rc);
    return rc;
}

int usb_ip_disable (struct usb_Ip *vd)
{
    int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    int rc=-1;

    rc = ioctl (vd->fd, VIDIOC_STREAMOFF, &type);
    if (rc < 0) 
    {
        vd->isstreaming = 0;
        ENC_DEBUG(" VIDEOC_STREAMON unable to stop capture fd %d rc %d\n", vd->fd, rc);
        return rc;
    }
    vd->isstreaming = 0;
    return 0;
}

struct v4l2_queryctrl queryctrl;
struct v4l2_querymenu querymenu;


static void enumerate_menu(int fd)
{

  //static void enumerate_menu(void)
  {
        printf("  Menu items:\n");
      
	memset(&querymenu, 0, sizeof(querymenu));
	querymenu.id = queryctrl.id;
	
	for (querymenu.index = queryctrl.minimum;
	     querymenu.index <= queryctrl.maximum;
	     querymenu.index++) {
	  if (0 == ioctl(fd, VIDIOC_QUERYMENU, &querymenu)) {
	      printf("  %s\n", querymenu.name);
	  }
	}
    }

}

int list_usb_stuff(int fd)
{
  int rc = 0;    
  
  memset(&queryctrl, 0, sizeof(queryctrl));
    
    for (queryctrl.id = V4L2_CID_BASE;
	 queryctrl.id < V4L2_CID_LASTP1;
	 queryctrl.id++) {
        if (0 == ioctl(fd, VIDIOC_QUERYCTRL, &queryctrl)) {
	    if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED)
	      continue;

	    printf("Control %s\n", queryctrl.name);
	    
	    if (queryctrl.type == V4L2_CTRL_TYPE_MENU)
	      enumerate_menu(fd);
	} else {
	  if (errno == EINVAL)
	    continue;
	  
	  perror("VIDIOC_QUERYCTRL");
	  //exit(EXIT_FAILURE);
	  rc = -1;
	  break;
	}
    }
    if(rc == 0)
      {
	for (queryctrl.id = V4L2_CID_PRIVATE_BASE;;
	     queryctrl.id++) {
	  if (0 == ioctl(fd, VIDIOC_QUERYCTRL, &queryctrl)) {
	    if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED)
	      continue;
	    
	    printf("Control %s\n", queryctrl.name);
	    
	    if (queryctrl.type == V4L2_CTRL_TYPE_MENU)
	      enumerate_menu(fd);
	  } else {
	    if (errno == EINVAL)
	      break;
	
	    perror("VIDIOC_QUERYCTRL");
	    rc = -1;
            break;
	    //exit(EXIT_FAILURE);
	  }
	}
      }
    return rc;
}

int list_usb_cap(int fd)
{
    int rc = 0;    
    struct v4l2_capability cap;
    memset(&cap, 0, sizeof(cap));
    rc = xioctl(fd, VIDIOC_QUERYCAP, &cap);
    printf("VIDEOC_QUERYCAP rc %d\n", rc);

    if(rc == 0)
      {
	if (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)
	  {
	    printf("fd %d is a video capture device\n", fd);
	    
	  }
	if (cap.capabilities & V4L2_CAP_STREAMING)
	  {
	    printf("fd %d is a video streaming device\n", fd);
	    
	  }

      }

    struct v4l2_standard std;
    struct v4l2_input input;
    int index;
    rc = ioctl(fd, VIDIOC_G_INPUT, &index);
    if (rc == 0)
      {
	printf("VIDIOC_G_INPUT index %d\n", index);
      }
    
    
    for (index = 0; index < 32; index++)
      {
	memset(&input, 0, sizeof(input));
	input.index = index;
	rc =  ioctl(fd, VIDIOC_ENUMINPUT, &input);
	if(rc==0)printf("ENUM index %d  rc %d\n", index, rc);
	if (rc == 0)
	  {
	    printf("         input: %s\n",    input.name);
	    printf("        status: %d\n",    input.status);
	    printf("           std: %08x\n",  (unsigned int)input.std);
	    printf("  capabilities: %08x\n",  input.capabilities);
	    printf("\n");
	  }
      }
    rc = 0;
    for ( index = 0 ; (rc>= 0) && (index < 32); index++)
      {
	memset(&std, 0, sizeof(std));
	std.index = index;
	
	rc =  ioctl(fd, VIDIOC_ENUMSTD, &std);
	if (rc == 0)
	  {
	    printf("STD index %d  rc %d\n", index, rc);
	    printf("         name: %s\n",    std.name);
	  }
      }
    return rc;
}

int main (int argc, char ** argv)
{
  int rc = 0;
  int i;
  int use_mjpeg = 0;
  struct usb_Ip usb_ip;
  struct usb_Ip *vd = &usb_ip;
  int rc_int;
  int count = 128;
  int fd =  usb_open();
  if(fd >0) 
    {
      list_usb_cap(fd);
      //list_usb_stuff(fd);
      //return 0;
    }
  vd->num_buffs = 0;
  if(fd >0) 
    {
      // 5 buffers of type 1 for raw capture 
      setup_buffs(vd, 5, 1024*1024, RAW_BUFF_TYPE);
      // 5 buffers of type 2 for mjpeg capture 
      setup_buffs(vd, 5, 1024*1024, MJ_BUFF_TYPE);

      vd->fd =  fd;
      rc = usb_open_v4l(vd, use_mjpeg);
      vd->isstreaming = 0;

      printf(" After open rc = %d\n", rc);
      usb_ip_enable(vd);
      rc_int = -1;
      if(rc >= 0)
      {
          while(count > 0)
          {
            // Grab a buffer from the v4l system        
              rc_int = usb_grab_v4l(vd);
              count--;
              small_pause(1000000/30);
          }
      }
      printf(" running disable\n");
      if(rc_int == 0)usb_ip_disable(vd);
      printf(" running close \n");

      close(fd);
    }
  return 0;
}
