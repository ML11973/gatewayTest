#ifndef DATANODE_H
#define DATANODE_H

#include "node.h"
#include <cmath>

using namespace std;

class DataNode : public Node
{
public:
    /**
     * @brief Default constructor
     * Init with address, group and lease duration = 0
     */
    DataNode();
    /**
     * @brief Static constructor
     * @param address and group
     * Init with arguments and lease duration = 0
     */
    DataNode(uint8_t _address, uint32_t _group);
    /**
     * @brief Dynamic node constructor
     * @param address and group
     * @param lease duration (multiple of NETWORK_LEASE_UNIT_MINUTES)
     */
    DataNode(uint8_t _address, uint32_t _group, uint8_t _leaseDuration);
    /**
     * @brief Constructor from existing Node
     * @param Node to transtype
     * Copy argument node address, group and lease duration.
     * Do not copy current callback flags
     */
    DataNode(Node & base);
    /**
     * @brief Destructor
     * Clears RX datagrams and frees their allocated memory
     */
    ~DataNode();
    /**
     * @brief printer function
     */
    void show();
    /**
     * @brief TX datagram to node
     * @param data and dataSize
     * Will not transmit if dataSize>DATA_MAX_DATAGRAM_LENGTH. Will segment
     * into multiple packets if data is too big for one ISM frame
     */
    uint16_t datagram_tx(uint8_t * data, uint16_t dataSize);
    /**
     * @return number of received complete datagrams
     */
    int readyRxDatagrams();
    /**
     * @brief read first ready frame
     * @param buffer to fill with data
     * @param buffer max size
     * @return size of filled data
     */
    uint16_t readDatagram(uint8_t * buffer, uint16_t maxSize);
    /**
     * @brief Clear datagrams and free their allocated memory
     */
    void clearDatagrams();

private:
    /**
     * @brief Data transfer protocol callback
     * @param frame data and size from rxCallback
     * @return none
     * Handles data transfer
     */
    void dataCallback(const uint8_t* data, uint8_t size);
    /**
     * @brief Print frame if DEBUG_DATA is defined
     * @param buffer and length, dir=true for TX frame
     * Prints in HEX notation
     */
    void printDataFrame(const uint8_t * buffer, uint8_t size, bool dir);

    vector<sDatagram> datagrams;    // Vector of RX datagrams
    int readyDatagrams=0;           // Number of complete RX datagrams
};

#endif // DATANODE_H
