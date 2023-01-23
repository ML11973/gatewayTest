#ifndef NODE_H
#define NODE_H

#include <vector>
#include <cstdint>
#include <string>
#include <string.h>   // memcpy etc
#include <iostream>
#include <ctime>

#include "cm4_utils.h"
#include "ism3_server.h"
#include "wpan.h"

using namespace std;

#define NODE_DESC_LENGTH 128
//#define NODE_UID32_WIDTH 3
//#define NODE_UID8_WIDTH ISM_UID_SIZE
#define NODE_ADDR_WIDTH 1



/**
 * Node class as seen by the network controller.
 * DO NOT CONFUSE WITH ACTUAL NODE, this class is only a soft representation
 * @todo write docs
 * @todo test renew lease function
 */
class Node
{
public:
    /**
     * Default constructor
     */
    Node();
    /**
     * @brief Static definition constructor
     * @param node address
     * @param node group
     */
    Node(uint8_t _address, uint32_t _group);
    /**
     * @brief Dynamic definition constructor
     * @param node address
     * @param node group
     * @param lease duration
     */
    Node(uint8_t _address, uint32_t _group, uint8_t _leaseDuration);

    /**
     * Destructor
     */
    virtual ~Node();

    friend ostream& operator<<(ostream & out, const Node & node);

    /**
     * @brief printer function
     */
    virtual void show();

    // GETTERS
    /**
     * @brief get unique device id
     * @param buffer to store the uid in
     * @param size of buffer, must be > NODE_UID8_WIDTH
     * @return number of bytes written to buffer
     */
    uint8_t getUid(uint8_t * buffer, uint8_t size);
    uint8_t getAddr();
    /**
     * @brief get address before address was changed
     * @return previous address
     * Useful if address was not changed as expected
     */
    uint8_t getOldAddr();
    /**
     * @brief get node group
     * @return group
     */
    uint32_t getGroup();
    /**
     * @brief get lease duration
     * @return lease duration
     */
    uint8_t getLeaseDuration();
    /**
     * @brief get lease start time
     * @return lease start time in UNIX epoch seconds
     */
    uint32_t getLeaseStartTime();
    /**
     * @brief get announced supported network protocols
     * @return protocol bitfield
     */
    uint8_t getProtocols();
    /**
     * @brief get class-supported network protocols
     * @return protocol bitfield
     */
    virtual uint8_t getNodeTypeProtocols();
    /**
     * @brief Wake low power group the node is part of
     * @return true if wakeup command was sent
     */
    bool wakeup();
    /**
     * @brief Unwake low power group the node is part of
     * @return true if unwake command was sent
     */
    bool sleep();

    // NET PROTOCOL COMMANDS
    /**
     * @param timeout in ms
     * @return ping result
     * If timeout is 0, function is non-blocking and returns false.
     * In this case, use pingStatus to check ping operation
     */
    bool net_ping(uint32_t timeoutMs);
    /**
     * @param timeout in ms
     * @return boolean uid get result
     * If timeout is 0, function is non-blocking and returns false.
     * In this case, use uidStatus to check command result
     */
    bool net_getUid(uint32_t timeoutMs);
    /**
     * @param new address
     * @return always true
     * This command is not confirmed, manual confirmation through ping is needed
     */
    bool net_setAddr(uint8_t newAddr);
    /**
     * @return always true
     * @param maxTries max number of new net_setAddr commands
     * @param timeoutMs timeout for EACH try in
     * @return true if address was successfully changed
     * Set node address again using net_setAddr to previous node address
     * Ping to confirm new address
     */
    bool net_setAddrAgain(uint8_t maxTries, uint32_t timeoutMs);
    /**
     * @param new group
     * @return always true
     * This command is not confirmed, manual confirmation through ping is needed
     */
    bool net_setGroup(uint32_t newGroup);
    /**
     * @brief disconnects Node
     * @param timeout in ms
     * @return true if disconnect was confirmed within timeout
     */
    bool net_disconnect(uint32_t timeoutMs);
    /**
     * @brief Get node supported protocols
     * @param timeout in ms
     * @return true if protocols were fetched within timeout
     */
    bool net_getProtocols(uint32_t timeoutMs);

    // NON-BLOCKING NETWORK COMMAND CHECKERS
    /**
     * @return ping result
     */
    bool pingStatus();
    /**
     * @return uid get cmd status
     */
    bool uidStatus();
    /**
     * @return get protocol cmd status
     */
    bool protocolsStatus();
    /**
     * @return connection status
     */
    bool isConnected();
    /**
     * @return true if lease is not expired or static
     */
    bool isLeaseValid();
    /**
     * @brief Send buffer to Node using unicast frame
     * @param unsigned char buffer
     * @param buffer length
     */
    uint8_t tx(uint8_t * buffer, uint8_t length);
    /**
     * @brief master RX handler, calls corresponding protocol handler
     * @param frame data and size from lower layer callback
     * @return none
     */
    void rxCallback(const uint8_t* data, uint8_t size);

protected:
    /**
     * @brief TX frame expecting arg flag to be raised, with timeout
     * @param frame and frame length
     * @param timeout in ms
     * @param flag to be set by RX callback function
     */
    bool txTimeout(uint8_t * frame, uint8_t length, uint32_t timeoutMs, bool*callbackFlag);

    uint8_t protocols=0;
    uint32_t leaseStartTime=0;

private:
    /**
     * @brief Network protocol RX handler
     * @param frame data and size from rxCallback
     * @return none
     * Handles network commands, lower is first
     */
    void netCmdCallback(const uint8_t* data, uint8_t size);
    /**
     * @brief Application protocol RX handler
     * @param frame data and size from rxCallback
     * @return none
     * Handles application commands, to be defined in inheriting class
     */
    virtual void appCmdCallback(const uint8_t * data, uint8_t size){}
    /**
     * @brief Application error protocol RX handler
     * @param frame data and size from rxCallback
     * @return none
     * Handles application error flags, to be defined in inheriting class
     */
    virtual void appErrCallback(const uint8_t * data, uint8_t size){}

    /**
     * @brief Data transfer protocol callback
     * @param frame data and size from rxCallback
     * @return none
     * Handles data transfer
     */
    virtual void dataCallback(const uint8_t * data, uint8_t size){}

    /**
     * @brief Print frame if DEBUG_RXTX is defined
     * @param buffer and length, dir=true for TX frame
     * Prints in HEX notation
     */
    void printFrame(const uint8_t * buffer, uint8_t size, bool dir);
    /**
     * @brief Print frame if DEBUG_NET is defined
     * @param buffer and length, dir=true for TX frame
     * Prints in HEX notation
     */
    void printNetFrame(const uint8_t * buffer, uint8_t size, bool dir);
    /**
     * @brief Print frame if DEBUG_APP is defined
     * @param buffer and length, dir=true for TX frame
     * Prints in HEX notation
     */
    virtual void printAppFrame(const uint8_t * buffer, uint8_t size, bool dir);
    /**
     * @brief Print frame if DEBUG_DATA is defined
     * @param buffer and length, dir=true for TX frame
     * Prints in HEX notation
     */
    virtual void printDataFrame(const uint8_t * buffer, uint8_t size, bool dir);

    uint8_t address=NETWORK_NACK_BASE_ADDR;
    uint8_t oldAddress=0;
    uint32_t group=NETWORK_NACK_GROUP;
    uint8_t uid[NETWORK_UID8_WIDTH]={0};
    bool pingCallback=false;
    bool uidCallback=false;
    bool addrCallback=false;
    bool protocolsCallback=false;
    bool disconnectStatus=false;
    uint8_t leaseDuration=0; // 0=static node
};



#endif // NODE_H
