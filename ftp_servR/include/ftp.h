#ifndef FTP_H
#define FTP_H

#include "cmdFtp.h"

#define MAX_RECEIVE     256
#define BUF_SIZE        1024
#define MAX_LOGIN       64
#define MAX_PORT        20
#define MAX_PASSWORD    64

int ftp_dataSock(int sfd, int debug, char * buf);
void receiveData(int dfd, char *buf);
void receiveFServ(int sockfd,char * buf);
int cmd_show(int sfd, char * filename, int debug,char * buf);
int cmd_dir(int sfd, int debug,char * buf);
int cmd_cd(int sockfd, char * directory, char * buf, int debug);
int cmd_get(int sockfd, char * buf, char * filename,  int debug);
int cmd_del(int sockfd, char * filename, char * buf, int debug);
int cmd_rmd(int sockfd, char * filename, char * buf, int debug);
int cmd_mkd(int sockfd, char * filename, char * buf, int debug);
int cmd_ren(int sockfd, char * filename, char * filename2, char * buf, int debug);
void writeData(int fd, FILE * ffd, char * buf);
int cmd_send(int sockfd, char * buf, char * filename,  int debug );

#endif