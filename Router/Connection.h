/*
 * Connection.h
 *
 *  Created on: Aug 29, 2022
 *      Author: leemarc
 */

/**
 * Connection between a UDP socket and a DataNode
 * The class is instantiated with a pointer to a DataNode
 * DataNode address determines UDP port open (BASE_GW_IN_PORT+address)
 * A tick function is called from outside to check for new data on both UDP
 * socket and DataNode datagram protocol.
 */

#ifndef CONNECTION_H_
#define CONNECTION_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// Sockets and internet libraries
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "datanode.h"
#include "wpan.h"
#include "netconfig.h"

#define MAX_FRAME_LENGTH 470

typedef enum{
	CLOSED,
	OPEN,
	SOCKETS_READY,
	ERROR_SOCKET
}eConnectionState;

class Connection {
public:
	/**
	 * @brief Constructor
	 * @param pointer to target dataNode
	 */
	Connection(DataNode * _pNode);
	/**
	 * @brief Destructor
	 * De-init used sockets
	 */
	~Connection();
	/**
	 * @brief check Connection IO
	 * check for new messages from socket and radio module
	 * Call communication handlers if new data must be transferred
	 */
	void tick();
	/**
	 * @brief getter function
	 * @return node address
	 */
	int getNodeAddr();
	/**
	 * @brief getter function
	 * @return status enum value
	 */
	eConnectionState getStatus();
private:
	/**
	 * @brief Unused constructor
	 */
	Connection();
	/**
	 * @brief deInit Connection
	 */
	void deInit();
	/**
	 * @brief initialize UDP socket depending on node address
	 */
	void initSocket();
	/**
	 * @brief deinit UDP socket
	 */
	void deInitSocket();
	/**
	 * @brief RX data from UDP socket and TX to WPAN Node
	 */
	int extToLocalHandler();
	/**
	 * @brief RX data from WPAN Node and TX to UDP socket
	 * TX to last RX address
	 */
	int localToExtHandler();

	int sock_ext;				// UDP socket file descriptor
	DataNode * pNode; 			// Linked DataNode
	uint8_t nodeAddr; 			// Node address
	struct sockaddr srcAddr; 	// UDP RX source address
	socklen_t srcAddrLen;		// IP source address length
	eConnectionState status; 	// Status of Connection
};

#endif /* CONNECTION_H_ */
