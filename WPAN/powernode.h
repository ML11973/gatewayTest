/**
 * @file powernode.h
 * @author marc.leemann@master.hes-so.ch
 * @brief PowerNode class definition
 */

#ifndef POWERNODE_H
#define POWERNODE_H

#include "node.h"

//#if defined(DEBUG_APP)
    #include <iostream>
    #include "cm4_utils.h"
//#endif

using namespace std;

/**
 * @page powernodes Power Nodes
 *
 * # Description
 *
 * PowerNode refers to @ref nodes that measure, consume and-or
 * produce electrical power in an electrical installation.
 *
 * The PowerNode class provides the user with a representation of the remote
 * node with the following:
 * - A text description
 * - Current measured power consumption or production
 * - Available power settings
 * - Regulation priority
 *
 * The class allows setting a target power in two ways:
 * - setting an arbitrary power target in kW
 * - setting a pre-defined power setting
 *
 * # Usage
 *
 * ## Instantiation
 *
 * Instantiate using any constructor. Constructors match Node type constructor.
 * A constructor for PowerNode instantiation from a Node object exists.
 *
 * ## Sending commands
 *
 * Use any function starting with app_ to issue an @subpage app_protocol command.
 *
 * Other operations are inherited from parent Node.
 *
 * ## Debugging
 *
 * wpan.h provides two DEBUG macros for this class.
 * Uncomment #DEBUG_RXTX to show all incoming and outgoing frames.
 * Uncomment #DEBUG_APP to show only @subpage app_protocol frames.
 *
 * # Implementation details
 *
 * ## Frame IO
 *
 * This class does not provide a method to directly transmit data to its remote
 * node. Instead, it provides wrappers like its parent class.
 *
 * ## Command functions
 *
 * As with the Node class, methods starting with app_, such as
 * PowerNode::app_getPower, are wrappers to send the corresponding command to
 * the remote node.
 * They reset the corresponding callback flag if an answer is expected.
 *
 * **As with Node, do not confuse application commands such as
 * PowerNode::app_getPower with getters such as PowerNode::getPower!**
 *
 * ## Callback flags
 *
 * These work the same as in the Node class, but with PowerNode::appCmdCallback.
 * See relevant subsection on the @ref nodes page.
 *
 * ## Class hierarchy
 * The PowerNode class is a child of the Node class.
 * It handles the @subpage app_protocol and leaves the rest to its parent.
 *
 * # Attribute specification
 *
 * ## Priority
 *
 * Priority describes the PowerNode's priority for electrical consumption or
 * production.
 * A value of zero means the node's production or consumption cannot be
 * regulated. A higher value means the node's production/consumption will be
 * adjusted before that of lower-value nodes.
 *
 * Production example :
 *
 * Let node A be a solar panel.
 * Let node B be a battery with inverter.
 * Let node C be a diesel generator.
 *
 * Target : minimizing power surplus.
 *
 * Node A's production cannot be adjusted. A priority = 0.
 * Node B is a backup system which we do not want to draw from except in
 * emergencies. B priority = 1.
 * Node C consumes valuable fuel, but it is acceptable burn fuel to meet energy
 * demand in our example policy. C priority > 1.
 *
 * If node A's production picks up, we will stop node B before node C first.
 * This means we stop using power from the emergency storage before we stop
 * using the diesel generator.
 *
 * Consumption example :
 *
 * Let node A be a freezer.
 * Let node B be a water heater.
 * Let node C be an electric car charging station.
 *
 * Target : minimizing power draw from the grid.
 *
 * Node A's consumption cannot be adjusted lest we sacrifice its contents.
 * A priority = 0.
 * Node B is useful, but also stores energy. Turning it off for a day would
 * not hurt its functionality. B priority > 1.
 * Node C ultimately does not need power every day since the charged car can go
 * a few days without charging. C priority > B priority.
 *
 * In case of low household power production, C would be shut down first,
 * then B.
 *
 * ## Power
 *
 * Power is the current power consumption or production.
 * Consumed power should be negative by convention.
 * Power should be in kW by convention.
 * These two conventions are ultimately left for the user application to specify.
 * Power data type can be modified by changing powerkW_t, be it to switch to
 * double precision or to accept integer precision and reduce channel usage.
 *
 * Nodes that can precisely regulate their consumption should allow setting
 * arbitrary power targets. This could be the case for an emergency backup
 * power bank's charging system, for example.
 *
 * In any case, measured power must be true and should be used for computations.
 *
 * ## Power settings
 *
 * Power settings can be compared to power modes.
 * A node cannot always regulate its power consumption precisely.
 * Some consumers may only allow ON/OFF, some may have 3 undefined power modes.
 *
 * Power setting description should be a power as defined in the previous
 * section. The ideal case for a regulator would be to have access to estimated
 * power consumptions per power mode. The PowerNode::app_getPowerSettings
 * method is built to get power figures in kW as descriptions of the power mode.
 * The application developer is however free to change the contents of power
 * setting descriptions, opting for simple integers or even characters.
 *
 * In any case, measured power should be used as the reference for computations.
 *
 * Settings are chosen by sending their index with
 * PowerNode::app_setPowerSetting.
 *
 *
 * ## Description
 *
 * This is a simple description sent by the remote node.
 * It is somewhat comparable to a hostname in local networks.
 * Its maximum size is limited by the PHY max size.
 * It is defined in #APP_MAX_DESC_LENGTH.
 */

/**
 * @brief Reprensentation of a distant power node.
 * See @ref powernodes page for details.
 */
class PowerNode : public Node
{
public:
    /**
     * @brief Static definition constructor
     * @param _address node address
     * @param _group node group
     *
     * Init with lease duration = 0
     */
    PowerNode(uint8_t _address, uint32_t _group);
    /**
     * @brief Dynamic definition constructor
     * @param _address node address
     * @param _group node group
     * @param _leaseDuration lease duration (multiple of NETWORK_LEASE_UNIT_MINUTES)
     */
    PowerNode(uint8_t _address, uint32_t _group, uint8_t _leaseDuration);
    /**
     * @brief Constructor from existing Node
     * @param base source Node to transtype cast
     *
     * Copy argument node address, group and lease duration.
     * Do not copy current callback flags
     */
    PowerNode(Node & base);

    /**
     * @brief Destructor
     */
    ~PowerNode();

    /**
     * @brief << operator redefinition for printing using iostream
     * @param out stream to write in
     * @param powerNode PowerNode to write in stream
     */
    friend ostream& operator<<(ostream & out, const PowerNode & powerNode);

    /**
     * @brief Printer function
     */
    void show();

    // GETTERS
    /**
     * @brief Getter for local power settings respective power targets
     * @return power targets vector
     */
    vector<powerTarget_t>   getPowerSettingsList();
    /**
     * @brief Getter for local measured power
     * @return measured power
     */
    powerkW_t               getPower();
    /**
     * @brief Getter for local current power setting
     * @return power setting
     */
    powerSetting_t          getPowerSetting();
    /**
     * @brief Getter for local manifest
     * @return manifest
     */
    sManifest               getManifest();
    /**
     * @brief Getter for local description
     * @return description
     */
    string                  getDescription();


    /**
     * @brief Get announced supported network protocols
     * @return protocol bitfield
     */
    uint8_t getNodeTypeProtocols();

    /* Application commands
     * All commands update relevant PowerNode attribute on callback
     * If unsuccessful, callback flag will read false
     * Read callback flags manually using getter functions, otherwise get it
     * as return value from application commands
     */
    /**
     * @brief Get power setting list from node
     * @param timeoutMs timeout in ms, non-blocking if 0
     * @return true if power settings got within timeout, false otherwise
     */
    bool app_getPowerSettings(uint32_t timeoutMs);
    /**
     * @brief Get current node power as float
     * @param timeoutMs timeout in ms, non-blocking if 0
     * @return true if power was received within timeout, false otherwise
     */
    bool app_getPower(uint32_t timeoutMs);
    /**
     * @brief Set node power as float
     * @param timeoutMs timeout in ms, non-blocking if 0
     * @param newPower new target power
     * @return true if power set ack got within timeout, false otherwise
     *
     * Applicability depends on distant node implementation (may return error
     * APP_ERR_UNDEFINEDCMD)
     */
    bool app_setPower(powerkW_t newPower, uint32_t timeoutMs);
    /**
     * @brief Get current node power setting
     * @param timeoutMs timeout in ms, non-blocking if 0
     * @return true if power setting was received within timeout, false otherwise
     */
    bool app_getPowerSetting(uint32_t timeoutMs);
    /**
     * @brief Set current node power setting
     * @param timeoutMs timeout in ms, non-blocking if 0
     * @param newPowerSetting new power setting index
     * @return true if power setting ack was received within timeout, false otherwise
     */
    bool app_setPowerSetting(powerSetting_t newPowerSetting, uint32_t timeoutMs);
    /**
     * @brief Get node manifest
     * @param timeoutMs timeout in ms, non-blocking if 0
     * @return true if power was received within timeout, false otherwise
     */
    bool app_getManifest(uint32_t timeoutMs);

    // CALLBACK FLAG GETTERS
    /**
     * @brief Get #APP_GETMANIFEST callback flag
     * @return true if command confirmation was received
     */
    bool getManifestStatus();
    /**
     * @brief Get #APP_SETPOWER callback flag
     * @return true if command confirmation was received
     */
    bool setPowerStatus();
    /**
     * @brief Get #APP_GETPOWER callback flag
     * @return true if command confirmation was received
     */
    bool getPowerStatus();
    /**
     * @brief Get #APP_SETPOWERSETTING callback flag
     * @return true if command confirmation was received
     */
    bool setPowerSettingStatus();
    /**
     * @brief Get #APP_GETPOWERSETTING callback flag
     * @return true if command confirmation was received
     */
    bool getPowerSettingStatus();
    /**
     * @brief Get #APP_GETPOWERSETTINGS callback flag
     * @return true if command confirmation was received
     */
    bool getPowerSettingsStatus();


private:
    /**
     * @brief Default constructor
     *
     * Init with address, group and lease duration = 0
     */
    PowerNode();
    /**
     * @brief Handles application command
     * @param data pointer to data to process
     * @param size data length in bytes
     *
     * See Node::appCmdCallback for general information
     */
    void appCmdCallback(const uint8_t * data, uint8_t size);
    /**
     * @brief Prints application error protocol messages
     * @param data pointer to data to process
     * @param size data length in bytes
     *
     * See Node::appErrCallback for general information
     */
    void appErrCallback(const uint8_t * data, uint8_t size);
    /**
     * @brief Handler function get and set power commands callbacks
     * @param buffer pointer to buffer from appCmdCallback
     * @param size data length in bytes
     * @return new callback flag value
     */
    bool getSetPowerHandler(const uint8_t * buffer, uint8_t size);
    /**
     * @brief Handler function get and set power setting commands callbacks
     * @param buffer pointer to buffer from appCmdCallback
     * @param size data length in bytes
     * @return new callback flag value
     */
    bool getSetPowerSettingHandler(const uint8_t * buffer, uint8_t size);
    /**
     * @brief Handler function for get manifest command callback
     * @param buffer pointer to buffer from appCmdCallback
     * @param size data length in bytes
     * @return width of copied manifest
     *
     * Copies received manifest in object attribute, also converts manifest
     * description to string and stores it in relevant attibute
     */
    uint8_t copyManifest(const uint8_t * buffer, uint8_t size);
    /**
     * @brief Handler function for get power settings command callback
     * @param buffer pointer to buffer from appCmdCallback
     * @param size data length in bytes
     * @return width of copied power setting array
     *
     * Converts and copies received power settings into local float vector
     */
    uint8_t copyPowerSettings(const uint8_t * buffer, uint8_t size);
    /**
     * @brief Conversion from sPowerSettings to float vector
     * @param powerSettings_ power settings received from app_getPowerSettings command
     * @return float vector of power settings
     */
    vector<powerTarget_t> sPowerSettingsToFloatVector(sPowerSettings powerSettings_);
    /**
     * @brief Print frame if DEBUG_APP is defined
     * @param buffer pointer to data to print
     * @param size data length in bytes
     * @param dir direction,true for TX frame
     *
     * Prints in HEX notation. Redefined from parent class.
     */
    void printAppFrame(const uint8_t * buffer, uint8_t size, bool dir);

    // Transcriptions
    string description="No description yet";///< Easy access to device description
    vector<powerTarget_t> powerSettingsList;///< Easy access to power settings


    // Callback flags
    bool manifestCallback=false; ///< true if manifest was received
    bool powerSetCallback=false; ///< true if power was confirmed as set
    bool powerGetCallback=false; ///< true if power was gotten
    bool powerSettingSetCallback=false; ///< true if power setting was confirmed as set
    bool powerSettingGetCallback=false; ///< true if power setting was gotten
    bool powerSettingsCallback=false; ///< true if power settings list was received

    powerkW_t powerkW=0; ///< last received measured power
    powerSetting_t powerSetting=0; ///< last received power setting
    sPowerSettings powerSettings; ///< list of available power settings
    sManifest manifest; ///< node manifest: description, priority
};

#endif // POWERNODE_H
