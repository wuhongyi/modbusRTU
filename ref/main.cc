// main.cc --- 
// 
// Description: 
// Author: Hongyi Wu(吴鸿毅)
// Email: wuhongyi@qq.com 
// Created: 五 4月  5 20:55:46 2019 (+0800)
// Last-Updated: 五 4月  5 20:56:11 2019 (+0800)
//           By: Hongyi Wu(吴鸿毅)
//     Update #: 1
// URL: http://wuhongyi.cn 

#include<stdio.h>
#include<stdlib.h>
#include"modbus/modbus.h"
 
int main(void)
{
  modbus_t *mb;
  uint16_t tab_reg[32]={0};
 
  mb = modbus_new_tcp("192.168.1.222",601);//由于是tcp client 连接，在同一程序中相同的端口可以连接多次
  modbus_set_slave(mb,1);//从机地址
  modbus_connect(mb);
 
  struct timeval t;
  t.tv_sec=0;
  t.tv_usec=1000000;  //设置modbus超时时间为1000毫秒  
  modbus_set_response_timeout(mb,&t);
 
  int regs=modbus_read_registers(mb,0,20,tab_reg);
 
  regs=modbus_write_register(mb,0x00,94);
 
  printf("%d %d %d %d %d %d %d %d\n",regs,tab_reg[0],tab_reg[1],tab_reg[2],tab_reg[3],tab_reg[4],tab_reg[5],tab_reg[6],tab_reg[7]);
 
  modbus_close(mb);
  modbus_free(mb);
  return 0;
}

// 
// main.cc ends here
