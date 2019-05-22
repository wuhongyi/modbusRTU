#!/usr/bin/python  
#-*-coding:utf-8-*-
# modbus.py --- 
# 
# Description: 
# Author: Hongyi Wu(吴鸿毅)
# Email: wuhongyi@qq.com 
# Created: 一 5月  6 13:14:06 2019 (+0800)
# Last-Updated: 五 5月 10 21:45:14 2019 (+0800)
#           By: Hongyi Wu(吴鸿毅)
#     Update #: 30
# URL: http://wuhongyi.cn 

import serial
import serial.tools.list_ports
import modbus_tk
import modbus_tk.defines as cst
from modbus_tk import modbus_rtu

PORT = '/dev/ttyUSB0'

def main():
    try:
        master = modbus_rtu.RtuMaster(serial.Serial(port=PORT,
		        baudrate=9600,
		        bytesize=8,
		        parity='N',
		        stopbits=1,
		        xonxoff=0))
        
        master.set_timeout(5)
        master.set_verbose(True)
        
        print('connected')
        #read 方法
        print(master.execute(1,cst.READ_HOLDING_REGISTERS,0,5))#slaveAddr funCode startAddr regNum
        #write方法
        #print(master.execute(35, cst.WRITE_MULTIPLE_REGISTERS, 9, output_value=[1]))
        print('end of read')
    except modbus_tk.modbus_rtu.ModbusInvalidResponseError as err:
        print(err)

if __name__ == '__main__' :
    main()


# 
# modbus.py ends here
