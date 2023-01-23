/*
 * Connection.cpp
 *
 *  Created on: Aug 29, 2022
 *      Author: leemarc
 */

#include "Connection.h"


Connection::Connection(DataNode * _pNode) {
	// Store connection info
	pNode = _pNode;
	status = CLOSED;
	nodeAddr = pNode->getAddr();
#ifdef DEBUG_CONNECTION
		printf("Connection: Open connection with node at address %u\n",nodeAddr);
#endif
	initSocket();
	if(status==SOCKETS_READY) status=OPEN;
}

Connection::~Connection() {
#ifdef DEBUG_CONNECTION
	printf("Connection: Close connection with node at address %u\n",nodeAddr);
#endif
	deInit();
}

void Connection::tick(){
	int readyRx=pNode->readyRxDatagrams();
	if(readyRx){
		localToExtHandler();
	}
	extToLocalHandler();
}

int Connection::getNodeAddr(){
	return nodeAddr;
}

eConnectionState Connection::getStatus(){
	return status;
}

void Connection::deInit(){
	// De-init sockets
	deInitSocket();
}

void Connection::initSocket(){
	struct sockaddr_in routaddr;

#ifdef DEBUG_SOCKETS
		printf("External socket init for node address %u \n",nodeAddr);
#endif
	// Non-blocking UDP socket
	sock_ext = socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK, 0);

#ifdef FORCE_BIND
	int sockopt=1;
	// set socket option to prevent bind errors
	setsockopt(sock_ext,SOL_SOCKET,SO_REUSEADDR,&sockopt,sizeof(int));
#endif

	if (sock_ext == -1) {
		printf("Connection: socket creation failed...\n");
		close(sock_ext);
		status=ERROR_SOCKET;
		return;
	}else{
#ifdef DEBUG_SOCKETS
		printf("Connection: socket successfully created\n");
#endif
	}

	bzero(&routaddr, sizeof(routaddr));

	// assign IP, PORT
	routaddr.sin_family = AF_INET;
	routaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	routaddr.sin_port = htons(BASE_GW_IN_PORT+nodeAddr);

	// Binding newly created socket to given IP and verification
	if ((bind(sock_ext, (struct sockaddr*)&routaddr, sizeof(routaddr))) != 0) {
		printf("Connection: socket bind failed...\n");
		close(sock_ext);
		status=ERROR_SOCKET;
		return;
	}else{
#ifdef DEBUG_SOCKETS
		printf("Connection: socket bind to port %u successful\n",BASE_GW_IN_PORT+nodeAddr);
#endif
	}
}

void Connection::deInitSocket(){
	// Close socket
	close(sock_ext);
}

int Connection::extToLocalHandler(){
	uint8_t buff[MAX_FRAME_LENGTH] = {0};
	int len;
	// read the message from client and copy it in buffer
	len = recvfrom(sock_ext,(void*) buff, sizeof(buff), 0, &srcAddr, &srcAddrLen);
	if(len<=0){
		// No data available
	}else{
		// print buffer which contains the client contents
#ifdef DEBUG_CONNECTION
		printf("Connection: forwarding from socket %u to node with address %u : %s\n", BASE_GW_IN_PORT+nodeAddr, nodeAddr, buff);
#endif
		// and send that buffer to server
		pNode->datagram_tx(buff, len);
	}
	return len;
}

int Connection::localToExtHandler(){
	uint8_t buff[MAX_FRAME_LENGTH] = {0};
	int len = 0;
	// read the message from server and copy it in buffer
	//len = recv(this->sock_local, buff, sizeof(buff), 0);
	len = pNode->readDatagram(buff, sizeof(buff));
	if(len<=0){
		// No data available
	}else{
		// print buffer which contains the client contents
#ifdef DEBUG_CONNECTION
		printf("Connection: forwarding from node with address %u to socket %u : %s\n", nodeAddr, BASE_GW_IN_PORT+nodeAddr, buff);
#endif
		// and send that buffer to client
		sendto(sock_ext, buff, len, 0, &srcAddr, srcAddrLen);
	}
	return len;
}
