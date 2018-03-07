/*************************************************************************
	> File Name: capture.c
	> Author: shuixianbing
	> Mail: shui6666@126.com 
	> Created Time: 2016年09月23日 星期五 15时59分17秒
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <time.h>

#define WIDTH  640
#define HEIGHT 480

static int fd = -1;
struct buffer{
	void *start;
	size_t length;
};

unsigned int count;
unsigned read_num = 20;

static unsigned int n_buffers = 0;

struct buffer *buffers = NULL;

static int camera_init(void){
	struct v4l2_input inp;
	struct v4l2_streamparm parms;
	int ret =0;
	fd = open("/dev/video0", O_RDWR);
	if(!fd){
		printf("open failed\n");
		return -1;
	}

	ret = ioctl(fd, VIDIOC_S_INPUT, &inp);
	if(ret == -1){
		printf("VIDIOC_S_INPUT error\n");
		return -1;
	}

	parms.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	parms.parm.capture.timeperframe.numerator   = 1;
	parms.parm.capture.timeperframe.denominator = 30;
	parms.parm.capture.capturemode = 0x0002;

	ret = ioctl(fd, VIDIOC_S_PARM, &parms);
	if(ret == -1){
		printf("VIDIOC_S_PARAM error\n");
		return -1;
	}
	
	return 0;
}

static int camera_fmt_set(void){
	int ret = -1;
	struct v4l2_format fmt;
	
	memset(&fmt, 0, sizeof(fmt));
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width = WIDTH;
	fmt.fmt.pix.height = HEIGHT;
	fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_GREY;
	fmt.fmt.pix.field = V4L2_FIELD_NONE;

	ret = ioctl(fd, VIDIOC_S_FMT, &fmt);
	if(ret == -1){
		printf("VIDIOC_S_FMT error\n");
		return -1;
	}

	//just test vidioc_g_fmt
	ret = ioctl(fd, VIDIOC_G_FMT, &fmt);
	if(ret == -1){
		printf("VIDIOC_G_FMT error\n");
		return -1;
	}

	return 0;
}

static int req_frame_buffers(void){
	int ret = -1;
	int i;
	struct v4l2_requestbuffers req;

	memset(&req, 0, sizeof(req));
	req.count = 4;
	req.type  = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_MMAP;

	ret = ioctl(fd, VIDIOC_REQBUFS, &req);
	if(ret == -1){
		printf("VIDIOC_REQBUFS error\n");
		return -1;
	}

	buffers = calloc(req.count, sizeof(*buffers));
	
	for(n_buffers =0; n_buffers < req.count; ++n_buffers){
		struct v4l2_buffer buf;
		memset(&buf,0, sizeof(buf));

		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = n_buffers;

		if(-1 == ioctl(fd, VIDIOC_QUERYBUF, &buf))
		  printf("VIDIOC_QUERYBUF error\n");

		buffers[n_buffers].length = buf.length;
		buffers[n_buffers].start = mmap(NULL,
					buf.length,
					PROT_READ | PROT_WRITE,
					MAP_SHARED,
					fd,
					buf.m.offset);
		if(MAP_FAILED == buffers[n_buffers].start){
			printf("mmap failed\n");
			return -1;
		}
	}

	for(i=0; i< n_buffers; ++i){
		struct v4l2_buffer buf;
		memset(&buf, 0, sizeof(buf));

		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index  = i;

		if(-1 == ioctl(fd, VIDIOC_QBUF, &buf)){
			printf("VIDIOC_QBUF failed\n");
			return -1;
		}
	}

	return 0;
}

static int free_frame_buffers(void){
	int i;
	for(i=0; i<n_buffers; ++i){
		if(-1 == munmap(buffers[i].start, buffers[i].length)){
			printf("munmap error\n");
			return -1;
		}
	}
	return 0;
}

int process_image(void *addr, int length){
	FILE *fp;
	static int num = 0;
	char picture_name[30];

	sprintf(picture_name, "/mnt/sdcard/picture%d.yuv", num++);			
		if((fp=fopen(picture_name, "w")) == NULL){
			perror("Fail to fopen");
			exit(EXIT_FAILURE);
		}				
	fwrite(addr, length, 1, fp);
	usleep(500);
	fclose(fp);
									
	return 0;
}
static int read_frame(void){
	struct v4l2_buffer buf;
	void *bfstart = NULL;
	int j,len;

	memset(&buf, 0, sizeof(buf));
	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;

	if(-1 == ioctl(fd, VIDIOC_DQBUF, &buf))
	  return -1;
	assert(buf.index < n_buffers);

	if(count == read_num/2){
	bfstart = buffers[buf.index].start;
	len = buffers[buf.index].length;
	//drop the low 2 bit data
	for(j=0; j<len;j++)
	  *(unsigned char*)(bfstart+j) = *(unsigned char*)(bfstart + j) << 2;

	//process_image(buffers[buf.index].start,buffers[buf.index].length);
	process_image(bfstart, len);
	}
	if(-1 == ioctl(fd, VIDIOC_QBUF, &buf))
	  return -1;

	return 0;
}

int main(int argc, char **argv)
{
	enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	camera_init();

	camera_fmt_set();

	req_frame_buffers();

	if(-1== ioctl(fd, VIDIOC_STREAMON, &type)){
		printf("VIDIOC_STREAMON failed\n");
		return -1;
	}else
	  printf("VIDIOC_STREAMON ok\n");

	count = read_num;
	while(count-->0){
		for(;;){
			fd_set fds;
			struct timeval tv;
			int r;

			FD_ZERO(&fds);
			FD_SET(fd, &fds);

			tv.tv_sec = 5;
			tv.tv_usec = 0;

			r = select(fd+1, &fds, NULL, NULL, &tv);

			if(-1 == r){
				if(EINTR == errno)
				  continue;
				printf("select err\n");
			}
			if(0 == r){
				fprintf(stderr, "select timeout\n");
				return -1;
			}

			if(!read_frame())
			  break;
			else
			  return -1;
		}
	}


	if(-1== ioctl(fd, VIDIOC_STREAMOFF, &type)){
		printf("VIDIOC_STREAMOFF failed\n");
		return -1;
	}else
	  printf("VIDIOC_STREAMOFF ok\n");

	if(-1 == free_frame_buffers())
	  return -1;

	close(fd);

	return 0;
}
