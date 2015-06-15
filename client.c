#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#define BUF_SIZE	700

int main(int argc, char *argv[]) {

	int sockid, retcode, nread, addrlen, i, j, fileSize, sendSize, recvSize, readSize, fileCount=0, percent;
	struct hostent *hostp;
	struct sockaddr_in my_addr, server_addr;
	//int sock;
	char msg[BUF_SIZE], buf[BUF_SIZE], recvFile[BUF_SIZE];
	//int bytesread;
	char command[256], *ip, *port, *fileName, *tok;
	FILE *file;

	if(argc != 3) {
		(void)fprintf(stderr,"usage:%s host port\n",argv[0]);
		exit(1);
	}


	while(strcmp(command, "quit")) {
            printf(">");
            fgets(command, sizeof(command), stdin);
            for(i=0; i<strlen(command); i++)
                  if(command[i] == '\n')
                        command[i] = '\0';
            
            if(!strcmp(command, "quit"))
                  continue;
            if(!strcmp(command, "put") || !strcmp(command, "get")) {
                  printf("Connect first\n");
                  continue;
            }
            if(!strcmp(command, "close")) {
                  printf("Socket is not connected\n");
                  continue;
            }
      
            tok=strtok(command, " ");
            strcpy(command, tok);
            if(!strcmp(command, "connect")) {
                  ip = strtok(NULL, " ");
                  port = strtok(NULL, " ");
            }
            else {
                  printf("Command error\n");
                  printf("---------------------------\n");
                  printf("connect [ip] [port]\nquit\n");
                  printf("---------------------------\n");
                  continue;
            }
            
            if((sockid = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
                  printf("Client: socket failed: %d\n",errno);
                  exit(0);
            }

            memset((char *) &my_addr, 0, sizeof(my_addr));
            my_addr.sin_family = AF_INET;
            my_addr.sin_addr.s_addr = INADDR_ANY;
            my_addr.sin_port = htons(0);

            if((bind(sockid, (struct sockaddr *) &my_addr, sizeof(my_addr)) < 0)) {
                  printf("Client: bind fail: %d\n",errno);
                  exit(1);
            }

            if ((hostp = gethostbyname(ip)) == 0) {
                  fprintf(stderr,"%s: unknown host\n", ip);
                  exit(1);
            }

            bzero((char *) &server_addr, sizeof(server_addr));
            server_addr.sin_family = AF_INET;
            server_addr.sin_addr.s_addr = inet_addr(ip);
            memcpy((void *) &server_addr.sin_addr, hostp->h_addr, hostp->h_length);
            server_addr.sin_port = htons((u_short)atoi(port));
      
            printf("Successfully connected.\n");
            while(strcmp(command, "close")) {
                  printf(">");
                  fgets(command, sizeof(command), stdin);
                  for(i=0; i<strlen(command); i++)
                        if(command[i] == '\n')
                              command[i] = '\0';
            
                  if(!strcmp(command, "close")) {
                        send(sockid, command, 10, 0);
                        close(sockid);
                        printf("Disconnected.\n");
                        continue;
                  }
                  if(!strcmp(command, "list")) {
                        system("ls");
                        continue;
                  }
                  tok = strtok(command, " ");
                  strcpy(command, tok);
                  if(!strcmp(command, "put") || !strcmp(command, "get"))
                        fileName = strtok(NULL, " ");
                  else {
                        printf("Command Error\n");
                        printf("---------------------------\n");
                        printf("put [file]\nget [file]\nlist\nclose\n");
                        printf("---------------------------\n");
                        continue;
                  }
                  sendto(sockid, command, 10, 0, (struct sockaddr *) &server_addr, sizeof(server_addr));
                  if (retcode <= -1) {
                        printf("client: sendto failed: %d\n",errno);
                        exit(1);
                  }
                  if(!strcmp(command, "put")) {
                        file = fopen(fileName, "rb");
                        if(!file) {
                              printf("No exist file in client\n");
                              continue;
                        }
                        fseek(file, 0L, SEEK_END);
                        fileSize = ftell(file);                     
                        memset(msg, 0, BUF_SIZE);
                        sprintf(msg, "%d", fileSize);
                        send(sockid, msg, strlen(msg), 0);
                        memset(msg, 0, BUF_SIZE);
                        sprintf(msg, "%s", fileName);
                        send(sockid, msg, strlen(msg), 0);
                        sendSize = 0;
                        fseek(file, 0L, SEEK_SET);
                        printf("[%s] (size: %d Bytes) is being sent\n", fileName, fileSize);
                        while(1) {
                              memset(msg, 0, BUF_SIZE);
                              nread = fread((void *)msg, 1, BUF_SIZE, file);
                              sendSize += nread;
                              send(sockid, msg, nread, 0);
                              memset(msg, 0, BUF_SIZE);
                              recv(sockid, msg, BUF_SIZE, 0);
                              
                              percent=((float)sendSize / (float)fileSize) * 100;
                              printf("[");
                              for(j=0; j<percent/10; j++)
                                    printf("*");
                              printf("]");
                              if(percent != 100) printf("\r");
                              fflush(stdout);
                              
                              if(sendSize == fileSize)
                                    break;
                        }
                        fclose(file);
                        printf("\nSuccessfully transferred\n");
                  }
                  if(!strcmp(command, "get")) {
                        send(sockid, fileName, 30, 0);
                        memset(recvFile, 0, BUF_SIZE);
                        strcpy(recvFile, fileName);
                        memset(msg, 0, BUF_SIZE);
                        recv(sockid, msg, BUF_SIZE, 0);           //yes or no
                        if(!strcmp(msg, "No")) {
                              printf("No exist file in server\n");
                              continue;
                        }
                        memset(msg, 0, BUF_SIZE);
                        recv(sockid, msg, BUF_SIZE, 0);           //file size
                        fileSize = atoi(msg);
                        recvSize = 0;
                        memset(fileName, 0, BUF_SIZE);
                        sprintf(fileName, "ClientGetFile%d", fileCount++);
                        file = fopen(fileName, "wb");
                        printf("[%s] (size: %d Bytes) is being received\n", recvFile, fileSize);
                        while(1) {                      
                              memset(msg, 0, BUF_SIZE);
                              nread = recv(sockid, msg, BUF_SIZE, 0);
                              recvSize += nread;
                              fwrite((void *)msg, 1, nread, file);
                              send(sockid, "Good", 4, 0);
                              
                              percent=((float)recvSize / (float)fileSize) * 100;
                              printf("[");
                              for(j=0; j<percent/10; j++)
                                    printf("*");
                              printf("]");
                              if(percent != 100) printf("\r");
                              fflush(stdout);
                              
                              if(recvSize == fileSize)
                                    break;
                        }
                        fclose(file);
                        printf("\nSuccessfully transferred\n");
                  }
            }
      }

}
