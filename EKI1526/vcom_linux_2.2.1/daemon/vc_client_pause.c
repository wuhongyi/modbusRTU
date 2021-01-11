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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>
#include <limits.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <linux/sockios.h>
#include <netdb.h>
#include "advioctl.h"
#include "advtype.h"
#include "vcom_proto.h"
#include "vcom_proto_cmd.h"
#include "vcom_proto_ioctl.h"
#include "vcom.h"
//#include "vcom_debug.h"
#include "vc_client_netdown.h"
#include "vc_client_idle.h"

#include "vc_client_common.h"

struct vc_ops vc_pause_ops;

#define ADV_THIS	(&vc_pause_ops)

static struct vc_ops * vc_pause_xmit(struct vc_attr * attr)
{
	return vc_common_xmit(attr);
}

static struct vc_ops * vc_pause_close(struct vc_attr * attr)
{
	vc_buf_clear(attr, ADV_CLR_RX|ADV_CLR_TX);
	vc_common_purge(attr, 0x0000000f);

	return vc_common_close(attr);
}

static struct vc_ops * vc_pause_open(struct vc_attr * attr)
{
	return vc_common_open(attr);
}

static struct vc_ops * vc_pause_error(struct vc_attr * attr, char * str, int num)
{
	struct stk_vc * stk;

	stk = &attr->stk;
	printf("%s: %s(%d)\n", __func__, str, num);
	stk_excp(stk);

	return stk_curnt(stk)->init(attr);
}



static struct vc_ops * vc_pause_init(struct vc_attr * attr)
{
	vc_buf_update(attr, VC_BUF_RX);
	vc_buf_update(attr, VC_BUF_TX);

	return ADV_THIS;
}

static struct vc_ops * vc_pause_ioctl(struct vc_attr * attr)
{
	return vc_common_ioctl(attr);
}

static struct vc_ops * vc_pause_recv(struct vc_attr * attr, char *buf, int len)
{
	return vc_common_recv(attr, buf, len);
}

static struct vc_ops * vc_pause_event(struct vc_attr * attr, 
	struct timeval * tv, fd_set * rfds, fd_set * wfds, fd_set * efds)
{
	struct stk_vc * stk;
	int buflen;

	stk = &attr->stk;
	if(attr->sk < 0){
		printf("shouldn't go here\n");
	}

	if(ioctl(attr->sk, SIOCINQ, &buflen)){
		printf("resume: failed to get SIOCINQ\n");
		stk_excp(stk);
		return stk_curnt(stk)->init(attr);
	}
	if(buflen){
		return ADV_THIS;
	}

	if(attr->xmit_pending == 0){
		FD_SET(attr->fd, rfds);
	}
	
	FD_SET(attr->fd, wfds);
	return ADV_THIS;
}

static int _resume_queue(struct vc_attr * attr)
{
	char pbuf[1024];
	int plen;
	struct vc_proto_packet * packet;
	
	packet = (struct vc_proto_packet *)pbuf;

	plen = vc_pack_qfsize(packet, attr->tid, 1024, sizeof(pbuf));

	if(plen <= 0){
		printf("failed to create WAIT_ON_MASK\n");
		return -1;
	}

	if(fdcheck(attr->sk, FD_WR_RDY, 0) == 0){
		printf("cannot send WAIT_ON_MASK\n");
		return -1;
	}

	if(send(attr->sk, packet, plen, MSG_NOSIGNAL) != plen){
		printf("failed to  send WAIT_ON_MASK\n");
		return -1;
	}
	attr->tid++;

	return 0;
}


static struct vc_ops * vc_pause_resume(struct vc_attr * attr)
{
	struct stk_vc * stk;
	int buflen;

	stk = &attr->stk;
	if(ioctl(attr->sk, SIOCOUTQ, &buflen)){
		printf("resume: failed to get SIOCOUTQ\n");
		stk_excp(stk);
	
		return stk_curnt(stk)->init(attr);
	}
	if(buflen){
		return ADV_THIS;
	}
	
	if(_resume_queue(attr)){
		printf("%s(%d)\n", __func__, __LINE__);
		stk_excp(stk);
		return stk_curnt(stk)->init(attr);
	}
	stk_rpls(stk, &vc_netup_ops);

	return stk_curnt(stk)->init(attr);
}


static struct vc_ops * vc_pause_poll(struct vc_attr * attr)
{
	struct stk_vc * stk;
	
	stk = &attr->stk;
	stk_push(stk, &vc_idle_ops);
	return stk_curnt(stk)->init(attr);
}

char * vc_pause_name(void)
{
	return "Pause";
}
#undef ADV_THIS

struct vc_ops vc_pause_ops = {
	.open = vc_pause_open,
	.close = vc_pause_close,
	.xmit = vc_pause_xmit,
	.err = vc_pause_error,
	.init = vc_pause_init,
	.recv= vc_pause_recv,
	.ioctl = vc_pause_ioctl,
	.poll = vc_pause_poll,
	.event = vc_pause_event,
	.resume = vc_pause_resume,
	.name = vc_pause_name,
};
