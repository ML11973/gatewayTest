#ifndef DATANODE_H
#define DATANODE_H

#include "node.h"
#include <cmath>

using namespace std;

/**
 * @todo write docs
 * TODO test data TX/RX
 */
class DataNode : public Node
{
public:
    /**
     * @todo write docs
     */
    DataNode();

    /**
     * @todo write docs
     */
    DataNode(uint8_t _address, uint32_t _group);

    /**
     * @todo write docs
     */
    DataNode(uint8_t _address, uint32_t _group, uint8_t _leaseDuration);

    DataNode(Node & base);

    ~DataNode();

    /**
     * @brief printer function
     */
    void show();

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

    bool datagramCallback=false;
    vector<sDatagram> datagrams;
    int readyDatagrams=0;
};

#endif // DATANODE_H
