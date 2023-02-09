#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>

// CLIENT APP

#define MAX 80
#define SA struct sockaddr

int main()
{
    struct sockaddr_in routaddr;
    struct sockaddr srcAddr;
    socklen_t srcAddrLen;
    char buff[MAX] = {0};
    int len;
    int sock_ext;
    // Non-blocking UDP socket
    sock_ext = socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK, 0);

#ifdef FORCE_BIND
    int sockopt=1;
    // set socket option to prevent bind errors
    setsockopt(sock_ext,SOL_SOCKET,SO_REUSEADDR,&sockopt,sizeof(int));
#endif

    if (sock_ext == -1) {
        printf("    External socket creation failed...\n");
        close(sock_ext);
        return -1;
    }else{
        printf("    External socket successfully created\n");
    }

    bzero(&routaddr, sizeof(routaddr));

    // assign IP, PORT
    routaddr.sin_family = AF_INET;
    routaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    routaddr.sin_port = htons(6001);

    // Binding newly created socket to given IP and verification
    if ((bind(sock_ext, (struct sockaddr*)&routaddr, sizeof(routaddr))) != 0) {
        printf("    External socket bind failed...\n");
        close(sock_ext);
        return -1;
    }else{
        #ifdef DEBUG_CONNECTION
        printf("    External socket bind to port 6001 successful\n");
        #endif
    }
    fcntl(sock_ext, F_SETFL, O_NONBLOCK);

    while(1){

        // read the message from client and copy it in buffer
        len = recvfrom(sock_ext,(void*) buff, sizeof(buff), 0, &srcAddr, &srcAddrLen);
        printf("len=%d\n",len);
        if(len<=0){
            // No data available
        }else{
            // print buffer which contains the client contents
            // and send that buffer to server
            printf("RX string: %s\n",buff);
            //send(this->sock_local, (void*)buff, len,0);
        }
        sleep(1);
    }
	// close the socket
	close(sock_ext);
    return 0;
}
