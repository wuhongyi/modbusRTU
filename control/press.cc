// press.cc --- 
// 
// Description: 
// Author: Hongyi Wu(吴鸿毅)
// Email: wuhongyi@qq.com 
// Created: 二 5月 21 14:59:08 2019 (+0800)
// Last-Updated: 二 5月 21 16:48:13 2019 (+0800)
//           By: Hongyi Wu(吴鸿毅)
//     Update #: 12
// URL: http://wuhongyi.cn 

// g++ press.cc -lmodbus -o 123

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
  mb = modbus_new_rtu("/dev/ttyUSB1",9600,'N',8,1);//open port
  if (mb == NULL) {
    fprintf(stderr, "Unable to create the libmodbus context\n");
    return -1;
  }
  modbus_set_debug(mb, true);
  modbus_set_slave(mb,0x1);//set slave address

  
  // if(modbus_rtu_set_serial_mode(mb,MODBUS_RTU_RS232) == -1)
  //   {
  //     fprintf(stderr, "%s\n", modbus_strerror(errno));
  //     return -1;
  //   }
  
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

  
  int regs = modbus_read_registers(mb, 0x006B, 0x0002, tab_reg);//0x3
  if (regs == -1) {
    fprintf(stderr, "%s\n", modbus_strerror(errno));
    return -1;
  }

  // for (int i = 0; i < regs; ++i)
  //   {
      
  //     std::cout<<i<<" "<<tab_reg[i]<<std::endl;
  //   }
  // printf("%d %d %d %d %d\n",regs,tab_reg[0],tab_reg[1],tab_reg[2],tab_reg[3]);

  short data1,data2,data3,data4;
  data2 = (tab_reg[0]&0xFF)-48;
  data1 = ((tab_reg[0]&0xFF00)>>8)-48;
  data4 = (tab_reg[1]&0xFF)-48;
  data3 = ((tab_reg[1]&0xFF00)>>8);//43+ 45-

  // std::cout<<data1<<" "<<data2<<" "<<data3<<" "<<data4<<std::endl;
  if(data3==43)
    std::cout<<data1<<"."<<data2<<"E+"<<data4<<std::endl;
  else
    std::cout<<data1<<"."<<data2<<"E-"<<data4<<std::endl;

  
  //0x6
  // int sv = 200;
  // if (modbus_write_register(mb,0x04,sv) == -1) {
  //   fprintf(stderr, "%s\n", modbus_strerror(errno));
  //   return -1;
  // }
  
  modbus_close(mb);  
  modbus_free(mb);
  return 0;
}

// 
// press.cc ends here
