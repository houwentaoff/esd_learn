1.打开读写器
int XCOpen(int com_port);
/**
 * @brief 打开读写器
 * @param com_port:要打开的串口号[0,1,2,3]
 * @return 串口文件描述符(重要)，-1:失败
 */
2.关闭读写器
int XCClose(int fd);
/**
 * @brief 关闭读写器
 * @param fd:串口文件描述符
 * @return 0:成功，-1失败
 */
3.开功放
int XCPowerOn(int fd, unsigned char ucAntenna)；
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
4.关功放
int XCPowerOff(int fd)；
/**
 * @brief 关闭功放 61H，停止读写器扫描标签工作
 * @param fd:描述符
 * @return 1:操作成功
 *		  -1:描述符错误
 *		  -4:接收数据长度不符
 *		  -5:数据格式错误
 *		  -20:其它错误失败
 */
5.读用户区数据
int XCGetUserData(int fd, unsigned char ucAntenna,
						  unsigned char isDoAffirm,
					      unsigned char *pTagPWD,
						  unsigned char ucStartAdd,
						  unsigned char ucLength);
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
6.接收用户区数据
int XCGetUserDataReport(int fd, unsigned char *pValue);
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
7.写用户区数据
int XCSetUserData(int fd, unsigned char ucAntenna,
						  unsigned char *pTagPWD,
						  unsigned char ucStartAdd,
						  unsigned char ucLength,
						  unsigned char *pValue);
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
8.读EPC编码信息
int XCIdentifyEpcCode(int fd, unsigned char ucAntenna,
					unsigned char ucScanTimes,
					unsigned char isDoAffirm);
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
9.写标签EPC码
int XCSetEpcCode(int fd, unsigned char ucAntenna,
						 unsigned char *pTagPWD,
						 unsigned char ucStartAdd,
						 unsigned char ucLength,
						 unsigned char *pEpcValue)；
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
10.读TID码
int XCIdentifyTIDCode(int fd, unsigned char ucAntenna,
					 unsigned char ucScanTimes,
					 unsigned char isDoAffirm,
					 unsigned char *pTagPWD);
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
11.接收TID码
int XCTIDCodeReport(int fd, unsigned char *pTIDCode)；
/**
 *@brief 接收读写器识别到的标签TID编码
 *@param id:文件描述符
 *@param *pTIDCode 返回信息包括：状态字 天线号 标签类型TID码
 *@return >3:操作成功后返回的长度
		 -1:文件描述符错误
		 -4:接收数据长度不符
 */

