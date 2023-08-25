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
#include "cmdFtp.h"
#include "ftp.h"

int main(void)
{
    
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    
    int sfd,        // socket
        s,          // ret getaddrinfo 
        type,       // store command
        endFtp,     // exit or ciao command
        debug,      // debugon or debugoff
        connected;  // connected or not

    char    buf[BUF_SIZE],      // buf receive server
            bufC[BUF_SIZE],     // buf arg
            trash[BUF_SIZE],
            host[BUF_SIZE],     // arg command
            host2[BUF_SIZE],    // arg 2 command
            login[MAX_LOGIN],
            password[MAX_PASSWORD],
            port[MAX_PORT];
    struct termios term, term_orig; // hide password
    
    /* initialization */
    endFtp      = 1;
    connected   = 0;
    debug       = 0;
    memset(host,'\0',BUF_SIZE);
    memset(port,'\0',20);
    
    /* Obtain address(es) matching host/port */
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;           /* Allow IPv4 or IPv6  */
    hints.ai_socktype = SOCK_STREAM;     /* Datagram socket     */
    hints.ai_flags = AI_PASSIVE;
    hints.ai_protocol = 0;               /* Any protocol        */

    printf("ftp> ");
    

    /* while we don't have a EXIT or CIAO command */
    while(endFtp)
    {   
        /* reset string */
        memset(host, '\0', BUF_SIZE);
        memset(host2, '\0', BUF_SIZE);
        memset(trash, '\0', BUF_SIZE);
        if(fgets(bufC, BUF_SIZE, stdin) == NULL)
        {
            printf("ftp> ");
            continue;
        }
        if(bufC[0] == '\n')
        {
            printf("ftp> ");
            continue;            
        }
        /* command = file cmdFtp.c => verify if client writes correctly */
        type = command(bufC);

        if(type == CMD_OPEN)
        {        
            sscanf(bufC, "%s %s %s\n",trash,host,port);

            s = getaddrinfo(host,port,&hints, &result);
            
            if (s != 0) 
            {
                fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
                exit(EXIT_FAILURE);
            }

            /* 
             * getaddrinfo() returns a list of address structures.
             * Try each address until we successfully connect(2).
             * If socket(2) (or connect(2)) fails, we (close the socket
             * and) try the next address. 
            */
            for (rp = result; rp != NULL; rp = rp->ai_next) 
            {
                sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
                if (sfd == -1) 
                    continue;

                if (connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1)
                   break;                  /* Success */

               close(sfd);
            }

            if (rp == NULL) 
            {               
                /* No address succeeded */
               fprintf(stderr, "Could not connect\n");
               printf("ftp> ");
               continue;
            }
            else
            {
                receiveFServ(sfd, buf);
                /* authentification */
                /* login */
                printf("Name (localhost:anonymous): ");
                fflush(stdout);
                scanf("%s", login);
                if (strncmp(login, "anonymous",9) == 0)
                {
                    printf("Any password, however type at least a letter or a number before <enter>\n");
                    fflush(stdout);
                }
                sprintf(buf, "USER %s\r\n", login);
                if (sendto(sfd, buf, strlen(buf),0,
                (struct sockaddr *)rp->ai_addr,rp->ai_addrlen) == -1) 
                {
                    perror("sendto");
                    exit(EXIT_FAILURE);
                }
                if (debug == 1)
                {
                    printf("---> USER %s\n", login);
                }
	            memset(buf, '\0', BUF_SIZE);
                receiveFServ(sfd, buf);

                /* end login */

                /* password */
                printf("Password: ");
                fflush(stdout);

                /* hide password */
                tcgetattr(STDIN_FILENO, &term);
                term_orig = term;
                term.c_lflag &= ~ECHO;
                tcsetattr(STDIN_FILENO, TCSANOW, &term);

                scanf("%s", password);

                tcsetattr(STDIN_FILENO, TCSANOW, &term_orig);
                /* hide password */
                sprintf(buf, "PASS %s\n", password);
                if (sendto(sfd, buf, strlen(buf),0, (struct sockaddr *)rp->ai_addr,rp->ai_addrlen) == -1) 
                {
                    perror("sendto");
                    exit(EXIT_FAILURE);
                }
                printf("\n");
                if (debug == 1)
                {
                    printf("---> PASS XXXX\n");
                }
                memset(buf, '\0', BUF_SIZE);
                receiveFServ(sfd, buf);

                /* end password */
                /* end authentification = client is connected */
                connected = 1;
                freeaddrinfo(result);           /* No longer needed */
                fgets(bufC, 1024, stdin); // get \n
            }
        } /* end if command open */
        else
        {
            /* if open has not work, client is not connected - can't use all command */
            if(!connected)
            {
                if (type == CMD_EXIT)
                {
                    endFtp = 0 ;
                }
                else if (type == CMD_DEBUGON)
                {
                    debug = 1;
                    printf("Debugging on (debug=1).\n");
                }
                else if (type == CMD_DEBUGOFF)
                {
                    debug = 0;
                    printf("Debugging on (debug=0).\n");
                }
                else
                    printf("You're not Connected.\n");
            }
            else
            { 
                /* if open has work, client is connected, he can use different command */
                switch(type)
                {
                    case CMD_EXIT :
                    case CMD_CIAO :
                        endFtp = 0 ;
                        send(sfd,"QUIT\r\n",6,0);
                        if (debug == 1)
                        {
                            printf("---> QUIT\n");
                        }
                        receiveFServ(sfd, buf);
                        break;
                    case CMD_DIR:
                        cmd_dir(sfd,debug,buf);
                        break;
                    case CMD_SHOW:
                        sscanf(bufC, "%s %s\n",trash,host);
                        cmd_show(sfd,host,debug,buf);
                        break;
                    case CMD_DEBUGON:
                        debug = 1;
                        printf("Debugging on (debug=1).\n");
                        break;
                    case CMD_DEBUGOFF:
                        debug = 0;
                        printf("Debugging on (debug=0).\n");
                        break;
                    case CMD_GET:
                        sscanf(bufC, "%s %s\n", trash, host);
                        cmd_get(sfd, buf, host, debug);
                        break;
                    case CMD_SEND:
                        sscanf(bufC, "%s %s\n", trash, host);
                        cmd_send(sfd, buf, host, debug);
                        break;
                    case CMD_REN:
                        sscanf(bufC, "%s %s %s\n", trash, host, host2);
                        cmd_ren(sfd, host, host2, buf, debug);
                        break;
                    case CMD_DEL:
                        sscanf(bufC, "%s %s\n", trash, host);
                        cmd_del(sfd, host, buf, debug);
                        break;
                    case CMD_CD:
                        sscanf(bufC, "%s %s\n", trash, host);
                        cmd_cd(sfd, host, buf, debug);
                        break;
                    case CMD_MKD:
                        sscanf(bufC, "%s %s\n", trash, host);
                        cmd_mkd(sfd, host, buf, debug);
                        break;
                    case CMD_RMD:
                        sscanf(bufC, "%s %s\n", trash, host);
                        cmd_rmd(sfd, host, buf, debug);
                        break;
                    default : 
                        break;
                } /* switch */
            } /* not connected */
        } /* end else command open */
        if (endFtp)
        { 
            printf("ftp> ");
        }
    } /* end while */
    return 0;
}