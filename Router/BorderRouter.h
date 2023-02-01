/*
 * BorderRouter.h
 *
 *  Created on: Aug 29, 2022
 *      Author: leemarc
 */

/**
 * @file BorderRouter.h
 */

/**
 * @page borderrouter Border router
 *
 * # Description
 *
 * This class maintains a @subpage connections list so that datagrams can be
 * forwarded to and from remote @ref datanodes.
 * It polls the @ref wpanManager_desc for new connected nodes and creates a new
 * Connection to them if applicable.
 *
 * # Usage
 * ## Calling methods
 *
 * This class is a Singleton. It builds and maintains its own instance.
 * Access the instance through BorderRouter::getInstance.
 * Instance cannot be initialized or accessed outside of this method.
 *
 * ## Initialization
 *
 * Initialize linked wpanManager first, then initialize border router with
 * BorderRouter::init.
 *
 * ## Ticking
 *
 * Tick regularly enough using BorderRouter::tick(). The latter function also
 * ticks wpanManager. It is possible to only tick wpanManager with its
 * wpanManager::tick, so border router can be ticked only periodically.
 * Ticking also ticks open @ref connections, polling their sockets and dataNodes
 * for new data.
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
/**
 * @brief Border router status enumerator
 *
 * OK if all Connections have been built.
 * NODELIST_OK if node list was correctly fetched.
 * NOT_INIT it not yet initialized.
 */
typedef enum{
	OK,
	NODELIST_OK,
	NOT_INIT
}eBorderRouterStatus;

/**
 * @brief Forwards external packets to remote @ref nodes. See @ref borderrouter.
 */
class BorderRouter {
public:
	/**
	 * @brief Singleton get instance
	 * @return reference to instance
	 */
	static BorderRouter& getInstance();
	/**
	 * @brief Copy constructor override
	 */
	BorderRouter(const BorderRouter&);
	/**
	 * @brief Assignment operator override
	 */
	void operator=(const BorderRouter&);

	/**
	 * @brief Initialization function
	 * @return own status
	 * WPAN manager must be initialized elsewhere
	 */
	eBorderRouterStatus init(wpanManager * _wpan);
	/**
	 * @brief DeInit
	 */
	void deInit();
	/**
	 * @brief Deinitialize then initialize
	 * @return own status
	 */
	eBorderRouterStatus reInit();
	/**
	 * @brief Status getter
	 * @return border router status
	 */
	eBorderRouterStatus getStatus();
	/**
	 * @brief Master ticker with delay
	 * @param delay in ms
	 * Ticks WPAN manager and all Connections. Updates node lists and
	 * Connections if a new DataNode was connected
	 */
	void tick(uint32_t delayMs);
private:
	/**
	 * @brief Unusable constructor
	 */
	BorderRouter(){};
	/**
	 * @brief Call deinit
	 */
	virtual ~BorderRouter();
	/**
	 * @brief Update node lists from WPAN instance
	 */
	void updateNodeList();
	/**
	 * @brief Update Connection vector to reflect WPAN connections
	 */
	void updateRoutes();


	vector<Connection> routes;	///< Opened Connection instance vector
	vector<Node*> nodes;		///< WPAN manager nodes
	eBorderRouterStatus status;	///< Own status
	wpanManager * wpan;			///< Pointer to WPAN manager instance
};

#endif /* BORDERROUTER_H_ */
