// main.cc --- 
// 
// Description: 
// Author: Hongyi Wu(吴鸿毅)
// Email: wuhongyi@qq.com 
// Created: 一 5月  6 10:12:06 2019 (+0800)
// Last-Updated: 六 5月 11 09:01:18 2019 (+0800)
//           By: Hongyi Wu(吴鸿毅)
//     Update #: 63
// URL: http://wuhongyi.cn 

// g++ main.cc -lmodbus -o 123

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
  mb = modbus_new_rtu("/dev/ttyUSB0",9600,'N',8,1);//open port
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
  int regs = modbus_read_registers(mb, 0x0000, 0x0005, tab_reg);//0x3
  if (regs == -1) {
    fprintf(stderr, "%s\n", modbus_strerror(errno));
    return -1;
  }

  for (int i = 0; i < regs; ++i)
    {
      
      std::cout<<i<<" "<<tab_reg[i]<<std::endl;
    }
  // printf("%d %d %d %d %d\n",regs,tab_reg[0],tab_reg[1],tab_reg[2],tab_reg[3]);


  //0x6
  int sv = 200;
  if (modbus_write_register(mb,0x04,sv) == -1) {
    fprintf(stderr, "%s\n", modbus_strerror(errno));
    return -1;
  }
  
  modbus_close(mb);  
  modbus_free(mb);
  return 0;
}

// 
// main.cc ends here
