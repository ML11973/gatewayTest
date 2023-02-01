/*
 * RoutingManager.cpp
 *
 *  Created on: Aug 29, 2022
 *      Author: leemarc
 */

#include "BorderRouter.h"

eBorderRouterStatus BorderRouter::init(wpanManager * _wpan) {
	wpan=_wpan;
	status = NOT_INIT;
	// Get server address list
	BorderRouter::updateRoutes();
#ifdef DEBUG_BORDERROUTER
	printf("Border router: init ok\n");
#endif
	return status;
}

void BorderRouter::deInit(){
	// This closes sockets from routes destructor
	routes.clear();
	status = NOT_INIT;
}

eBorderRouterStatus BorderRouter::reInit(){
	BorderRouter::deInit();
	return BorderRouter::init(wpan);
}

eBorderRouterStatus BorderRouter::getStatus(){
	return status;
}

BorderRouter::~BorderRouter(){
	BorderRouter::getInstance().deInit();
}

BorderRouter& BorderRouter::getInstance() {
	static BorderRouter instance;
	return instance;
}

void BorderRouter::tick(uint32_t delayMs){
	for(int i=0; i<routes.size(); i++){
		routes.at(i).tick();
	}
	wpan->tick(delayMs);
	if(wpan->nodeListUpdated()){
		updateRoutes();
	}
}

void BorderRouter::updateNodeList(){
	nodes=wpan->getNodeList();
#ifdef DEBUG_BORDERROUTER
	printf("Border router: update node list\n");
#endif
	status=NODELIST_OK;
}

void BorderRouter::updateRoutes(){
	int i=0;

	updateNodeList();
#ifdef DEBUG_BORDERROUTER
	printf("Border router: update routes\n");
#endif
	routes.clear();
	while(i<nodes.size()){
		if((nodes.at(i)->getProtocols() & DATA_PROTOCOL_ID)!=0){
			routes.emplace_back((DataNode*)nodes.at(i));
			nodes.at(i)->wakeup();
		}
		i++;
	}
	status=OK;
}


