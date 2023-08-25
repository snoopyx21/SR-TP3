#ifndef STRING_H
#define STRING_H

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <netdb.h>
#include <ctype.h>
#include <termios.h>

#define CMD_OPEN        1 // open
#define CMD_DIR         2 // dir
#define CMD_SHOW        3 // show
#define CMD_CIAO        4 // ciao
#define CMD_DEBUGON     5 // debugon
#define CMD_DEBUGOFF    6 // debugoff
#define CMD_GET         7 // get
#define CMD_SEND        8 // send 
#define CMD_REN         9 // ren
#define CMD_DEL         10 // del
#define CMD_CD          11 // cd
#define CMD_MKD         12 // mkd
#define CMD_RMD         13 // rmd
#define CMD_EXIT        14 // exit
#define NBCLE           14
#define ERROR           -1

typedef struct 
{ 
    char * tcommand; 
    int val; 
    int size;
} t_symcle;

int command(char * command_client);

#endif