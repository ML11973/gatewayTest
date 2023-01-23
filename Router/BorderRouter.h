/*
 * RoutingManager.h
 *
 *  Created on: Aug 29, 2022
 *      Author: leemarc
 */

/**
 * Master class for Connections
 * Manages Connections depending on current node list given by WPAN manager
 * Takes all DataNodes and creates a connection to them.
 * Master ticker also handles WPAN manager ticks.
 * Usage: init WPAN manager then init Border router. Tick using Border router.
 */

#ifndef BORDERROUTER_H_
#define BORDERROUTER_H_

#include <stdint.h>
#include <stdio.h>
#include <vector>

#include "netconfig.h"
#include "Connection.h"
#include "wpan.h"
#include "wpanManager.h"


using namespace std;

typedef enum{
	OK,
	NODELIST_OK,
	NOT_INIT
}eBorderRouterStatus;

class BorderRouter {
public:
	/**
	 * @brief singleton get instance
	 */
	static BorderRouter& getInstance();
	BorderRouter(const BorderRouter&);
	void operator=(const BorderRouter&);

	/**
	 * @brief init function
	 * @return own status
	 * WPAN manager must be initialized elsewhere
	 */
	eBorderRouterStatus init(wpanManager * _wpan);
	/**
	 * @brief deInit
	 */
	void deInit();
	/**
	 * @brief deInit then init
	 * @reutnr own status
	 */
	eBorderRouterStatus reInit();
	/**
	 * @brief status getter
	 * @return border router status
	 */
	eBorderRouterStatus getStatus();
	/**
	 * @brief master ticker with delay
	 * @param delay in ms
	 * Ticks WPAN manager and all Connections. Updates node lists and
	 * Connections if a new DataNode was connected
	 */
	void tick(uint32_t delayMs);
private:
	/**
	 * @brief unused constructor
	 */
	BorderRouter(){};
	/**
	 * @brief call deinit
	 */
	virtual ~BorderRouter();

	/**
	 * @brief update node lists from WPAN instance
	 */
	void updateNodeList();
	/**
	 * @brief update Connection vector to reflect WPAN connections
	 */
	void updateRoutes();


	vector<Connection> routes;	// Opened Connection instance vector
	vector<Node*> nodes;		// WPAN manager nodes
	eBorderRouterStatus status;	// Own status
	wpanManager * wpan;			// Pointer to WPAN manager instance
};

#endif /* BORDERROUTER_H_ */
