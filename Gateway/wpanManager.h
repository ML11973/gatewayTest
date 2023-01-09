#ifndef WPANMANAGER_H
#define WPANMANAGER_H

#include <vector>
#include <cstdint>
#include <algorithm>    // std::find
#include <string>
#include <sstream>
#include <fstream> // read from file
//#include <string.h>   // memcpy etc

#define DEBUG_WPANMANAGER

#include "ism3_server.h"
#include "node.h"

using namespace std;
/**
 * @todo write docs
 */
class wpanManager
{
public:
    /**
     * Default constructor
     */
    wpanManager();
    wpanManager(vector<Node*> nodeList_);
    wpanManager(uint8_t power_, uint8_t power_dbm_);
    /**
     * Constructor for static address list
     */
    wpanManager(vector<Node*> nodeList_, uint8_t power_, uint8_t power_dbm_);

    /**
     * Destructor
     */
    //~wpanManager();

    //uint8_t staticDiscovery(vector<Node> nodelist);
/*
    uint8_t discoverNodes();

    uint8_t connectNodes();

    bool connectNode(Node);
*/
    // Getters
    vector<Node*> getNodeList();
    vector<Node*> getStaticNodeList();

    void rxHandler(const uint8_t* data, uint8_t size, uint8_t source);

    // ping all nodes and remove unreachable nodes
    uint8_t updateConnectedNodeList();

    void printNodes();

private:


    void rxHandler_oldAddr(const uint8_t* data, uint8_t size, uint8_t source);


    const uint32_t netCmdTimeout=1000;
    const uint8_t maxPingRetries=5;
    uint8_t txpower;
    uint8_t txpower_dbm;
    vector<Node*> nodes;                // Static+dynamically-addressed nodes

    // UNUSED YET
    vector<Node*> staticNodes;       // Memory zone for static nodes
    vector<Node*> dynamicNodes;      // Unused yet

    vector<Node*> connectedNodes;       // Nodes that responded to a ping
};

#endif // WPANMANAGER_H
