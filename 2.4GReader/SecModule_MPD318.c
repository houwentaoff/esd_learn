/*************************************************************************
	> File Name: SecModule_MPD318.c
	> Author: shuixianbing
	> Mail: shui6666@126.com 
	> Created Time: 2015年11月23日 星期一 15时11分45秒
 ************************************************************************/

#include"SecModule_MPD318.h"
int gpio_fd;
int spi_fd;
//获取异或校验值
uint8_t GetVerify(uint8_t *src, int length){
	int i;
	uint8_t value = *src;
	src++;
	for(i=1; i<length;i++){
		value ^= *src;
		src++;
	}
	return value;
}
void SecModule_Send_Ext(uint8_t *src, int length){
	uint8_t i;
	printf("\n spi write\n");
	src[length] = GetVerify(src, length);//计算校验
	//发送首字节数据
	spi_write(spi_fd, src, 1);
	usleep(100);

	i = 1;
	while(length--){
		spi_write(spi_fd,src+i,1);//发送数据
		i++;
		usleep(10);
	}

}
//将输入数据先计算校验,然后发送到保密模块
uint8_t SecModule_Recv_Ext(uint8_t *src){
	uint8_t TotalDataLen = 0,i,res;
	res = 0;
	printf("\n spi read \n");
	//先读取第一个字节,表示总长度
	spi_read(spi_fd, &TotalDataLen, 1);
	usleep(100);
	src[0] = TotalDataLen;//第一个数据
	//防止数据出错时最大数据超过缓存值造成溢出
	if(TotalDataLen > sizeof(SECURITY_REP)-1){
		TotalDataLen = sizeof(SECURITY_REP) - 1;
		res = 1;
	}
	i = 1;
	while(TotalDataLen--){
		spi_read(spi_fd,src+i,1);//读取数据
		i++;//数据位置偏移
		usleep(10);
	}

	return res;
}
/**
 *等待State引脚为低电平
 *返回值 0 返回低电平 -1 超时
 */
uint8_t SecModule_WaitState(int times){
		
	while(times--){
		if(ioctl(gpio_fd, 4, 0)==0){
			printf("\nThe SecModule State ready\n");
			return 0;
		}
		usleep(10000);
	//printf("The SecModule State not ready times=%d\n", times);
	}

	printf("The SecModule State not ready times=%d\n", times);
	return -1;
}

/**
 *操作函数
 *失败返回-1
 */
uint8_t SecModule_Operate(uint8_t CmdType, uint8_t Length){

	SecModuleCMD.CmdType = CmdType;
	SecModuleCMD.Length = Length;
	usleep(10000);
	ioctl(gpio_fd, 1, 0);//CS引脚低
	SecModule_Send_Ext((uint8_t *)&SecModuleCMD,SecModuleCMD.Length);
	usleep(10000);
	if(0 == SecModule_WaitState(1500)){
		SecModule_Recv_Ext((uint8_t *)&SecModuleREP);

		ioctl(gpio_fd, 1, 1);//CS 引脚置高
		if(((uint8_t *)&SecModuleREP)[SecModuleREP.Length]!= GetVerify((uint8_t *)&SecModuleREP,SecModuleREP.Length)){
			return SecModule_OP_VerifyCodeError;//检验值错误
		}
		if(SecModuleREP.repCode){
			printf("\n CMD[0x%x] repCode Error! repCode=0x%x\n", CmdType, SecModuleREP.repCode);
			return SecModuleREP.repCode;//如果操作失败返回错误码
		}
		if(SecModuleREP.CmdType != SecModuleCMD.CmdType){
			
			printf("\n CMD[0x%x] CMDCode Error! Code=0x%x\n", CmdType, SecModule_OP_CMDCodeError);
			return SecModule_OP_CMDCodeError;//命令码错误

		}
		printf("\n send CMD [0x%x] Success\n", CmdType);
		//操作成功
		return SecModule_OP_SUCCESS;
	}
		ioctl(gpio_fd, 1, 1);//CS引脚置高
		printf("\n send CMD [0x%x] Failing\n", CmdType);		
		return -1;
}
/**
 *保密模块自毁
 *输入参数　MID　保密模块ID
 */
uint8_t SecModule_SelfDestory(uint8_t MID[8]){
	int ret = 0;
	
	memset(&SecModuleCMD,0,sizeof(SECURITY_CMD));
	memcpy(SecModuleCMD.Data.SelfDestory.MID,MID,8);
	ret = SecModule_Operate(CMD_SELF_DESTORY, 0x0A);
	if(ret == -1)
		return SecModule_OP_SelfDestoryFail;
	else
	return ret;
}
/**
 *保密模块随机数自检
 *
 */
uint8_t SecModule_RandomCheck(void){
	int ret = 0;
	memset(&SecModuleCMD,0,sizeof(SECURITY_CMD));
	ret = SecModule_Operate(CMD_RANDOM_CHECK, 0x02);
	if(ret == -1)
		return SecModule_OP_RandomCheckFail;
	else
		return ret;
}
/**
 *保密模块请求随机数
 *输入参数: RandomType 请求随机数的长度类型
 * 0x00 保留　0x01 8bit长度随机数 0x02 16bit长度随机数
 * 0x03 32bit长度随机数 0x04 64bit长度随机数
 * 0x05 128bit长度随机数 0x06 192bit长度随机数
 *输出参数 *RandomData
 */
uint8_t SecModule_RandomRequest(uint8_t RandomType, uint8_t *RandomData){
	int ret = 0;
	memset(&SecModuleCMD, 0, sizeof(SECURITY_CMD));
	SecModuleCMD.Data.RandomRequest.RandomType = RandomType;
	ret = SecModule_Operate(CMD_RANDOM_REQUEST, 0x03);
	if(ret == -1)
		return SecModule_OP_RandomRequestFail;
	else{
		if(ret == SecModule_OP_SUCCESS)
		memcpy(RandomData,SecModuleREP.Data.RandomRequest.RandomData,
				SecModuleREP.Length-3);
		return ret;
	}

}

/**
 *生成标签口令
 *输入参数  TID 标签TID
 *			Index 口令索引 0x0000	初始化口令
 *0x0001	灭活口令	0x0002	就绪口令
 *0x0003	休眠口令	0x0004	唤醒口令
 *0x0005	锁定口令	0x0006	管理员口令
 *0x0007	读访问口令	0x0008	写访问口令
 *0x0001	灭活口令	0x0002	就绪口令
 *			Mode	0x01 列表文件口令
 *	0x02	文件有/无效口令
 *	0x03	读　检索口令
 *	0x04	更新谁的口令
 *输出参数	Key	返回口令
 */
uint8_t SecModule_GenerateTagKey(uint8_t TID[8], uint16_t Index,
		uint8_t Mode, uint8_t Key[4]){
	int ret = 0;
	uint16_t tIndex;
	memset(&SecModuleCMD, 0, sizeof(SECURITY_CMD));
	memcpy(SecModuleCMD.Data.GenerateTagKey.TID, TID, 8);
	tIndex = ((Index&0xff00)>>8)|((Index&0x00ff)<<8);
	SecModuleCMD.Data.GenerateTagKey.Index = tIndex;//索引,高字节在前
	SecModuleCMD.Data.GenerateTagKey.Mode = Mode;
	
	ret = SecModule_Operate(CMD_GENERATE_TAG_KEY, 0x0D);
	if(ret == -1)
		return SecModule_OP_GenerateTagKeyFail;
	else{
		if(ret == SecModule_OP_SUCCESS)
			memcpy(Key,SecModuleREP.Data.GenerateTagKey.Key,4);//口令
		return ret;
	}
}
/**
 *修改管理口令
 *输入参数	OldPassword 旧密码
 *			NewPassword 新密码
 */
uint8_t SecModule_ModifyPassword(uint8_t OldPassword[8], uint8_t NewPassword[8]){
	int ret = 0;
	memset(&SecModuleCMD,0,sizeof(SECURITY_CMD));
	memcpy(SecModuleCMD.Data.ModifyPassword.OldPassword, OldPassword,8);
	memcpy(SecModuleCMD.Data.ModifyPassword.NewPassword, NewPassword,8);
		
	ret = SecModule_Operate(CMD_MODIFY_PASSWORD, 0x12);
	if(ret == -1)
		return SecModule_OP_ModifyPasswordFail;
	else
		return ret;

}
/**
 *加密无源标签数据并计算MAC
 *输入参数  TID 标签TID
 *输入输出参数	*DataLen 输入参数时表示加密数据长度,输出参数表示密文数据	
 *	*Data	输入表示加密数据,输出表示密文数据
 *	MAC	输入表示初始参量,输出表示输出MAC
 */
uint8_t SecModule_EncryptWYData(uint8_t TID[8],uint8_t *DataLen,
		uint8_t *Data, uint8_t MAC[32]){
	int ret = 0;
	uint8_t len = 1+8+32+1+1+(*DataLen);
	memset(&SecModuleCMD,0,sizeof(SECURITY_CMD));
	memcpy(SecModuleCMD.Data.EncryptWYData.TID,TID,8);
	memcpy(SecModuleCMD.Data.EncryptWYData.MAC,MAC,32);
	SecModuleCMD.Data.EncryptWYData.DataLen=(*DataLen);//数据长度
	memcpy(SecModuleCMD.Data.EncryptWYData.Data,Data,(*DataLen));//数据
	
	ret = SecModule_Operate(CMD_ENCRYPT_WY_DATA,len);
	if(ret == -1)
		return SecModule_OP_EncryptDataFail;
	else{
		if(ret == SecModule_OP_SUCCESS){
			printf("密码长度0x%x\n", SecModuleREP.Data.EncryptWYData.DataLen);
			*DataLen = SecModuleREP.Data.EncryptWYData.DataLen;//密文长度
			memcpy(Data,SecModuleREP.Data.EncryptWYData.Data,(*DataLen));//密文
			memcpy(MAC,&SecModuleREP.Data.EncryptWYData.Data[*DataLen],32);
		}
		return ret;
	}
}
/**
 *解密无源标签数据并计算MAC
 *输入参数	TID	标签TID
 *输入输出参数 *DataLen 输入参数时表示密文数据长度,输出参数时表示明文数据长度
 *	*Data 输入表示密文数据,输出表示明文数据
 *	MAC 输入参数时表示初始参量,输出参数表示输出MAC
 */
uint8_t SecModule_DecryptWYData(uint8_t TID[8], uint8_t *DataLen,
		uint8_t *Data, uint8_t MAC[32]){
		int ret=0;
		int len;
		memset(&SecModuleCMD, 0, sizeof(SECURITY_CMD));
		len = 1+8+32+1+1+(*DataLen);
		memcpy(SecModuleCMD.Data.DecryptWYData.TID,TID,8);
		memcpy(SecModuleCMD.Data.DecryptWYData.MAC,MAC,32);//初始参量
		SecModuleCMD.Data.DecryptWYData.DataLen = (*DataLen);//数据长度
		memcpy(SecModuleCMD.Data.DecryptWYData.Data,Data,(*DataLen));

		ret = SecModule_Operate(CMD_DECRYPT_WY_DATA, len);
		if(ret == -1)
			return SecModule_OP_DecryptDataFail;
		else{
			if(ret == SecModule_OP_SUCCESS){
			   *DataLen = SecModuleREP.Data.DecryptWYData.DataLen;//明文长度
			   printf("the Datalen from =0x%x\n", *DataLen);
				memcpy(Data, SecModuleREP.Data.DecryptWYData.Data,(*DataLen));
				memcpy(MAC, &SecModuleREP.Data.DecryptWYData.Data[*DataLen], 32);

			}
			return ret;
		}

}
/**
 *标签对称鉴别请求
 *输入参数	Sign 标识 0x00 单向鉴别 0x01 双向鉴别
 *			KeyVersion	密钥版本
 *			RID	读写器ID
 *			TID	标签ID
 *输出参数	MAC	32字节MAC
 *输入输出参数	RandomData	输入:有源标签产生的随机数,输出:鉴别请求随机数
 */
uint8_t SecModule_TagAthRequest(uint8_t Sign, uint8_t KeyVersion,
		uint8_t RID[3], uint8_t TID[8], uint8_t RandomData[8],uint8_t MAC[32]){
	int ret =0;
	memset(&SecModuleCMD,0,sizeof(SECURITY_CMD));
	SecModuleCMD.Data.TagAthRequest.Sign = Sign;
	SecModuleCMD.Data.TagAthRequest.KeyVersion = KeyVersion;
	memcpy(SecModuleCMD.Data.TagAthRequest.RID,RID,3);
	memcpy(SecModuleCMD.Data.TagAthRequest.TID,TID,8);
	memcpy(SecModuleCMD.Data.TagAthRequest.Random,RandomData,8);

	ret = SecModule_Operate(CMD_TAG_ATH_REQUEST, 0x17);
	if(ret == -1){
		return SecModule_OP_AthRequestFail;
	}else{
		if(ret == SecModule_OP_SUCCESS){
			memcpy(RandomData,SecModuleREP.Data.TagAthRequest.RandomData,8);
			memcpy(MAC, SecModuleREP.Data.TagAthRequest.MAC,32);
		}

		return ret;
	}

}
/**
 *标签对称鉴别应答
 *输入参数	Sign 0x00　单向鉴别 0x01 双向鉴别
 *			KeyVersion 密钥版本
 *			RID		读写器ID
 *			TID		标签ID
 *输出参数	MAC	32字节MAC
 *输入输出参数	RandomData	输入:鉴别应答随机数,输出:执行后的随机数
 *
 */
uint8_t SecModule_TagAthReply(uint8_t Sign, uint8_t KeyVersion,
		uint8_t RID[3], uint8_t TID[8], uint8_t RandomData[8],
		uint8_t MAC[32], uint16_t *AthResult){
	int ret = 0;
	memset(&SecModuleCMD, 0, sizeof(SECURITY_CMD));

	SecModuleCMD.Data.TagAthReply.Sign = Sign;
	SecModuleCMD.Data.TagAthReply.KeyVersion = KeyVersion;
	memcpy(SecModuleCMD.Data.TagAthReply.RID, RID, 3);
	memcpy(SecModuleCMD.Data.TagAthReply.TID, TID, 8);
	memcpy(SecModuleCMD.Data.TagAthReply.Random, RandomData, 8);
	memcpy(SecModuleCMD.Data.TagAthReply.MAC, MAC, 32);

	ret = SecModule_Operate(CMD_TAG_ATH_REPLY, 0x37);
	if(ret == -1)
		return SecModule_OP_AthReplyFail;
	else{
		if(ret == SecModule_OP_SUCCESS){
			*AthResult = SecModuleREP.Data.TagAthReply.AthResult;
		}
		return ret;
	}
}
/**
 *读写器对称鉴别请求
 *输入参数	Sign 0x00 单向鉴别　0x01 双向鉴别
 *			MID	有源标签保密模块编号
 *			RID	读写器ID
 *			TID	标签ID
 *输出参数	MAC	32字节MAC
 *输入输出参数 RandomData	输入:有源标签产生的随机数,输出:鉴别请求随机数
 */
uint8_t SecModule_ReaderAthRequest(uint8_t Sign, uint8_t MID[8], uint8_t RID[3],
		uint8_t TID[8], uint8_t RandomData[8], uint8_t MAC[32]){
		int ret = 0;

		memset(&SecModuleCMD, 0, sizeof(SECURITY_CMD));
		SecModuleCMD.Data.ReaderAthRequest.Sign = Sign;
		memcpy(SecModuleCMD.Data.ReaderAthRequest.MID, MID, 8);
		memcpy(SecModuleCMD.Data.ReaderAthRequest.RID, RID, 3);
		memcpy(SecModuleCMD.Data.ReaderAthRequest.TID, TID, 8);
		memcpy(SecModuleCMD.Data.ReaderAthRequest.Random, RandomData, 8);

		ret = SecModule_Operate(CMD_READER_ATH_REQUEST, 0x1E);
		if(ret == -1){
			return SecModule_OP_AthRequestFail;
		}else{
			if(ret == SecModule_OP_SUCCESS){
				memcpy(RandomData,SecModuleREP.Data.ReaderAthRequest.RandomData, 8);

				memcpy(MAC, SecModuleREP.Data.ReaderAthRequest.MAC, 32);
			}

			return ret;
		}

}
/**
 *读写器对称鉴别应答
 *输入参数	Sign	0x00 单向鉴别　0x01 双向鉴别
 *			MID RID TID MAC
 *输入输出参数	RandomData	输入:鉴别应答随机数,输出:执行后的随机数
 */
uint8_t SecModule_ReaderAthReply(uint8_t Sign, uint8_t MID[8], uint8_t RID[3],
		uint8_t TID[8], uint8_t MAC[32], uint8_t RandomData[8], uint16_t *AthResult){
		int ret = 0;
	
		memset(&SecModuleCMD, 0, sizeof(SECURITY_CMD));
		SecModuleCMD.Data.ReaderAthReply.Sign = Sign;
		memcpy(SecModuleCMD.Data.ReaderAthReply.MID, MID, 8);
		memcpy(SecModuleCMD.Data.ReaderAthReply.RID, RID, 3);
		memcpy(SecModuleCMD.Data.ReaderAthReply.Random, RandomData, 8);
		memcpy(SecModuleCMD.Data.ReaderAthReply.MAC, MAC, 32);

		ret = SecModule_Operate(CMD_READER_ATH_REPLY, 0x3E);
		if(ret == -1)
			return SecModule_OP_AthReplyFail;
		else{
			if(ret == SecModule_OP_SUCCESS){
				memcpy(RandomData,
						SecModuleREP.Data.ReaderAthReply.RandomData, 8);
				*AthResult = SecModuleREP.Data.ReaderAthReply.AthResult;
			}

			return ret;
		}
}
/**
 * 加密通信
 * *inData : In 待加密数据
 * inDataLen: In 待加密数据长度
 * *outData: Out	密文数据
 * *outDataLen Out	密文数据长度
 * MAC: Out	加密后的MAC
 */
uint8_t SecModule_EncryptComm(uint8_t *inData,
		uint8_t inDataLen,	uint8_t *outData,
		uint8_t *outDataLen,	uint8_t MAC[4]){
	int ret = 0;
	memset(&SecModuleCMD, 0, sizeof(SECURITY_CMD));
	SecModuleCMD.Data.EncryptComm.DataLen = inDataLen;
	memcpy(SecModuleCMD.Data.EncryptComm.Data, inData, inDataLen);

	ret = SecModule_Operate(CMD_ENCRYPT_COMM, 3+inDataLen);

	if(ret == -1){
		return SecModule_OP_EncryptCommFail;
	}else{
		if(ret == SecModule_OP_SUCCESS){
			*outDataLen = SecModuleREP.Data.EncryptComm.DataLen;
			memcpy(outData,
					SecModuleREP.Data.EncryptComm.Data, (*outDataLen));
			memcpy(MAC, &SecModuleREP.Data.EncryptComm.Data[*outDataLen], 4);
		}

		return ret;
	}
}

/**
 *解密通信
 * *inData: In 输入密文数据
 * inDataLen: In 密文数据长度
 * MAC:		In MAC
 * *outData: Out 明文数据
 * outDataLen: Out 明文数据长度
 */
uint8_t SecModule_DecryptComm(uint8_t *inData, uint8_t inDataLen,
		uint8_t MAC[4], uint8_t *outData, uint8_t *outDataLen){
	int ret = 0;
	memset(&SecModuleCMD, 0, sizeof(SECURITY_CMD));

	SecModuleCMD.Data.DecryptComm.DataLen = inDataLen;
	memcpy(SecModuleCMD.Data.DecryptComm.Data, inData, inDataLen);
	memcpy(&SecModuleCMD.Data.DecryptComm.Data[inDataLen], MAC, 4);

	ret = SecModule_Operate(CMD_DECRYPT_COMM, 7+inDataLen);
	if(ret == -1){
		return SecModule_OP_EncryptCommFail;
	}else{
		if(ret == SecModule_OP_SUCCESS){
			*outDataLen = SecModuleREP.Data.DecryptComm.DataLen;
			memcpy(outData, SecModuleREP.Data.DecryptComm.Data, (*outDataLen));
		}

		return ret;
	}
}



/**
 *保密模块进入低功耗
 */
uint8_t SecModule_EnterLowPower(void){
	int ret = 0;
	memset(&SecModuleCMD,0,sizeof(SECURITY_CMD));
	ret = SecModule_Operate(CMD_LOW_POWER, 0x02);
	if(ret == -1)
		return SecModule_OP_EnterLowPowerFail;
	else
		return ret;
}
/**
 *获取安全参数
 *输出参数 *KeyPara 密钥参数
 *输出参数 *SecAbility 安全能力参数
 *输出参数 *SecBasicInfo 保密模块基本信息
 */
uint8_t SecModule_GetPara(uint16_t *KeyPara, uint16_t *SecAbility, SEC_BASIC_INFO *SecBasicInfo){
	int ret=0;

	memset(&SecModuleCMD,0,sizeof(SECURITY_CMD));
	ret = SecModule_Operate(CMD_GET_PARA, 0x02);
	if(ret == -1){
		return SecModule_OP_GetParaFail;
	}else{
		if(ret == SecModule_OP_SUCCESS){
		*KeyPara = SecModuleREP.Data.GetSecPara.KeyParam;
		*SecAbility = SecModuleREP.Data.GetSecPara.SecAbility;
		memcpy((uint8_t *)SecBasicInfo, (uint8_t *)&SecModuleREP.Data.GetSecPara.BasicInfo, sizeof(SEC_BASIC_INFO));
		}
		return ret;
	}
	

}
/**
 *获取基本参数
 *输出参数 *SecBasicInfo
 *
 */
void SecModule_GetBasicPara(SEC_BASIC_INFO *SecBasicInfo){
	memcpy((void *)SecBasicInfo,(void*)&SecModule_BasicInfo,sizeof(SEC_BASIC_INFO));
}
/**
 *自检初始化
 *输入参数 Sign 0x00 有源标签调用保密模块
 * 0x01:读写器调用保密模块
 *输入参数 InitPasswork 保密模块自检初始化密码
 *
 */
uint8_t SecModule_SelfInit(uint8_t Sign, uint8_t InitPassword[8]){
	int ret = 0;

	memset(&SecModuleCMD,0,sizeof(SECURITY_CMD));
	SecModuleCMD.Data.SelfInit.Sign = Sign;
	memcpy(SecModuleCMD.Data.SelfInit.InitPassword, InitPassword, 8);
	ret = SecModule_Operate(CMD_SELF_INIT, 0x0B);
	if(ret == -1)
		return SecModule_OP_SelfInitFail;
	else
		return ret;
}


/**
 *保密模块初始化
 *输入参数:Sign 标志
 *0 有源标签调用保密模块
 *1 读写器调用保密模块
 *Password:输入参数 保密模块自检初始化密码
 */
uint8_t SecModule_Init(uint8_t Sign,
		uint8_t Password[8]){
	uint8_t res=0;
	uint8_t dly=1;

	memset(&SecModuleCMD, 0 ,sizeof(SECURITY_CMD));
	ioctl(gpio_fd, 1, 1);//CS引脚先置高
	ioctl(gpio_fd, 3, 1);//复位引脚先置高
	usleep(10000);
	ioctl(gpio_fd, 3, 0);//复位引脚置低复位
	usleep(10000);
	ioctl(gpio_fd, 3, 1);//复位引脚置高
	usleep(10000);
	//等待State引脚为低直到超时
	if(0 == SecModule_WaitState(1500)){
		ioctl(gpio_fd, 1, 0);//CS引脚置低
	SecModule_Recv_Ext((uint8_t *)&SecModuleREP);//获取返回值
		ioctl(gpio_fd, 1, 1);
	}

	if(SecModuleREP.Length != 0x13){//返回码有误
		return SecModule_OP_ResetFail;
	}
	//复位成功
	while(dly--){
		usleep(10000);
	}

	res = SecModule_RandomCheck();//随机数自检
	if(res){
		return res;
	}

	dly = 1;
	while(dly--){
		usleep(10000);
	}

	res = SecModule_GetPara(&SecModule_KeyPara,&SecModule_SecAbility,&SecModule_BasicInfo);
	if(res){
		return res;
	}
	dly = 1;
	while(dly--){
		usleep(10000);
	}
	res = SecModule_SelfInit(Sign, Password);//自检初始化
	if(res){
		return res;
	}

	return SecModule_OP_SUCCESS;
}
/**
 *保密模块检测
 *返回值:0有保密模块,-1无保密模块
 */
int SecModule_Check(){
	uint8_t i;
	ioctl(gpio_fd, 3, 0);//先置低复位信号
	ioctl(gpio_fd, 3, 1);
	//重复检测20次
	for(i=20; i>0; i--){
		if(ioctl(gpio_fd, 4,0)==0){
			printf("The Device have SecModule\n");
			return 0;
		}else{
			printf("Not Check the Device have SecModule\n");
		}
	}
	
	return -1;
}

int main(void)
{
	int i=0;
	gpio_fd = open("/dev/gpio_secmodule", O_RDWR);
	if(gpio_fd < 0){
		printf("open secmodule gpio error\n");
		return 0;
	}
	else
		printf("open secmodule gpio success\n");
	spi_fd = spi_init();
	if(spi_fd < 0){
		printf("spi open fail\n");
		return 0;
	}
	else
		printf("spi open success\n");
	//SecModule_Check(gpio_fd);
	SecModule_Init(0x01, "Welcome!");
	//SecModule_EnterLowPower();
	//uint8_t RandomData[128];
	//SecModule_RandomRequest(0x01, RandomData);
	uint8_t TID[8]={0x01,0x01,0x02,0x03,0x04,0x05,0x06,0x07};
	uint8_t Key[4];
	uint8_t Data[128]={0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,
	0x0a,0x0b,0x0c,0x0d,0x0e,0x0f};
	uint8_t *DataLen;
	uint8_t len = 16;
	DataLen =&len;
	uint8_t MAC[32] ={0x00};
	uint8_t MAC1[32] = {0x00};
	//SecModule_GenerateTagKey(TID,0x0000,0x01,Key);
	//SecModule_ModifyPassword("Welcome!","Hello123");
	//SecModule_EncryptWYData(TID,DataLen,Data,MAC);
	//uint8_t Data1[128];
	//memcpy(Data1,Data,*DataLen);
	//SecModule_DecryptWYData(TID,DataLen,Data1,MAC1);
	//close(gpio_fd);
	//close(spi_fd);
	return 0;
}
