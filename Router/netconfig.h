/**
 * @file netconfig.h
 *
 * Config file for UDP sockets in Connection.h
 */

#ifndef __NETCONFIG_H__
#define __NETCONFIG_H__

#define MAX_CONNS 255 ///< Max number of concurrent connections for BorderRouter
#define BASE_GW_IN_PORT 	6000 ///< Base port for Connection UDP sockets
#define FORCE_BIND  ///< Uncomment to force socket bind in Linux

#endif
