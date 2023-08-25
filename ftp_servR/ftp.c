#include "ftp.h"

int cmd_ren(int sockfd, char * filename, char * filename2, char * buf, int debug)
{
	sprintf(buf, "RNFR %s\r\n", filename);
	if (debug == 1)
	{
		printf("---> %s\n", buf);
	}
	send(sockfd,buf,strlen(buf),0);
	receiveFServ(sockfd,buf);
	sprintf(buf, "RNTO %s\r\n", filename2);
	if (debug == 1)
	{
		printf("---> %s\n", buf);
	}
	send(sockfd,buf,strlen(buf),0);
	receiveFServ(sockfd,buf);
	return 0;
}

int cmd_mkd(int sockfd, char * filename, char * buf, int debug)
{
	sprintf(buf, "MKD %s\r\n", filename);
	if (debug == 1)
	{
		printf("---> %s\n", buf);
	}
	send(sockfd,buf,strlen(buf),0);
	receiveFServ(sockfd,buf);
	return 0;
}


int cmd_rmd(int sockfd, char * filename, char * buf, int debug)
{
	sprintf(buf, "RMD %s\r\n", filename);
	if (debug == 1)
	{
		printf("---> %s\n", buf);
	}
	send(sockfd,buf,strlen(buf),0);
	receiveFServ(sockfd,buf);
	return 0;
}

int cmd_del(int sockfd, char * filename, char * buf, int debug)
{
	sprintf(buf, "DELE %s\r\n", filename);
	if (debug == 1)
	{
		printf("---> %s\n", buf);
	}
	send(sockfd,buf,strlen(buf),0);
	receiveFServ(sockfd,buf);
	return 0;
}

int ftp_dataSock(int sfd, int debug, char * buf)
{
	int fd; 
	if((fd = socket(AF_INET,SOCK_STREAM,0)) == -1) 
	{
		perror("socket");
		return -1;
	}

	struct sockaddr_in addr;
	socklen_t addr_size = sizeof(struct sockaddr_in);
	getsockname(sfd, (struct sockaddr*)&addr, &addr_size);
	addr.sin_port = ntohs(0);



	if (bind(fd,(const struct sockaddr *)&addr,addr_size)) //get a port for data
	{
		perror("bind");
		close(fd);
		return 1;
	}
	listen(fd, 5);

	getsockname(fd, (struct sockaddr*)&addr, &addr_size);
 	memset(buf, '\0', BUF_SIZE);
	sprintf(buf, "PORT %d,%d,%d,%d,%d,%d\r\n",
				addr.sin_addr.s_addr&0xFF,
				(addr.sin_addr.s_addr&0xFF00)>>8,
				(addr.sin_addr.s_addr&0xFF0000)>>16,
				(addr.sin_addr.s_addr&0xFF000000)>>24,
				 ntohs(addr.sin_port)/256,
				 ntohs(addr.sin_port)%256);
	send(sfd,buf,strlen(buf),0);
	if(debug)
		printf("---> %s",buf);
	receiveFServ(sfd,buf);
	fflush(stdout);
	return fd;

}

void receiveData(int dfd, char *buf)
{
	int count, fd;
	struct sockaddr_in addr;
	socklen_t addr_size = sizeof(struct sockaddr_in);
	if((fd = accept(dfd,(struct sockaddr *)&addr,&addr_size))== -1)
    {
        perror("accept");
        close(dfd);
        return;
    }
	do
	{
		memset(buf, '\0', BUF_SIZE);
		count = recv(fd, buf, BUF_SIZE, 0);
		printf("%s",buf);
	} while(count == BUF_SIZE);	
}

void writeData(int fd, FILE * ffd, char * buf)
{
	int count;
	struct sockaddr_in addr;
	socklen_t addr_size = sizeof(struct sockaddr_in);
	if((fd = accept(fd,(struct sockaddr *)&addr,&addr_size))== -1)
    {
        perror("accept");
        close(fd);
        return;
    }
	do
	{
		memset(buf, '\0', BUF_SIZE);
		count = recv(fd, buf, BUF_SIZE, 0);

		fprintf(ffd,"%s", buf);
	} while(count == BUF_SIZE);	
}

void writeSocket(int fd, FILE * ffd, char * buf)
{
	int count, dfd;
	struct sockaddr_in addr;
	socklen_t addr_size = sizeof(struct sockaddr_in);
	if((dfd = accept(fd,(struct sockaddr *)&addr,&addr_size))== -1)
    {
        perror("accept");
        close(dfd);
        return;
    }
	do
	{
		memset(buf, '\0', BUF_SIZE);
		count = fread(buf, 1, BUF_SIZE, ffd);
		send(dfd, buf, strlen(buf),0);
		fprintf(ffd,"%s", buf);
	} while(count == BUF_SIZE);
	close(dfd);
}

void receiveFServ(int sfd, char * buf) 
{
	int count;
	do
	{
		memset(buf, '\0', BUF_SIZE);
		count = recv(sfd, buf, BUF_SIZE, 0);
		int end = 1;
		char *s = buf;
		while(*s != '\0')
		{
			if(*s == '\r')
			{
				s++;
				if(*s=='\n')
					end = 0;
			}
			if(end)
				printf("%c", *s);
			else
				break;
			s++;
		}
		if(!end)
		{
			printf("\n");
			break;
		}
	} while(count == BUF_SIZE);
}

/*
---> PORT 127,0,0,1,202,175
200 PORT command successful. Consider using PASV.
---> RETR constan.txt
150 Opening BINARY mode data connection for constan.txt (10 bytes).
226 Transfer complete.
*/
int cmd_show(int sfd, char * filename, int debug,char * buf)
{
	int dfd = ftp_dataSock(sfd,debug,buf);
	if(dfd == -1)
	{
		printf("ERROR\n");
		return 0;
	}
	sprintf(buf, "RETR %s\r\n",filename);
	if(debug)
		printf("---> %s",buf);
	send(sfd,buf,strlen(buf),0);
	receiveFServ(sfd,buf);
	receiveData(dfd,buf);
	receiveFServ(sfd,buf);
	close(dfd);    
	return 0;
}


/*
 * 200 PORT command successful. Consider using PASV.
 * ---> LIST
 * 150 Here comes the directory listing.
 * 226 Directory send OK.
*/
int cmd_dir(int sfd, int debug,char * buf)
{
	int dfd = ftp_dataSock(sfd,debug,buf);
	if(dfd == -1)
	{
		printf("ERROR\n");
		return 0;
	}
	sprintf(buf, "LIST\r\n");
	if(debug)
		printf("---> %s",buf);
	send(sfd,buf,strlen(buf),0);
	receiveFServ(sfd,buf);
	receiveData(dfd,buf);
	receiveFServ(sfd,buf);
	close(dfd);
	return 0;
}

/*
 * ---> CWD Documents
 * 250 Directory successfully changed.
*/
int cmd_cd(int sockfd, char * directory, char * buf, int debug)
{
	sprintf(buf, "CWD %s\r\n", directory);
	if (debug == 1)
	{
		printf("---> %s\n", buf);
	}
	send(sockfd,buf,strlen(buf),0);
	receiveFServ(sockfd,buf);
	return 0;
}


/* 
 * local: <file> remote: <file>
 * ---> TYPE I
 * 200 Switching to Binary mode.
 * ---> PORT 127,0,0,1,223,33
 * 200 PORT command successful. Consider using PASV.
 * ---> RETR <file>
 * 150 Opening BINARY mode data connection for <file> (<byte> bytes).
 * 226 Transfer complete.
 * <byte> bytes received in 0.01 secs (95.3139 kB/s)
*/
int cmd_get(int sockfd, char * buf, char * filename,  int debug)
{
	FILE * fd;
	int dfd;

	dfd = ftp_dataSock(sockfd, debug, buf);
	if(dfd == -1)
	{
		printf("ERROR\n");
		return 0;
	}
	sprintf(buf, "RETR %s\r\n", filename);
	if (debug == 1)
	{
		printf("---> %s\n", buf);
	}
	/* creer fichier du meme nom sur le rep actuel */
	fd = fopen(filename, "w+");
	send(sockfd,buf,strlen(buf),0);
	receiveFServ(sockfd,buf);
	writeData(dfd, fd, buf); 
	receiveFServ(sockfd,buf);
	close(dfd);
	fclose(fd);
	return 0;
}

/*
ftp> send blal
local: blal remote: blal
ftp: setsockopt (ignored): Permission denied
---> PORT 127,0,0,1,204,209
200 PORT command successful. Consider using PASV.
---> STOR blal
150 Ok to send data.
226 Transfer complete.
16 bytes sent in 0.00 secs (143.3486 kB/s)
*/
int cmd_send(int sockfd, char * buf, char * filename,  int debug )
{
	int dfd;
	FILE * fd;
	
	dfd = ftp_dataSock(sockfd, debug, buf);
	if(dfd == -1)
	{
		printf("ERROR\n");
		return 0;
	}
	sprintf(buf, "STOR %s\r\n", filename);
	if (debug == 1)
	{
		printf("---> %s\n", buf);
	}
	fd = fopen(filename, "r");
	send(sockfd,buf,strlen(buf),0);
	receiveFServ(sockfd,buf);
	writeSocket(dfd, fd, buf);
	fclose(fd);
	shutdown(dfd, 2);
	close(dfd); 
	receiveFServ(sockfd,buf);
	
	
	return 0;
}