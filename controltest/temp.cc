// temp.cc --- 
// 
// Description: 
// Author: Hongyi Wu(吴鸿毅)
// Email: wuhongyi@qq.com 
// Created: 五 8月 23 20:35:13 2019 (+0800)
// Last-Updated: 三 8月 28 19:35:40 2019 (+0800)
//           By: Hongyi Wu(吴鸿毅)
//     Update #: 14
// URL: http://wuhongyi.cn 

// g++ temp.cc -lmodbus -o 123

#include<stdio.h>
#include<stdlib.h>
#include <string.h>
#include"modbus/modbus.h"
#include <errno.h>
#include <iostream>

int main(void)
{
  modbus_t *mb = NULL;
  uint16_t tab_reg[64]={0};
  mb = modbus_new_rtu("/dev/ttyUSB3",9600,'N',8,1);//open port
  if (mb == NULL) {
    fprintf(stderr, "Unable to create the libmodbus context\n");
    return -1;
  }
  modbus_set_debug(mb, true);
  modbus_set_slave(mb,0x1);//set slave address

  
  if (modbus_connect(mb) == -1) {
    fprintf(stderr, "Connect failed: %s\n", modbus_strerror(errno));
    modbus_free(mb);
    return -1;
  }

  if(modbus_flush(mb) == -1)
    {
      fprintf(stderr, "%s\n", modbus_strerror(errno));
      return -1;
    }

  
  struct timeval t;
  t.tv_sec=0;
  t.tv_usec=1000000;//set modbus time 1000ms
  // modbus_set_response_timeout(mb,&t);
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
  int regs = modbus_read_registers(mb, 0x0028, 0x0004, tab_reg);//0x3
  if (regs == -1) {
    fprintf(stderr, "%s\n", modbus_strerror(errno));
    return -1;
  }

  for (int i = 0; i < regs; ++i)
    {
      
      std::cout<<i<<" "<<tab_reg[i]<<std::endl;
    }
  printf("%d %d %d %d %d\n",regs,tab_reg[0],tab_reg[1],tab_reg[2],tab_reg[3]);


  uint16_t a = 0xffff;
  // printf("%x\n", a);
  // printf("%x\n", ((uint16_t)(~a)+1));

  if(a>=0x8000)
    {
      std::cout<<((uint16_t)(~a)+1)*(-0.1)<<std::endl;
    }
  else
    {
      std::cout<<0.1*a<<std::endl;
    }

  //0x6
  // int sv = 2;
  // if (modbus_write_register(mb,0x0001,sv) == -1) {
  //   fprintf(stderr, "%s\n", modbus_strerror(errno));
  //   return -1;
  // }
  
  modbus_close(mb);  
  modbus_free(mb);
  return 0;
}

// 
// temp.cc ends here
