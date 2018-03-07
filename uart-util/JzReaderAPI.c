#include "JzReaderAPI.h"

char CMD_SOF[1]={0x55};
char CHECK0_CMD[2]={0x56, 0x56};
char CHECK1_CMD[2]={0x56, 0x57};
unsigned char uSizeOfTagID;

// crc lookup table for POLYNOMIAL = x^16 + x^15 + x^2 + 1
unsigned short crctable[256] = {
 0x0000, 0x8005, 0x800f, 0x000a, 0x801b, 0x001e, 0x0014, 0x8011, 
 0x8033, 0x0036, 0x003c, 0x8039, 0x0028, 0x802d, 0x8027, 0x0022, 
 0x8063, 0x0066, 0x006c, 0x8069, 0x0078, 0x807d, 0x8077, 0x0072, 
 0x0050, 0x8055, 0x805f, 0x005a, 0x804b, 0x004e, 0x0044, 0x8041, 
 0x80c3, 0x00c6, 0x00cc, 0x80c9, 0x00d8, 0x80dd, 0x80d7, 0x00d2, 
 0x00f0, 0x80f5, 0x80ff, 0x00fa, 0x80eb, 0x00ee, 0x00e4, 0x80e1, 
 0x00a0, 0x80a5, 0x80af, 0x00aa, 0x80bb, 0x00be, 0x00b4, 0x80b1, 
 0x8093, 0x0096, 0x009c, 0x8099, 0x0088, 0x808d, 0x8087, 0x0082, 
 0x8183, 0x0186, 0x018c, 0x8189, 0x0198, 0x819d, 0x8197, 0x0192, 
 0x01b0, 0x81b5, 0x81bf, 0x01ba, 0x81ab, 0x01ae, 0x01a4, 0x81a1, 
 0x01e0, 0x81e5, 0x81ef, 0x01ea, 0x81fb, 0x01fe, 0x01f4, 0x81f1, 
 0x81d3, 0x01d6, 0x01dc, 0x81d9, 0x01c8, 0x81cd, 0x81c7, 0x01c2, 
 0x0140, 0x8145, 0x814f, 0x014a, 0x815b, 0x015e, 0x0154, 0x8151, 
 0x8173, 0x0176, 0x017c, 0x8179, 0x0168, 0x816d, 0x8167, 0x0162, 
 0x8123, 0x0126, 0x012c, 0x8129, 0x0138, 0x813d, 0x8137, 0x0132, 
 0x0110, 0x8115, 0x811f, 0x011a, 0x810b, 0x010e, 0x0104, 0x8101, 
 0x8303, 0x0306, 0x030c, 0x8309, 0x0318, 0x831d, 0x8317, 0x0312, 
 0x0330, 0x8335, 0x833f, 0x033a, 0x832b, 0x032e, 0x0324, 0x8321, 
 0x0360, 0x8365, 0x836f, 0x036a, 0x837b, 0x037e, 0x0374, 0x8371, 
 0x8353, 0x0356, 0x035c, 0x8359, 0x0348, 0x834d, 0x8347, 0x0342, 
 0x03c0, 0x83c5, 0x83cf, 0x03ca, 0x83db, 0x03de, 0x03d4, 0x83d1, 
 0x83f3, 0x03f6, 0x03fc, 0x83f9, 0x03e8, 0x83ed, 0x83e7, 0x03e2, 
 0x83a3, 0x03a6, 0x03ac, 0x83a9, 0x03b8, 0x83bd, 0x83b7, 0x03b2, 
 0x0390, 0x8395, 0x839f, 0x039a, 0x838b, 0x038e, 0x0384, 0x8381, 
 0x0280, 0x8285, 0x828f, 0x028a, 0x829b, 0x029e, 0x0294, 0x8291, 
 0x82b3, 0x02b6, 0x02bc, 0x82b9, 0x02a8, 0x82ad, 0x82a7, 0x02a2, 
 0x82e3, 0x02e6, 0x02ec, 0x82e9, 0x02f8, 0x82fd, 0x82f7, 0x02f2, 
 0x02d0, 0x82d5, 0x82df, 0x02da, 0x82cb, 0x02ce, 0x02c4, 0x82c1, 
 0x8243, 0x0246, 0x024c, 0x8249, 0x0258, 0x825d, 0x8257, 0x0252, 
 0x0270, 0x8275, 0x827f, 0x027a, 0x826b, 0x026e, 0x0264, 0x8261, 
 0x0220, 0x8225, 0x822f, 0x022a, 0x823b, 0x023e, 0x0234, 0x8231, 
 0x8213, 0x0216, 0x021c, 0x8219, 0x0208, 0x820d, 0x8207, 0x0202 
};
int m_isReturn = 0;
int m_isReturn_r = 0;//是否返回确认
void SetReturn(int isreturn){
	m_isReturn_r = isreturn;
}

/**
 *@brief crc计算
*/
unsigned short crc16(unsigned char pMsg, unsigned short crc){
	crc = (crc<<8)^crctable[(crc>>8)^pMsg];
	return crc;
}
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
	cfsetispeed(&options, B19200);
	cfsetospeed(&options, B19200);
	tcsetattr(fd, TCSANOW, &options);

	return fd;
}
/**
 * @brief 打开读写器
 * @param com_port:要打开的串口号[0,1,2,3]
 * @return 串口文件描述符(重要)
 */
int XCOpen(int com_port){
	return open_port(com_port);
}
/**
 * @brief 关闭读写器
 * @param fd:串口文件描述符
 * @return 0:成功，-1失败
 */
int XCClose(int fd){
	return close(fd);
}

/**
 *@brief 从串口读取一个字节数据
 *返回成功：读取到的数据，失败-1
 */
int sio_getch(int fd){
	unsigned char readbuffer[1]={0};
	int pRet = -1;
	int retval;
	fd_set rfds;
	struct timeval tv;
	int flag=1;
	int nread = 0;
	tv.tv_sec = 1;
	tv.tv_usec = 0;
	printf("flag=%d\n",flag);
	while(flag){
		FD_ZERO(&rfds);
		FD_SET(fd,&rfds);
		retval = select(fd+1,&rfds,NULL,NULL,&tv);
		printf("retval =%d\n",retval);
		if(retval == -1){
			perror("select()");
			break;
		}else if(retval){
			nread = read(fd,readbuffer,1);
			printf("nread=%d\n",nread);
			if(nread>0){
				pRet = readbuffer[0];
			}else{
				printf("sio_getch error\n");
				return -1;
			}
		}else{
			printf("no data\n");
			flag = 0;
			break;
		}
	}
printf("pRet=%d\n",pRet);
	return pRet;
}

/**
 *@brief 向串口写入一个字节数据
 *努力写3次
 *@return 返回1成功 0失败
 */
int sio_putch(int fd, unsigned char writebuf){
	int nwrite = 0;
	int pRet = 0;
	int i;
	for(i=0; i<3; i++){
		nwrite = write(fd,&writebuf,1);
		if(nwrite>0){
			pRet = 1;
			break;
		}
	}
	return pRet;
}

/**
 * @brief 发一个字节到串口，如果是0x55,发0x56,0x56
 * 如果是0x56,发0x56,0x57
 */
int com_send(int fd, unsigned char in){
	unsigned char temp[1];
	temp[0] = in;
	if(temp[0]==0x55){
		if(write(fd, CHECK0_CMD, 2)<0){
			printf("com_send error\n");
			return -1;
		}
	}else if(temp[0] == 0x56){
		if(write(fd, CHECK1_CMD, 2)<0){
			printf("com_send error\n");
			return -1;
		}
	}else {
		if(write(fd, temp, 1)<0){
			printf("com_send error\n");
			return -1;
		}
	}
	return 1;
}
/**
 * @brief 发一个字节到串口如果是0x55,发0x56,0x56
 * 如果是0x56,发0x56,0x57
 */
int comm_send(int fd, unsigned char in){
	int revalue;
	int iwhile;
	if(fd <=0)
		return 0;
	if(in==0x55){
		revalue = 0;
		iwhile = 0;
		//发0x56到成功
		while(revalue!=1){
			iwhile++;
			if(fd<=0 || iwhile>100)
				return 0;
			revalue = sio_putch(fd,0x56);
		}
		revalue = 0;
		iwhile = 0;
		while(revalue!=1){
			iwhile++;
			if(fd<=0 || iwhile>100)
				return 0;
			revalue = sio_putch(fd, 0x56);
		}
	}else if(in==0x56){
		revalue = 0;
		iwhile = 0;
		while(revalue!=1){
			iwhile++;
			if(fd<=0 || iwhile>100)
				return 0;
		}
		revalue = 0;
		iwhile = 0;
		while(revalue !=1){
			iwhile++;
			if(fd<=0 || iwhile>100)
				return 0;
			revalue = sio_putch(fd,0x57);
		}
	}else{
		revalue = 0;
		iwhile = 0;
		while(revalue!=1){
			iwhile++;
			if(fd<=0 || iwhile>100)
				return 0;
			revalue = sio_putch(fd, in);
		}
	}
	return 1;
}
/**
 * @brief 向串口发送数据
 * @param fd:文件描述符
 * @param *p_in:数据
 * @param size:长度不包括包头和CRC
 * @return 发送的字节数 
 * -1失败
 */
int comm_frame_send(int fd, unsigned char *p_in, int size){
	int revalue;
	unsigned short crc_value = 0;
	int i;
	if(fd<=0)
		return 0;
	//帧头
	revalue = 0;
	int iwhile = 0;
	while(revalue != 1){
		iwhile++;
		if(fd <=0 || iwhile >100)
			return 0;
		revalue = sio_putch(fd, 0x55);
	}
	for(i=0; i<size; i++){
		crc_value = crc16(*(p_in+i),crc_value);
		if(!comm_send(fd,*(p_in+i)))
			return i;
	}
	//crc 1
	if(!comm_send(fd,(crc_value>>8)&0xff))
		return 0;
	//crc 0
	if(!comm_send(fd, (crc_value)&0xff))
		return 0;

	return size;
}
int com_frame_send(int fd, unsigned char *p_in, int size){
	unsigned short crc_value=0;
	int i;
	//发送帧头
	if(write(fd,CMD_SOF,1)<0){
		printf("com_frame_send error\n");
		return -1;
	}	
	//发数据
	for(i=0; i<size; i++){
		crc_value = crc16(*(p_in+i),crc_value);
		if(!com_send(fd, *(p_in+i)))
			return i;
	}
	//发送crc1
	if(!com_send(fd, (crc_value >> 8)&0xff))
		return 0;
	//发送crc0
	if(!com_send(fd, (crc_value)&0xff))
		return 0;

	return size;
}
/*
 *测试crc码值
 */
void crc_test(unsigned char *p_in, int size){
	unsigned short crc_value = 0;
	int i;
	unsigned char crc0,crc1;
	for(i=0; i<size; i++)
		crc_value = crc16(*(p_in+i),crc_value);
		crc0=(crc_value>>8)&0xff;
		crc1=(crc_value)&0xff;
	
	printf("the crc0=0x%x crc1=0x%x\n",crc0, crc1);

}



/**
 * @brief  接收一个字节从串口
 * 如果是0x56,0x56 p_out是0x55
 * 如果是0x56,0x57 p_out是0x56
 * @param fd:文件描述符
 * @param *p_out:接收到的数据
 * @return 0:无效数据1：正常数据2：帧头-1:错误
 */
int com_recv(int fd, unsigned char *p_out){
	 int revalue;
	 int time_out =  0;
	 if(fd <=0 )
		 return 0;
	 revalue = -1;
	 while(revalue <0 && time_out <= 1){
		revalue = sio_getch(fd);		
		time_out++;
	 }
	 if(revalue == 0x55){
		*p_out = 0x55;
		return 2;//帧头
	 }else{
		if(revalue ==0x56){
			revalue = -1;
			time_out = 0;
			while(revalue<0 && time_out<=1){
				revalue = sio_getch(fd);
				time_out++;
			}
			if(revalue == 0x55){
				*p_out = 0x55;
				return 2;//帧头
			}else{
				if(revalue == 0x56){
					*p_out = 0x55;
					return 1;
				}else{
					if(revalue == 0x57){
						*p_out = 0x56;
						return 1;
					}else{
						if(revalue == -1){
							*p_out = 0x56;
						}else{
							*p_out = revalue;
						}
						return 1;
					}
				}
			}
		}else{
			if(revalue == -1){
				return -1;//com time_out
			}else{
				*p_out = revalue;
				return 1;
			}
		}
		
	 }

}
/**
 *@brief 从串口接收数据
 *
 */
int comm_frame_rev(int fd, unsigned char *p_out,
					int buffer_size, unsigned char uCmdWord){

	int state = 0;
	unsigned short crc_value;
	int i;
	unsigned char buffer[1];
	int temp = 0;
	//取帧头
	int isreturnOK = 0;
	while(!isreturnOK){
		while(state !=2){
			//printf("--1--state=%d\n",state);
			state=com_recv(fd,buffer);
			//printf("--0--state=%d\n",state);
			if(state == 2){//帧头
				break;
			}else{
				if(state == 1){
				//数据
				continue;
				}else{
			//printf("--2--state=%d\n",state);
					return 0;
				}
			}
		}
head:
		//取其它数据
		crc_value = 0;
		i = 0;
		buffer_size = 6;
		int Max = 0;
		while(1){
			buffer[0] = 0;
			state = 0;
			if(i<buffer_size){
				state = com_recv(fd, buffer);
				if(state == 2){
					goto head;
				}
				//接收成功
				if(state > 0){
					Max = 0;
					if((*(p_out+i-1) == 0x56)
							&&((buffer[0]==0x56)
								||(buffer[0]==0x57))){
						buffer[0]=(buffer[0]==0x56)?0x55:0x56;
						i--;
					}else{
						crc_value = crc16(buffer[0],crc_value);
					}
					*(p_out+i) = buffer[0];
					//第一二字节是包的长第一们为高位+4
					if(i==2){
						buffer_size = p_out[0]*256 + p_out[1] +4;
					}
					if(buffer_size > 256){
						buffer_size = 0;
					}
						i++;
				}
				else{
					Max++;
					if(Max>300){
						return 0;
					}
					continue;
				}
			}else{
				if(crc_value == 0){//校验成功
					//该数据包是本次命令的返回?
					if((uCmdWord==0)||(p_out[2]==uCmdWord)){
						isreturnOK = 1;
						if(m_isReturn_r){
							XCDataReturn(fd, uCmdWord,0x00);
							m_isReturn_r = 0;
						}
						temp = i;
					}else{
						if(m_isReturn_r){
							XCDataReturn(fd,uCmdWord,0x02);
							m_isReturn_r = 0;
						}
						isreturnOK = 0;
						temp = 0;
					}
					break;
				}else{
					if(m_isReturn_r){
						XCDataReturn(fd, uCmdWord,0x03);
						m_isReturn_r = 0;
					}
					temp = 0;
					break;
				}
			}
		}
	
	}
	return temp;
}


/**
 *@brief 从串口接收数据
 *@param fd:文件描述符
 *@param *p_out:返回接收的数据
 *@param buffer_size:接收长度，不包括包头和CRC
 *@return 
 */

int com_frame_rev(int fd, unsigned char *p_out, int buffer_size){
	int retval;
	int i;
	int state ;
	unsigned char buffer[1];
	unsigned short crc_value;
	fd_set rfds;
	struct timeval tv;
	tv.tv_sec  = 0;
	tv.tv_usec = 50000;
	 int flag1 = 1;
	
	while(flag1){

		FD_ZERO(&rfds);
		FD_SET(fd,&rfds);
		
		retval = select(fd+1, &rfds, NULL, NULL, &tv);
		if(retval == -1){
			printf("select return!\n");
			break;
		}else if(retval){
		
			//接收数据
			while(state != 2){
		
				state = com_recv(fd,buffer);
				printf("SOA buffer[]=0x%x ",buffer[0]);
				if(state == 2){
					break;//帧头
				}else{
					if(state == 1){
				
						continue;//数据
					}else{
				
						return 0;

					}
				}
			}
	head:
			//取其它数据
		i = 0;
		crc_value = 0;
		while(1){
		
			buffer[0] = 0;
			state = 0;
			if(i<=buffer_size){
				state = com_recv(fd, buffer);
				if(state == 2){
					goto head;
				}
					//printf("------------6---state=%d---------\n",state);
				if(state > 0 ){
				//接收成功
				crc_value = crc16(buffer[0], crc_value);
			
			    	*(p_out+i) = buffer[0];
					i++;
					//	printf("i=%d buffer[0]=%x\n",i,buffer[0]);
					if(i == buffer_size){
				
						break;
						}
				}
				}
				
			}
		}else{
		//	printf("No data \n");
			flag1 = 0;
			break;
		}
			
	}
	//printf("flag1=%d\n",flag1);
	return 1;
}

/**
 * @brief 打开射频功放 60H
 * @param fd:描述符
 * @param ucAntenna 天线号00H表示读写器自动扫描
 * 天线，选择信号好的天线01-03H
 * @return  1:开功放成功
 *		   -1:文件描述符错误
 *		   -4:接收数据长度错误
 *		   -5:接收数据错
 *		   -8:第n号天线没有连接
 *		   -20:其它错误
 */
int XCPowerOn(int fd, unsigned char ucAntenna){
	if(fd == -1)
		return ERROR_FD_VALUE; 
	unsigned char *pSendBuffer;
	unsigned char RecvBuffer[MAX_REC_NUM];
	unsigned char buf[MAX_REC_NUM];
	char temp = 0;
	int i,size;
	pSendBuffer = buf;
	pSendBuffer[0] = 0x00;//数据长度
	pSendBuffer[1] = 0x02;
	pSendBuffer[2] = 0x60;
	pSendBuffer[3] = ucAntenna;//天线号

	temp = comm_frame_send(fd, pSendBuffer,4);

	usleep(50);

	i=comm_frame_rev(fd,RecvBuffer,6,pSendBuffer[2]);

	if(i>=MIN_COMM_NUM){
		size = (((int)RecvBuffer[0])<<8) + (int)RecvBuffer[1];
		if((i-4)!=size)
			temp = ERROR_RECEIVEDATA_LEN;
		else
			temp = RecvBuffer[3];
	}else
		temp = ERROR_RECEIVEDATA_LEN;
	switch(temp){
		case 0x3F://接收数据错
			temp = ERROR_SENDDATA_FAIL;
			break;
		case 0x00://打开功放成功
			temp = FUCCESS_RETURN;
			break;
		case 0x3A://第n号天线没有连接
			temp = ERROR_IDENTIFY_FAIL;
			break;
		default:
			temp = ERROR_OTHER_FAIL;
			break;
	}

//	com_frame_send(fd, pSendBuffer, 4);
	//printf("temp=%d\n",temp);
	return temp;

}
/**
 * @brief 关闭功放 61H，停止读写器扫描标签工作
 * @param fd:描述符
 * @return 1:操作成功
 *		  -1:描述符错误
 *		  -4:接收数据长度不符
 *		  -5:数据格式错误
 *		  -20:其它错误失败
 */
int XCPowerOff(int fd){
	if(fd == -1)
		return ERROR_FD_VALUE;

	unsigned char *pSendBuffer;
	unsigned char RecvBuffer[MAX_REC_NUM];
	unsigned char buf[MAX_REC_NUM];
	char temp = 0;
	int i=0,size,k;
	pSendBuffer = buf;
	pSendBuffer[0] = 0x00;
	pSendBuffer[1] = 0x01;
	pSendBuffer[2] = 0x61;

	//com_frame_send(fd, pSendBuffer, 4);
	temp = comm_frame_send(fd,pSendBuffer,3);
	for(k=0; i<1;k++){
		usleep(50);
		i = comm_frame_rev(fd,RecvBuffer,6,pSendBuffer[2]);
		if(i>=MIN_COMM_NUM)
			break;
	}
	if(i>=MIN_COMM_NUM){
		size = (((int)RecvBuffer[0])<<8) + (int)RecvBuffer[1];
		if((i-4)!=size){
			temp = ERROR_RECEIVEDATA_LEN;
		}else{
			temp = RecvBuffer[3];
		}
	}else{
		temp = ERROR_RECEIVEDATA_LEN;
	}
	switch(temp){
		case 0x3F:
			temp = ERROR_SENDDATA_FAIL;
			break;
		case 0x00:
			temp = FUCCESS_RETURN;
			break;
		default:
			temp = ERROR_OTHER_FAIL;
			break;
	}
	return temp;
}

/**
 * @brief 读EPC编码信息 81H
 * 识别EPC G2 标签的EPC编码信息
 * @param fd:描述符
 * @param ucAntenna:指定天线号
 * @param ucScanTimes:标签扫描数
 * @param isDoAffirm:是否需要返回确认
 * 00H确认，对同一个标签的EPC码只读一次
 * 01H不确认，读取到的所有标签EPC全部上传
 * @return >1 发送指令的字节数
 *		   -1:描述符错误
 */
int XCIdentifyEpcCode(int fd, unsigned char ucAntenna,
							  unsigned char ucScanTimes,
							  unsigned char isDoAffirm){
	if(fd == -1)
		return ERROR_FD_VALUE;
	
	unsigned char *pSendBuffer;
	char uResult = 0;
	unsigned char buf[MAX_REC_NUM];
	pSendBuffer = buf;
	pSendBuffer[0] = 0x00;
	pSendBuffer[1] = 0x04;
	pSendBuffer[2] = 0x81;
	pSendBuffer[3] = ucAntenna;
	pSendBuffer[4] = ucScanTimes;
	pSendBuffer[5] = isDoAffirm;

	if(isDoAffirm == 0){
		m_isReturn = 1;
	}else{
		m_isReturn = 0;
	}
	SetReturn(m_isReturn);

	uResult = comm_frame_send(fd,pSendBuffer,6);

//	com_frame_send(fd, pSendBuffer, 6);

	return uResult;
}
/**
 * @brief 接收标签EPC码
 * 接收读写器识别到的标签EPC编码
 * @param fd:文件描述符
 * @param *pEpcCode:识别成功后的返回信息包括：状态字 天线号 标签类型EPC码
 */
int XCEpcCodeReport(int fd, unsigned char *pEpcCode){

	if(com_frame_rev(fd, pEpcCode,19)==0){
		printf("read error\n");
		exit(1);
	}
	return 0;
}
/**
 *@brief 识别EPC G2 标签的TID编码信息 82H
 *@param fd:文件描述符
 *@param ucAntenna:天线号
 *@param ucScanTimes:标签扫描数
 *@param isDoAffirm:是否需要返回确认00：对同一个标签的TID码
 *读写器返回一次，不确认：01：读到的标签TID全部上传
 *@param *pTagPWD:标签访问密码
 *@return >1:返回发送的指令长度
	      -1:错误的描述符
 */
int XCIdentifyTIDCode(int fd, unsigned char ucAntenna,
							  unsigned char ucScanTimes,
							  unsigned char isDoAffirm,
							  unsigned char *pTagPWD){
	if(fd == -1)
		return ERROR_FD_VALUE;
	
	unsigned char *pSendBuffer;
	char uResult = 0;
	unsigned char buf[MAX_REC_NUM];
	pSendBuffer = buf;
	pSendBuffer[0]=0x00;
	pSendBuffer[1]=0x08;
	pSendBuffer[2]=0x82;
	pSendBuffer[3]=ucAntenna;
	pSendBuffer[4]=ucScanTimes;
	pSendBuffer[5]=isDoAffirm;
	pSendBuffer[6]=pTagPWD[0];
	pSendBuffer[7]=pTagPWD[1];
	pSendBuffer[8]=pTagPWD[2];
	pSendBuffer[9]=pTagPWD[3];
	
	if(isDoAffirm == 0){
		m_isReturn = 1;
	}else{
		m_isReturn = 0;
	}
	SetReturn(m_isReturn);
	uResult = comm_frame_send(fd,pSendBuffer,10);

//	com_frame_send(fd, pSendBuffer, 10);
	
	return uResult;
}

/**
 *@brief 接收读写器识别到的标签TID编码
 *@param id:文件描述符
 *@param *pTIDCode 返回信息包括：状态字 天线号 标签类型TID码
 *@return >3:操作成功后返回的长度
		 -1:文件描述符错误
		 -4:接收数据长度不符
 */
int XCTIDCodeReport(int fd, unsigned char *pTIDCode){
	if(fd == -1){
		return ERROR_FD_VALUE;
	}
	unsigned char *pRecvBuffer;
	char uResult = 0;
	unsigned char buf[MAX_REC_NUM];
	int i,size;
	pRecvBuffer = buf;

	i = comm_frame_rev(fd,pRecvBuffer,6,0x82);
	if(i>MIN_COMM_NUM)
	{
		size = (((int)pRecvBuffer[0])<<8)+(int)pRecvBuffer[1];
		if(size>=i){
			return ERROR_RECEIVEDATA_LEN;
		}
		if((i-4)!=size){
			uResult = 0;
		}else{
			if(size>1){
				uResult = XCIsError(pRecvBuffer);
				if(uResult == FUCCESS_RETURN){
					for(i=0; i<size-1;i++){
						*(pTIDCode+i)=pRecvBuffer[3+i];
					}
					uResult = size;
				}
			}
		}
	}else{
		uResult = ERROR_NODATA_RETURN;
	}
	return uResult;
}
/**
 *@brief 写标签EPC码 83H
 *@param fd:文件描述符
 *@param ucAntenna:天线号
 *@param pTagPWD:标签访问密码
 *@param ucStartAdd:标签数据区首地址0x00-0x0C
 *@param ucLength: 要写入数据的长度
 *@param *pEpcValue:要写入的EPC编码数据
 *@return 1:操作成功返回
		 -1:错误的描述符
		 -3:用户数据地址错
		 -2:用户数据长度错
		 -5:数据格式错误
		 -7:读写数据失败
		 -20:其它错误失败
 */
int XCSetEpcCode(int fd, unsigned char ucAntenna,
						 unsigned char *pTagPWD,
						 unsigned char ucStartAdd,
						 unsigned char ucLength,
						 unsigned char *pEpcValue){
	if(fd == -1)
		return ERROR_FD_VALUE;
	if((ucStartAdd<0)||(ucStartAdd>12)){
		return ERROR_USERDATA_ADDRESS;
	}
	if((ucStartAdd%2)>0){
		return ERROR_USERDATA_ADDRESS;
	}
	if((ucLength%2)>0){
		return ERROR_USERDATA_LEN;
	}
	unsigned char *pSendBuffer;
	unsigned char RecvBuffer[MAX_REC_NUM];
	char result = 0;
	unsigned char buf[MAX_REC_NUM];
	int i,j,k,size;
	pSendBuffer = buf;
	uSizeOfTagID=10;//标签长度
	
		pSendBuffer[0]=0x00;
		pSendBuffer[1]=uSizeOfTagID+ucLength-2;
		pSendBuffer[2]=0x83;//命令字
		pSendBuffer[3]=ucAntenna;
		pSendBuffer[4]=pTagPWD[0];
		pSendBuffer[5]=pTagPWD[1];
		pSendBuffer[6]=pTagPWD[2];
		pSendBuffer[7]=pTagPWD[3];
		pSendBuffer[8]=0x00;
		pSendBuffer[9]=ucStartAdd;
		//写EPC数据内容
		for(j=0; j<ucLength; j++){
			pSendBuffer[10+j] = *(pEpcValue+j);
		}

		result = comm_frame_send(fd,pSendBuffer,ucLength+uSizeOfTagID);
		for(k=0; k<1;k++)
		{
			usleep(200);
			i = comm_frame_rev(fd,RecvBuffer,6,pSendBuffer[2]);
			if(i>=MIN_COMM_NUM)
				break;
		}
		if(i>MIN_COMM_NUM){
			size = (((int)RecvBuffer[0])<<8) + (int)RecvBuffer[1];
			if((i-4)!=size){
				result = ERROR_RECEIVEDATA_LEN;
			}else{
				result = XCIsError(RecvBuffer);
			}
		}else{
			result = ERROR_RECEIVEDATA_LEN;
		}
		//com_frame_send(fd, pSendBuffer, ucLength+uSizeOfTagID);
		return result;
}

/**
 *@brief 读取用户区数据 84H
 *发送读取用户数据信息
 *@param fd:文件描述符
 *@param ucAntenna:天线号
 *@param *pTagPWD:标签访问密码
 *@param ucStartAdd:标签数据区首地址(0x00-0x1A)
 *@param ucLength:要读取数据的长度
 *@param isDoAffirm:是否需要返回确认00:确认01不确认
 *@return >0 返回发送的字节数，指令发送成功
		 -1:错误的描述符
		 -2:用户数据长度错
		 -3:用户数据地址错
 */
int XCGetUserData(int fd, unsigned char ucAntenna,
						  unsigned char isDoAffirm,
					      unsigned char *pTagPWD,
						  unsigned char ucStartAdd,
						  unsigned char ucLength){						
	if(fd == -1)
		return ERROR_FD_VALUE;
	if((ucStartAdd<0x00)||(ucStartAdd>0x1a)||(ucStartAdd%2)!=0)
		return ERROR_USERDATA_ADDRESS;
	if((ucLength*2 + ucStartAdd)>28)
		return ERROR_USERDATA_LEN;

	unsigned char *pSendBuffer;
	char result = 0;
	unsigned char buf[MAX_REC_NUM];
	pSendBuffer = buf;
		uSizeOfTagID = 11;
		pSendBuffer[0]=0x00;
	    pSendBuffer[1]=uSizeOfTagID-2;
	    pSendBuffer[2]=0x84;
	    pSendBuffer[3]=ucAntenna;
		pSendBuffer[4]=isDoAffirm;
		pSendBuffer[5]=pTagPWD[0];
		pSendBuffer[6]=pTagPWD[1];
		pSendBuffer[7]=pTagPWD[2];
		pSendBuffer[8]=pTagPWD[3];
		pSendBuffer[9]=ucStartAdd;
		pSendBuffer[10]=ucLength;

		if(isDoAffirm == 0){
			m_isReturn = 1;
		}else{
			m_isReturn = 0;
		}
		SetReturn(m_isReturn);
		result = comm_frame_send(fd,pSendBuffer,uSizeOfTagID);
		//com_frame_send(fd, pSendBuffer, 11);
		return result;

}
/**
 *@brief 接收用户区数据，读取用户数据返回
 *@param fd:文件描述符
 *@param *pVale:返回读到数据的指针(包括：状态字 天线号 标签类型
 *用户数据)
 *@return >3:操作成功后返回的长度
           0:没有数据返回，接收数据长度不足
		   -1:错误的描述符
		   -4:接收数据长度不符
		   -5:数据格式错误
		   -7;读写数据失败
		   -20:其它错误
 */
int XCGetUserDataReport(int fd, unsigned char *pValue){

	if(fd == -1)
		return ERROR_FD_VALUE;
	unsigned char *pRecvBuffer;
	int uResult = 0;
	unsigned char buf[MAX_REC_NUM];
	int i,size;
	pRecvBuffer = buf;
	i = comm_frame_rev(fd,pRecvBuffer,MAX_REC_NUM,0x84);
	if(i>MIN_COMM_NUM){
		size = (((int)pRecvBuffer[0])<<8)+(int)pRecvBuffer[1];
		if(size>=i){
			return ERROR_RECEIVEDATA_LEN;
		}
		if((i-4)!=size)
			uResult = 0;
		else{
			if(size > 1){
				uResult = XCIsError(pRecvBuffer);
				if(uResult == FUCCESS_RETURN){
					for(i=0; i<size-1;i++){
						*(pValue+i)=pRecvBuffer[3+i];
					}
					uResult = size;
				}
			}
		}
	}else{
		uResult = ERROR_NODATA_RETURN;
	}
	

	/*
	if(com_frame_rev(fd, pValue, 35)==0){
		printf("----%s-----read error\n",__func__);
		exit(1);
	}*/
	return uResult;
}
/**
 *@brief 判断返回值正误
 *@param  *RevData:返回来的数据
 *@return 1:成功，不成功返回为错误代码<0
 *        0:没有数据返回，接收数据长度不足
 *        指令发送失败返回ERROR_NODATA_RETURN
 *
 */
int XCIsError(unsigned char *RevData){
	char rechar = FUCCESS_RETURN;
	unsigned char ucLog = 0;
	ucLog = RevData[3];
	if(ucLog == 0x00){
		//成功
		return rechar;
	}else{
		rechar = 0xFF - RevData[4];
	}
	return rechar;
}
/**
 *@brief 写用户区数据 85H
 *@param fd:文件描述符
 *@param ucAntenna:天线号
 *@param *pTagPWD;标签访问密码
 *@param ucStartAdd:标签数据区首地址(0x00-0x1A)
 *@param ucLength:要写入数据的长度
 *@param pValue:要写入的用户数据
 *@return 1:操作成功返回
		 -1:错误的描述符
		 -3:用户数据地址错
		 -2:用户数据长度错
		 -4:接收数据长度不符
		 -5:数据格式错误
		 -7:读写数据失败
		 -20:其它错误失败
 */
int XCSetUserData(int fd, unsigned char ucAntenna,
						  unsigned char *pTagPWD,
						  unsigned char ucStartAdd,
						  unsigned char ucLength,
						  unsigned char *pValue){
	if(fd == -1)
		return ERROR_FD_VALUE;
	if((ucStartAdd<0)||(ucStartAdd>0x1A))
		return ERROR_USERDATA_ADDRESS;
	if((ucStartAdd%2)>0){
		return ERROR_USERDATA_LEN;
	}
	if((ucLength*2+ucStartAdd)>28){
		return ERROR_USERDATA_LEN;
	}
	unsigned char *pSendBuffer;
	unsigned char RecvBuffer[MAX_REC_NUM];
	unsigned char buf[MAX_REC_NUM];
	char result = 0;
	int i,j,k,size;
	pSendBuffer = buf;
	uSizeOfTagID = 9;
	
		pSendBuffer[0]=0x00;
		pSendBuffer[1]=uSizeOfTagID + ucLength*2 - 2;
		pSendBuffer[2]=0x85;
		pSendBuffer[3]=ucAntenna;
		pSendBuffer[4]=pTagPWD[0];
		pSendBuffer[5]=pTagPWD[1];
		pSendBuffer[6]=pTagPWD[2];
		pSendBuffer[7]=pTagPWD[3];
		pSendBuffer[8]=ucStartAdd;

		for(j=0; j<ucLength*2; j++)
			pSendBuffer[9+j]=*(pValue+j);//写数据内容

		result = comm_frame_send(fd, pSendBuffer,ucLength*2
				+uSizeOfTagID);
		for(k=0; k<1;k++){
			//读数据区接收查询，根据uReadTries来多次查询等待数据返回
			usleep(200);
			i = comm_frame_rev(fd,RecvBuffer,7,pSendBuffer[2]);
			if(i>=MIN_COMM_NUM)
				break;
		}
		if(i>=MIN_COMM_NUM){
			size = (((int)RecvBuffer[0])<<8) +(int)RecvBuffer[1];
			//去除数据长度和CRC校验4个字节
			if((i-4)!=size){
				result = ERROR_RECEIVEDATA_LEN;
			}else{
				result = XCIsError(RecvBuffer);
			}
		}else{
			result = ERROR_RECEIVEDATA_LEN;
		}
		//com_frame_send(fd, pSendBuffer,ucLength*2 + uSizeOfTagID);

		return result;
}
/**
 *@brief 查询设备参数 65H
 *由于下位机内在空间的限制，每次查询信息的长度不允许
 *超过128个字节，size<=128
 *@param fd:文件描述符
 *@param iInfoType:信息类型
		0x00	IP地址	12字节
		0x01	跳频	30字节
		0x02	天线数	1字节
		0x03	测试模式	1字节配置信息为0时
		0x04	MAX地址	6字节
 *@param size:得到数据个数最大32字节
 *@param *pData:得到数据
 */
int XCSysQuery(int fd, int iInfoType,
					   unsigned char size,
					   unsigned char *pData){
	unsigned char pSendBuffer[5];
		pSendBuffer[0]=0x00;
		pSendBuffer[1]=0x03;
		pSendBuffer[2]=0x64;
		pSendBuffer[3]=iInfoType;
		pSendBuffer[4]=size;

		com_frame_send(fd, pSendBuffer, 5);
		usleep(50);//50ms
		com_frame_rev(fd, pData, size+6);

		return 0;
}
/**
 *@brief 设备参数设定 65H
 *@param fd:文件描述符
 *@param iInfoType:信息类型
 *@param size:设置数据字节数 最大32个字节
 *@param Param:设置参数
 */
int XCSysFastSet(int fd, int iInfoType,
						 int size,
						 unsigned char *Param){
	unsigned char pSendBuffer[256];
	int i;
	//高位在前
	pSendBuffer[0]=(3+size)/256;
	pSendBuffer[1]=(3+size)&0xff;
	pSendBuffer[2]=0x65;
	pSendBuffer[3]=iInfoType;
	pSendBuffer[4]=size;
	for(i=0;i<size;i++){
		pSendBuffer[5+i]=Param[i];
	}

	com_frame_send(fd, pSendBuffer, 5+size);
	usleep(200);//配置信息时，读写器写的慢，需要延时
	
	return 0;
}
/**
 *@brief 锁定用户区 8AH
 *锁住用户数据
 *@param fd:文件描述符
 *@param ucAntenna:天线号
 *@param *pTagPWD:标签访问密码
 *@param ucLocked:锁定解锁标志
 *@param ucDistrict:操作数据区域
 */
int XCSetDataLocked(int fd, unsigned char ucAntenna,
							unsigned char *pTagPWD,
							unsigned char ucLocked,
							unsigned char ucDistrict){
	unsigned char pSendBuffer[10];
		pSendBuffer[0]=0x00;
		pSendBuffer[1]=0x08;
		pSendBuffer[2]=0x8A;
		pSendBuffer[3]=ucAntenna;
		pSendBuffer[4]=pTagPWD[0];
		pSendBuffer[5]=pTagPWD[1];
		pSendBuffer[6]=pTagPWD[2];
		pSendBuffer[7]=pTagPWD[3];
		pSendBuffer[8]=ucLocked;
		pSendBuffer[9]=ucDistrict;

		com_frame_send(fd, pSendBuffer, 10);
		return 0;
}
/**
 *@brief 设置访问密码信息 88H
 *@param fd:文件描述符
 *@param ucAntenna:天线号
 *@param *pTagPWD:标签访问密码
 *@param *pNewTagPWD:标签新访问密码
 */
int XCSetVisitPWD(int fd, unsigned char ucAntenna,
						  unsigned char *pTagPWD,
						  unsigned char *pNewTagPWD){
	unsigned char pSendBuffer[12];
		pSendBuffer[0]=0x00;
		pSendBuffer[1]=0x0A;
		pSendBuffer[2]=0x88;
		pSendBuffer[3]=ucAntenna;
		pSendBuffer[4]=pTagPWD[0];
		pSendBuffer[5]=pTagPWD[1];
		pSendBuffer[6]=pTagPWD[2];
		pSendBuffer[7]=pTagPWD[3];
		pSendBuffer[8]=pNewTagPWD[0];
		pSendBuffer[9]=pNewTagPWD[1];
		pSendBuffer[10]=pNewTagPWD[2];
		pSendBuffer[11]=pNewTagPWD[3];

		com_frame_send(fd, pSendBuffer, 12);

		return 0;
}
/**
 *@brief 设置灭活密码 89H
 *设置用户销毁密码信息
 *@param fd:文件描述符
 *@param ucAntenna:天线号
 *@param *pTagPWD;标签访问密码
 *@param *pNewTagPWD:标签销毁密码
 */
int XCSetDestroyPWD(int fd, unsigned char ucAntenna, 
						    unsigned char *pTagPWD,
						    unsigned char *pNewTagPWD){
	unsigned char pSendBuffer[12];
		pSendBuffer[0]=0x00;
		pSendBuffer[1]=0x0A;
		pSendBuffer[2]=0x89;
		pSendBuffer[3]=ucAntenna;
		pSendBuffer[4]=pTagPWD[0];
		pSendBuffer[5]=pTagPWD[1];
		pSendBuffer[6]=pTagPWD[2];
	    pSendBuffer[7]=pTagPWD[3];
		pSendBuffer[8]=pNewTagPWD[0];
		pSendBuffer[9]=pNewTagPWD[1];
		pSendBuffer[10]=pNewTagPWD[2];
		pSendBuffer[11]=pNewTagPWD[3];

		com_frame_send(fd, pSendBuffer, 12);

		return 0;	
}
/**
 *@brief 灭活标签 8BH 销毁标签信息
 *@param fd:文件描述符
 *@param ucAntenna:天线号
 *@param *pDestroyPWD:标签销毁密码
 */
int XCSetDestroyTag(int fd, unsigned char ucAntenna,
							unsigned char *pDestroyPWD){
	unsigned char pSendBuffer[8];
		pSendBuffer[0]=0x00;
		pSendBuffer[1]=0x06;
		pSendBuffer[2]=0x8B;
		pSendBuffer[3]=ucAntenna;
		pSendBuffer[4]=pDestroyPWD[0];
		pSendBuffer[5]=pDestroyPWD[1];
		pSendBuffer[6]=pDestroyPWD[2];
		pSendBuffer[7]=pDestroyPWD[3];

		com_frame_send(fd, pSendBuffer, 8);
		return 0;
}
/**
 *@brief 返回数据确认 68H
 *@param fd:文件描述符
 *@param command:需要返回确认的指令命令字
 *@param DataType:确认状态
 */
int XCDataReturn(int fd, unsigned char Command,
						 unsigned char DataType){
	unsigned char pSendBuffer[5];
		pSendBuffer[0]=0x00;
		pSendBuffer[1]=0x03;
		pSendBuffer[2]=0x68;
		pSendBuffer[3]=Command;
		pSendBuffer[4]=DataType;

		com_frame_send(fd, pSendBuffer, 5);
		return 0;
}
/**
 *@brief 打印函数
 *@param *Tag:打印标志
 *@param *pbuf:内存
 *@param size:内存长度
 */
void display(char *Tag, unsigned char *pbuf, int size){
	printf("\n----------------%s---------------------\n", Tag);
	int i;
	for(i=0; i<size;i++){
		printf("0x%x ", pbuf[i]);
	}
	printf("\n----------------%s END----------------\n",Tag);
}
