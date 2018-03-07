/**********************************************************************************************************
 *
 *                                          UhfReader_API header file
 *
 **********************************************************************************************************
 * FileName:        UhfReader_API.h
 *
 * Dependencies:    NONE
 *
 * Company:         Ray-Links Technologies
 *
 * Author           Date              Version           Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Xuanzg           2011-12-14        2.0               File Created
 *
 * YaoQing          2012-4-20         2.1               File Modified
 *********************************************************************************************************/

#if !defined _UHFREADER_API_H__
#define _UHFREADER_API_H__


typedef struct		//SRECORD
{
    unsigned char   Sindex;
    unsigned char   Slen;
    unsigned char   Target;
    unsigned char   Action;
    unsigned char   bank;
    unsigned char   Ptr[2];
    unsigned char   Len;
    unsigned char   Mask[32];
    unsigned char   Truncate;

}SRECORD;

typedef struct		//TRECORD
{
    int		Tindex;
    int		Len;
    unsigned char   UII[256];
}TRECORD;


/**********************************************************************************************************
 *
 *                                         Open port and connect
 *
 *********************************************************************************************************/
int UhfReaderConnect (char* cPort, int baudRate, unsigned char flagCrc);

/**********************************************************************************************************
 *
 *										   Disconnect and close port
 *
 *********************************************************************************************************/
int UhfReaderDisconnect (int hCom, unsigned char flagCrc);

/**********************************************************************************************************
 *
 *                                         Get RLM status
 *
 *********************************************************************************************************/
int UhfGetPaStatus (int hCom, unsigned char* uStatus, unsigned char flagCrc);

/**********************************************************************************************************
 *
 *                                         Get RLM power setting
 *
 *********************************************************************************************************/
int UhfGetPower (int hCom, unsigned char* uPower, unsigned char flagCrc);

/**********************************************************************************************************
 *
 *                                         Set RLM power
 *
 *********************************************************************************************************/
int UhfSetPower (int hCom, unsigned char uOption, unsigned char uPower, unsigned char flagCrc);

/**********************************************************************************************************
 *
 *                                         Get RLM frequency setting
 *
 *********************************************************************************************************/
int UhfGetFrequency(int hCom, unsigned char* uFreMode, unsigned char* uFreBase, unsigned char* uBaseFre, unsigned char* uChannNum, unsigned char* uChannSpc, unsigned char* uFreHop, unsigned char flagCrc);

/**********************************************************************************************************
 *
 *                                         Set RLM frequency
 *
 *********************************************************************************************************/
int UhfSetFrequency(int hCom, unsigned char uFreMode, unsigned char uFreBase, unsigned char* uBaseFre, unsigned char uChannNum, unsigned char uChannSpc, unsigned char uFreHop, unsigned char flagCrc);

/**********************************************************************************************************
 *
 *                                         Read RLM Uid
 *
 *********************************************************************************************************/
int UhfGetReaderUID (int hCom, unsigned char* uUid, unsigned char flagCrc);

/**********************************************************************************************************
 *
 *                                         RLM inventory
 *
 *********************************************************************************************************/
int UhfStartInventory (int hCom, unsigned char flagAnti, unsigned char initQ, unsigned char flagCrc);

/**********************************************************************************************************
 *
 *                                         Get received data
 *
 *********************************************************************************************************/
int UhfReadInventory (int hCom, unsigned char* uLenUii, unsigned char* uUii);

/**********************************************************************************************************
 *
 *                                         RLM stop get
 *
 *********************************************************************************************************/
int UhfStopOperation (int hCom, unsigned char flagCrc);

/**********************************************************************************************************
 *
 *                                         RLM read data
 *
 *********************************************************************************************************/
int UhfReadDataByEPC (int hCom, unsigned char* uAccessPwd, unsigned char uBank, unsigned char* uPtr, unsigned char uCnt, unsigned char* uUii, unsigned char* uReadData, unsigned char* uErrorCode, unsigned char flagCrc);

/**********************************************************************************************************
 *
 *                                         RLM write data
 *
 *********************************************************************************************************/
int UhfWriteDataByEPC (int hCom, unsigned char* uAccessPwd, unsigned char uBank, unsigned char* uPtr, unsigned char uCnt, unsigned char* uUii, unsigned char* uWriteData, unsigned char* uErrorCode, unsigned char flagCrc);

/**********************************************************************************************************
 *
 *                                         RLM erase data
 *
 *********************************************************************************************************/
int UhfEraseDataByEPC (int hCom, unsigned char* uAccessPwd, unsigned char uBank, unsigned char* uPtr, unsigned char uCnt, unsigned char* uUii, unsigned char* uErrorCode, unsigned char flagCrc);

/**********************************************************************************************************
 *
 *                                         RLM lock memory
 *
 *********************************************************************************************************/
int UhfLockMemByEPC (int hCom, unsigned char* uAccessPwd, unsigned char* uLockData, unsigned char* uUii, unsigned char* uErrorCode, unsigned char flagCrc);

/**********************************************************************************************************
 *
 *                                         RLM kill tag
 *
 *********************************************************************************************************/
int UhfKillTagByEPC (int hCom, unsigned char* uKillPwd, unsigned char* uUii, unsigned char* uErrorCode, unsigned char flagCrc);

/**********************************************************************************************************
 *
 *                                         RLM Get Version
 *
 *********************************************************************************************************/
int UhfGetVersion (int hCom, unsigned char* uSerial, unsigned char* uVersion, unsigned char flagCrc);

/**********************************************************************************************************
 *
 *                                         RLM single inventory
 *
 *********************************************************************************************************/
int UhfInventorySingleTag (int hCom, unsigned char* uLenUii, unsigned char* uUii , unsigned char flagCrc);

/**********************************************************************************************************
 *
 *                                         RLM  read data single
 *
 *********************************************************************************************************/
int UhfReadDataFromSingleTag (int hCom, unsigned char* uAccessPwd, unsigned char uBank, unsigned char* uPtr, unsigned char uCnt, unsigned char* uReadData, unsigned char* uUii, unsigned char* uLenUii, unsigned char* uErrorCode, unsigned char flagCrc);

/**********************************************************************************************************
 *
 *                                         RLM write data single
 *
 *********************************************************************************************************/
int UhfWriteDataToSingleTag (int hCom, unsigned char* uAccessPwd, unsigned char uBank, unsigned char* uPtr, unsigned char uCnt, unsigned char* uWriteData, unsigned char* uUii, unsigned char* uLenUii, unsigned char* uErrorCode, unsigned char flagCrc);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//                                          Advanced Function  
//
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**********************************************************************************************************
 *
 *                                         RLM write mutil-data
 *
 *********************************************************************************************************/
int UhfBlockWriteDataByEPC (int hCom, unsigned char* uAccessPwd, unsigned char uBank, unsigned char* uPtr, unsigned char uCnt, unsigned char* uUii, unsigned char* uWriteData, unsigned char* uErrorCode, unsigned char* uStatus, unsigned char* uWritedLen, unsigned char* RuUii, unsigned char flagCrc);

/**********************************************************************************************************
 *
 *                                         RLM read data for CNT is zero
 *
 *********************************************************************************************************/
int UhfReadMaxDataByEPC (int hCom, unsigned char* uAccessPwd, unsigned char uBank, unsigned char* uPtr, unsigned char* uUii, unsigned char* Data_len, unsigned char* uReadData, unsigned char* uErrorCode, unsigned char flagCrc);

/**********************************************************************************************************
 *
 *                                         RLM  read data single for CNT is zero
 *
 *********************************************************************************************************/
int UhfReadMaxDataFromSingleTag (int hCom, unsigned char* uAccessPwd, unsigned char uBank, unsigned char* uPtr, unsigned char* Data_len, unsigned char* uReadData, unsigned char* uUii, unsigned char* uLenUii, unsigned char* uErrorCode, unsigned char flagCrc);


/**********************************************************************************************************
 *
 *                                         RLM erase data single
 *
 *********************************************************************************************************/
int UhfEraseDataFromSingleTag  (int hCom, unsigned char* uAccessPwd, unsigned char uBank, unsigned char* uPtr, unsigned char uCnt, unsigned char* uUii, unsigned char* uErrorCode, unsigned char flagCrc);

/**********************************************************************************************************
 *
 *                                         RLM lock memory single
 *
 *********************************************************************************************************/
int UhfLockMemFromSingleTag (int hCom, unsigned char* uAccessPwd, unsigned char* uLockData, unsigned char* uUii, unsigned char* uErrorCode, unsigned char flagCrc);

/**********************************************************************************************************
 *
 *                                         RLM kill tag single
 *
 *********************************************************************************************************/
int UhfKillSingleTag (int hCom, unsigned char* uKillPwd, unsigned char* uUii, unsigned char* uErrorCode, unsigned char flagCrc);

/**********************************************************************************************************
 *
 *                                         RLM write mutil-data single
 *
 *********************************************************************************************************/
int UhfBlockWriteDataToSingleTag (int hCom, unsigned char* uAccessPwd, unsigned char uBank, unsigned char* uPtr, unsigned char uCnt, unsigned char* uWriteData, unsigned char* uUii, unsigned char* uLenUii, unsigned char* uStatus, unsigned char* uErrorCode, unsigned char* uWritedLen, unsigned char flagCrc);

/**********************************************************************************************************
 *
 *                                         RLM  read data anti single
 *
 *********************************************************************************************************/
int UhfStartReadDataFromMultiTag (int hCom, unsigned char* uAccessPwd, unsigned char uBank, unsigned char* uPtr, unsigned char uCnt, unsigned char uOption, unsigned char* uPayLoad, unsigned char flagCrc);

/**********************************************************************************************************
 *
 *                                         RLM get read data anti single
 *
 *********************************************************************************************************/
int UhfGetDataFromMultiTag (int hCom, unsigned char* uStatus, unsigned char* ufData_len, unsigned char* ufReadData, unsigned char* usData_len, unsigned char* usReadData, unsigned char* uUii, unsigned char* uLenUii);

/**********************************************************************************************************
 *
 *                                         RLM Read Register
 *
 *********************************************************************************************************/
int UhfGetRegister (int hCom, int RADD, int RLEN, unsigned char* STATUS, unsigned char* REG, unsigned char flagCrc);

/**********************************************************************************************************
 *
 *                                         RLM Write Register
 *
 *********************************************************************************************************/
int UhfSetRegister (int hCom, int RADD, int RLEN, unsigned char* REG_DATA, unsigned char* STATUS, unsigned char flagCrc);

/**********************************************************************************************************
 *
 *                                         RLM Reset Register
 *
 *********************************************************************************************************/
int UhfResetRegister (int hCom, unsigned char flagCrc);

/**********************************************************************************************************
 *
 *                                         RLM Save Register
 *
 *********************************************************************************************************/
int UhfSaveRegister (int hCom, unsigned char flagCrc);

/**********************************************************************************************************
 *
 *                                         RLM Add select
 *
 *********************************************************************************************************/
int UhfAddFilter (int hCom, SRECORD* pSRecord, unsigned char* STATUS, unsigned char flagCrc);

/**********************************************************************************************************
 *
 *                                         RLM Delete select
 *
 *********************************************************************************************************/
int UhfDeleteFilterByIndex (int hCom, unsigned char SINDEX, unsigned char* STATUS, unsigned char flagCrc);

/**********************************************************************************************************
 *
 *                                         RLM Get select
 *
 *********************************************************************************************************/
int UhfStartGetFilterByIndex (int hCom, unsigned char SINDEX, unsigned char SNUM, unsigned char* STATUS, unsigned char flagCrc);

/**********************************************************************************************************
 *
 *                                         Get select received data
 *
 *********************************************************************************************************/
int UhfReadFilterByIndex(int hCom, unsigned char* STATUS, SRECORD* pSRecord);

/**********************************************************************************************************
 *
 *                                         RLM Choose select
 *
 *********************************************************************************************************/
int UhfSelectFilterByIndex (int hCom, unsigned char SINDEX, unsigned char SNUM, unsigned char* STATUS, unsigned char flagCrc);

/**********************************************************************************************************
 *
 *                                         RLM Sleep
 *
 *********************************************************************************************************/
int UhfEnterSleepMode (int hCom, unsigned char flagCrc);

/**********************************************************************************************************
 *
 *                                         RLM BootLoader Start Update
 *
 *********************************************************************************************************/
int UhfUpdateInit (int* hCom, char* cPort, int baudRate, unsigned char* STATUS, unsigned char* RN32, unsigned char flagCrc);

/**********************************************************************************************************
 *
 *                                         RLM BootLoader Inverse
 *
 *********************************************************************************************************/
int UhfUpdateSendRN32 (int hCom, unsigned char* RN32, unsigned char* STATUS, unsigned char flagCrc);

/**********************************************************************************************************
 *
 *                                         RLM BootLoader Start trans
 *
 *********************************************************************************************************/
int UhfUpdateSendSize (int hCom, unsigned char* STATUS, unsigned char* FILESIZE, unsigned char flagCrc);

/**********************************************************************************************************
 *
 *                                         RLM BootLoader Start trans
 *
 *********************************************************************************************************/
int UhfUpdateSendData (int hCom, unsigned char* STATUS, unsigned char PACKNUM, unsigned char LASTPACK, int Data_len, unsigned char* TRANDATA, unsigned char flagCrc);

/**********************************************************************************************************
 *
 *                                         RLM BootLoader End Update
 *
 *********************************************************************************************************/
int UhfUpdateCommit  (int hCom, unsigned char* STATUS, unsigned char flagCrc);

/**********************************************************************************************************
 *
 *                                         RLM write EPC single
 *
 *********************************************************************************************************/
int UhfBlockWriteEPCToSingleTag (int hCom, unsigned char* uAccessPwd, unsigned char uCnt, unsigned char* uWriteData, unsigned char* uUii, unsigned char* uLenUii, unsigned char* uStatus, unsigned char* uErrorCode, unsigned char* uWritedLen, unsigned char flagCrc);

/**********************************************************************************************************
 *
 *                                         RLM write EPC
 *
 *********************************************************************************************************/
int UhfBlockWriteEPCByEPC (int hCom, unsigned char* uAccessPwd, unsigned char uCnt, unsigned char* uUii, unsigned char* uWriteData, unsigned char* uErrorCode, unsigned char* uStatus, unsigned char* uWritedLen, unsigned char* RuUii, unsigned char flagCrc);

#endif
