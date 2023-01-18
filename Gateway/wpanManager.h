#ifndef WPANMANAGER_H
#define WPANMANAGER_H

#include <vector>
#include <cstdint>
#include <algorithm>    // std::find
#include <string>
#include <sstream>
#include <fstream> // read from file
#include <string.h>   // memcpy etc

//#define DEBUG_TICKS
//#define DEBUG_WPANMANAGER
//#define DEBUG_DORA

#include "ism3_server.h"
#include "node.h"
#include "powernode.h"
#include "datanode.h"

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
    wpanManager(vector<Node> nodeList_);
    wpanManager(uint8_t power_, uint8_t power_dbm_);
    /**
     * Constructor for static address list
     */
    wpanManager(vector<Node> nodeList_, uint8_t power_, uint8_t power_dbm_);

    /**
     * Destructor
     */
    //~wpanManager();

    // Getters
    vector<Node*> getNodeList();
    vector<Node*> getStaticNodeList();
    vector<PowerNode*> getPowerNodeList();
    vector<DataNode*> getDataNodeList();

    void clearNodeLists();
    void clearDynamicNodeList();
    void clearStaticNodeList();

    /**
     * @brief ticker for WPAN manager
     * @param tick delay in ms
     * ISM tick
     */
    void tick(uint32_t delayMs_);

    /**
     * @brief handle RX data
     * @param RX data and size
     * @param data source
     * Call relevant callback if source is a registered node.
     * Process DORA requests
     */
    void rxHandler(const uint8_t* data, uint8_t size, uint8_t source);

    /**
     * @brief update node types from dynamicNodes list
     * Ask dynamically-addressed nodes for their supported protocols.
     * If received, recreate a Node child class instance that matches
     * the node type
     */
    void updateDynamicNodeTypes();

    /**
     * @brief update static node types from staticNodes list
     * Ask nodes for their supported protocols.
     * If received, recreate a Node child class instance that matches
     * the node type
     */
    void updateStaticNodeTypes();

    /**
     * @brief update node types from nodes list
     * Ask nodes for their supported protocols.
     * If received, recreate a Node child class instance that matches
     * the node type
     */
    void updateNodeTypes();

    /**
     * @brief print node list
     * Prints statically and dynamically-addressed nodes
     */
    void printNodes();
    /**
     * @brief print static node list
     * Prints statically-addressed nodes
     */
    void printStaticNodes();
    /**
     * @brief Start dynamic discovery
     * Wake up dynamic discovery group
     */
    void startDynamicDiscovery();
    /**
     * @brief Stop dynamic discovery
     * Put dynamic discovery group to sleep
     */
    void stopDynamicDiscovery();

private:

    void rxHandler_oldAddr(const uint8_t* data, uint8_t size, uint8_t source);
    void rebuildNodeLists();
    void rebuildStaticNodeLists();
    void rebuildDynNodeLists();
    /**
     * @brief Handler for DORA frames
     * @param RX data and size
     * Master handler for DORA dynamic addressing. Handées all DORA frames
     */
    uint8_t dora(const uint8_t* data, uint8_t size);
    /**
     * @brief Store a DORA address offer
     * @param offered address
     * @param target UID
     * Store address and uid in addressOffers and offerUidHashes vectors.
     * UID is stored as a hash for simplicity
     */
    void storeOffer(uint8_t address, uint8_t * uid);
    /**
     * @brief Check if DORA address was offered to UID
     * @param address to check
     * @param UID
     * @return true if address was offered to uid
     * Check if the address, uid pair is in addressOffers and offerUidHashes
     * vectors. If it is in both at the same index, return true and erase offer
     * from vectors. Otherwise return false
     */
    bool checkOfferMatch(uint8_t address, uint8_t * uid);
    /**
     * @brief hash a uid
     * @param pointer to uid first byte
     * @return hashed value
     * Apply a XOR operation to NETWORK_UID8_WIDTH bytes starting from pointer
     * Does not check proper buffer width, use wisely
     */
    uint8_t hashUid(uint8_t * uid);
    /**
     * @brief Build a DORA frame from arguments
     * @param buffer and maxSize to build frame in
     * @param command to send
     * @param destination UID pointer (data must be of NETWORK_UID8_WIDTH)
     * @param frame data and size
     * @return built frame length
     * Builds a DORA frame as specified in wpan.h
     * Input UID size is not checked, use wisely.
     */
    uint8_t buildDoraFrame(uint8_t * buffer, uint8_t maxSize, uint8_t cmd, uint8_t * destUID, uint8_t * data, uint8_t dataSize);


    const uint32_t netCmdTimeout=1000;
    const uint8_t maxPingRetries=5;
    uint8_t txpower;
    uint8_t txpower_dbm;
    uint8_t leaseDuration=1;
    uint8_t uid[NETWORK_UID8_WIDTH];
    vector<uint8_t> addressOffers;
    vector<uint8_t> offerUidHashes;
    vector<Node*> pNodes;               // Static+dynamically-addressed nodes

    vector<Node*> pStaticNodes;         // Static node pointer array
    vector<Node*> pDynNodes;            // Pointers for dynamic nodes

    // Statically-addressed typed nodes
    vector<Node> staticBaseNodes;       // base nodes, ideally empty
    vector<PowerNode> staticPowerNodes; // identified power nodes
    vector<DataNode> staticDataNodes;   // identified data nodes

    // Dynamically-addressed typed nodes
    vector<Node> dynBaseNodes;          // base nodes, ideally empty
    vector<PowerNode> dynPowerNodes;    // identified power nodes
    vector<DataNode> dynDataNodes;      // identified data nodes
};

#endif // WPANMANAGER_H
