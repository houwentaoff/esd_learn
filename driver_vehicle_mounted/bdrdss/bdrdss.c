#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <termios.h>


/**
 *@brief 打开串口
 *@param com_port:串口号
 *@return 打开串口的文件描述符
 */
int open_port(int com_port){
	int fd;
	struct termios options;
	char *dev[] = {"/dev/ttyS0", "/dev/ttyS1","/dev/ttyS2","/dev/ttyS3"};
	if((com_port < 0) || (com_port > 3)){
		printf("the com_port error\n");
		return -1;
	}

	if((fd = open(dev[com_port], O_RDWR|O_NOCTTY)) == -1){
		printf("Can't open the serial com %s\n", dev[com_port]);
		return -1;
	}else{
		printf("Open the com [%s] Success!\n", dev[com_port]);
	}
	
	tcgetattr(fd, &options);
	options.c_cflag |= (CLOCAL | CREAD);
	options.c_cflag &= ~CSIZE;
	options.c_cflag &= ~CRTSCTS;
	options.c_cflag |= CS8;
	options.c_cflag &= ~CSTOPB;
	options.c_iflag |= IGNPAR;
	options.c_oflag = 0;
	options.c_lflag = 0;
	cfsetispeed(&options, B115200);
	cfsetospeed(&options, B115200);
	tcsetattr(fd, TCSANOW, &options);

	return fd;
}

int main(int argc, char **argv){
	int fd;
	int ret;
	char BD_CCICA[] = "$CCICA,0,00*7B\r\n";
	char BD_TXA[] = "$CCTXA,244883,1,0,一*24\r\n";

	fd = open_port(3);
	if(fd < 0)
	  printf("open port error\n");

	//ret = write(fd, &BD_CCICA, sizeof(BD_CCICA));
	//printf("ret=%d\n", ret);
	ret = write(fd, &BD_TXA, sizeof(BD_TXA));
	printf("ret=%d\n", ret);
	
	close(fd);



}
