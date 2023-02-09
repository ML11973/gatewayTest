/**
 * @file wpanManager.h
 * @brief WPAN manager definition
 * @author marc.leemann@master.hes-so.ch
 *
 */

#ifndef WPANMANAGER_H
#define WPANMANAGER_H

#include <vector>
#include <cstdint>
#include <string.h>   // memcpy etc

#include "ism3_server.h"
#include "node.h"
#include "powernode.h"
#include "datanode.h"

/**
 * @class wpanManager
 * @brief Master handler of radio module and connected nodes
 *
 * *For general usage information please read @ref wpanManager_desc.*
 *
 * Describe the inner functioning:
 *
 * Nodes, Node lists, rxHandler
 * Dynamic address configuration
 * Ticks, periodic checks (lease, node types)
 * Node type update, Node type management
 */
/**
 * @page wpanManager_desc WPAN Manager
 * # Description
 * The wpanManager class is a multi-part master handler for a WPAN.
 *
 * It handles:
 * - radio module initialization in gateway mode
 * - radio module ticks
 * - various @subpage nodes lists
 * - dynamic address definition
 * - packet reception and subsequent dispatching to node class instances for
 * processing
 *
 * # Usage
 * ## Initialization
 * Initialize with constructor and relevant parameters depending on application.
 * ISM3 module does not need to be initialized beforehand. At the moment, static
 * node addresses can only be provided at startup.
 * ## Data handling
 * The instance's master rxHandler has to be called from the relevant
 * ism3_handler functions (unicast and multicast) and be passed the data and
 * source address. This RX handler distributes handling to Node type instances.
 * It also handles not-yet-instantiated nodes before it attributes them an
 * address (see DORA description)
 * ## Addressing
 * ### Static addressing
 * WPAN manager can be instantiated with a static Node list. Nodes should
 * be instantiated with relevant address and group.
 * ### Dynamic addressing (DORA)
 * DORA stands for Discover, Offer, Request, Acknowledge. This is a classic
 * way to describe DHCP operation. For more information, consult the
 * <a href="https://en.wikipedia.org/wiki/Dynamic_Host_Configuration_Protocol#Operation"> DHCP Wikipedia page</a>
 * or @subpage dora_protocol.
 * The present dynamic address attribution is a custom implementation of the
 * Wikipedia excerpt. Protocol details are covered in wpan.h. All frames are
 * transmitted as broadcasts until client node receives an ACK frame from
 * gateway.
 *
 *
 * ## Radio module initialization
 * WPAN manager uses ism3_server.h to configure
 * gateway radio at desired or default power.
 *
 * Radio module ticks: WPAN manager tick implements a sleep-delayed tick function
 * that calls the ISM3 driver tick function.
 *
 * Node lists: There are several overlapping node lists. As of now, WPAN manager
 * maintains 2*n separate lists, n being the number of different node types
 * (Base, Data, Power). Statically and dynamically-addressed nodes are kept in
 * different node lists. This implementation should be changed to a node type
 * -agnostic implementation to simplify code and execution.
 * Handles radio module init, ticks, address attribution and packet redirection
 * to client node handlers (Node class and sub-classes). Exports a list of
 * connected nodes to be used elsewhere
 */
/**
 * @brief Offer storage data coupling
 *
 * sOffer allows storage of a pair of address and UID hash. It is used
 * to store DORA offers
 */
typedef struct{
    uint8_t address; ///< offered address
    uint8_t uidHash; ///< UID has the address was offered to
}sOffer;

using namespace std;

class wpanManager
{
public:
    /* CONSTRUCTORS ***********************************************************/
    /**
     * @brief Default constructor
     *
     * Init with default server power and no static nodes
     */
    wpanManager();
    /**
     * @brief Static address list constructor
     * @param nodeList_ static node vector
     * Init with default server power
     */
    wpanManager(vector<Node> nodeList_);
    /**
     * @brief Power setting constructor
     * @param power_ power setting
     * @param power_dbm_ matching dBm
     *
     * Init with custom server power and no static nodes
     */
    wpanManager(uint8_t power_, uint8_t power_dbm_);
    /**
     * @brief Complete constructor
     * @param nodeList_ static node vector
     * @param power_ power setting
     * @param power_dbm_ matching dBm
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
     *
     * Send disconnect command to all nodes to notify them
     */
    void clearNodeLists();
    /**
     * @brief Clear dynamically-addressed nodes
     *
     * Send disconnect command to deleted nodes to notify them
     */
    void clearDynamicNodeList();
    /**
     * @brief Clear statically-addressed nodes
     *
     * Send disconnect command to deleted nodes to notify them
     */
    void clearStaticNodeList();
    /* COMMANDS ***************************************************************/
    /**
     * @brief ticker for WPAN manager
     * @param delayMs_ tick delay in ms
     *
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
     *
     * Put dynamic discovery group to sleep
     */
    void stopDynamicDiscovery();
    /**
     * @brief update node types from nodes list
     *
     * Ask non-identified base nodes for their supported protocols.
     */
    void updateNodeTypes();

    /* UTILITIES **************************************************************/
    /**
     * @brief print node list
     *
     * Prints statically and dynamically-addressed nodes
     */
    void printNodes();
    /**
     * @brief print static node list
     *
     * Prints statically-addressed nodes
     */
    void printStaticNodes();

    /* HANDLERS ***************************************************************/
    /**
     * @brief handle RX data
     * @param data pointer to RX data
     * @param size RX data size
     * @param source data source address
     *
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
     *
     * Clears node pointer vectors and rebuilds them according to new instances
     * stored in base, power and data nodes vector
     */
    void rebuildStaticNodeLists();
    /**
     * @brief Rebuild dynamically-addressed node lists
     *
     * Clears node pointer vectors and rebuilds them according to new instances
     * stored in base, power and data nodes vector
     */
    void rebuildDynNodeLists();
    /**
     * @brief update node types from dynamicNodes list
     *
     * Ask dynamically-addressed nodes for their supported protocols
     */
    void updateDynamicNodeTypes();
    /**
     * @brief update static node types from staticNodes list
     *
     * Ask nodes for their supported protocols
     */
    void updateStaticNodeTypes();
    /**
     * @brief update node type after receiving net_getProtocols answer
     *
     * Triggers a node list rebuild
     */
    void updateNodeTypesCallback();
    /* DORA METHODS ***********************************************************/
    /**
     * @brief Check node list for expired leases
     *
     * Sets a timer for next lease expiry check
     */
    void checkLeaseExpiry();
    /**
     * @brief Handler for DORA frames
     * @param data pointer to RX data
     * @param size RX data size
     *
     * Master handler for DORA dynamic addressing. Handées all DORA frames
     */
    uint8_t dora(const uint8_t* data, uint8_t size);
    /**
     * @brief Store a DORA address offer
     * @param address offered address
     * @param uid pointer to target UID
     *
     * Store address and uid in addressOffers and offerUidHashes vectors.
     * UID is stored as a hash for simplicity
     */
    void storeOffer(uint8_t address, uint8_t * uid);
    /**
     * @brief Check if DORA address was offered to UID
     * @param address address to check
     * @param uid pointer to UID to check
     * @return true if address was offered to uid
     *
     * Check if the address, uid pair is in addressOffers and offerUidHashes
     * vectors. If it is in both at the same index, return true and erase offer
     * from vectors. Otherwise return false
     */
    bool checkOfferMatch(uint8_t address, uint8_t * uid);
    /**
     * @brief hash a uid
     * @param uid pointer to uid
     * @return hashed value
     *
     * Apply a XOR operation to NETWORK_UID8_WIDTH bytes starting from pointer
     * Does not check proper buffer width, use wisely
     */
    uint8_t hashUid(uint8_t * uid);
    /**
     * @brief Build a DORA frame from arguments
     * @param buffer pointer to buffer to fill
     * @param maxSize buffer max size
     * @param cmd command to send
     * @param destUID destination UID pointer (data must be of NETWORK_UID8_WIDTH)
     * @param data pointer to DORA command data
     * @param dataSize DORA command data size
     * @return built frame length
     *
     * Builds a DORA frame as specified in wpan.h
     * Input UID size is not checked, use wisely.
     */
    uint8_t buildDoraFrame(uint8_t * buffer, uint8_t maxSize, uint8_t cmd, uint8_t * destUID, uint8_t * data, uint8_t dataSize);

    /* VARIABLES **************************************************************/
    uint8_t txpower;        ///< Server TX power
    uint8_t txpower_dbm;    ///< Server dBm TX power equivalent
    uint32_t awakeGroups=0; ///< To restore awake groups after a special command
    bool newNodeList=false; ///< Indicate node list has been updated
    /* TICK VARIABLES *********************************************************/
    const uint32_t leaseExpiryCheckPeriodS=NETWORK_LEASE_UNIT_MINUTES*60/2;
    ///< Lease expiry check period in seconds
    uint32_t nextLeaseExpiryCheckS=0; ///< Next lease expiry check timestamp
    const uint32_t nodeTypeUpdatePeriodS=60;
    ///< Node type update period in seconds
    uint32_t nextNodeTypeUpdateS=0;   ///< Next node type update timestamp
    bool updateNodeTypesFlag=false;   ///< Flag for node type update
    /* DORA VARIABLES *********************************************************/
    uint8_t leaseDuration=1;            ///< Default lease duration
    uint8_t uid[NETWORK_UID8_WIDTH];    ///< Server UID
    vector<sOffer> offers;              ///< Current DORA offers
    /* NODE LISTS *************************************************************/
    vector<Node*> pNodes;               ///< Static+dynamically-addressed nodes

    vector<Node*> pStaticNodes;         ///< Static node pointer array
    vector<Node*> pDynNodes;            ///< Pointers for dynamic nodes

    // Statically-addressed typed nodes
    vector<Node> staticBaseNodes;       ///< base nodes, ideally empty
    vector<PowerNode> staticPowerNodes; ///< identified power nodes
    vector<DataNode> staticDataNodes;   ///< identified data nodes

    // Dynamically-addressed typed nodes
    vector<Node> dynBaseNodes;          ///< base nodes, ideally empty
    vector<PowerNode> dynPowerNodes;    ///< identified power nodes
    vector<DataNode> dynDataNodes;      ///< identified data nodes
};

#endif // WPANMANAGER_H
