/**
 * @file Connection.h
 *
 *  Created on: Aug 29, 2022
 *      Author: leemarc
 */

/**
 * @page connections Connections
 *
 * ## Description
 *
 * A Connection is a datagram forwarder from a UDP socket to a DataNode.
 * This is the core of the @ref borderrouter functionality.
 *
 * ## Usage
 * ### Instantiation
 *
 * The class is instantiated with a pointer to its DataNode.
 * It opens a socket to UDP port = #BASE_GW_IN_PORT + DataNode::address.
 * Any device can then connect to a DataNode provided they can reach the
 * UDP port.
 *
 * ### Ticking
 *
 * Connections must be manually polled since DataNode data reception does not
 * throw interrupts yet.
 *
 * ### Packet forwarding
 *
 * As of now, Connection forwards any incoming UDP datagrams to its remote node.
 * It forwards all datagrams coming from its node to the **last UDP sender**.
 * This is basic, can be unpractical and unsafe, but functional for now.
 * Security is left to the user via access control on the gateway's sockets.
 *
 * A way to eliminate this problem would be to include source address and port
 * in datagrams transmitted via @ref data_protocol.
 * This would however reduce already limited payload, since an IPv6 address is
 * 16-byte long and a port number is 2-byte long.
 * This would thus take away 18 bytes out of at least one @ref data_protocol
 * packet or out of every packet depending on implementation.
 *
 * Given the probable use case of one local server centralizing all WPAN
 * information and making it accessible online, the security gains of reliable
 * UDP (!) transfer are slim provided the user configures the border router's
 * firewall correctly.
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

#define MAX_FRAME_LENGTH 470 ///< Experimentally defined max datagram length. User is free to modify this value. Approx 50% successful transfers above 705

/**
 * @brief State of Connection
 *
 * CLOSED if Connection is not initialized.
 * OPEN if Connection is properly initialized.
 * SOCKETS_READY if IP socket is initialized.
 * ERROR_SOCKET if IP socket was not properly initialized.
 */
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
	 * @param _pNode pointer to target dataNode
	 */
	Connection(DataNode * _pNode);
	/**
	 * @brief Destructor
	 *
	 * De-initialize used sockets
	 */
	~Connection();
	/**
	 * @brief check Connection IO
	 *
	 * Check for new messages from socket and DataNode.
	 * Call communication handlers if new data is available for transfer.
	 */
	void tick();
	/**
	 * @brief Getter function for node address
	 * @return node address
	 */
	int getNodeAddr();
	/**
	 * @brief Getter function for Connection status
	 * @return eConnectionState status enum value
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

	int sock_ext;				///< UDP socket file descriptor
	DataNode * pNode; 			///< Linked DataNode
	uint8_t nodeAddr; 			///< Node address
	struct sockaddr srcAddr; 	///< UDP RX source address
	socklen_t srcAddrLen;		///< IP source address length
	eConnectionState status; 	///< Status of Connection
};

#endif /* CONNECTION_H_ */
