#ifndef MENU_H
#define MENU_H
/**
 * @file menu.h
 * @brief Main menu for test application
 */

#include <iostream>
#include <string>
#include <vector>
#include "cm4_utils.h"
#include "ism3.h"
#include "ism3_handlers.h"    // ISM3 handler functions
#include "ism3_server.h"      // ISM3 server utilities
#include "node.h"             // Client node
#include "powernode.h"        // Client power node
#include "datanode.h"         // Client data node
#include "wpanManager.h"      // WPAN manager
#include "BorderRouter.h"

using namespace std;

/**
 * @brief Main menu
 * @param gateway_ reference to the gateway instance
 * @return 0 if program should exit
 *
 * Master menu that links to all sub menus.
 * Takes a reference to the gateway instance and passes it down to the sub menus
 */
int mainMenu(wpanManager & gateway_);

#endif
