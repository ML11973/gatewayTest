/**
 * @file node.h
 * @author marc.leemann@master.hes-so.ch
 * @brief Node class definition
 */

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

#define NODE_DESC_LENGTH 128 ///< Node description length
//#define NODE_UID32_WIDTH 3
//#define NODE_UID8_WIDTH ISM_UID_SIZE
#define NODE_ADDR_WIDTH 1 ///< Node address width in bytes

/**
 * @page nodes Nodes
 *
 * # Description
 *
 * Nodes refer to WPAN clients. They typically provide telemetry data and can
 * control devices.
 * In this software, nodes connected to the gateway are represented as Node
 * instances in wpanManager.
 *
 * The Node class provides a user with a representation of the available
 * information about the physical node. The class provides a simple way to
 * handle commands coming from and going to Nodes.
 *
 * This class currently has two children:
 * - @subpage powernodes
 * - @subpage datanodes
 *
 * # Usage
 *
 * ## Instantiation
 *
 * Instantiate using any constructor.
 * Using a constructor that does not specify lease duration or setting lease
 * duration to zero will create a statically-addressed Node.
 * Manually changing address and group is not allowed once instantiated, so
 * it is important to set the correct values on constructor call.
 *
 * ## Sending commands
 *
 * Use any function starting with net_ to issue a @subpage net_protocol command.
 * Class will handle frame building and behavior on answer, provided its
 * Node::rxCallback is called with the relevant frames.
 *
 * Protocol commands that require an answer typically set a callback flag that
 * is updated on answer reception through Node::rxCallback. Failure to call
 * handler will cause the command to time out.
 *
 * Protocol commands can be blocking or non-blocking depending on argument
 * timeout value. If timeout is 0, command is non-blocking. Status can be
 * manually checked using a Node::pingStatus-style function.
 *
 * ## Debugging
 *
 * wpan.h provides two DEBUG macros for this class.
 * Uncomment #DEBUG_RXTX to show all incoming and outgoing frames.
 * Uncomment #DEBUG_NET to show only @subpage net_protocol frames.
 *
 * # Implementation details
 *
 * ## Frame IO
 *
 * The Node::tx method is a wrapper for the radio driver's ism_tx function.
 * It transmits a buffer to the Node address attribute.
 *
 * The Node::txTimeout method allows transmission of a buffer and expects an
 * answer flag to be raised on answer within the specified timeout.
 * The method resets the flag and polls it for a true value.
 * If the flag remains false until the timeout, the method returns.
 *
 * The Node::rxCallback method assumes the packet it receives is coming from
 * the remote node with address matching the Node address attribute.
 * A dispatcher function has to be used to ensure the proper Node instance
 * receives data from its matching remote node.
 *
 * The wpanManager class provides a rxHandler that dispatches packets to
 * destination Node instance Node::rxCallback.
 * See below an example sequence diagram of data protocol reception.
 *
 * @image html seq_rx.png Data protocol RX sequence
 * @image latex seq_rx.png Data protocol RX sequence
 *
 * ## Command functions
 *
 * Methods starting with net_, such as Node::net_getUid, are wrappers to send
 * the corresponding command to the remote node. They also reset the
 * corresponding callback flag. Using these methods is the safest and most
 * convenient way to send @subpage net_protocol.
 *
 * **Do not confuse network commands such as Node::net_getUid with getters such
 * as Node::getUid!**
 *
 * ## Callback flags
 *
 * These flags signal whether an answer to a network command requiring
 * confirmation has been received. They are reset by their respective network
 * command method and set upon confirmation reception in Node::netCmdCallback.
 *
 * ## Class hierarchy
 * The Node class is the parent of DataNode and PowerNode.
 * It handles the common @subpage net_protocol.
 *
 * **This class and its children are runtime-polymorphic.**
 * This means that accessing a child class instance through a parent class
 * pointer is the same as accessing the child class instance through a child
 * class pointer. Changing a Node's behavior requires reinstantiation through
 * the child class' relevant constructor.
 *
 * ## Polymorphic behavior
 * This means that a Node or child of Node will have its virtual methods bound
 * at runtime on instance creation.
 * In practice, this means that a Node-type pointer to a child class will
 * execute the child class virtual methods, while a static method bind would
 * have it execute the pointer-type methods.
 * <a href="https://www.geeksforgeeks.org/virtual-function-cpp/">This article
 * will explain it better</a>
 *
 * For example, calling Node::appCmdCallback from a Node-type pointer to a
 * PowerNode instance will execute PowerNode::appCmdCallback instead of
 * Node::appCmdCallback.
 *
 * It is therefore impossible to change an instance's behavior by using a
 * different type pointer. Changing a Node to PowerNode type requires
 * destruction of the Node instance and creation of a PowerNode instance.
 * Fortunately, this is covered by the relevant constructor in child classes.
 */

/**
 * @brief Reprensentation of a distant node. See @ref nodes page for details.
 */
class Node
{
public:
    /**
     * @brief Default constructor
     */
    Node();
    /**
     * @brief Static definition constructor
     * @param _address node address
     * @param _group node group
     *
     * Init with lease duration = 0
     */
    Node(uint8_t _address, uint32_t _group);
    /**
     * @brief Dynamic definition constructor
     * @param _address node address
     * @param _group node group
     * @param _leaseDuration lease duration (multiple of NETWORK_LEASE_UNIT_MINUTES)
     */
    Node(uint8_t _address, uint32_t _group, uint8_t _leaseDuration);

    /**
     * @brief Destructor
     */
    virtual ~Node();

    /**
     * @brief << operator redefinition for printing using iostream
     * @param out stream to write in
     * @param node Node to write in stream
     */
    friend ostream& operator<<(ostream & out, const Node & node);

    /**
     * @brief Printer function
     */
    virtual void show();

    // GETTERS
    /**
     * @brief Get unique device id
     * @param buffer pointer to buffer to store the uid in
     * @param size size of buffer, must be > NODE_UID8_WIDTH
     * @return number of bytes written to buffer
     */
    uint8_t getUid(uint8_t * buffer, uint8_t size);
    /**
     * @brief Get address
     * @return node address
     */
    uint8_t getAddr();
    /**
     * @brief Get address before address was changed
     * @return previous address
     * Useful if address was not changed as expected
     */
    uint8_t getOldAddr();
    /**
     * @brief Get node group
     * @return group
     */
    uint32_t getGroup();
    /**
     * @brief Get lease duration
     * @return lease duration
     */
    uint8_t getLeaseDuration();
    /**
     * @brief Get lease start time
     * @return lease start time in UNIX epoch seconds
     */
    uint32_t getLeaseStartTime();
    /**
     * @brief Get announced supported network protocols
     * @return protocol bitfield
     */
    uint8_t getProtocols();
    /**
     * @brief Get class-supported network protocols
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

    /* NETWORK PROTOCOL COMMANDS **********************************************/
    /**
     * @brief Ping node
     * @param timeoutMs timeout in ms, non-blocking if 0
     * @return ping result
     *
     * If timeout is 0, function is non-blocking and returns false.
     * In this case, use pingStatus to check ping operation
     */
    bool net_ping(uint32_t timeoutMs);
    /**
     * @brief Get node UID
     * @param timeoutMs timeout in ms, non-blocking if 0
     * @return boolean uid get result
     *
     * If timeout is 0, function is non-blocking and returns false.
     * In this case, use uidStatus to check command result
     */
    bool net_getUid(uint32_t timeoutMs);
    /**
     * @brief Change node address
     * @param newAddr desired new address
     * @return always true
     *
     * This command is not confirmed, manual confirmation through ping is needed
     */
    bool net_setAddr(uint8_t newAddr);
    /**
     * @brief Try changing node address until it is confimed.
     * @return always true
     * @param maxTries max number of new net_setAddr commands
     * @param timeoutMs timeout in ms, non-blocking if 0
     * @return true if address was successfully changed
     *
     * Set node address again using net_setAddr to previous node address
     * Ping to confirm new address
     */
    bool net_setAddrAgain(uint8_t maxTries, uint32_t timeoutMs);
    /**
     * @brief Change node group
     * @param newGroup new group
     * @return always true
     *
     * This command is not confirmed, manual confirmation through ping is needed
     */
    bool net_setGroup(uint32_t newGroup);
    /**
     * @brief Disconnects Node
     * @param timeoutMs timeout in ms, non-blocking if 0
     * @return true if disconnect was confirmed within timeout
     */
    bool net_disconnect(uint32_t timeoutMs);
    /**
     * @brief Get node supported protocols
     * @param timeoutMs timeout in ms, non-blocking if 0
     * @return true if protocols were fetched within timeout
     */
    bool net_getProtocols(uint32_t timeoutMs);

    // NON-BLOCKING NETWORK COMMAND CHECKERS
    /**
     * @brief Get #NETWORK_PING command status
     * @return ping result
     */
    bool pingStatus();
    /**
     * @brief Get #NETWORK_GETUID command status
     * @return uid get cmd status
     */
    bool uidStatus();
    /**
     * @brief Get #NETWORK_GET_PROTOCOLS command status
     * @return get protocol cmd status
     */
    bool protocolsStatus();
    /**
     * @brief Get node connection status (#NETWORK_DISCONNECT)
     * @return connection status
     */
    bool isConnected();
    /**
     * @brief Get address lease status
     * @return true if lease is not expired or static
     */
    bool isLeaseValid();
    /**
     * @brief Send buffer to Node using unicast frame
     * @param buffer pointer to data to send
     * @param length data length in bytes
     */
    uint8_t tx(uint8_t * buffer, uint8_t length);
    /**
     * @brief Master RX handler, calls corresponding protocol handler
     * @param data pointer to RX data
     * @param size data length in bytes
     */
    void rxCallback(const uint8_t* data, uint8_t size);

protected:
    /**
     * @brief TX frame expecting arg flag to be raised, with timeout
     * @param frame pointer to frame to transmit
     * @param length frame length in bytes
     * @param timeoutMs timeout in ms, non-blocking if 0
     * @param callbackFlag pointer to flag to be set by RX callback function
     */
    bool txTimeout(uint8_t * frame, uint8_t length, uint32_t timeoutMs, bool*callbackFlag);

    uint8_t protocols=0; ///< Supported protocols as announced by Node
    uint32_t leaseStartTime=0; ///< Start time of lease in UNIX epoch

private:
    /**
     * @brief Network protocol RX handler
     * @param data pointer to data to process
     * @param size data length in bytes
     *
     * Handles network commands, lower is first
     */
    void netCmdCallback(const uint8_t* data, uint8_t size);
    /**
     * @brief Application protocol RX handler
     * @param data pointer to data to process
     * @param size data length in bytes
     *
     * Handles application commands, to be defined in inheriting class
     */
    virtual void appCmdCallback(const uint8_t * data, uint8_t size){}
    /**
     * @brief Application error protocol RX handler
     * @param data pointer to data to process
     * @param size data length in bytes
     *
     * Handles application error flags, to be defined in inheriting class
     */
    virtual void appErrCallback(const uint8_t * data, uint8_t size){}

    /**
     * @brief Data transfer protocol callback
     * @param data pointer to data to process
     * @param size data length in bytes
     *
     * Handles data transfer
     */
    virtual void dataCallback(const uint8_t * data, uint8_t size){}

    /**
     * @brief Print frame if DEBUG_RXTX is defined
     * @param buffer pointer to data to print
     * @param size data length in bytes
     * @param dir direction,true for TX frame
     *
     * Prints in HEX notation
     */
    void printFrame(const uint8_t * buffer, uint8_t size, bool dir);
    /**
     * @brief Print frame if DEBUG_NET is defined
     * @param buffer pointer to data to print
     * @param size data length in bytes
     * @param dir direction,true for TX frame
     *
     * Prints in HEX notation
     */
    void printNetFrame(const uint8_t * buffer, uint8_t size, bool dir);
    /**
     * @brief Print frame if DEBUG_APP is defined
     * @param buffer pointer to data to print
     * @param size data length in bytes
     * @param dir direction,true for TX frame
     *
     * Prints in HEX notation
     */
    virtual void printAppFrame(const uint8_t * buffer, uint8_t size, bool dir);
    /**
     * @brief Print frame if DEBUG_DATA is defined
     * @param buffer pointer to data to print
     * @param size data length in bytes
     * @param dir direction,true for TX frame
     *
     * Prints in HEX notation
     */
    virtual void printDataFrame(const uint8_t * buffer, uint8_t size, bool dir);

    uint8_t address=NETWORK_NACK_BASE_ADDR; ///< Node address
    uint8_t oldAddress=0; ///< Node previous address
    uint32_t group=NETWORK_DORA_GROUP; ///< Node group
    uint8_t uid[NETWORK_UID8_WIDTH]={0}; ///< Node unique identifier
    bool pingCallback=false; ///< Callback flag for #NETWORK_PING command
    bool uidCallback=false; ///< Callback flag for #NETWORK_GETUID command
    bool protocolsCallback=false; ///< Callback flag for #NETWORK_GET_PROTOCOLS command
    bool disconnectStatus=false; ///< Callback flag for #NETWORK_DISCONNECT command
    uint8_t leaseDuration=0; ///< Address lease duration, static address if 0
};



#endif // NODE_H
