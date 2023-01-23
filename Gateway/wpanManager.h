#ifndef WPANMANAGER_H
#define WPANMANAGER_H

#include <vector>
#include <cstdint>
#include <string.h>   // memcpy etc

#include "ism3_server.h"
#include "node.h"
#include "powernode.h"
#include "datanode.h"

// TODO type-agnostic node arrays using malloc
typedef struct{
    uint8_t address;
    uint8_t uidHash;
}sOffer;

using namespace std;
/**
 * @brief WPAN manager
 * Handles radio module init, ticks, address attribution and packet redirection
 * to client node handlers (Node class and sub-classes). Exports a list of
 * connected nodes to be used elsewhere
 */
class wpanManager
{
public:
    /* CONSTRUCTORS ***********************************************************/
    /**
     * @brief Default constructor
     * Init with default server power and no static nodes
     */
    wpanManager();
    /**
     * @brief Static address list constructor
     * @param Static node vector
     * Init with default server power
     */
    wpanManager(vector<Node> nodeList_);
    /**
     * @brief Power setting constructor
     * @param Power setting and matching dBm
     * Init with custom server power and no static nodes
     */
    wpanManager(uint8_t power_, uint8_t power_dbm_);
    /**
     * @brief Complete constructor
     * @param Static node vector
     * @param Power setting and matching dBm
     */
    wpanManager(vector<Node> nodeList_, uint8_t power_, uint8_t power_dbm_);

    /* GETTERS ****************************************************************/
    /**
     * @return Vector of pointers to all nodes
     */
    vector<Node*> getNodeList();
    /**
     * @return Vector of pointers to all static nodes
     */
    vector<Node*> getStaticNodeList();
    /**
     * @return Vector of pointers to all power nodes
     */
    vector<PowerNode*> getPowerNodeList();
    /**
     * @return Vector of pointers to all data nodes
     */
    vector<DataNode*> getDataNodeList();
    /**
     * @return True if node list has changed since last poll
     */
    bool nodeListUpdated();

    /* SETTERS ****************************************************************/
    /**
     * @brief Clear all node lists
     * Send disconnect command to all nodes to notify them
     */
    void clearNodeLists();
    /**
     * @brief Clear dynamically-addressed nodes
     * Send disconnect command to deleted nodes to notify them
     */
    void clearDynamicNodeList();
    /**
     * @brief Clear statically-addressed nodes
     * Send disconnect command to deleted nodes to notify them
     */
    void clearStaticNodeList();
    /* COMMANDS ***************************************************************/
    /**
     * @brief ticker for WPAN manager
     * @param tick delay in ms
     * Handles ISM tick, periodic node list update and lease expiry checks
     * Tick delay is implemented here as sleep period
     */
    void tick(uint32_t delayMs_);
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
    /**
     * @brief update node types from nodes list
     * Ask non-identified base nodes for their supported protocols.
     */
    void updateNodeTypes();

    /* UTILITIES **************************************************************/
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

    /* HANDLERS ***************************************************************/
    /**
     * @brief handle RX data
     * @param RX data and size
     * @param data source
     * Call relevant callback if source is a registered node.
     * Call DORA handler on DORA command reception
     */
    void rxHandler(const uint8_t* data, uint8_t size, uint8_t source);
private:
    /**
     * @brief Rebuild all node lists
     */
    void rebuildNodeLists();
    /**
     * @brief Rebuild statically-addressed node lists
     * Clears node pointer vectors and rebuilds them according to new instances
     * stored in base, power and data nodes vector
     */
    void rebuildStaticNodeLists();
    /**
     * @brief Rebuild dynamically-addressed node lists
     * Clears node pointer vectors and rebuilds them according to new instances
     * stored in base, power and data nodes vector
     */
    void rebuildDynNodeLists();
    /**
     * @brief update node types from dynamicNodes list
     * Ask dynamically-addressed nodes for their supported protocols
     */
    void updateDynamicNodeTypes();
    /**
     * @brief update static node types from staticNodes list
     * Ask nodes for their supported protocols
     */
    void updateStaticNodeTypes();
    /**
     * @brief update node type after receiving net_getProtocols answer
     * Triggers a node list rebuild
     */
    void updateNodeTypesCallback();
    /* DORA METHODS ***********************************************************/
    /**
     * @brief Check node list for expired leases
     * Sets a timer for next lease expiry check
     */
    void checkLeaseExpiry();
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

    /* VARIABLES **************************************************************/
    uint8_t txpower;        // Server TX power
    uint8_t txpower_dbm;    // Server dBm TX power equivalent
    uint32_t awakeGroups=0; // To restore awake groups after a special command
    bool newNodeList=false; // Indicate node list has been updated
    /* TICK VARIABLES *********************************************************/
    const uint32_t leaseExpiryCheckPeriodS=NETWORK_LEASE_UNIT_MINUTES*30;
          uint32_t nextLeaseExpiryCheckS=0; // Lease expiry check timers
    const uint32_t nodeTypeUpdatePeriodS=60;
          uint32_t nextNodeTypeUpdateS=0;   // Node type update timers
    bool updateNodeTypesFlag=false;         // Flag for node type update
    /* DORA VARIABLES *********************************************************/
    uint8_t leaseDuration=1;            // Default lease duration
    uint8_t uid[NETWORK_UID8_WIDTH];    // Server UID
    vector<sOffer> offers;              // Current DORA offers
    /* NODE LISTS *************************************************************/
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
