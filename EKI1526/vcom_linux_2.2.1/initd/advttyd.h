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

#ifndef _ADVTTY_H
#define _ADVTTY_H

#include "advconf.h"

#define CF_MAXSTRLEN	32
#define CF_LOGNAME  	("advttyd.log")
#define CF_CONFNAME 	("advttyd.conf")
#define CF_PORTPROG 	("vcomd")
#define CF_VERSION	("1.20")

#define CF_MAXPORTS	VCOM_PORTS

//#define ADVTTYD_DEBUG
#ifdef ADVTTYD_DEBUG
#define ADV_LOGMSG(FMT, ...) \
        do { \
        char buf[1024]; \
        sprintf(buf, FMT, ## __VA_ARGS__); \
        log_msg(buf); \
        } while(0)
#else
#define ADV_LOGMSG(FMT, ...) while(0)
#endif

#endif
