#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>

// CLIENT APP

#define MAX 800
#define SA struct sockaddr

#define EXT_SERV_ADDR "192.168.0.10"

int main()
{
	int sockfd, connfd;
	struct sockaddr_in servaddr, cli;
    struct timeval tv;
    int port;
    tv.tv_sec=10;
    tv.tv_usec=0;

    char srvaddr[INET_ADDRSTRLEN] = {0};
	// socket create and verification
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd == -1) {
		printf("socket creation failed...\n");
		exit(0);
	}
	else
		printf("Socket successfully created\n");
    // Set timeout
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
	bzero(&servaddr, sizeof(servaddr));

    printf("Enter target port: ");
    scanf("%d", &port);
    while(getchar() != '\n');
	// assign IP, PORT
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr(EXT_SERV_ADDR);
	servaddr.sin_port = htons(port);
	inet_ntop(AF_INET, &(servaddr.sin_addr), srvaddr, INET_ADDRSTRLEN);
	printf("Connecting to server with address %s:%u \n",srvaddr,ntohs(servaddr.sin_port));

	// function for chat
    char buff[MAX];
    int n;
    int retVal;
    int rxlen;
    int txlen;
    socklen_t servAddrLen;
    while(1){
        bzero(buff, sizeof(buff));
        printf("Enter the string : ");
        n = 0;
        while ((buff[n++] = getchar()) != '\n');
        buff[n-1]='\0';
        printf("TX string: %s\n",buff);
        txlen=strlen(buff);
        retVal=sendto(sockfd, buff, txlen, 0,(SA*) &servaddr, sizeof(servaddr));
        bzero(buff, sizeof(buff));
        rxlen=recvfrom(sockfd, buff, sizeof(buff), 0,(SA*) &servaddr, &servAddrLen);
        printf("RX string: %s\n",buff);

    }

	// close the socket
	close(sockfd);
    return(retVal);
}
