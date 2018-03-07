/*
 *imx6 uart test
 */
#include <termio.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <stdlib.h>

#define DEFAULT_RATE 115200
char *command;
int trun;
int rrun;
static unsigned int baudrate_map(unsigned long b){
	unsigned int ret;
	switch(b){
		case 110:
			ret = B110;
			break;
		case 300:
			ret = B300;
			break;
		case 1200:
			ret = B1200;
			break;
		case 2400:
			ret = B2400;
			break;
		case 4800:
			ret = B4800;
			break;
		case 9600:
			ret = B9600;
			break;
		case 19200:
			ret = B19200;
			break;
		case 38400:
			ret = B38400;
			break;
		case 57600:
			ret = B57600;
			break;
		case 115200:
			ret = B115200;
			break;
	}
	return ret;
}
//打开串口，返回文件描述符
int open_port(int argc, char **argv)
{
	int fd;
	int i;
	unsigned long baudrate = DEFAULT_RATE;
	struct termios options;
	fd = open(argv[1], O_RDWR | O_NOCTTY);
	if (fd == -1){
		printf("open_port: unable to open serial\n");
		return -1;
	}
	printf("open the serial [%s]\n", argv[1]);

	fcntl(fd, F_SETFL, 0);//改变open设置的标志
	tcgetattr(fd, &options);
	options.c_cflag &= ~CSTOPB;//停止位1位
	options.c_cflag &= ~CSIZE;
	options.c_cflag |= PARENB;
	options.c_cflag &= ~PARODD;
	options.c_cflag |= CS8;
	options.c_cflag &= ~CRTSCTS;

	options.c_lflag &= ~(ICANON | IEXTEN | ISIG | ECHO);
	options.c_oflag &= ~OPOST;
	options.c_iflag &= ~(ICRNL | INPCK | ISTRIP | IXON | BRKINT);
	
	options.c_cc[VMIN]  = 1;
	options.c_cc[VTIME] = 0;

	options.c_cflag |= (CLOCAL | CREAD);

	for(i=2; i < argc; i++){
		if(!strcmp(argv[i], "-S")){
			options.c_cflag |= CSTOPB;
			continue;
		}
		if(!strcmp(argv[i], "-O")){
			options.c_cflag |= PARODD;
			options.c_cflag &= ~PARENB;
			continue;
		}
		if(!strcmp(argv[i], "-E")){
			options.c_cflag &= ~PARODD;
			options.c_cflag |= PARENB;
			continue;
		}
		if(!strcmp(argv[i], "-HW")){
			options.c_cflag |= CRTSCTS;
			continue;
		}
		if(!strcmp(argv[i], "-B")){
			i++;
			baudrate = atoi(argv[i]);
			if(!baudrate_map(baudrate))
				baudrate = DEFAULT_RATE;
			continue;
		}
		if(!strcmp(argv[i], "-CMD")){
			i++;
			command = argv[i];
			continue;
		}

	}
	if(baudrate){
		cfsetispeed(&options, baudrate_map(baudrate));
		cfsetospeed(&options, baudrate_map(baudrate));
	}
	tcsetattr(fd, TCSANOW, &options);
	printf("UART %lu, %dbit, %dstop, %s, HW flow %s, CMD %s\n",
			baudrate, 8,
			(options.c_cflag & CSTOPB)?2:1,
			(options.c_cflag & PARODD)?"PARODD":"PARENB",
			(options.c_cflag & CRTSCTS)?"enabled":"disabled",
			command
			);



	return fd;
}

static void print_usage(const char *pname){
	printf("Usage:%s device [-S] [-O] [-E] [-HW] [-B baudrate] [-CMD command]"
			"\n\t'-S' for 2 stop bit"
			"\n\t'-O' for PARODD"
			"\n\t'-E' for PARENB"
			"\n\t'-HW' for HW flow control enable"
			"\n\t'-B baudrate' for different baudrate"
			"\n\t'-CMD command' for the uart send command\n", pname);
}

typedef struct{
	int fd;
	char *buf;
}thread_info;

void *Uartsend(void *arg){
	int size;
	//char *buf;
	thread_info *tinfo = (thread_info *)arg;
	size = strlen(tinfo->buf);
	//buf = (char *)malloc(size);
	while(trun){
		sleep(1);
		write(tinfo->fd, tinfo->buf, size);
	}
	printf("size=%d buf=%s\n", size, tinfo->buf);
	
	return 0;
}

void *Uartread(void *arg){
	char *buf;
	thread_info *tinfo = (thread_info *)arg;
	int iocount;
	
	while(rrun){
		iocount = 0;
		ioctl(tinfo->fd, FIONREAD, &iocount);
		printf("iocount=%d\n",iocount);
		if(!iocount)
			continue;
		buf = (char *)malloc(iocount);
		read(tinfo->fd, buf, iocount);
		printf("[reply]:[%s]\n", buf);
		free(buf);
	}

	return 0;
}
int main(int argc, char **argv){
	pthread_t p_Uartsend, p_Uartread;	
	void *thread_res;
	int com_fd;
	int ret;
	thread_info *tinfo;

	if(argc <2 || strncmp(argv[1],"/dev/tty",8)){
		print_usage(argv[0]);
		return -1;
	}
	com_fd = open_port(argc, argv);
	trun = 1;
	rrun = 1;
	tinfo->fd  = com_fd;
	tinfo->buf = command;
	ret = pthread_create(&p_Uartsend, NULL, Uartsend,tinfo);
	if(ret < 0){
		printf("pthread_create error\n");
		return -1;
	}
	ret = pthread_create(&p_Uartread, NULL, Uartread, tinfo);
	if(ret < 0){
		printf("pthread_create error\n");
		return -1;
	}
	trun = 0;
	ret = pthread_join(p_Uartsend, &thread_res);
	if(ret < 0){
		printf("fail to stop Uartsend thread\n");
		return -1;
	}

//	sleep(1);
	rrun = 0;
	ret = pthread_join(p_Uartread, &thread_res);
	if(ret < 0){
		printf("fail to stop Uartread thread\n");
		return -1;
	}



	return 0;
}
