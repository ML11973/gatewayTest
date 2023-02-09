/**
 * @file datanode.h
 * @author marc.leemann@master.hes-so.ch
 * @brief DataNode class definition
 */

#ifndef DATANODE_H
#define DATANODE_H

#include "node.h"
#include <cmath>


/**
 * @page datanodes Data Nodes
 *
 * # Description
 *
 * DataNode refers to @ref nodes that support datagram-based communication.
 *
 * The DataNode class allows transmission and reception of datagrams split over
 * several packets.
 *
 * # Usage
 *
 * ## Instantiation
 *
 * Instantiate using any constructor. Constructors match Node type constructor.
 * A constructor for DataNode instantiation from a Node object exists.
 *
 * ## Sending and receiving datagrams
 *
 * Use DataNode::datagram_tx to transmit a datagram with the
 * @subpage data_protocol.
 *
 * Use DataNode::readDatagram to read the oldest received datagram.
 * Datagrams are cleared on read.
 *
 * Other operations are inherited from parent Node.
 *
 * ## Debugging
 *
 * wpan.h provides two DEBUG macros for this class.
 * Uncomment #DEBUG_RXTX to show all incoming and outgoing frames.
 * Uncomment #DEBUG_DATA to show only @subpage data_protocol frames.
 *
 * # Implementation details
 *
 * ## Frame IO
 *
 * This class only provides the DataNode::datagram_tx method.
 * Network protocol support is inherited from Node class.
 *
 * ## Class hierarchy
 * The DataNode class is a child of the Node class.
 * It handles the @subpage data_protocol and leaves the rest to its parent.
 */

using namespace std;

/**
 * @brief Datagram-capable node. See @ref datanodes.
 */
class DataNode : public Node
{
public:
    /**
     * @brief Static definition constructor
     * @param _address node address
     * @param _group node group
     *
     * Init with lease duration = 0
     */
    DataNode(uint8_t _address, uint32_t _group);
    /**
     * @brief Dynamic definition constructor
     * @param _address node address
     * @param _group node group
     * @param _leaseDuration lease duration (multiple of NETWORK_LEASE_UNIT_MINUTES)
     */
    DataNode(uint8_t _address, uint32_t _group, uint8_t _leaseDuration);
    /**
     * @brief Constructor from existing Node
     * @param base source Node to transtype cast
     *
     * Copy argument node address, group and lease duration.
     * Do not copy current callback flags
     */
    DataNode(Node & base);
    /**
     * @brief Destructor
     *
     * Clears RX datagrams and frees their allocated memory
     */
    ~DataNode();
    /**
     * @brief Printer function
     */
    void show();
    /**
     * @brief TX datagram to node
     * @param data pointer to datagram to transmit
     * @param dataSize size of datagram
     *
     * Will not transmit if dataSize>DATA_MAX_DATAGRAM_LENGTH. Will segment
     * into multiple packets if data is too big for one ISM frame
     */
    uint16_t datagram_tx(uint8_t * data, uint16_t dataSize);
    /**
     * @brief Get number of complete received datagrams
     * @return number of received complete datagrams
     */
    int readyRxDatagrams();
    /**
     * @brief Read first ready frame
     * @param buffer pointer to memory zone to fill with data
     * @param maxSize memory zone size
     * @return size of filled data
     */
    uint16_t readDatagram(uint8_t * buffer, uint16_t maxSize);
    /**
     * @brief Clear datagrams and free their allocated memory
     */
    void clearDatagrams();

private:
    /**
     * @brief Default constructor
     *
     * Init with address, group and lease duration = 0
     */
    DataNode();
    /**
     * @brief Data transfer protocol callback
     * @param data RX frame data
     * @param size RX size from rxCallback
     *
     * Handles datagram reception
     */
    void dataCallback(const uint8_t* data, uint8_t size);
    /**
     * @brief Print frame if DEBUG_DATA is defined
     * @param buffer pointer to data to print
     * @param size data length in bytes
     * @param dir direction,true for TX frame
     *
     * Prints in HEX notation. Redefined from parent class
     */
    void printDataFrame(const uint8_t * buffer, uint8_t size, bool dir);

    vector<sDatagram> datagrams;    ///< Vector of RX datagrams
    int readyDatagrams=0;           ///< Number of complete RX datagrams
};

#endif // DATANODE_H
