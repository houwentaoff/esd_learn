/*************************************************************************
	> File Name: SecModule_MPD318.h
	> Author: shuixianbing
	> Mail: shui6666@126.com 
	> Created Time: 2015年11月23日 星期一 10时32分55秒
 ************************************************************************/
#ifndef _SECMODULE_MPD318_H_
#define _SECMODULE_MPD318_H_

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include "spi.h"
#define uint8_t  unsigned char
#define uint16_t unsigned short
#pragma pack(1)
//命令类型定义
#define CMD_GET_PARA		0xC0 //获取安全参数
#define CMD_SELF_DESTORY	0xC1 //保密模块自毁
#define CMD_LOW_POWER		0xC2 //低功耗模式
#define CMD_RANDOM_CHECK	0xC3 //随机数自检
#define CMD_RANDOM_REQUEST	0xC4 //请求随之机数
#define CMD_SELF_INIT		0xC7 //自检初始化
#define CMD_GENERATE_TAG_KEY 0xD0 //生成标签口令
#define CMD_MODIFY_PASSWORD 0xD1 //修改管理口令
#define CMD_ENCRYPT_WY_DATA	0xD2 //加密无源标签数据和计算MAC
#define CMD_DECRYPT_WY_DATA 0xD3 //解密无源标签数据和计算MAC
#define CMD_ENCRYPT_YY_DATA 0xD4 //加密有源标签数据
#define CMD_DECRYPT_YY_DATA 0xD5 //解密有源标签数据
#define CMD_CALC_YY_DATA_MAC 0xD6 //对有源标签存储数据计算MAC
#define CMD_TAG_ATH_REQUEST 0xD7 //标签对称鉴别请求
#define CMD_TAG_ATH_REPLY	0xD8 //标签对称鉴别应答
#define CMD_READER_ATH_REQUEST 0xD9 //读写器对称鉴别请求
#define CMD_READER_ATH_REPLY	0xDA //读定器对称鉴别应答
#define CMD_ENCRYPT_COMM	0xDB //安全通信加密
#define CMD_DECRYPT_COMM	0xDC //安全通信解密

//生成标签口令索引
#define SEC_KEYINDEX_INIT	0x0000	//初始化口令
#define SEC_KEYINDEX_KILL	0x0001	//灭活口令
#define SEC_KEYINDEX_READY	0x0002	//就绪口令
#define SEC_KEYINDEX_SLEEP	0x0003	//睡眠口令
#define SEC_KEYINDEX_WAKEUP	0x0004	//唤醒口令
#define SEC_KEYINDEX_LOCK	0x0005  //锁定口令
#define SEC_KEYINDEX_ADMIN	0x0006	//管理员口令
#define SEC_KEYINDEX_READ	0x0007	//读口令
#define SEC_KEYINDEX_WRITE  0x0008	//写口令

//文件标识符0x3F00-0xFFFF
#define SEC_KEYMODE_LIST	0x01	//列表文件口令
#define SEC_KEYMODE_VALID	0x02	//文件有效/无效
#define SEC_KEYMODE_READ	0x03	//读文件 检索
#define SEC_KEYMODE_WRITE	0x04	//写文件

//返回值定义
typedef uint8_t SecModule_OP; //操作返回
#define SecModule_OP_SUCCESS			0		//操作成功
#define SecModule_OP_GetParaFail		0xA0	//获取安全参数失败
#define SecModule_OP_SelfDestoryFail	0xA1	//自毁失败
#define SecModule_OP_EnterLowPowerFail	0xA2	//进入低功耗失败
#define SecModule_OP_RandomCheckFail	0xA3	//随机数自检失败
#define	SecModule_OP_RandomRequestFail	0xA4	//请求随机数失败
#define SecModule_OP_SelfInitFail		0xA5	//自检初始化失败
#define SecModule_OP_GenerateTagKeyFail	0xA6	//生成标签口令失败
#define SecModule_OP_ModifyPasswordFail 0xA7	//修改管理口令失败
#define SecModule_OP_EncryptDataFail	0xA8	//对无源标签数据加密失败
#define SecModule_OP_DecryptDataFail	0xA9	//对无源标签数据解密失败
#define SecModule_OP_AthRequestFail		0xAA	//对称鉴别请求失败
#define SecModule_OP_AthReplyFail		0xAB	//对称鉴别应答失败
#define	SecModule_OP_EncryptCommFail	0xAC	//通信加密失败
#define SecModule_OP_DecryptCommFail	0xAD	//通信解密失败
#define SecModule_OP_VerifyCodeError	0xAE	//返回数据的校验值错误
#define SecModule_OP_CMDCodeError		0xAF	//返回数据的命令码错误
#define	SecModule_OP_ResetFail			0xB0	//保密模块复位失败
#define	SecModule_OP_CalcMACFail		0xB1	//保密模块计算有源标签数据MAC失败


//安全指令帧格式
typedef struct _SECURITY_CMD{
	uint8_t Length;		//数据包长度
	uint8_t CmdType;	//命令类型
	//数据
	union{
		//获取安全参数无数据
		//保密模块自毁
		struct{
			uint8_t MID[8];//保密模块编号
		}SelfDestory;
		//进入低功耗模式无数据
		//随机数检验无数据
		//向保密模块请求随机数
		struct{
			uint8_t RandomType;//随机数长度
		}RandomRequest;
		//自检初始化
		struct{
			uint8_t Sign;//标识
			uint8_t InitPassword[8];//保密模块管理口令
		}SelfInit;
		//生成标签口令
		struct{
			uint8_t TID[8];
			uint16_t Index;//口令索引
			uint8_t	Mode;//模式标识
		}GenerateTagKey;
		//修改保密模块管理口令
		struct{
			uint8_t OldPassword[8];//旧口令
			uint8_t NewPassword[8];//新口令
		}ModifyPassword;
		//对无源标签存储数据加密和计算MAC
		struct{
			uint8_t TID[8];
			uint8_t MAC[32]; //初始参量
			uint8_t DataLen; //数据长度
			uint8_t Data[128]; //无源标签待加密明文数据
		}EncryptWYData;
		//对无源标签存储数据解密和计算MAC
		struct{
			uint8_t TID[8];
			uint8_t MAC[32];//初始参量
			uint8_t DataLen;//密文数据长度
			uint8_t Data[128];//待解密的密文信息
		}DecryptWYData;
		//安全通信加密　对有源标签数据和读写器之间安全通信加密
		struct{
			uint8_t DataLen;
			uint8_t Data[128];
		}EncryptYYData;
		//安全通信解密 对有源标签数据进行解密
		struct{
			uint8_t DataLen;
			uint8_t Data[128];
		}DecryptYYData;
		//对有源标签数据计算MAC
		struct{
			uint8_t MAC[32];
			uint8_t DataLen;
			uint8_t Data[128];
		}CalcYYDataMAC;
		//读写器对称鉴别请求
		struct{
			uint8_t Sign;//标识　值为0单向鉴别　1双向鉴别
			uint8_t MID[8];//标签的保密模块编号
			uint8_t RID[3];//RID
			uint8_t TID[8];//TID
			uint8_t Random[8];//随机数
		}ReaderAthRequest;
		//读写器对称鉴别应答
		struct{
			uint8_t Sign;//标识　0单向鉴别　1双向鉴别
			uint8_t MID[8];//有源标签保密模块编号
			uint8_t RID[3];
			uint8_t TID[8];
			uint8_t Random[8];//鉴别应答随机数
			uint8_t MAC[32];//密文鉴别数据
		}ReaderAthReply;
		//标签对称鉴别请求
		struct{
			uint8_t Sign;//0单向鉴别,1双向鉴别
			uint8_t KeyVersion;//密钥版本
			uint8_t RID[3];
			uint8_t TID[8];
			uint8_t Random[8];//随机数
		}TagAthRequest;
		//标签对称鉴别应答
		struct{
			uint8_t Sign;//0单向 1双向
			uint8_t KeyVersion;//密钥版本
			uint8_t RID[3];
			uint8_t TID[8];
			uint8_t Random[8];//随机数
			uint8_t MAC[32];//密文鉴别数据
		}TagAthReply;
		//对数据进行加密同时获取MAC
		struct{
			uint8_t DataLen;//数据长度,指示明文长度
			uint8_t Data[128];//除最后一包<=128,其它均为128byte
		}EncryptComm;
		//解密
		struct{
			uint8_t DataLen;//密文长度
			uint8_t Data[128+4];//密文+完整性校验码
		}DecryptComm;
	}Data;
	//注意最后要补填校验位
}SECURITY_CMD,*P_SECURITY_CMD;

//保密模块基本信息
typedef struct _SEC_BASIC_INFO{
	uint8_t MID[8];//保密模块ID
	uint8_t DATE[4];//保密模块制发日期"YYYYMMDD"
	uint8_t M_STATE;//保密模块个人状态
	uint8_t W_STATE;//保密模块工作状态
	uint8_t COS_V;//保密模块软件版本
	uint8_t KEY_V;//密钥最新版本号
	uint8_t MANUFACTURE_C;//研制单位代号
	uint8_t PRODUCE_C;//制发单位代号
	uint8_t USAGE_M;//保密模块使用授权模式设置
	uint8_t RESERVER;//字节保留
}SEC_BASIC_INFO,*P_SEC_BASIC_INFO;

//安全指令帧响应格式
typedef struct _SECURITY_REP{
	uint8_t Length;//数据长度
	uint8_t CmdType;//命令类型
	uint8_t repCode;//返回码
	union{
		//获取安全参数
		struct{
			uint16_t KeyParam;//密钥参数
			uint16_t SecAbility;//安全能力参数
			SEC_BASIC_INFO BasicInfo;//保密模块基本信息
		}GetSecPara;
		//保密模块自毁无返回数据
		//低功耗模式无返回数据
		//随机数检验无返回数据
		//向保密模块请求随机数
		struct{
			uint8_t RandomData[24];//最长对应24字节　
		}RandomRequest;
		//保密模块自检初始化,无返回数据
		struct{
			uint8_t Key[4];
		}GenerateTagKey;
		//修改保密模块管理口令无返回数据
		//加密无源标签数据和计算MAC
		struct{
			uint8_t DataLen;//密文长度
			uint8_t Data[128+32];//加密后的密文信息+MAC
		}EncryptWYData;
		//解密无源标签数据和计算MAC
		struct{
			uint8_t DataLen;
			uint8_t Data[128+32];
		}DecryptWYData;
		//对有源标签数据进行加密
		struct{
			uint8_t DataLen;
			uint8_t Data[128];
		}EncryptYYData;
		//对有源标签数据进行解密
		struct{
			uint8_t DataLen;
			uint8_t Data[128];
		}DecryptYYData;
		//对有源标签数据计算MAC
		struct{
			uint8_t MAC[32];
		}CalcYYDataMAC;
		//读写器对称鉴别请求
		struct{
			uint8_t RandomData[8];
			uint8_t MAC[32];
		}ReaderAthRequest;
		//读写器对称鉴别应答
		struct{
			uint16_t AthResult;//鉴别结果
			uint8_t RandomData[8];
		}ReaderAthReply;
		//标签对称鉴别请求
		struct{
			uint8_t RandomData[8];
			uint8_t MAC[32];
		}TagAthRequest;
		//标签对称鉴别应答
		struct{
			uint16_t AthResult;//鉴别结果
		}TagAthReply;
		struct{
			uint8_t DataLen;//密文长度
			uint8_t Data[128+4];//4字节完整性校验码要加在后面
		}EncryptComm;
		struct{
			uint8_t DataLen;
			uint8_t Data[128+4];
		}DecryptComm;
	}Data;
}SECURITY_REP,*P_SECURITY_REP;

static SECURITY_CMD SecModuleCMD;//向保密模块发送的命令数据
static SECURITY_REP SecModuleREP;//从保密模块接收的命令数据
static uint16_t SecModule_KeyPara;//保密模块密钥参数
static uint16_t SecModule_SecAbility;//保密模块安全能力
static SEC_BASIC_INFO SecModule_BasicInfo;//保密模块基本参数
int SecModule_Check();//保密模块检测
//uint8_t SecModule_Init(uint8_t Sign, uint8_t Password[8]);//保密模块初始化
#endif
