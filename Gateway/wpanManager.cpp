#include "wpanManager.h"

// Call constructor with power initialization with default values from ism server
wpanManager::wpanManager() :
wpanManager{ism_server_get_power(),ism_server_get_power_dbm()} {}

// Main constructor
wpanManager::wpanManager(uint8_t power_, uint8_t power_dbm_)
{
    txpower=power_;
    txpower_dbm=power_dbm_;
    ism_server_init(txpower,txpower_dbm);
}

// Call empty constructor and initialize node list after
wpanManager::wpanManager(vector<Node*> nodeList_) : wpanManager() {
    nodes=nodeList_;
    staticNodes=nodeList_;
#ifdef DEBUG_WPANMANAGER
    cout<<"WPAN manager node vector size: "<<nodes.size()<<endl;
    cout<<"Content: "<<endl;
    printNodes();
#endif
}

// Call main constructor and initialize node list after
wpanManager::wpanManager(vector<Node*> nodeList_,
                         uint8_t power_,
                         uint8_t power_dbm_) : wpanManager{power_, power_dbm_}
{
    nodes=nodeList_;
    staticNodes=nodeList_;
#ifdef DEBUG_WPANMANAGER
    cout<<"WPAN manager node vector size: "<<nodes.size()<<endl;
    cout<<"Content: "<<endl;
    printNodes();
#endif
}


vector<Node*> wpanManager::getNodeList()
{
    return nodes;
}

vector<Node*> wpanManager::getStaticNodeList()
{
    return staticNodes;
}

void wpanManager::rxHandler(const uint8_t* data, uint8_t size, uint8_t source){
    bool found=false;
    uint8_t nodeAddr=0;

#ifdef DEBUG_WPANMANAGER
    cout<<"RX HANDLER"<<endl;
    cout<<"Source: "<<to_string(source)<<endl;
    cout<<"Data: ";
    printBufferHex(data, size);
#endif

    // Iterate through list to find node that sent the packet
    for(uint8_t i=0; i<nodes.size(); i++){
        nodeAddr=nodes[i]->getAddr();

#ifdef DEBUG_WPANMANAGER
        cout<<"Checking node "<<to_string(i)<<", address "<<to_string(nodeAddr)<<endl;
#endif

        if(nodeAddr==source){
            nodes[i]->rxCallback(data,size);
            found=true;

#ifdef DEBUG_WPANMANAGER
            cout<<"Found destination node"<<endl;
#endif
        }
    }
    /*
    // If none found, it may come from a node's old address
    // In this case, try to set the node's address again
    if(!found){
        rxHandler_oldAddr(data, size, source);
    }*/
}


void wpanManager::rxHandler_oldAddr(const uint8_t* data, uint8_t size, uint8_t source){
    for(uint8_t i=0; i<nodes.size(); i++){
        // If source matches an old address
        if(nodes[i]->getOldAddr()==source){
            // execute RX callback as usual
            nodes[i]->rxCallback(data,size);
            if(!(nodes[i]->net_setAddrAgain(maxPingRetries, netCmdTimeout))){
                // If all pings were unsuccessful
                // find and remove node from connectedNodes list
// TODO fix the whole connected/unconnected/nodes nonsensical mess
                vector<Node*>::iterator it;
                it = find(connectedNodes.begin(),connectedNodes.end(),nodes[i]);
                if(it!=connectedNodes.end()){
                    connectedNodes.erase(it);
                }
            }
        }
    }
}
// TODO write?
// ping all nodes and remove unreachable nodes
uint8_t wpanManager::updateConnectedNodeList(){
    return 0;
}


void wpanManager::printNodes(){
    for(uint16_t i=0; i<nodes.size(); i++){
        cout<<to_string(i+1)<<" - "<<*(nodes[i])<<endl;
    }
}




/*
wpanManager::~wpanManager()
{

}*/
/*
uint8_t wpanManager::staticDiscovery(vector<Node> nodelist){

    return connectedNodes.size();
}*/
/*
uint8_t Router::discoverNodes(){
    // Broadcast search message on beacon data
    // TODO implement dynamic discovery
    return 0;
}

uint8_t Router::connectNodes(){

    return 0;
}

bool Router::connectNode(Node){

    return true;
}
*/
