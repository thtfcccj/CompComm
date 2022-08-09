
## 组件层-通讯相关模块 接口及其实现(均与应用无关)，每个目录为一个模块，以下按字母排序：
  * Adler32: Adler32校验码算法
  * Adr:  获取8位拔码开关地址值，内置不同MCU的硬件实现。
  * AtUsart: At指令驱动(建立在UsartDev之上)
  * bReader: 比特流数据流读取器
  * BusBase: 多总线时的总线基类，内部：
    + BusCount: 基类根，实现了通讯质量计数：报文总数，有效报文，无效报文
    + BusBase:  以BusCount为基类，封装了总线ID(如I2C, USART统一分配)及供派生类使用的数据
    + BusMount: 实现总线的开机动态挂接，以支持对应总线的开启与关闭
    + BusCount_TMenu: 即在BusCount在TMenu上的菜单实现，用于显示报文总数，有效报文，无效报文等
  * BusSlvUsart: Usart从机实现，以BusBase为基类，以数据间时间间隔为一帧数据判断。
  * Code：  UTF8与GB2312等转码模块
  * DA: 12~16位高精度,数模转换模块，含：
    + DA.h：为此模块的标准接口
    + DA_PWM_MCU系列：为不同MCU对应硬件使用PWM实现DA时的实现
    + DA_Adj: 建立在DA.h接口之上，为DA提供零点与满度标定功能。
    + DA_Adj_SMenu: 为DA_Adj的SMenu(增加版4位数码管菜单系统)菜单实现，为DA提供零点与标定功能提供人机接口
    + DA_Adj_QMenu: 为DA_Adj的SMenu(普通4位数码管菜单系统)菜单实现，为DA提供零点与标定功能提供人机接口
  * DA8: 8位低精度数模转换模块，含：
    + DA8.h：为此模块的标准接口
    + DA8_PWM_MCU系列：为不同MCU对应硬件实现DA时的实现
    + DA8_Adj: 建立在DA8.h接口之上，为DA8提供零点保存功能(不支持满度标定)

  * DeflateNano: Deflate 精简型 解码器,主要用于Deflate压缩格式，如GZIP PNG图片的解码(依赖下述HuffmanTree模块)
  * ESP8266： ESP8266驱动程序，实现了与ESP8288的指令对接，如设置工作模式，自动或手动链接WIFI，直到建立透传通道
  * ESP8266St： 置 ESP8266 Station(S首字母)与+TCP Client(C首字母)模式，以与实现其智能配网，读写IP地址等AT功能
  * HuffmanTree： 哈夫曼树的嵌入式版实现，实现了哈夫曼解码，固化了固定哈夫曼树及其生成的常量表以提高PNG图版等解码效率
  * IPv4Cfg： IP4地址(含端口)的定义与配置保存，含其在QMenu中的调整。
  * LRC： LRC 累加和校验(8位，16位等)的实现

  * ModbusCodec：实现了Modbus RTU模式的编解码，对接底层数据帧与上层应用层。
  * ModbusRtuMng：  ModbusRtu模式的管理，集成了：
    + ModbusRtuMng: 实现了Modbus从机数据的收发，CRC校验，以及地址，波特率配置等，建立在UsartDev之上。 
    + ModbusRtuMng_CRC: ModbusRtuMng用到的CRC16校验，实现了查找方式，以及时间换空间的查表方式实现
    + ModbusRtuMng_QMenu: ModbusRtuMng在QMenu(普通4位数码管菜单系统)菜单中的实现。为其参数调整与与显示提供人机接口
  * Mqtt: MQTT协议设备端实现，底层依赖第三方“paho MQTT协议栈”：
    + MqttMng： 管理器，实现与MQTT服务器的连接，订阅，发送消息等
    + MqttConUser： 管理MQTT用户名与密码保存等
    + MqttConUserAT： Mqtt用户内容编码器：MqttMng配合完成数据编解码。
  * MsgPacket: 网络传输用消息包及队列：
    + MsgPacket: 消息包定义
    + MsgPacketQ: 消息包队列,多个消息包按FIFO组成的队列管理。用于如应用层，与总线数据发收管理层的消息传递管理
  * SHA256： SHA256算法的嵌入式版本
    + SHA256_unstd.c: 基础算法移值至：https://www.jianshu.com/p/0251bb005d70
    + SHA256ctx: fork至： https://github.com/monkeyDemon

  * Spi3mIo: 3线制SPI，在多个片选IO实现模块，且于支持与多个3线制SPI接口芯片的通讯，用IO口模拟方式(独立于硬件)
  * StrDefParser:  如MQTT等，字符串解析用到的常用字符串定义
  * TiCommMng：  以时间间隔区分数据帧的通用通讯管理器(仅支持单一总线)
    + 在ModbusRtuMng为基础，脱离Modbus协议仅保留数据管理，以实现非modbus等多种通讯管理用途
    + TiCommMng_UsartTiny 内含建立在UsartTiny上的实现
  * TiCommMngM： TiCommMng 支持多路总线的TiCommMng，用在同一MCU，多个通讯接口的场景应用

  * UsartTiny: Usart通讯的的简化及单例化实现：
    + UsartTiny.h: 标准接口
    + UsartTiny_DevS: 建立在UsartDev(https://gitee.com/thtfcccj/Usart)上的UsartTiny接口实现
    + UsartTiny_MCU： 不同MCU上的UsartTiny接口实现
  * UsartTinyM Usart通讯的多例化实现， 支持多态：如硬件UART与IO模拟UART一起使用此模块管理
  * winWriter: 与bReader(比特流数据流读取器)相对，用于比特流输出与缓冲
    + 支持嵌入式环境时，如: 一边生成数据(如解压)，一边输出数据(如写FLASH,显示屏)的方式工作
    + 支持窗口缓存数据与用户数据管理。
  * ZlibDecompress: ZLIB格式比特流解码,修改出自"lodepnge"项目，优化以用于如PNG图像在嵌入式系统中的应用,依赖于：
    + bReader
    + DeflateNano 间接依赖于 HuffmanTree模块
    + winWriter
    

  
  









