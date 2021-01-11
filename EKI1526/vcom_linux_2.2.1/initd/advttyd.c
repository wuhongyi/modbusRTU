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

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <stdarg.h> 
#include <ctype.h>
#include <dirent.h>
#include <syslog.h>


#ifdef	STREAM
#include <sys/ptms.h>
#endif

#include "advttyd.h"

#define MON_PATH "/tmp/advmon"

typedef struct _TTYINFO {
	pid_t   advttyp_pid;                        // process id for the port	                 
	char	mpt_nameidx_str[CF_MAXSTRLEN];      // Master pseudo TTY index
	char    dev_type_str[CF_MAXSTRLEN];         // Device Type
	char	dev_ipaddr_str[INET6_ADDRSTRLEN];       // Device IP address
	char    dev_portidx_str[CF_MAXSTRLEN];      // Device Port Index
	char	dev_redundant_ipaddr_str[INET6_ADDRSTRLEN];
	int has_redundant_ip;
} TTYINFO;

int __log_fd = -1;
int _restart;

static int  parse_env(char * cmdpath, char * workpath);
//static int  daemon_init(void);
static int  paser_config(char * conf_name, TTYINFO ttyinfo[]);
static void spawn_ttyp(char * work_path, int nrport, TTYINFO ttyinfo[]);
//static void shutdown_ttyp(int nrport, TTYINFO ttyinfo[]);
//static void restart_handle();
//static u_long device_ipaddr(char * ipaddr);
static int  hexstr(char * strp);
//static int  log_open(char * log_name);
//static void log_close(void);
#ifdef ADVTTY_DEBUG
static void log_msg(const char * msg);
#endif

int __pid_search_fd(int pid, char * file)
{
	static DIR *dir = 0;
	struct dirent *entry;
	char *name;
	int n;
	int fd;
	char status[1204];
	char buf[1024];
	char fdpath[1024];
	struct stat sb;
	snprintf(fdpath, 1024, "/proc/%d/fd", pid);
	if (!dir) {
		dir = opendir(fdpath);
		if(!dir){
			syslog(LOG_DEBUG, "Can't open /proc/fd: %s", fdpath);
			return -1;
		}
	}
	for(;;) {
		if((entry = readdir(dir)) == NULL) {
			closedir(dir);
			dir = 0;
			return -1;
		}
		name = entry->d_name;
		if (!(*name >= '0' && *name <= '9'))
			continue;


		fd = atoi(name);

		sprintf(status, "/proc/%d/fd/%d", pid, fd);
		if(stat(status, &sb)){
			syslog(LOG_DEBUG, "stat failed");
			continue;
		}
		if(lstat(status, &sb) < 0){
			syslog(LOG_DEBUG, "lstat failed");
			continue;
		}

		n = readlink(status, buf, sizeof(buf));

		if(n <= 0){
			syslog(LOG_DEBUG, "readlink failed");
			closedir(dir);
			return -1;
		}

		if(n !=  strlen(file) ){
			//can't be the same file since the langth is different
			continue;
		}

		if(memcmp(buf, file, strlen(file)) == 0){
			closedir(dir);
			dir = 0;
			return pid;
		}
	}

}

int __cmd_search_file(char * cmd, char * file, char *retcmd, int len)
{
	static DIR *dir = 0;
	struct dirent *entry;
	char *name;
	int n;
	char status[32];
	char buf[1024];
	int retlen;
	FILE *fp;
	int pid;
	struct stat sb;

	if (!dir) {
		dir = opendir("/proc");
		if(!dir){
			syslog(LOG_DEBUG, "Can't open /proc");
			return -1;
		}
	}
	for(;;) {
		if((entry = readdir(dir)) == NULL) {
			syslog(LOG_DEBUG, "%s(%d)readdir failed", __func__, __LINE__);
			closedir(dir);
			dir = 0;
			return -1;
		}

		name = entry->d_name;
		if (!(*name >= '0' && *name <= '9'))
			continue;


		pid = atoi(name);

		sprintf(status, "/proc/%d", pid);
		if(stat(status, &sb))
			continue;
		sprintf(status, "/proc/%d/cmdline", pid);
		if((fp = fopen(status, "r")) == NULL){
			syslog(LOG_DEBUG, "%s(%d)fopen failed", __func__, __LINE__);
			continue;
		}

		if((n=fread(buf, 1, sizeof(buf)-1, fp)) > 0) {

			if(buf[n-1]=='\n'){
				buf[--n] = 0;
			}
			name = buf;
			while(n) {
				if(((unsigned char)*name) < ' ')
					*name = ' ';
				name++;
				n--;
			}
			*name = 0;
			/* if NULL it work true also */
		}
		fclose(fp);

		if(memcmp(buf, cmd, strlen(cmd)) == 0){
			if(__pid_search_fd(pid, file) > 0){

				if(strlen(buf) > len){
					retlen = len -1;
				}else{
					retlen = strlen(buf);
				}

				memcpy(retcmd, buf, retlen);
				retcmd[retlen] = '\0';

				closedir(dir);
				dir = 0;
				//printf("command %s pid %d\n", cmd, pid);
				return pid;
			}else{
				//printf("pid %dcmd %s\n", pid, cmd);
			}
		}
	}
}

void __close_stdfd(void)
{
	close(0);
	close(1);
	close(2);
}

int main(int argc, char * argv[])
{
	int nrport;
	TTYINFO ttyinfo[CF_MAXPORTS];
	char work_path[PATH_MAX];
	char file_name[PATH_MAX];

	nrport = 0;

	__close_stdfd();

	if(parse_env(argv[0], work_path) < 0)
		return -1;

	sprintf(file_name,"%s/%s", work_path, CF_CONFNAME);
	if((nrport = paser_config(file_name, ttyinfo)) <= 0) {
		syslog(LOG_DEBUG, "failed to paser config file");
		return 0;
	}
	ADV_LOGMSG("Advantech Virtual TTY daemon program - %s\n", CF_VERSION);
	spawn_ttyp(work_path, nrport, ttyinfo);

	return 0;
}

static int parse_env(char * cmdpath, char * workpath)
{
	int i;
	char currpath[PATH_MAX], tmpbuf[PATH_MAX];

	getcwd(currpath, sizeof(currpath));
	strcpy(tmpbuf, cmdpath);
	for(i = strlen(tmpbuf) - 1; i > 0; --i) {
		if(tmpbuf[i] == '/')
			break;
	}
	if(i) {
		tmpbuf[i] = 0;
		chdir(tmpbuf);
	}
	getcwd(workpath, PATH_MAX);
	chdir(currpath);
	return 0;
}
/*
static int daemon_init(void)   
{   
	pid_t   pid;  

	if(getppid() == 1)
		goto L_EXIT; 

#ifdef SIGTTOU
	signal(SIGTTOU, SIG_IGN);
#endif
#ifdef SIGTTIN
	signal(SIGTTIN, SIG_IGN);
#endif
#ifdef SIGTSTP
	signal(SIGTSTP, SIG_IGN);
#endif

	if((pid = fork()) < 0)
		return(-1);
	if(pid != 0)                // parent process
		exit(0);

	if(setpgrp() == -1) {
		return(-1);
	}
	signal(SIGHUP, SIG_IGN);	// immune from pgrp leader death
	setsid();                   // become session leader
	if((pid = fork()) < 0)
		return(-1);
	if(pid != 0)                // parent process
		exit(0);

L_EXIT:
	signal(SIGCLD, SIG_IGN);
	signal(SIGTERM, SIG_IGN);
	errno = 0;
	close(0);
	close(1);
	close(2);
	chdir("/");                 // change working directory
	umask(0);                   // clear   file   mode   creation
	return(0);   
}   
*/

static int paser_config(char * conf_name, TTYINFO ttyinfo[])
{
	int nrport;
	int int_tmp;
	FILE * conf_fp;
	char mpt_nameidx_str[CF_MAXSTRLEN];
	char dev_type_str[CF_MAXSTRLEN];
	char dev_portidx_str[CF_MAXSTRLEN];
	char dev_ipaddr_str[INET6_ADDRSTRLEN];
	char dev_redundant_ipaddr_str[INET6_ADDRSTRLEN];
	int matchCount=0;
	char conf_dp[256];

	nrport = 0;    
	if((conf_fp = fopen(conf_name, "r")) == NULL) {
		syslog(LOG_DEBUG, "Open the configuration file [%s] fail", conf_name);
		return nrport;
	}
	while(nrport < CF_MAXPORTS) {
		if(fgets(conf_dp, sizeof(conf_dp), conf_fp) == NULL)
			break;
		/*
		 * FORMATE
		 * Read configuration & the data format of every data line is :
		 * [Minor] [Device-Type] [Device-IP] [Port-Idx] [redundant-ip]
		 */
		matchCount = sscanf(conf_dp, "%s%s%s%s%s",
				mpt_nameidx_str, dev_type_str,
				dev_ipaddr_str, dev_portidx_str, dev_redundant_ipaddr_str);

		if ( matchCount < 4) {
			continue;
		}
		//printf("matchCount = %d\n", matchCount);
		if(atoi(mpt_nameidx_str) > CF_MAXPORTS)
			continue;
		if ((int_tmp = hexstr(dev_type_str)) <= 0 || int_tmp <= 0x1000)
			continue;
		/*
		   ulong_tmp = device_ipaddr(dev_ipaddr_str);
		   if ((ulong_tmp == (u_long)0xFFFFFFFF) || (ulong_tmp == 0))
		   continue;
		 */
		if ((int_tmp = atoi(dev_portidx_str)) <= 0 || int_tmp > 16)
			continue;
		memset(&ttyinfo[nrport], 0, sizeof(TTYINFO));
		strcpy(ttyinfo[nrport].mpt_nameidx_str, mpt_nameidx_str);
		strcpy(ttyinfo[nrport].dev_type_str, dev_type_str);
		strcpy(ttyinfo[nrport].dev_ipaddr_str, dev_ipaddr_str);
		strcpy(ttyinfo[nrport].dev_portidx_str, dev_portidx_str);
		if (matchCount > 4)
		{
			//ulong_tmp = device_ipaddr(dev_redundant_ipaddr_str);
			ADV_LOGMSG("redundant ip = %s\n", dev_redundant_ipaddr_str);
			//if ((ulong_tmp != (u_long)0xFFFFFFFF) && (ulong_tmp != 0))
			if(1)			
			{
				strcpy(ttyinfo[nrport].dev_redundant_ipaddr_str, dev_redundant_ipaddr_str);
				ADV_LOGMSG("redundant ip copied= %s\n", ttyinfo[nrport].dev_redundant_ipaddr_str);
				ttyinfo[nrport].has_redundant_ip = 1;
			}
			else
			{
				ttyinfo[nrport].has_redundant_ip = 0;
			}		
		}
		++nrport;
	}
	fclose(conf_fp);
	if(nrport == 0)
		ADV_LOGMSG("There is no configuration data for Advantech TTY\n");
	return nrport;
}

static void spawn_ttyp(char * work_path, int nrport, TTYINFO ttyinfo[])
{
	int idx;
	int oldpid;
	int cmdidx;
	char cmd[PATH_MAX];
	char log[PATH_MAX];
	char mon[PATH_MAX];
	char oldcmd[2048];
	char vcomif[1024];
	char syscmd[1024];
	char killcmd[256];


	sprintf(cmd, "%s/%s", work_path, CF_PORTPROG);
	sprintf(log, "%s/%s", work_path, CF_LOGNAME);


	for(idx = 0; idx < nrport; ++idx) {
		sprintf(mon, "%s/advtty%s", MON_PATH, ttyinfo[idx].mpt_nameidx_str);
		
		snprintf(vcomif, sizeof(vcomif), "/proc/vcom/advproc%s", ttyinfo[idx].mpt_nameidx_str);
		
		oldpid = __cmd_search_file(cmd, vcomif, oldcmd, sizeof(oldcmd));
		if(ttyinfo[idx].has_redundant_ip) {		
			ADV_LOGMSG("executing command %s -l %s -t %s -d %s -a %s -p %s -r %s \n", 
					cmd,
					log, 
					ttyinfo[idx].mpt_nameidx_str, 
					ttyinfo[idx].dev_type_str, 
					ttyinfo[idx].dev_ipaddr_str, 
					ttyinfo[idx].dev_portidx_str, 
					ttyinfo[idx].dev_redundant_ipaddr_str);
			cmdidx = snprintf(syscmd, sizeof(syscmd), 
					"%s -l%s -t%s -d%s -a%s -p%s -r%s ", 
						cmd,
						mon, 
						ttyinfo[idx].mpt_nameidx_str,
						ttyinfo[idx].dev_type_str,
						ttyinfo[idx].dev_ipaddr_str,
						ttyinfo[idx].dev_portidx_str,
						ttyinfo[idx].dev_redundant_ipaddr_str
						);

		}else{
			cmdidx = snprintf(syscmd, sizeof(syscmd), 
					"%s -l%s -t%s -d%s -a%s -p%s ", 
						cmd,
						mon, 
						ttyinfo[idx].mpt_nameidx_str,
						ttyinfo[idx].dev_type_str,
						ttyinfo[idx].dev_ipaddr_str,
						ttyinfo[idx].dev_portidx_str
						);

		}

		if(oldpid > 0){
			if(strcmp(syscmd, oldcmd) == 0){
				continue;
			}else{
				snprintf(killcmd, sizeof(killcmd), "kill -9 %d", oldpid);
				system(killcmd);
			}
		}

		snprintf(&syscmd[cmdidx], sizeof(syscmd) - cmdidx - 1, "&");
		system(syscmd);
	}

	return;
}
/*
static void shutdown_ttyp(int nrport, TTYINFO ttyinfo[])
{
	int idx;
	int wait_port;
	pid_t pid;

	for(idx = 0; idx < nrport; ++idx) {
		kill(ttyinfo[idx].advttyp_pid, SIGTERM);
	}

	for(wait_port = nrport; wait_port;) {
		for(idx = 0; idx < nrport; ++idx) {
			if(ttyinfo[idx].advttyp_pid == 0)
				continue;
			pid = waitpid(ttyinfo[idx].advttyp_pid, NULL, WNOHANG);
			if((pid == ttyinfo[idx].advttyp_pid) ||
					((pid < 0) && (errno == ECHILD))) {
				ttyinfo[idx].advttyp_pid = 0;
				--wait_port;
			}
		}
		if(wait_port)
			usleep(200 * 1000);
	}
	return;
}
*/
/*static void restart_handle()
{
	_restart = 1;
	return;
} */

static int hexstr(char * strp)
{
	int i, ch, val;
	for(i = val = 0; (ch = *(strp + i)) != '\0'; ++i) {
		if(ch >= '0' && ch <= '9') {
			val = 16 * val + ch - '0';
		}
		else if((ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F')) {
			ch = toupper(ch);
			val = 16 * val + ch - 'A' + 10;
		}
		else
			return 0;
	}
	return val;
}

/*static int log_open(char * log_name)
{
	return __log_fd = open(log_name,
			O_WRONLY | O_CREAT | O_APPEND | O_NDELAY, 0666);
}*/
/*
static void log_close(void)
{
	if(__log_fd >= 0) {
		close(__log_fd);
		__log_fd = -1;
	}
	return;
}*/
#ifdef ADVTTYD_DEBUG
static void log_msg(const char * msg)
{
	long t;
	struct tm * lt;
	char msg_buf[1024];

	if(__log_fd < 0)
		return;

	t = time(0);
	lt = localtime(&t);	
	sprintf(msg_buf, "%02d-%02d-%4d %02d:%02d:%02d  ",
			lt->tm_mon + 1, lt->tm_mday, lt->tm_year + 1900,
			lt->tm_hour, lt->tm_min, lt->tm_sec);
	strcat(msg_buf, msg);

	if(flock(__log_fd, LOCK_EX) < 0)
		return;
	write(__log_fd, msg_buf, strlen(msg_buf));
	flock(__log_fd,  LOCK_UN);
	return;
}
#endif
