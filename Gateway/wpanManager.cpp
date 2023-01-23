#include "wpanManager.h"

// Call constructor with power initialization with default values from ism server
wpanManager::wpanManager() :
wpanManager{ism_server_get_power(),ism_server_get_power_dbm()} {}

// Main constructor
wpanManager::wpanManager(uint8_t power_, uint8_t power_dbm_){
#ifdef SHOW_TASKS
    cout<<"WPAN manager: init"<<endl;
#endif
    txpower=power_;
    txpower_dbm=power_dbm_;
    ism_server_init(txpower,txpower_dbm);
    ism_get_uid(uid, ISM_UID_SIZE);
}

// Call empty constructor and initialize node list after
wpanManager::wpanManager(vector<Node> nodeList_) : wpanManager{nodeList_,ism_server_get_power(),ism_server_get_power_dbm()} {}

// Call main constructor and initialize node list after
wpanManager::wpanManager(vector<Node> nodeList_,
                         uint8_t power_,
                         uint8_t power_dbm_) : wpanManager{power_, power_dbm_}{
    staticBaseNodes=nodeList_;
    for(uint8_t i=0; i<staticBaseNodes.size(); i++){
      pStaticNodes.push_back(&staticBaseNodes.at(i));
    }
    pNodes=pStaticNodes;
#ifdef DEBUG_WPANMANAGER
    cout<<"WPAN manager: init static nodes: "<<endl;
    printNodes();
#endif
}


vector<Node*> wpanManager::getNodeList(){
    return pNodes;
}

vector<Node*> wpanManager::getStaticNodeList(){
    return pStaticNodes;
}

vector<PowerNode*> wpanManager::getPowerNodeList(){
    vector<PowerNode*> retVec;
    for(int i=0;i<staticPowerNodes.size();i++){
        retVec.push_back(&staticPowerNodes.at(i));
    }
    for(int i=0;i<dynPowerNodes.size();i++){
        retVec.push_back(&dynPowerNodes.at(i));
    }
    return retVec;
}

vector<DataNode*> wpanManager::getDataNodeList(){
    vector<DataNode*> retVec;
    for(int i=0;i<staticDataNodes.size();i++){
        retVec.push_back(&staticDataNodes.at(i));
    }
    for(int i=0;i<dynDataNodes.size();i++){
        retVec.push_back(&dynDataNodes.at(i));
    }
    return retVec;
}

bool wpanManager::nodeListUpdated(){
    if(newNodeList){
        // Flag reset
        newNodeList=false;
        return true;
    }
    return newNodeList;
}

void wpanManager::clearNodeLists(){
#ifdef SHOW_TASKS
    cout<<"WPAN manager: disconnect all nodes"<<endl;
#endif
    clearDynamicNodeList();
    clearStaticNodeList();
    pNodes.clear();
}

void wpanManager::clearDynamicNodeList(){
    ism_server_wakeup_group(0xFFFFFFFF);
    tick(20);
    for(int i=0; i<pDynNodes.size(); i++){
#ifdef DEBUG_WPANMANAGER
        cout<<"Disconnecting node with address ";
        cout<<to_string(pDynNodes.at(i)->getAddr());
        cout<<", group ";
        cout<<to_string(pDynNodes.at(i)->getGroup())<<endl;
#endif
        pDynNodes.at(i)->wakeup();
        tick(20);
        pDynNodes.at(i)->net_disconnect(0);
        tick(20);

    }
    pDynNodes.clear();
    dynBaseNodes.clear();
    dynPowerNodes.clear();
    dynDataNodes.clear();
    newNodeList=true;
}

void wpanManager::clearStaticNodeList(){
    ism_server_wakeup_group(0xFFFFFFFF);
    tick(20);
    for(int i=0; i<pStaticNodes.size(); i++){
        pStaticNodes.at(i)->wakeup();
        tick(20);
        pStaticNodes.at(i)->net_disconnect(0);
        tick(20);
    }
    pStaticNodes.clear();
    staticBaseNodes.clear();
    staticPowerNodes.clear();
    staticDataNodes.clear();
    newNodeList=true;
}

void wpanManager::tick(uint32_t delayMs_){
#ifdef DEBUG_TICKS
    cout<<"WPAN manager: TICK"<<endl;
#endif
    ism_tick();
    delayMs(delayMs_);
    uint32_t currentTimeS = ((uint32_t)time(NULL));
    if(updateNodeTypesFlag){
        updateNodeTypesCallback();
    }
    // Check for expired nodes regularly
    if(currentTimeS>=nextLeaseExpiryCheckS){
        checkLeaseExpiry();
    }
    if(currentTimeS>=nextNodeTypeUpdateS){
        updateNodeTypes();
    }
    ism_tick();
}

void wpanManager::startDynamicDiscovery(){
#ifdef SHOW_TASKS
    cout<<"WPAN manager: Starting dynamic discovery"<<endl;
#endif
    ism_server_wakeup_group(NETWORK_NACK_GROUP);
}

void wpanManager::stopDynamicDiscovery(){
#ifdef SHOW_TASKS
    cout<<"WPAN manager: Stopping dynamic discovery"<<endl;
#endif
    ism_server_unwake_group(NETWORK_NACK_GROUP);
}

void wpanManager::updateNodeTypes(){
#ifdef SHOW_TASKS
    cout<<"WPAN Manager: update Node types"<<endl;
#endif
    nextNodeTypeUpdateS = ((uint32_t)time(NULL))+nodeTypeUpdatePeriodS;
    awakeGroups=ism_server_get_awake();
    // Wake up all groups but DORA group
    ism_server_wakeup_group(~NETWORK_DORA_GROUP);
    updateNodeTypesFlag=false;
    updateStaticNodeTypes();
    updateDynamicNodeTypes();
}

void wpanManager::printNodes(){
    if(pNodes.empty()){
        cout<<"Node list empty"<<endl;
    }else{
        for(int i=0; i<pNodes.size(); i++){
            cout<<to_string(i+1)<<" - ";
            pNodes.at(i)->show();
            cout<<endl;
        }
    }
}

void wpanManager::printStaticNodes(){
    for(int i=0; i<pStaticNodes.size(); i++){
        cout<<to_string(i+1)<<" - ";
        pStaticNodes[i]->show();
        cout<<endl;
    }
}

void wpanManager::rxHandler(const uint8_t* data, uint8_t size, uint8_t source){
    uint8_t nodeAddr=0;

#ifdef DEBUG_WPANMANAGER
    cout<<"WPAN Manager: RX handler"<<endl;
    cout<<"Source: "<<to_string(source)<<endl;
    cout<<"Data: ";
    printBufferHex(data, size);
#endif
    if(size>=2 && data[0]==NETWORK_PROTOCOL_ID &&
        (data[1]==NETWORK_DISCOVER||data[1]==NETWORK_REQUEST) ){
#ifdef DEBUG_DORA
        cout<<"DORA function called"<<endl;
#endif
        dora(data+1,size-1);
        return;
    }
    // Iterate through list to find node that sent the packet
    for(uint8_t i=0; i<pNodes.size(); i++){
        nodeAddr=pNodes[i]->getAddr();
        if(nodeAddr==source){
            pNodes[i]->rxCallback(data,size);
            if(data[0]==NETWORK_PROTOCOL_ID&&data[1]==NETWORK_GET_PROTOCOLS)
                updateNodeTypesFlag=true;

#ifdef DEBUG_WPANMANAGER
            cout<<"WPAN Manager: found destination node"<<endl;
#endif
        }
    }
}

/* PRIVATE METHODS ************************************************************/
void wpanManager::rebuildNodeLists(){
    rebuildStaticNodeLists();
    rebuildDynNodeLists();
#ifdef DEBUG_WPANMANAGER
    printNodes();
#endif
}

void wpanManager::rebuildStaticNodeLists(){
    // Remove existing pointers from pNodes
    for(int i=0; i<pStaticNodes.size(); i++){
        for(int j=pNodes.size()-1; j>=0; j--){
            if(pNodes.at(j)==pStaticNodes.at(i)){
                pNodes.erase(pNodes.begin()+j);
            }
        }
    }
    // Clear pStaticNodes array
    pStaticNodes.clear();

    // Rebuild pStaticNodes array
    for(int i=0;i<staticBaseNodes.size();i++){
        pStaticNodes.push_back(&(staticBaseNodes.at(i)));
    }
    for(int i=0;i<staticPowerNodes.size();i++){
        pStaticNodes.push_back(&(staticPowerNodes.at(i)));
    }
    for(int i=0;i<staticDataNodes.size();i++){
        pStaticNodes.push_back(&(staticDataNodes.at(i)));
    }

    // Refill pNodes array
    for(int i=0;i<pStaticNodes.size();i++){
        pNodes.push_back(pStaticNodes.at(i));
    }
    newNodeList=true;
}

void wpanManager::rebuildDynNodeLists(){
    // Remove existing pointers from pNodes
    for(int i=0; i<pDynNodes.size(); i++){
        for(int j=pNodes.size()-1; j>=0; j--){
            if(pNodes.at(j)==pDynNodes.at(i)){
                pNodes.erase(pNodes.begin()+j);
            }
        }
    }
    // Clear pDynNodes array
    pDynNodes.clear();

    // Rebuild pDynNodes array
    for(int i=0;i<dynBaseNodes.size();i++){
        pDynNodes.push_back(&(dynBaseNodes.at(i)));
    }
    for(int i=0;i<dynPowerNodes.size();i++){
        pDynNodes.push_back(&(dynPowerNodes.at(i)));
    }
    for(int i=0;i<dynDataNodes.size();i++){
        pDynNodes.push_back(&(dynDataNodes.at(i)));
    }

    for(int i=0;i<pDynNodes.size();i++){
        pNodes.push_back(pDynNodes.at(i));
    }
    newNodeList=true;
}

void wpanManager::updateDynamicNodeTypes(){
    // Send net_getprotocols to all nodes
#ifdef DEBUG_NODETYPES
        cout<<"Send get protocols to all unknown dynamic nodes"<<endl;
#endif
    for(int i=0; i<dynBaseNodes.size(); i++){
#ifdef DEBUG_NODETYPES
        cout<<"Unknown dynamic node ";
        dynBaseNodes.at(i).show();
        cout<<endl;
#endif
        dynBaseNodes.at(i).net_getProtocols(0);
        tick(20);
    }
}

void wpanManager::updateStaticNodeTypes(){
    // Send net_getprotocols to all nodes
#ifdef DEBUG_NODETYPES
        cout<<"Send get protocols to all unknown static nodes"<<endl;
#endif
    for(int i=0; i<staticBaseNodes.size(); i++){
#ifdef DEBUG_NODETYPES
        cout<<"Unknown static node ";
        staticBaseNodes.at(i).show();
        cout<<endl;
#endif
        staticBaseNodes.at(i).net_getProtocols(0);
        tick(20);
    }
}

void wpanManager::updateNodeTypesCallback(){
    uint8_t nodeProtocols=0;
    uint8_t nodeTypeProtocols=0;
    updateNodeTypesFlag=false;


    // Block nodes from sending data during list update
    ism_server_unwake_groups();

    // Dynamic nodes
    // Reassign nodes so that class matches protocols
    for(int i=dynBaseNodes.size()-1; i>=0; i--){
        nodeProtocols=dynBaseNodes.at(i).getProtocols();
        nodeTypeProtocols=dynBaseNodes.at(i).getNodeTypeProtocols();
#ifdef DEBUG_NODETYPES
        cout<<"Find type for ";
        dynBaseNodes.at(i).show();
        cout<<endl;
        cout<<"Protocols for node type: "<<to_string(nodeTypeProtocols)<<endl;
        cout<<"Node has announced protocols: "<<to_string(nodeProtocols)<<endl;
#endif
        if(nodeProtocols!=nodeTypeProtocols){
            switch(nodeProtocols){
                case NETWORK_PROTOCOL_ID|DATA_PROTOCOL_ID:
                    // DataNode
#ifdef DEBUG_NODETYPES
                    cout<<"Move node "<<to_string(i)<<" to DataNodes"<<endl;
#endif
                    dynDataNodes.emplace_back(dynBaseNodes.at(i));
                    dynBaseNodes.erase(dynBaseNodes.begin()+i);
                    break;
                case NETWORK_PROTOCOL_ID|APP_PROTOCOL_ID|APP_ERR_PROTOCOL_ID:
                    // PowerNode
#ifdef DEBUG_NODETYPES
                    cout<<"Move node "<<to_string(i)<<" to PowerNodes"<<endl;
#endif
                    dynPowerNodes.emplace_back(dynBaseNodes.at(i));
                    dynBaseNodes.erase(dynBaseNodes.begin()+i);
                break;
                case NETWORK_PROTOCOL_ID|APP_PROTOCOL_ID|APP_ERR_PROTOCOL_ID|DATA_PROTOCOL_ID:
                    // Data Node
#ifdef DEBUG_NODETYPES
                    cout<<"Move node "<<to_string(i)<<" to DataNodes"<<endl;
#endif
                    dynDataNodes.emplace_back(dynBaseNodes.at(i));
                    dynBaseNodes.erase(dynBaseNodes.begin()+i);
                break;
                default:
                    // Keep node in dynBaseNodes
                break;
            }
        }
    }

    // Static nodes
    // Reassign nodes so that class matches protocols
    for(int i=staticBaseNodes.size()-1; i>=0; i--){
        nodeProtocols=staticBaseNodes.at(i).getProtocols();
        nodeTypeProtocols=staticBaseNodes.at(i).getNodeTypeProtocols();
#ifdef DEBUG_NODETYPES
        cout<<"Find type for ";
        staticBaseNodes.at(i).show();
        cout<<endl;
        cout<<"Protocols for node type: "<<to_string(nodeTypeProtocols)<<endl;
        cout<<"Node has announced protocols: "<<to_string(nodeProtocols)<<endl;
#endif
        if(nodeProtocols!=nodeTypeProtocols){
            newNodeList=true;
            switch(nodeProtocols){
                case NETWORK_PROTOCOL_ID|DATA_PROTOCOL_ID:
                    // DataNode
#ifdef DEBUG_NODETYPES
                    cout<<"Move node "<<to_string(i)<<" to DataNodes"<<endl;
#endif
                    staticDataNodes.push_back(staticBaseNodes.at(i));
                    staticBaseNodes.erase(staticBaseNodes.begin()+i);
                    break;
                case NETWORK_PROTOCOL_ID|APP_PROTOCOL_ID|APP_ERR_PROTOCOL_ID:
                    // PowerNode
#ifdef DEBUG_NODETYPES
                    cout<<"Move node "<<to_string(i)<<" to PowerNodes"<<endl;
#endif
                    staticPowerNodes.push_back(staticBaseNodes.at(i));
                    staticBaseNodes.erase(staticBaseNodes.begin()+i);
                break;
                case NETWORK_PROTOCOL_ID|APP_PROTOCOL_ID|APP_ERR_PROTOCOL_ID|DATA_PROTOCOL_ID:
                    // Data Node
#ifdef DEBUG_NODETYPES
                    cout<<"Move node "<<to_string(i)<<" to DataNodes"<<endl;
#endif
                    staticDataNodes.push_back(staticBaseNodes.at(i));
                    staticBaseNodes.erase(staticBaseNodes.begin()+i);
                break;
                default:
                    // Keep node in staticBaseNodes
                break;
            }
        }
    }

    rebuildNodeLists();

    // Put groups back to previous state
    ism_server_wakeup_group(awakeGroups);
}

/* DORA METHODS ***************************************************************/
void wpanManager::checkLeaseExpiry(){
#ifdef SHOW_TASKS
    cout<<"WPAN manager: Checking lease expiry"<<endl;
#endif
    nextLeaseExpiryCheckS = ((uint32_t)time(NULL))+leaseExpiryCheckPeriodS;
    awakeGroups=ism_server_get_awake();
    ism_server_wakeup_group(~NETWORK_DORA_GROUP);
    tick(100);
    for(int i=pNodes.size()-1;i>0;i--){
        // If leaseDuration is over 0 -> dynamic addressing and expired
        if(pNodes.at(i)->getLeaseDuration()>0 &&
            !pNodes.at(i)->isLeaseValid()){
            pNodes.at(i)->net_disconnect(0);
            tick(10);
#ifdef DEBUG_DORA
            cout<<"DORA: Lease expired, deleting Node: ";
            pNodes.at(i)->show();
            cout<<endl;
#endif
            newNodeList=true;
            for(int j=0; j<dynBaseNodes.size(); j++){
                if(pNodes.at(i)==&(dynBaseNodes.at(j))){
                    dynBaseNodes.erase(dynBaseNodes.begin()+j);
                }
            }
            for(int j=0; j<dynPowerNodes.size(); j++){
                if(pNodes.at(i)==&(dynPowerNodes.at(j))){
                    dynPowerNodes.erase(dynPowerNodes.begin()+j);
                }
            }
            for(int j=0; j<dynDataNodes.size(); j++){
                if(pNodes.at(i)==&(dynDataNodes.at(j))){
                    dynDataNodes.erase(dynDataNodes.begin()+j);
                }
            }
            pNodes.erase(pNodes.begin()+i);
        }
    }
    ism_server_unwake_groups();
    ism_server_wakeup_group(awakeGroups);
}

uint8_t wpanManager::dora(const uint8_t* data, uint8_t size){
    /*
     * DHCP-like registration:
        Unregistered clients are in group 0x80000000 by default
        Clients have a random default address between 0xF0 and 0xFE
        Server wakes unregistered group
        Clients broadcast a message asking for addresses, giving their uids
        Server broadcasts a message assigning an address to a uid
        Client gets message, compares with its uid and sets address if correct
    */
    /* Frame data:
     * NETWORK_PROTOCOL_ID
     * NETWORK_CMD
     * source UID
     * destination UID
     * NETWORK_CMD_DATA
     *
     * DATA per command
     * DISCOVER:    none
     * OFFER:       address
     * REQUEST:     address     group
     * ACK:         address     group   lease duration (0 if NACK)
     */
    uint8_t dataIndex=0;
    uint8_t dataSize=size;
    uint8_t sourceUID[NETWORK_UID8_WIDTH];
    uint8_t destinationUID[NETWORK_UID8_WIDTH];
    uint8_t cmd;
    uint8_t frame[NETWORK_DORA_MAX_FRAME_SIZE]={0};
    uint8_t frameLength=0;
    uint8_t addr=0; // node temp address
    uint8_t newAddr=1; // Default for OFFER command
    uint8_t newLease=leaseDuration;
    uint32_t group=NETWORK_NACK_GROUP;
    uint8_t cmdData[sizeof(newAddr)+sizeof(group)+sizeof(leaseDuration)]={0};
    uint8_t cmdDataLength=0;
    bool addrUsed=false;

#ifdef DEBUG_DORA_FRAMES
            cout<<"DORA RX handler: "<<endl;
            cout<<"Data: ";
            printBufferHex(data, size);
#endif
    if(dataSize>=1){
        cmd=data[dataIndex];
        dataIndex++;
        dataSize--;
    }else{
        cmd=0;
    }
    // Decode source UID regardless of destination UID
    if(dataSize>=NETWORK_UID8_WIDTH){
        memcpy(sourceUID, &data[dataIndex], NETWORK_UID8_WIDTH);
        dataIndex+=NETWORK_UID8_WIDTH;
        dataSize-=NETWORK_UID8_WIDTH;
    }
    if(dataSize>=NETWORK_UID8_WIDTH){
        memcpy(destinationUID, &data[dataIndex], NETWORK_UID8_WIDTH);
        dataIndex+=NETWORK_UID8_WIDTH;
        dataSize-=NETWORK_UID8_WIDTH;
    }

    ism_server_wakeup_group(NETWORK_DORA_GROUP);
    ism_tick();
    switch(cmd){
        case NETWORK_DISCOVER:

#ifdef DEBUG_DORA
            cout<<"DORA: RX discover"<<endl;
#endif
            // If DISCOVER command
            // Find first free address in nodes vector
            for(newAddr=1;newAddr<255;newAddr++){
                // Compare newAddr with all existing addresses
                addrUsed=false;
                for(int i=0;i<pNodes.size();i++){
                    addr=pNodes.at(i)->getAddr();
                    if(newAddr==addr){
                        // If address is used, try next address
                        addrUsed=true;
                        break;
                    }
                }
                if(!addrUsed){
                    // If newAddr is not used in the node list, use it
                    break;
                }
            }

            // OFFER free address
            cmdData[0]=newAddr;
            cmdDataLength++;
            frameLength=buildDoraFrame(frame,ISM_MAX_DATA_SIZE, NETWORK_OFFER, sourceUID, cmdData, cmdDataLength);
            storeOffer(newAddr, sourceUID);
            // Broadcast offer
#ifdef DEBUG_DORA
            cout<<"DORA: TX offer address "<<to_string(newAddr)<<endl;
#endif
            ism_broadcast(NETWORK_DORA_GROUP, 1, frame, frameLength);
            break;
        case NETWORK_REQUEST:
#ifdef DEBUG_DORA
            cout<<"DORA: RX request "<<endl;
#endif
            // If REQUEST command
            // Get address, group
            if(dataSize>=(sizeof(newAddr)+sizeof(group))){
                addr=data[dataIndex];
                dataIndex++;
                memcpy(&group, &data[dataIndex], NETWORK_GROUP_WIDTH);
                dataIndex+=NETWORK_GROUP_WIDTH;
            }
            // Check destination UID matches own UID
            // If no match, override with default values
            if(!checkOfferMatch(addr,sourceUID)){
                // Build frame with NACK address, NACK group and 0 lease
                addr=NETWORK_NACK_BASE_ADDR+offers.size();
                group=NETWORK_DORA_GROUP;
                newLease=0;
            }
            // Fill answer
            cmdData[0]=addr;
            memcpy(&cmdData[NETWORK_ADDR_WIDTH],&group,NETWORK_GROUP_WIDTH);
            cmdData[NETWORK_ADDR_WIDTH+NETWORK_GROUP_WIDTH]=newLease;
            cmdDataLength+=NETWORK_ADDR_WIDTH+NETWORK_GROUP_WIDTH+NETWORK_LEASE_WIDTH;
            // Send ACK message with lease duration
            frameLength=buildDoraFrame(frame,ISM_MAX_DATA_SIZE, NETWORK_ACK, sourceUID, cmdData, cmdDataLength);
#ifdef DEBUG_DORA
            cout<<"DORA: TX ack for address "<<to_string(addr)<<", group "<<to_string(group)<<", lease duration "<<to_string(leaseDuration)<<endl;
#endif
            ism_broadcast(NETWORK_DORA_GROUP, 1, frame, frameLength);

            // If ACK, Create node with new address and group
            if(newLease>0){
                 // Register node in dynBaseNodes array
                dynBaseNodes.emplace_back(addr, group, newLease);
                // Add reference to
                pNodes.push_back(&(dynBaseNodes.back()));
                pDynNodes.push_back(&(dynBaseNodes.back()));
                pNodes.back()->wakeup();
                pNodes.back()->net_getProtocols(0);
#ifdef SHOW_TASKS
                cout<<"WPAN Manager: New connection: ";
                pNodes.back()->show();
                cout<<endl;
#endif
            }
            break;
        default:
            // NETWORK_DORA_ERR message, no data
#ifdef DEBUG_DORA
            cout<<"DORA: RX unspecified command, TX error"<<endl;
#endif
            cmdDataLength=0;
            frameLength=buildDoraFrame(frame,ISM_MAX_DATA_SIZE, NETWORK_DORA_ERR, sourceUID, cmdData, cmdDataLength);
            ism_broadcast(NETWORK_DORA_GROUP, 1, frame, frameLength);
            break;
    }

    return 0;
}

void wpanManager::storeOffer(uint8_t address, uint8_t * uid){
    sOffer offer={address,hashUid(uid)};
    offers.push_back(offer);
}

bool wpanManager::checkOfferMatch(uint8_t address, uint8_t * uid){
    uint8_t hash=hashUid(uid);
    sOffer offer;

    for(uint8_t i=0;i<offers.size();i++){
        offer=offers.at(i);
        // If match is found
        if(hash==offer.uidHash && address==offer.address){
            // Delete stored pair and return true
            offers.erase(offers.begin()+i);
            return true;
        }
    }
    return false;
}

uint8_t wpanManager::hashUid(uint8_t* uid){
    uint8_t hash=0;
    for(uint8_t i=0;i<NETWORK_UID8_WIDTH;i++){
        hash^=uid[i];
    }
    return hash;
}

uint8_t wpanManager::buildDoraFrame(uint8_t * buffer, uint8_t maxSize, uint8_t cmd, uint8_t * destUID, uint8_t * data, uint8_t dataSize){
    /* Frame data:
     * NETWORK_PROTOCOL_ID
     * NETWORK_CMD
     * source UID
     * destination UID
     * NETWORK_CMD_DATA
     */
    uint8_t frameLength=0;

    // Check buffer is big enough
    if(maxSize>=NETWORK_MAX_FRAME_SIZE){
        // Network protocol ID
        buffer[0]=NETWORK_PROTOCOL_ID;
        // Network cmd
        buffer[1]=cmd;
        frameLength+=2;
#ifdef DEBUG_DORA_FRAMES
            cout<<"DORA: Build frame"<<endl;
            cout<<"Source UID: ";
            printBufferHex(uid, NETWORK_UID8_WIDTH);
            cout<<"Destination UID: ";
            printBufferHex(destUID, NETWORK_UID8_WIDTH);
            cout<<"Command data: ";
            printBufferHex(data, dataSize);
#endif
        // Source UID
        memcpy(&buffer[frameLength],uid,NETWORK_UID8_WIDTH);
        frameLength+=NETWORK_UID8_WIDTH;
        // Dest UID
        memcpy(&buffer[frameLength],destUID,NETWORK_UID8_WIDTH);
        frameLength+=NETWORK_UID8_WIDTH;
        // Cmd data
        memcpy(&buffer[frameLength],data,dataSize);
        frameLength+=dataSize;
    }

    return frameLength;
}
