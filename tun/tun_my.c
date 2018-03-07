/**
  *test the virtual net
*/
#include <unistd.h>
#include <stdio.h>
//#include <curses.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <unistd.h>
#include <linux/if_tun.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <linux/if.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>

int tun_create(char *dev, int flags)
{

	struct ifreq ifr;
	int fd,err;
	assert(dev != NULL);
	if((fd = open("/dev/net/tun", O_RDWR))<0)
	{
		return fd;
	}
	memset(&ifr, 0, sizeof(ifr));
	ifr.ifr_flags |= flags;
	if(*dev != '\0')
		strncpy(ifr.ifr_name, dev, IFNAMSIZ);
	if((err = ioctl(fd, TUNSETIFF, (void*)&ifr))<0){
		close(fd);
		return err;
	}
	strcpy(dev, ifr.ifr_name);
	return fd;
}

int main(){
	int tun,ret;
	char tun_name[IFNAMSIZ];
	unsigned char buf[4096];
	tun_name[0]='\0';

	tun = tun_create(tun_name, IFF_TUN | IFF_NO_PI);
	if(tun < 0){
		perror("tun_create");
		return 1;
	}

	printf("TUN name is %s\n", tun_name);
	while(1){
		unsigned char ip[4];
		ret = read(tun, buf, sizeof(buf));
		if(ret < 0 )
			break;
		int i=0;
		for(i=0; i<100; i++)
		printf("%d ", buf[i]);
		printf("\n");
		memcpy(ip, &buf[12], 4);
		memcpy(&buf[12], &buf[16], 4);
		memcpy(&buf[16], ip, 4);
		buf[20] = 0;
		*((unsigned short *)&buf[22]) += 8;
		printf("read %d bytes\n", ret);
		ret = write(tun, buf, ret);
		printf("write %d bytes\n", ret);
	
	}
	return 0;
}
