#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#define BUF_SIZE	700

int select(int numdes, fd_set *readmask, fd_set *writemask, fd_set *exceptmask, struct timeval *timeout);

int main(int argc, char *argv[]) {
	
	//request_sock->sockid, new_sock->nread
	int sockid, nread, retcode, addrlen, i, j, fileSize, sendSize, recvSize, fileCount=0, percent;
	struct sockaddr_in server, remote;
	//int new_sock;
	char msg[BUFSIZ], fileName[BUFSIZ], recvFileName[BUFSIZ];
	FILE *file;
	time_t initTime, curTime;
	
	if(argc != 2) {
		(void)fprintf(stderr,"usage:%s port\n",argv[0]);
		exit(1);
	}
	
	if((sockid = socket(PF_INET,SOCK_STREAM,IPPROTO_TCP))<0) {
		perror("socket");
		exit(1);
	}

	memset((void*)&server,0,sizeof(server));
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons((u_short)atoi(argv[1]));

	if(bind(sockid,(struct sockaddr *)&server,sizeof(server))<0) {
		perror("bind");
		exit(1);
	}

	if(listen(sockid, SOMAXCONN)<0) {
		perror("listen");
		exit(1);
	}

	//=====================================================================
	addrlen = sizeof(remote);
	nread = recv(sockid, msg, 12, 0);

	while(strcmp(msg, "close")) {
		if(!strcmp(msg, "put")) {
			memset(msg, 0, BUFSIZ);
			nread = recv(sockid, msg, BUFSIZ, 0);
			fileSize = atoi(msg);
			memset(msg, 0, BUFSIZ);
			nread = recv(sockid, msg, BUFSIZ, 0);
			strcpy(recvFileName, msg);
			memset(fileName, 0, BUFSIZ);
          sprintf(fileName, "ClientPutFile%d", fileCount++);
			file = fopen(fileName, "wb");
			recvSize = 0;
			initTime = time(NULL);
			printf("Transfer status:recv [%s] [0%% , 0 Bytes / %d Bytes]\n", recvFileName, fileSize);

			while(1) {
				memset(msg, 0, BUFSIZ);
				nread = recv(sockid, msg, BUFSIZ, 0);
				recvSize += nread;
				fwrite((void*)msg, 1, nread, file);
				send(sockid, "Good", 4, 0);
				curTime = time(NULL);
				if(curTime - initTime == 1 && recvSize != fileSize) {
					initTime = curTime;
					percent = ((float)recvSize / (float)fileSize) * 100;
                    printf("Transfer status:recv [%s] [%d%% , %d Bytes / %d Bytes]\n", recvFileName, percent, recvSize, fileSize);
                }
				if(recvSize == fileSize)
					break;
			}
			fclose(file);
			printf("Transfer status:recv [%s] [100%% , %d Bytes / %d Bytes]\n", recvFileName, recvSize, fileSize);
            printf("Successfully transferred\n");
		}
		if(!strcmp(msg, "get")) {
			memset(fileName, 0, BUFSIZ);
			recv(sockid, fileName, BUFSIZ, 0);
			file = fopen(fileName, "rb");
			if(!file) {
				send(sockid, "No", 2, 0);
				continue;
			}
			send(sockid, "Yes", 3, 0);
			fseek(file, 0L, SEEK_END);
			fileSize = ftell(file);
			memset(msg, 0, BUFSIZ);
			sprintf(msg, "%d", fileSize);
			send(sockid, msg, strlen(msg), 0);
			sendSize = 0;
			fseek(file, 0L, SEEK_SET);
			initTime=time(NULL);
            printf("Transfer status:send [%s] [0%% , 0 Bytes / %d Bytes]\n", fileName, fileSize);
            while(1) {
                memset(msg, 0, BUFSIZ);
                nread = fread((void *)msg, 1, BUFSIZ, file);
                sendSize += nread;
                send(sockid, msg, nread, 0);
                memset(msg, 0, BUFSIZ);
                recv(sockid, msg, BUFSIZ, 0);
                curTime = time(NULL);
                if(curTime - initTime == 1 && sendSize != fileSize) {
                    initTime = curTime;
                    percent = ((float)sendSize / (float)fileSize) * 100;
                    printf("Transfer status:send [%s] [%d%% , %d Bytes / %d Bytes]\n", fileName, percent, sendSize, fileSize);
                }
                if(sendSize == fileSize)
                    break;
            }
            fclose(file);
            printf("Transfer status:send [%s] [100%% , %d Bytes / %d Bytes]\n", fileName, sendSize, fileSize);
            printf("Successfully transferred\n");
        }
		recv(sockid, msg, BUFSIZ, 0);
	}
	close(sockid);
	return 0;
	
	/*for(;;) {
		addrlen = sizeof(remote);
		new_sock = accept(request_sock,(struct sockaddr*)&remote,&addrlen);
		
		if(new_sock < 0) {
			perror("accept");
			exit(1);
		}
		
		printf("I'm waiting a client.\n");
		printf("New connection from host %s, port %d, socket %d\n",
			inet_ntoa(remote.sin_addr),ntohs(remote.sin_port),
			new_sock);

		for(;;) {
			bytesread = read(new_sock,buf,sizeof(buf)-1);
			if(bytesread <= 0) {
				printf("Got Q from the client, Close the connection.\n");
				//printf("server:end of file on %d\n",new_sock);
				if(close(new_sock))
					perror("close");
				break;
			}
			buf[bytesread-1]='\0';
			printf("Got %s from the client.\n",buf);

			result = atoi(buf);
			sum += result;
			sprintf(buf,"%d\n",sum);
			
			//for(i=0; i<bytesread; i++)
				//buf[i] = toupper(buf[i]);
			
			//echo it back
			if(write(new_sock,buf,bytesread)!=bytesread)
				perror("echo");
		}
	}*/
}
