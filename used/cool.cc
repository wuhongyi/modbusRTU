// cool.cc --- 
// 
// Description: 
// Author: Hongyi Wu(吴鸿毅)
// Email: wuhongyi@qq.com 
// Created: 三 5月 22 20:12:11 2019 (+0800)
// Last-Updated: 三 5月 22 20:52:36 2019 (+0800)
//           By: Hongyi Wu(吴鸿毅)
//     Update #: 9
// URL: http://wuhongyi.cn 

// g++ cool.cc `mysql_config --cflags --libs` -lmodbus -o cool

// cool
// ts timestamp,
// exp int,
// th int

#include "modbus/modbus.h"
#include <mysql/mysql.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <iostream>
#include <unistd.h>

#define SLEEPTIME 5
int main(int argc, char *argv[])
{
  if(argc != 2)
    {
      std::cout<<"argc != 2 ..."<<std::endl;
      std::cout<<"eg: "<<argv[0]<<"  /dev/ttyUSBX"<<std::endl;
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



      //0x00 测量值（PV）         R
      //0x01 指示灯输出状态        R
      //0x02 输出百分比（输出 1）  手R/W 自动R
      //0x03 手动/自动状态         R/W
      //0x04 设定值(SV)          R/W
      //0x05 输出限幅             R/W
      //0x06 自整定              R/W
      //0x07 报警值 1            R/W
      //0x08 报警值 2            R/W
      //0x09 报警不灵敏区 1       R/W
      //0x0A 报警不灵敏区 2       R/W
      //0x0B 报警方式 1          R/W
      //0x0C 报警方式 2          R/W
      //0x0D 比例带设置          R/W
      //0x0E 积分设置            R/W
      //0x0F 微分设置            R/W
      //0x10 积分限幅设置         R/W
      //0x11 输出周期设置         R/W
      //0x12 测量值修正           R/W
      //0x13 整定限幅设置         R/W
      //0x14 输入分度号           R/W
      //0x15 小数点设置           R/W
      //0x16 滤波系数             R/W
      //0x17 测量量程上限          R/W
      //0x18 测量量程下限          R/W
      //0x19 传感器故障时间        R/W
      //0x1A 故障温度判断          R/W
      //0x1B 温度单位选择          R/W
      if (modbus_read_registers(mb, 0x0000, 0x0005, tab_reg) == -1)
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
	      sprintf(savedata,"insert into cool values(now(),%u,%u)",tab_reg[0],tab_reg[4]);

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
// cool.cc ends here
