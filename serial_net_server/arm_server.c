#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <assert.h>
#include <linux/if_tun.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <errno.h>
//#include <curses.h>

#define BUFFER_SIZE 2048

#define END 0300
#define ESC 0333
#define ESC_END 0334
#define ESC_ESC 0335
#define _DEBUG
int tun_creat(char *dev, int flags){
	struct ifreq ifr;
	int fd,err;
	assert(dev != NULL);
	fd = open("/dev/tun", O_RDWR);
	if(fd < 0)
	{
		printf("creat tun device error\n");
		return fd;
	}
	printf("creat tun device ok\n");
	memset(&ifr, 0, sizeof(ifr));
	ifr.ifr_flags |= flags;
	if(*dev != '\0')
		strncpy(ifr.ifr_name, dev, IFNAMSIZ);
	if((err = ioctl(fd, TUNSETIFF, (void *)&ifr)) < 0){
		close(fd);
		return err;
	}
	strcpy(dev, ifr.ifr_name);
	return fd;
}

int serial_creat(char *dev){
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

/*pack the frame and prepare for sending the packets*/
int frame_pack(unsigned char *src, unsigned char *des,
		int len){
	unsigned char *ptr = des;
	unsigned char c;
	*ptr++ = END;
	while(len-- > 0){
		switch(c = *src++){
			case END:
				*ptr++ = ESC;
				*ptr++ = ESC_END;
				break;
			case ESC:
				*ptr++ = ESC;
				*ptr++ = ESC_ESC;
				break;
			defalut:
				*ptr++ = c;
				break;
		}
	}
	*ptr++ = END;
	return(ptr - des);
}

/*unpack the frame*/
int frame_unpack(unsigned char *src, unsigned char *des){
	unsigned char *ptr = des;
	unsigned char c;
	int count = 0;
	int esc_flag = 0;
	if(*src++ != END)
		return 0;
	while(((c=*src++)!=END) && (count < BUFFER_SIZE)){
		switch(c){
			case ESC_END:
				if(esc_flag == 1){
					*ptr++ = END;
					esc_flag = 0;
				}else
					*ptr++ = c;
				break;
			case ESC_ESC:
				if(esc_flag == 1){
					*ptr++ = ESC;
					esc_flag = 0;
				}
				break;
			case END:
				esc_flag = 0;
				break;
			case ESC:
				esc_flag = 1;
				break;
			default:
				*ptr++ = c;
				esc_flag = 0;
				break;
		}
	}
	if(count >= BUFFER_SIZE){
		printf("some error happen in frame unpack()\n");
		return BUFFER_SIZE;
	}
	return (ptr-des);
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
int main(int argc, char **argv){
	int com_fd,net_fd;
	unsigned char buffer_rec[BUFFER_SIZE];
	unsigned char buffer_tx[BUFFER_SIZE];
	unsigned char buffer_tx_pack[BUFFER_SIZE];
	unsigned char buf[4096];
	unsigned char buf2[4096];

	unsigned char buffer[BUFFER_SIZE];
	unsigned char *buffer_ptr;
	unsigned char *rec_ptr;
	unsigned char *tx_ptr;
	int flag;
	int nbytes;
	pid_t pid;
	int tx_length;
	int rec_length;
	int i;
	int ret;
	rec_ptr = buffer_rec;
	tx_ptr = buffer_tx;
	buffer_ptr = buffer;
	int rec_count;
	int tx_count;
	char tun_name[IFNAMSIZ];
	if(argc != 2){
		printf("Usage:[cmd] [/dev/tty*] \n");
		return -1;
	}
	tun_name[0]='\0';
	com_fd = serial_creat(argv[1]);
	net_fd = tun_creat(tun_name, IFF_TAP | IFF_NO_PI);
	if(com_fd <0 || net_fd < 0){
		close(com_fd);
		close(net_fd);
		return 0;
	}
	printf("TUN name is %s\n", tun_name);
	pid = fork();
	if(pid <0){
		printf("can not creat the process\n");
		close(com_fd);
		close(net_fd);
		return 0;
	}
	if(pid == 0){
		/*read data from /dev/tty and write the data
		 into buffer_rec*/
		for(;;){
			printf("----------pid=0\n");
			ret = read(net_fd, buf, sizeof(buf));
			printf("tun read size %d\n", ret);
			ret = serial_send(com_fd, buf, ret);
			printf("write to serial:\n");
			for(i=0; i<ret; i++)
				printf("[%d]",buf[i]);
			printf("\n");
#if 0 
			flag = 0;
			rec_count = 0;
			buffer_ptr = buffer;
			while((nbytes = read(com_fd, rec_ptr, BUFFER_SIZE -1))>0){
				rec_length = 0;
				for(i=0; i<nbytes; i++){
					rec_count += 1;
					rec_length = rec_length+1;
					if(buffer_rec[i] == END){
						flag = flag+1;
						if(flag == 2)
							break;
					}
				}

				if(rec_count < BUFFER_SIZE){
					memcpy(buffer_ptr, rec_ptr, rec_length);
					buffer_ptr += rec_length;
					rec_length = 0;
				}
				if(flag == 2){
					flag = 0;
					buffer_ptr = buffer;
					memset(buffer_rec, 0, sizeof(buffer_rec));
					rec_ptr = buffer;
					rec_count = frame_unpack(buffer_ptr, rec_ptr);

					#ifdef _DEBUG
					printf("\n Service recieve:\n");
					for(i=0; i<rec_count; i++)
						printf(" %02x", rec_ptr[i]&0xff);
					printf("\n");
					#endif
					write(net_fd, rec_ptr, rec_count);
				}
			}
#endif		
		}
	}else if(pid >0){
		int length;
		for(;;){
			printf("----------pid=1\n");
			ret = serial_recv(com_fd, buf2);
			printf("serial read size %d\n", ret);
			for(i=0; i<ret; i++)
				printf("[%d]", buf2[i]);
			ret = write(net_fd, buf2, ret);
			printf("\ntun write\n");
#if 0
			/*read data from net ,and write the data into
			 buffer_rec*/
			tx_ptr = buffer_tx;
			if((nbytes = read(net_fd, tx_ptr, BUFFER_SIZE-1))>0){
				tx_count = frame_pack(tx_ptr, buffer_tx_pack,
						nbytes);
				#ifdef _DEBUG
				printf("\nSend bytes:%d\n",nbytes);
				printf("\nServer send\n");
				for(i=0; i<tx_count; i++)
					printf(" %02x", buffer_tx_pack[i]);
				printf("\n");
				#endif
				tx_length = 0;
				tx_ptr = buffer_tx_pack;
				while(tx_length < tx_count){
					length = write(com_fd, tx_ptr, tx_count-tx_length);
					tx_length = tx_length + length;
					tx_ptr += tx_length;
				}
			}
#endif
		}
	}
	close(com_fd);
	close(net_fd);
	return 0;
}
