<!-- README.md --- 
;; 
;; Description: 
;; Author: Hongyi Wu(吴鸿毅)
;; Email: wuhongyi@qq.com 
;; Created: 五 4月  5 20:49:15 2019 (+0800)
;; Last-Updated: 五 5月  3 18:27:40 2019 (+0800)
;;           By: Hongyi Wu(吴鸿毅)
;;     Update #: 4
;; URL: http://wuhongyi.cn -->

# README

想要读取串口数据，modbus是一个很好用的第三方库。该库适用于windows和Linux，支持RTP、RTU等协议。

```bash
yum install libmodbus.x86_64 libmodbus-devel.x86_64
```


```cpp
// 连接串口并做一些参数配置。

m_modbus = modbus_new_rtu("/dev/ttyS0"/*serialPort*/,9600/*baud*/,78,8/*dataBits  */,1/*stopBits*/); 
//ttys0是Linux第一个串口，有的时候默认串口是ttys1，最好是枚举一下。在windows下，默认串口设备名为“COM1”
modbus_set_debug(m_modbus, 0);

// 设置等待时间，超过时间没连接上则报错。

struct timeval response_timeout;
response_timeout.tv_sec = 1;
response_timeout.tv_usec = 0;
modbus_set_response_timeout(m_modbus,&response_timeout);

// 从串口读取数据，一般是开关量数据，只有0、1两态。

int i;
if(m_modbus == NULL) 
{
printf("No connection,check your device!\n");
return;
}


uint8_t dest[1024]; //setup memory for data
uint16_t * dest16 = (uint16_t *) dest;
memset(dest, 0, 1024);
int ret = -1; //return value from read functions
int is16Bit = 0;


modbus_set_slave(m_modbus, slave);

ret = modbus_read_input_bits(m_modbus, 0/*startAddress*/, 24/*noOfItems*/, dest); //24值得是读取端口数量

// 关闭modbus连接。

if(m_modbus) {
modbus_close(m_modbus);
modbus_free(m_modbus);
m_modbus = NULL;
}
```




----

> https://blog.csdn.net/zhu530548851/article/details/22070335  
> https://blog.csdn.net/doyoung1/article/details/49804311   


<!-- README.md ends here -->
