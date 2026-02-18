#ifndef _HUDL_
#define _HUDL_

#include <EVT/dev/LCD.hpp>
#include <EVT/io/CANOpenMacros.hpp>
#include <EVT/io/CANopen.hpp>
#include <EVT/io/GPIO.hpp>
#include <EVT/io/SPI.hpp>
#include <HUDL.hpp>

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
     * The internal LCD used for for the HUDL to display.
     */
    DEV::LCD lcd;

private:
    /**
     * The corner of the display that the content will be displayed in.
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

    /** NODE ID for the IMU */
    static constexpr uintptr_t IMU_NODE_ID = 9;

    /**  Tracks whether or not the headers have been set. */
    bool setHeaders = false;

    /**
    * 0. VECTOR_EULER_X - vectorXValues[0]
    * 1. VECTOR_GYROSCOPE_X - vectorXValues[1]
    * 2. VECTOR_LINEAR_ACCEL_X - vectorXValues[2]
    * 3. VECTOR_ACCELEROMETER_X - vectorXValues[3]
     */
    uint16_t vectorXValues[4] = {};

    /**
    * 0. VECTOR_EULER_Y - vectorYValues[0]
    * 1. VECTOR_GYROSCOPE_Y - vectorYValues[1]
    * 2. VECTOR_LINEAR_ACCEL_Y - vectorYValues[2]
    * 3. VECTOR_ACCELEROMETER_Y - vectorYValues[3]
     */
    uint16_t vectorYValues[4] = {};

    /**
    * 0. VECTOR_EULER_Z - vectorZValues[0]
    * 1. VECTOR_GYROSCOPE_Z - vectorZValues[1]
    * 2. VECTOR_LINEAR_ACCEL_Z - vectorZValues[2]
    * 3. VECTOR_ACCELEROMETER_Z - vectorZValues[3]
     */
    uint16_t vectorZValues[4] = {};


    /**
     * Retrieves the column number for a specified corner. This is used
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
     * The size of the CANopen object dictionary.
     */
    static constexpr uint16_t OBJECT_DICTIONARY_SIZE = 53;

    /**
     * The CANopen object dictionary.
     */
    CO_OBJ_T objectDictionary[OBJECT_DICTIONARY_SIZE + 1] = {
        MANDATORY_IDENTIFICATION_ENTRIES_1000_1014,
        HEARTBEAT_PRODUCER_1017(2000),
        IDENTITY_OBJECT_1018,
        SDO_CONFIGURATION_1200,

        RECEIVE_PDO_SETTINGS_OBJECT_140X(0, 0, IMU_NODE_ID, RECEIVE_PDO_TRIGGER_ASYNC),
        RECEIVE_PDO_SETTINGS_OBJECT_140X(1, 1, IMU_NODE_ID, RECEIVE_PDO_TRIGGER_ASYNC),
        RECEIVE_PDO_SETTINGS_OBJECT_140X(2, 2, IMU_NODE_ID, RECEIVE_PDO_TRIGGER_ASYNC),

        RECEIVE_PDO_MAPPING_START_KEY_16XX(0, 4),
        RECEIVE_PDO_MAPPING_ENTRY_16XX(0, 1, PDO_MAPPING_UNSIGNED16), // VECTOR_EULER_X
        RECEIVE_PDO_MAPPING_ENTRY_16XX(0, 2, PDO_MAPPING_UNSIGNED16), // VECTOR_GYROSCOPE_X
        RECEIVE_PDO_MAPPING_ENTRY_16XX(0, 3, PDO_MAPPING_UNSIGNED16), // VECTOR_LINEAR_ACCEL_X
        RECEIVE_PDO_MAPPING_ENTRY_16XX(0, 4, PDO_MAPPING_UNSIGNED16), // VECTOR_ACCELEROMETER_X

        RECEIVE_PDO_MAPPING_START_KEY_16XX(1, 4),
        RECEIVE_PDO_MAPPING_ENTRY_16XX(1, 1, PDO_MAPPING_UNSIGNED16), // VECTOR_EULER_Y
        RECEIVE_PDO_MAPPING_ENTRY_16XX(1, 2, PDO_MAPPING_UNSIGNED16), // VECTOR_GYROSCOPE_Y
        RECEIVE_PDO_MAPPING_ENTRY_16XX(1, 3, PDO_MAPPING_UNSIGNED16), // VECTOR_LINEAR_ACCEL_Y
        RECEIVE_PDO_MAPPING_ENTRY_16XX(1, 4, PDO_MAPPING_UNSIGNED16), // VECTOR_ACCELEROMETER_Y

        RECEIVE_PDO_MAPPING_START_KEY_16XX(2, 4),
        RECEIVE_PDO_MAPPING_ENTRY_16XX(2, 1, PDO_MAPPING_UNSIGNED16), // VECTOR_EULER_Z
        RECEIVE_PDO_MAPPING_ENTRY_16XX(2, 2, PDO_MAPPING_UNSIGNED16), // VECTOR_GYROSCOPE_Z
        RECEIVE_PDO_MAPPING_ENTRY_16XX(2, 3, PDO_MAPPING_UNSIGNED16), // VECTOR_LINEAR_ACCEL_Z
        RECEIVE_PDO_MAPPING_ENTRY_16XX(2, 4, PDO_MAPPING_UNSIGNED16), // VECTOR_ACCELEROMETER_Z

        DATA_LINK_START_KEY_21XX(0, 4),
        DATA_LINK_21XX(0, 1, CO_TUNSIGNED16, &vectorXValues[0]), // VECTOR_EULER_X
        DATA_LINK_21XX(0, 2, CO_TUNSIGNED16, &vectorXValues[1]), // VECTOR_GYROSCOPE_X
        DATA_LINK_21XX(0, 3, CO_TUNSIGNED16, &vectorXValues[2]), // VECTOR_LINEAR_ACCEL_X
        DATA_LINK_21XX(0, 4, CO_TUNSIGNED16, &vectorXValues[3]), // VECTOR_ACCELEROMETER_X

        DATA_LINK_START_KEY_21XX(1, 4),
        DATA_LINK_21XX(1, 1, CO_TUNSIGNED16, &vectorYValues[0]), // VECTOR_EULER_Y
        DATA_LINK_21XX(1, 2, CO_TUNSIGNED16, &vectorYValues[1]), // VECTOR_GYROSCOPE_Y
        DATA_LINK_21XX(1, 3, CO_TUNSIGNED16, &vectorYValues[2]), // VECTOR_LINEAR_ACCEL_Y
        DATA_LINK_21XX(1, 4, CO_TUNSIGNED16, &vectorYValues[3]), // VECTOR_ACCELEROMETER_Y

        DATA_LINK_START_KEY_21XX(2, 4),
        DATA_LINK_21XX(2, 1, CO_TUNSIGNED16, &vectorZValues[0]), // VECTOR_EULER_Z
        DATA_LINK_21XX(2, 2, CO_TUNSIGNED16, &vectorZValues[1]), // VECTOR_GYROSCOPE_Z
        DATA_LINK_21XX(2, 3, CO_TUNSIGNED16, &vectorZValues[2]), // VECTOR_LINEAR_ACCEL_Z
        DATA_LINK_21XX(2, 4, CO_TUNSIGNED16, &vectorZValues[3]), // VECTOR_ACCELEROMETER_Z

        CO_OBJ_DICT_ENDMARK,
    };
};

}// namespace HUDL

#endif