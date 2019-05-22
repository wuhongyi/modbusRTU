// press1.cc --- 
// 
// Description: 
// Author: Hongyi Wu(吴鸿毅)
// Email: wuhongyi@qq.com 
// Created: 三 5月 22 20:58:25 2019 (+0800)
// Last-Updated: 三 5月 22 21:14:11 2019 (+0800)
//           By: Hongyi Wu(吴鸿毅)
//     Update #: 5
// URL: http://wuhongyi.cn 

// g++ press1.cc `mysql_config --cflags --libs` -lmodbus -o press1

// cool
// ts timestamp,
// num int unsigned,

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

      if (modbus_read_registers(mb, 0x006B, 0x0002, tab_reg) == -1)
	{
	  std::cout<<modbus_strerror(errno)<<std::endl;
	  modbus_free(mb);
	  sleep(SLEEPTIME);// s
	  continue;
	}
      else
	{
	  modbus_close(mb);  
	  modbus_free(mb);
	  
	  MYSQL conn;
	  mysql_init(&conn);
	  if(mysql_real_connect(&conn,"localhost","data","123456","monitor",0,NULL,CLIENT_FOUND_ROWS)) //"data":数据库管理员 "12346":root密码 "monitor":数据库的名字
	    {
	      char savedata[1024];
	      unsigned int vaule = (((unsigned int)tab_reg[0])<<16) + tab_reg[1];
	      sprintf(savedata,"insert into press1 values(now(),%u)",vaule);
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

      sleep(SLEEPTIME);// s
    }
  
  
  return 0;
}

// 
// press1.cc ends here
