#include <unistd.h>
#include <stdio.h> 
#include <curses.h> 
#include <string.h>
#include <assert.h> 
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <linux/if_tun.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <fcntl.h>
#include <linux/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <termios.h>
int tun_creat(char *dev, int flags)
{
	struct ifreq ifr;
	int fd, err;
	assert(dev != NULL);
	if((fd=open("/dev/net/tun", O_RDWR)) < 0)
		return fd;
	memset(&ifr, 0, sizeof(ifr));
	ifr.ifr_flags |= flags;
	if(*dev != '\0')
		strncpy(ifr.ifr_name, dev, IFNAMSIZ);
	if((err = ioctl(fd, TUNSETIFF, (void *)&ifr)) < 0)
	{
		close(fd);
		return err;
	}
	strcpy(dev, ifr.ifr_name);
	return fd;
}
int serial_creat(char *dev)
{
	int fd;
	int status;
	struct termios opt;
	fd = open(dev, O_RDWR);
	if(fd == -1)
		perror("open serial error\n");
	tcgetattr(fd, &opt);
	tcflush(fd, TCIOFLUSH);
	/*set bitrate*/
	cfsetispeed(&opt, B9600);
	cfsetospeed(&opt, B9600);
	/*set data bit*/
	opt.c_cflag &= ~CSIZE;
	opt.c_cflag |= CS8;
	/*set parity*/
	//opt.c_cflag |= (PARODD | PARENB);
	//opt.c_iflag |= INPCK;
	opt.c_cflag &= ~PARENB;
	opt.c_iflag &= ~INPCK;
	/*set stop bit*/
	opt.c_cflag &= ~CSTOPB;
	/*close flow control*/
	opt.c_iflag &= ~(ICRNL | IXON);
	/*set timeout*/
	opt.c_cc[VTIME] = 150;
	opt.c_cc[VMIN] = 0;
	/*raw mode*/
//	opt.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
//	opt.c_oflag &= ~OPOST;

	status = tcsetattr(fd, TCSANOW, &opt);
	if(status != 0)
	{
		perror("tcsetattr fd error\n");
		return -1;
	}
	tcflush(fd, TCIOFLUSH);
	return fd;
}
int serial_send(int fd, char *buf, int num)
{
	int ret;
	unsigned char key[2];


	ret = write(fd, buf, num);
	if(ret != num)
		printf("send buf error\n");

	key[0] = 'c';
	key[1] = 'k';
	ret = write(fd, key, 2);

	return num;	
}

int serial_recv(int fd, char *buf)
{
	int ret, i;
	unsigned char key;
	i = 0;
	while(1)
	{
		ret = read(fd, &key, 1);
		if(key == 'c')
		{
			read(fd, &key, 1);
			if(key == 'k')
				return i;
			else
			{
				buf[i] = 'c';
				i++;
				buf[i + 1] = key;
				i++;
			}
		}
		else
		buf[i] = key;
		i++;
	
	}
		
	
}
int main()
{
	int tun, fd, ret, serialret, i;
	char tun_name[IFNAMSIZ];
	unsigned char buf[4096];
	unsigned char buf2[4096];
	tun_name[0] = '\0';
	tun = tun_creat(tun_name, IFF_TAP | IFF_NO_PI);
	if(tun < 0)
	{
		perror("tun_create");
		return 1;
	}
	fd = serial_creat("/dev/ttyUSB0");

	printf("TUN name is %s\n", tun_name);
	while(1)
	{
		ret = read(tun, buf, sizeof(buf));
		printf("tun read size %d\n", ret);
		if(ret < 0)
		{
			printf("tun read error\n");
		}
		serialret = serial_send(fd, buf, ret);
		for(i = 0; i< serialret; i++)
			printf("%d,",buf[i]);
		printf("\n");
		printf("serial wirte size %d\n", serialret);
		if(serialret < 0)
		{
			printf("serial write error\n");
		}
		serialret = serial_recv(fd, buf2);
		printf("serial read size %d\n", serialret);
		if(serialret < 0)
		{
			printf("serial read error\n");
		}
		//ret = write(tun, buf2, serialret);
		ret = write(tun, buf2, serialret);
		printf("tun write size %d\n", ret);
		if(ret < 0)
		{
			printf("tun write error%d\n", serialret);
		}

	}
}
