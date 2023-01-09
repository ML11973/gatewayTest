#ifndef MENU_H
#define MENU_H


#include <iostream>
#include <string>
#include <vector>
#include "cm4_utils.h"
#include "ism3.h"
#include "ism3_handlers.h"    // ISM3 handler functions
#include "ism3_server.h"      // ISM3 server utilities
#include "node.h"             // Client node
#include "powernode.h"        // Client power node
#include "wpanManager.h"      // WPAN manager

using namespace std;

int mainMenu(PowerNode & testClient_, wpanManager & gateway_);

#endif