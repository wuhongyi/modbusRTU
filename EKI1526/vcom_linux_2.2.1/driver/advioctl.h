/*	This file is part of Advantech VCOM Linux.
 *
 *	Copyright (c) 2009 - 2018 ADVANTECH CORPORATION.  All rights reserved. 
 *
 *	Advantech VCOM Linux is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License 2 as published by
 *	the Free Software Foundation.
 *
 *	Advantech VCOM Linux  is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with Advantech VCOM Linux.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __ADV_IOCTL_H
#define __ADVIOCTL_H
#define ADVVCOM_IOC_MAGIC	'A'
#define ADVVCOM_IOCGTXHEAD	_IOR(ADVVCOM_IOC_MAGIC, 1, int)
#define ADVVCOM_IOCGTXTAIL	_IOR(ADVVCOM_IOC_MAGIC, 2, int)
#define ADVVCOM_IOCGTXSIZE	_IOR(ADVVCOM_IOC_MAGIC, 3, int)
#define ADVVCOM_IOCGRXHEAD	_IOR(ADVVCOM_IOC_MAGIC, 4, int)
#define ADVVCOM_IOCGRXTAIL	_IOR(ADVVCOM_IOC_MAGIC, 5, int)
#define ADVVCOM_IOCGRXSIZE	_IOR(ADVVCOM_IOC_MAGIC, 6, int)
#define ADVVCOM_IOCSTXTAIL	_IOW(ADVVCOM_IOC_MAGIC, 7, int)
#define ADVVCOM_IOCSRXHEAD	_IOW(ADVVCOM_IOC_MAGIC, 8, int)
#define ADVVCOM_IOCGTXBEGIN	_IOR(ADVVCOM_IOC_MAGIC, 9, int)
#define ADVVCOM_IOCGRXBEGIN	_IOR(ADVVCOM_IOC_MAGIC, 10, int)
#define ADVVCOM_IOCGATTRBEGIN	_IOR(ADVVCOM_IOC_MAGIC, 11, int)
#define ADVVCOM_IOCGATTRPTR	_IOR(ADVVCOM_IOC_MAGIC, 12, int)
#define ADVVCOM_IOCSINTER	_IOW(ADVVCOM_IOC_MAGIC, 13, int)
#define ADVVCOM_IOCSMCTRL	_IOW(ADVVCOM_IOC_MAGIC, 14, int)
#define ADVVCOM_IOCSCLR		_IOW(ADVVCOM_IOC_MAGIC, 15, int)

#define ADVVCOM_IOCMAX		15


#define ADVVCOM_EVENT_TRANS_EN	0x1;
#define ADVVCOM_EVENT_RECV_EN	0x2;

#define ADV_INT_RX	(0x1 << 0)
#define ADV_INT_TX	(0x1 << 1)

#define ADV_CLR_RX	(0x1 << 0)
#define ADV_CLR_TX	(0x1 << 1)

#endif
