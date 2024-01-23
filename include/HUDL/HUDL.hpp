#ifndef _HUDL_
#define _HUDL_

#include <EVT/dev/LCD.hpp>
#include <EVT/io/CANOpenMacros.hpp>
#include <EVT/io/CANopen.hpp>
#include <EVT/io/GPIO.hpp>
#include <EVT/io/SPI.hpp>
#include <HUDL/HUDL.hpp>

#define REFRESH_RATE 16128

namespace IO = EVT::core::IO;
namespace DEV = EVT::core::DEV;

namespace HUDL {

/**
 * Interfaces with the HUDL device. Provides an object dictionary
 * for communicating with other devices on the CAN network and functionality
 * for displaying what a user wants to an LCD screen
 */
class HUDL : public CANDevice {
public:
    /**
     * Default Constructor for the HUDL class
     *
     * @param[in] reg_select is the register select pin
     * @param[in] reset is the reset pin
     * @param[in] spi is the SPI instance
     */
    HUDL(IO::GPIO& reg_select, IO::GPIO& reset, IO::SPI& spi);

    /**
     * Initializes LCD for use
     */
    void initLCD();

    /**
     * Gets the object dictionary
     *
     * @return an object dictionary
     */
    CO_OBJ_T* getObjectDictionary() override;

    /**
     * Gets the size of the Object Dictionary
     *
     * @return size of the Object Dictionary
     */
    uint8_t getNumElements() override;

    /**
    * Get the device's node ID
    *
    * @return The node ID of the can device.
     */
    uint8_t getNodeID() override;

    /**
     * Standard process function for the HUDL. Updates the HUDL display on a set refresh rate.
     * The refresh rate is defined as REFRESH_RATE
     */
    void process();

    /**
     * +The internal LCD used for for the HUDL to display.
     */
    DEV::LCD lcd;

private:
    /**
     * The corner of the display that will content will be displayed in.
     */
    enum Corner {
        TOP_LEFT,
        TOP_RIGHT,
        BOTTOM_LEFT,
        BOTTOM_RIGHT
    };

    /**
     * Updates the LCD display with values received from the CAN network
     */
    void updateLCD();

    /**
     * A counter to limit the rate at which we update the display.
     */
    uint16_t displayCounter = 0;

    /**
     * The node IDs used to identify the device on the CAN network.
     */
    /** NODE ID for the HUDL */
    static constexpr uint8_t NODE_ID = 11;
    /** NODE ID for the TMS */
    static constexpr uintptr_t TMS_NODE_ID = 8;
    /** NANO ID for the Motor Controller */
    static constexpr uintptr_t MC_NODE_ID = 1;

    /** A boolean that tracks whether or not the headers have been set. */
    bool setHeaders = false;

    /** An unused value for holding a place in the CAN Object Dictionary */
    uint16_t dummyValue = 0;

    /** The total voltage from the Motor Controller */
    uint16_t totalVoltage = 0;

    /** An array of four temperatures from the TMS */
    uint16_t thermTemps[4] = {};

    /** The status word provided by the MC node over CAN. Found in the first 16 bits of the 1st PDO coming from the MC. */
    uint16_t statusWord = 0;

    /** The torque actual value provided by the MC node over CAN. Found in the first 16 bits of the 4th PDO coming from the MC. */
    uint16_t torqueActual = 0;

    /** The current position of the Motor Controller. Used to calculate speed, RPM and more. */
    int16_t actualPosition = 0;

    /**
     * A static function that retrieves the column number for a specified corner. This is used
     * to start drawing at a specific x column when we want to display in one of the corners.
     *
     * @param corner the corner to retrieve a column number for.
     * @return the column index that indicates the start of the given corner.
     */
    static uint8_t columnForCorner(Corner corner);

    /**
     * A static function that retrieves the page number for a specified corner. This is used
     * to retrieve the y page where drawing should begin for a specific corner.
     *
     * @param corner the corner to retrieve the page for.
     * @return the page index that indicates the start of the given corner.
     */
    static uint8_t pageForCorner(Corner corner);

    /**
     * Sets the header for a given corner to the specific text
     *
     * @param corner the corner to change text for.
     * @param text the header text to display.
     */
    void headerForCorner(Corner corner, const char* text);

    /**
     * Sets the data to display for a given corner
     *
     * @param corner the corner to set data for.
     * @param text the data to display in the corner.
     */
    void dataForCorner(Corner corner, const char* text);

    /**
     * The size of the CAN object dictionary.
     */
    static constexpr uint16_t OBJECT_DICTIONARY_SIZE = 46;

    /**
     * The CAN object dictionary.
     */
    CO_OBJ_T objectDictionary[OBJECT_DICTIONARY_SIZE + 1] = {
        MANDATORY_IDENTIFICATION_ENTRIES_1000_1014,
        HEARTBEAT_PRODUCER_1017(2000),
        IDENTITY_OBJECT_1018,
        SDO_CONFIGURATION_1200,

        RECEIVE_PDO_SETTINGS_OBJECT_140X(0, 0, TMS_NODE_ID, RECEIVE_PDO_TRIGGER_ASYNC),
        RECEIVE_PDO_SETTINGS_OBJECT_140X(1, 0, MC_NODE_ID, RECEIVE_PDO_TRIGGER_ASYNC),
        RECEIVE_PDO_SETTINGS_OBJECT_140X(2, 1, MC_NODE_ID, RECEIVE_PDO_TRIGGER_ASYNC),

        RECEIVE_PDO_MAPPING_START_KEY_16XX(0, 4),
        RECEIVE_PDO_MAPPING_ENTRY_16XX(0, 1, PDO_MAPPING_UNSIGNED16),// Temperature One
        RECEIVE_PDO_MAPPING_ENTRY_16XX(0, 2, PDO_MAPPING_UNSIGNED16),// Temperature Two
        RECEIVE_PDO_MAPPING_ENTRY_16XX(0, 3, PDO_MAPPING_UNSIGNED16),// Temperature Three
        RECEIVE_PDO_MAPPING_ENTRY_16XX(0, 4, PDO_MAPPING_UNSIGNED16),// Temperature Four

        RECEIVE_PDO_MAPPING_START_KEY_16XX(1, 3),
        RECEIVE_PDO_MAPPING_ENTRY_16XX(1, 1, PDO_MAPPING_UNSIGNED16),// Status Word
        RECEIVE_PDO_MAPPING_ENTRY_16XX(1, 2, PDO_MAPPING_UNSIGNED16),// Position Actual
        RECEIVE_PDO_MAPPING_ENTRY_16XX(1, 3, PDO_MAPPING_UNSIGNED16),// Torque Actual

        RECEIVE_PDO_MAPPING_START_KEY_16XX(2, 2),
        RECEIVE_PDO_MAPPING_ENTRY_16XX(2, 1, PDO_MAPPING_UNSIGNED16),// Unused Value
        RECEIVE_PDO_MAPPING_ENTRY_16XX(2, 2, PDO_MAPPING_UNSIGNED16),// Battery Voltage

        DATA_LINK_START_KEY_21XX(0, 4),
        DATA_LINK_21XX(0, 1, CO_TUNSIGNED16, &thermTemps[0]),// Temperature One
        DATA_LINK_21XX(0, 2, CO_TUNSIGNED16, &thermTemps[1]),// Temperature Two
        DATA_LINK_21XX(0, 3, CO_TUNSIGNED16, &thermTemps[2]),// Temperature Three
        DATA_LINK_21XX(0, 4, CO_TUNSIGNED16, &thermTemps[3]),// Temperature Four

        DATA_LINK_START_KEY_21XX(1, 3),
        DATA_LINK_21XX(1, 1, CO_TUNSIGNED16, &statusWord),    // Status Word
        DATA_LINK_21XX(1, 2, CO_TUNSIGNED16, &actualPosition),// Actual Position
        DATA_LINK_21XX(1, 3, CO_TUNSIGNED16, &torqueActual),  // Torque Actual

        DATA_LINK_START_KEY_21XX(2, 2),
        DATA_LINK_21XX(2, 1, CO_TUNSIGNED16, &dummyValue),  // Unused Value
        DATA_LINK_21XX(2, 2, CO_TUNSIGNED16, &totalVoltage),// Total Voltage

        CO_OBJ_DICT_ENDMARK,
    };
};

}// namespace HUDL

#endif