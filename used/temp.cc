// temp.cc --- 
// 
// Description: 
// Author: Hongyi Wu(吴鸿毅)
// Email: wuhongyi@qq.com 
// Created: 三 8月 28 19:43:14 2019 (+0800)
// Last-Updated: 三 8月 28 20:03:42 2019 (+0800)
//           By: Hongyi Wu(吴鸿毅)
//     Update #: 3
// URL: http://wuhongyi.cn 

// g++ temp.cc `mysql_config --cflags --libs` -lmodbus -o temp

// create table temp(
// ts timestamp,
// no int,
// t1 int,
// t2 int,
// t3 int,
// t4 int);


#include "modbus/modbus.h"
#include <mysql/mysql.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <iostream>
#include <unistd.h>
#include <sstream>
#include <cstring>

#define SLEEPTIME 3
int main(int argc, char *argv[])
{
  if(argc != 3)
    {
      std::cout<<"argc != 2 ..."<<std::endl;
      std::cout<<"eg: "<<argv[0]<<"  /dev/ttyUSBX  [1/2]"<<std::endl;
      return 1;
    }

  int mod;
  std::stringstream s2i;
  s2i.clear();//重复使用前一定要清空
  s2i<<argv[2];
  s2i>>mod;

  if(mod!=1 && mod!=2)
    {
      std::cout<<"The parameter mod can only be set to 1 or 2."<<std::endl;
      return 1;
    }
 
  while(true)
    {
      uint16_t tab_reg[64] = {0};
      modbus_t *mb = NULL;
      mb = modbus_new_rtu(argv[1],9600,'N',8,1);
      if (mb == NULL)
	{
	  std::cout<<"Unable to create the libmodbus context"<<std::endl;
	  sleep(SLEEPTIME);// s
	  continue;
	}
      // modbus_set_debug(mb, true);
      modbus_set_slave(mb,0x1);//set slave address
      if (modbus_connect(mb) == -1)
	{
	  std::cout<<"Connect failed: "<<modbus_strerror(errno)<<std::endl;
	  modbus_free(mb);
	  sleep(SLEEPTIME);// s
	  continue;
	}

      struct timeval t;
      t.tv_sec = 0;
      t.tv_usec = 1000000;//set modbus time 1000ms
      modbus_set_byte_timeout(mb, &t);

      // 0x0001 设备地址
      // 0x0003 波特率
      // 0x0028 通道 1 温度值
      // 0x0029 通道 2 温度值
      // 0x002A 通道 3 温度值
      // 0x002B 通道 4 温度值
      // 0x01F5 通道 1 温度值修正(-12.8~+12.7)
      // 0x01F6 通道 2 温度值修正(-12.8~+12.7)
      // 0x01F7 通道 3 温度值修正(-12.8~+12.7)
      // 0x01F8 通道 4 温度值修正(-12.8~+12.7)
      if (modbus_read_registers(mb, 0x0028, 0x0004, tab_reg) == -1)
	{
	  std::cout<<modbus_strerror(errno)<<std::endl;
	  modbus_free(mb);
	  sleep(SLEEPTIME);// s
	  continue;
	}
      else
	{
	  MYSQL conn;
	  mysql_init(&conn);
	  if(mysql_real_connect(&conn,"localhost","data","123456","monitor",0,NULL,CLIENT_FOUND_ROWS)) //"data":数据库管理员 "12346":root密码 "monitor":数据库的名字
	    {
	      char savedata[1024];	      
	      sprintf(savedata,"insert into temp values(now(),%u,%u,%u,%u,%u)",mod,tab_reg[0],tab_reg[1],tab_reg[2],tab_reg[3]);

	      if(mysql_query(&conn,savedata))
		{
		  std::cout<<"insert data error..."<<std::endl;
		}
	      mysql_close(&conn);
	    }
	  else
	    {
	      std::cout<<"connect mysql error..."<<std::endl;
	    }
	}

      modbus_close(mb);  
      modbus_free(mb);
      sleep(SLEEPTIME);// s
    }
  
  
  return 0;
}

// 
// temp.cc ends here
