#ifndef POWERNODE_H
#define POWERNODE_H

#include "node.h"

//#if defined(DEBUG_APP)
    #include <iostream>
    #include "cm4_utils.h"
//#endif

using namespace std;


/**
 * @todo write docs
 */
class PowerNode : public Node
{
public:
    /**
     * Default constructor
     */
    PowerNode();

    /**
     * Static definition constructor
     */
    PowerNode(uint8_t _address, uint32_t _group);

    /**
     * Destructor
     */
    ~PowerNode();

    friend ostream& operator<<(ostream & out, const PowerNode & powerNode);

    /**
     * Copy constructor
     *
     * @param other TODO
     */
    //PowerNode(const PowerNode& other);

    /**
     * Assignment operator
     *
     * @param other TODO
     * @return TODO
     */
    //PowerNode& operator=(const PowerNode& other);

    /**
     * @todo write docs
     *
     * @param other TODO
     * @return TODO
     */
    //bool operator==(const PowerNode& other) const;

    /**
     * @todo write docs
     *
     * @param other TODO
     * @return TODO
     */
    //bool operator!=(const PowerNode& other) const;

    // GETTERS
    vector<powerTarget_t>   getPowerSettingsList();
    powerkW_t               getPower();
    powerSetting_t          getPowerSetting();
    sManifest               getManifest();
    string                  getDescription();

    /* Application commands
     * All commands update relevant PowerNode attribute on callback
     * If unsuccessful, callback flag will read false
     * Read callback flags manually using getter functions, otherwise get it
     * as return value from application commands
     */
    /*
     * @brief get power setting list from node
     * @param timeout in ms, no timeout if 0
     * @return true if power settings got within timeout, 0 otherwise
     */
    bool app_getPowerSettings(uint32_t timeoutMs);
    /*
     * @brief get current node power as float
     * @param timeout in ms, no timeout if 0
     * @return true if power was received within timeout, 0 otherwise
     */
    bool app_getPower(uint32_t timeoutMs);
    /*
     * @brief set node power as float
     * @param timeout in ms, no timeout if 0
     * @param target power
     * @return true if power set ack got within timeout, 0 otherwise
     * Applicability depends on distant node implementation (will return error
     * APP_ERR_UNDEFINEDCMD)
     */
    bool app_setPower(powerkW_t powerkW_t, uint32_t timeoutMs);
    /*
     * @brief get current node power setting
     * @param timeout in ms, no timeout if 0
     * @return true if power setting was received within timeout, 0 otherwise
     */
    bool app_getPowerSetting(uint32_t timeoutMs);
    /*
     * @brief set current node power setting
     * @param timeout in ms, no timeout if 0
     * @param new power setting index
     * @return true if power setting ack was received within timeout, 0 otherwise
     */
    bool app_setPowerSetting(powerSetting_t powerSetting_t, uint32_t timeoutMs);
    /*
     * @brief get node manifest
     * @param timeout in ms, no timeout if 0
     * @return true if power was received within timeout, 0 otherwise
     */
    bool app_getManifest(uint32_t timeoutMs);

    // CALLBACK FLAG GETTERS
    bool getManifestStatus();
    bool setPowerStatus();
    bool getPowerStatus();
    bool setPowerSettingStatus();
    bool getPowerSettingStatus();
    bool getPowerSettingsStatus();


private:
    /**
     * General information in node.h
     * @brief Handles application command
     * @param buffer data and length
     */
    void appCmdCallback(const uint8_t * data, uint8_t size);
    /**
     * General information in node.h
     * @brief Prints application error protocol messages
     * @param buffer data and length
     */
    void appErrCallback(const uint8_t * data, uint8_t size);

    /**
     * @brief Handler function get and set power commands callbacks
     * @param buffer and size from appCmdCallback
     * @return new callback flag value
     */
    bool getSetPowerHandler(const uint8_t * buffer, uint8_t size);
    /**
     * @brief Handler function get and set power setting commands callbacks
     * @param buffer and size from appCmdCallback
     * @return new callback flag value
     */
    bool getSetPowerSettingHandler(const powerSetting_t setting);
    /**
     * @brief Handler function for get manifest command callback
     * @param buffer and size from appCmdCallback
     * @return width of copied manifest
     * Copies received manifest in object attribute, also converts manifest
     * description to string and stores it in relevant attibute
     */
    uint8_t copyManifest(const uint8_t * buffer, uint8_t size);
    /**
     * @brief Handler function for get power settings command callback
     * @param buffer and size from appCmdCallback
     * @return width of copied power setting array
     * Converts and copies received power settings into local float vector
     */
    uint8_t copyPowerSettings(const uint8_t * buffer, uint8_t size);
    /**
     * @brief Conversion from sPowerSettings to float vector
     * @param sPowerSettings received from app_getPowerSettings command
     * @return float vector of power settings
     */
    vector<powerTarget_t> sPowerSettingsToFloatVector(sPowerSettings powerSettings_);
    /**
     * @brief Print frame if DEBUG_APP is defined
     * @param buffer and length, dir=true for TX frame
     * Prints in HEX notation
     * Redefinition from parent class
     */
    void printAppFrame(const uint8_t * buffer, uint8_t size, bool dir);

    // Transcriptions
    string description="No description yet";// Use this for easy access to device description
    vector<powerTarget_t> powerSettingsList;// Use this for easy access to power settings


    // Callback flags
    bool manifestCallback=false;
    bool powerSetCallback=false;
    bool powerGetCallback=false;
    bool powerSettingSetCallback=false;
    bool powerSettingGetCallback=false;
    bool powerSettingsCallback=false;

    powerkW_t powerkW=0;
    powerSetting_t powerSetting=0;
    sPowerSettings powerSettings;
    sManifest manifest;
};

#endif // POWERNODE_H
