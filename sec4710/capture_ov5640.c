#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <assert.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>

#define V4L2_PIX_FMT_SBGGR10 v4l2_fourcc('B','G','1', '0')
#define V4L2_PIX_FMT_SGRBG10 v4l2_fourcc('B', 'A', '1', '0') 
#define VIDIOC_EXPOSURE        _IOWR('V', 7,int**)'')

typedef struct{
	void *start;
	int length;
} BUFTYPE;

BUFTYPE *user_buf;
int n_buffer = 0;
/**
 *打开摄像头设备
 * @param char *dev 设备路径
 * @return  设备fd
 */
int open_camera_device(char *dev){
	int fd;
 	fd = open(dev, O_RDWR);
	if(fd < 0){
	printf("open %s error\n", dev);
	exit(EXIT_FAILURE);
	}
	return fd;
}

void close_camera_device(int fd){
	int ret;
	ret = close(fd);
	if(ret == -1){
		perror("Fail to close fd\n");
		exit(EXIT_FAILURE);
	}
}

/**
 *映射摄像头内存到用户空间
 */
int init_mmap(int fd){
	int i=0;
	struct v4l2_requestbuffers reqbuf;
	int ret;
	bzero(&reqbuf, sizeof(reqbuf));
	reqbuf.count = 3;
	reqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	reqbuf.memory = V4L2_MEMORY_MMAP;
	/*申请驱动的缓存空间，可能会修改reqbuf.count的值
	修改为实际成功申请缓冲区个数*/
	printf("-----init_map\n");
	ret = ioctl(fd, VIDIOC_REQBUFS, &reqbuf);
	if(ret == -1){
		perror("Fail to ioctl VIDIOC_REQBUFS");
		return -1;
	}
	
	n_buffer = reqbuf.count;
	printf("申请成功的缓冲区个数 [%d]\n", n_buffer);

	user_buf = calloc(reqbuf.count, sizeof(*user_buf));
	if(user_buf == NULL){
		fprintf(stderr, "Out of memory\n");
		return -1;
	}
	//将内核缓冲区映射到用户进程空间
	for(i=0; i<reqbuf.count; i++){
		struct v4l2_buffer buf;//代表驱动的一帧
		bzero(&buf, sizeof(buf));
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = i;
		//查询申请到内核缓冲区的信息
	if(-1 == ioctl(fd, VIDIOC_QUERYBUF, &buf)){
		perror("Fail to ioctl:VIDIOC_QUERYBUF");
		return -1;
	}

	user_buf[i].length = buf.length;
	printf("一帧的长度 [%d]\n", buf.length);
	user_buf[i].start =
		mmap(
			NULL,
			buf.length,
			PROT_READ | PROT_WRITE,
			MAP_SHARED,
			fd,
			buf.m.offset
	);
	
	if(MAP_FAILED == user_buf[i].start){
		perror("Fail to mmap");
		return -1;
	}
	}
	return 0;
}

/**
 *初始化视频设备
 *&param fd
 *&return 0 成功 1 失败
 */
int init_camera_device(int fd){
	struct v4l2_fmtdesc fmt;//视频格式
	struct v4l2_capability cap;//当前属性结构
	struct v4l2_format stream_fmt; //当前视频格式结构体
	int level=10;
	int *mode_level = &level;
	int ret;
	printf("-----%s-----\n", __func__);
	//1.当前视频设备支持的视频格式
	memset(&fmt, 0, sizeof(fmt));
	fmt.index = 0;
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	//2.查询当前设备支持的视频格式
	while((ret = ioctl(fd, VIDIOC_ENUM_FMT, &fmt)) == 0){
		fmt.index++;
	printf("{支持的视频格式有=%c%c%c%c}, 描述'%s'\n",
		fmt.pixelformat & 0xff,  (fmt.pixelformat >> 8)&0xff,
		(fmt.pixelformat >> 16)&0xff, (fmt.pixelformat>>24)&0xff,
		fmt.description);
	}
	//3.设置摄像头采集数据格式，长宽，图像格式
	stream_fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE; //视频捕捉设备
	stream_fmt.fmt.pix.width = 640;
	stream_fmt.fmt.pix.height = 480;
	//stream_fmt.fmt.pix.width = 1633;
	//stream_fmt.fmt.pix.height = 1244;
	stream_fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV ;
	 //V4L2_PIX_FMT_SBGGR10;
	//V4L2_PIX_FMT_UYVY;
		//V4L2_PIX_FMT_SBGGR10;
		//V4L2_PIX_FMT_YUYV;
	//V4L2_PIX_FMT_SGRBG10;
	stream_fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;
	printf("--------1--------\n");
	if(-1 == ioctl(fd, VIDIOC_S_FMT, &stream_fmt)){
		perror("Fail to ioctl");
		return -1;
	}

	
	printf("------2--------\n");
	//4.初始化视频采集方式mmap
	if(-1==init_mmap(fd))
		return -1;

	return 0;
}

//启动数据采集
int start_capturing(int fd){
	unsigned int i;
	enum v4l2_buf_type type;
	printf("------%s-----\n", __func__);
	//将申请的内核缓冲区放入一个队列中
	for(i=0; i<n_buffer; i++){
		struct v4l2_buffer buf;
		bzero(&buf, sizeof(buf));
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = i;

	if(-1 == ioctl(fd, VIDIOC_QBUF, &buf)){
		perror("Fail to ioctl VIDIOC_QBUF");
		return -1;
	}
	}
	//开始采集数据
	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if(-1 == ioctl(fd, VIDIOC_STREAMON, &type)){
		printf("i=%d\n", i);
		perror("Fail to ioctl VIDIOC_STREAMON");
		return -1;
	}
	printf("开始采集数据 %s\n", __func__);
	return 0;
}

//将采集好的数据放到文件中
int process_image(void *addr, int length){
	FILE *fp;
	static int num = 0;
	char picture_name[20];

	sprintf(picture_name, "/etc/picture%d.yuv", num++);

	if((fp=fopen(picture_name, "w")) == NULL){
		perror("Fail to fopen");
		exit(EXIT_FAILURE);
	}

	fwrite(addr, length, 1, fp);
	usleep(500);
	fclose(fp);

	return 0;
}

int read_frame(int fd){
	struct v4l2_buffer buf;
	unsigned int i;

	bzero(&buf, sizeof(buf));
	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;
	//从队列中取缓冲区
	if(-1 == ioctl(fd, VIDIOC_DQBUF, &buf)){
		perror("Fail to ioctl VIDIOC_DQBUF");
		exit(EXIT_FAILURE);
	}

	assert(buf.index < n_buffer);
	//读取进程空间的数据到一个文件中
	process_image(user_buf[buf.index].start, 
	user_buf[buf.index].length);

	if(-1 == ioctl(fd, VIDIOC_QBUF, &buf)){
		perror("Fail to ioctl VIDIOC_QBUF");
		exit(EXIT_FAILURE);
	}

	return 1;
}

int mainloop(int fd){
	int count = 5;
	
	while(count-- > 0){
		for(;;){
		fd_set fds;
		struct timeval tv;
		int r;
	FD_ZERO(&fds);
	FD_SET(fd, &fds);
	tv.tv_sec = 2;
	tv.tv_usec = 0;

	r = select(fd+1, &fds, NULL, NULL, &tv);
	if(-1 == r){
	 if(EINTR == errno)
		continue;
	perror("Fail to select");
	exit(EXIT_FAILURE);
	}
	if(0 == r){
		fprintf(stderr, "select Timeout\n");
		exit(EXIT_FAILURE);
	}

	if(read_frame(fd))
		break;
	}
	}
	return 0;
}

void stop_capturing(int fd){
	enum v4l2_buf_type type;

	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if(-1 == ioctl(fd, VIDIOC_STREAMOFF, &type)){
		perror("Fail to ioctl VIDIOC_STREAMOFF");
		exit(EXIT_FAILURE);
	}
	return ;
}

void uninit_camera_device(){
	unsigned int i;
	for(i=0; i<n_buffer; i++){
		if(-1 == munmap(user_buf[i].start,
		user_buf[i].length)){
		exit(EXIT_FAILURE);
	}
	}
	free(user_buf);
	return ;
}

int main(){
	int fd;
	fd = open_camera_device("/dev/video0");
	if(-1 == init_camera_device(fd))
		return 0;


    struct v4l2_frmsizeenum  vid_frmsize;
    memset(&vid_frmsize, 0, sizeof(struct v4l2_frmsizeenum));	
    ioctl(fd,VIDIOC_TRY_FMT, &vid_frmsize);	
    		  

	if(start_capturing(fd))//映射数据到用户空间
		return 0;
	mainloop(fd);//读取并处理数据
	stop_capturing(fd);
	uninit_camera_device(fd);
	close_camera_device(fd);
	return 0;
	
}
