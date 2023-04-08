#ifndef _HUDL_
#define _HUDL_

#include <Canopen/co_core.h>
#include <EVT/dev/LCD.hpp>
#include <EVT/io/CANopen.hpp>
#include <EVT/io/GPIO.hpp>
#include <EVT/io/SPI.hpp>
#include <HUDL/HUDL.hpp>
#include <stdint.h>

namespace IO = EVT::core::IO;
namespace DEV = EVT::core::DEV;

namespace HUDL {

/**
 * Interfaces with the HUDL device. Provides an object dictionary
 * for communicating with other devices on the CAN network and functionality
 * for displaying what a user wants to an LCD screen
 */
class HUDL {
public:
    /**
     * The node ID used to identify the device on the CAN network.
     */
    static constexpr uint8_t NODE_ID = 0x11;

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
    CO_OBJ_T* getObjectDictionary();

    /**
     * Gets the size of the Object Dictionary
     *
     * @return size of the Object Dictionary
     */
    uint16_t getObjectDictionarySize() const;

    /**
     * Updates the LCD display with values received from the CAN network
     */
    void updateLCD();

    /**
     * reg_select PA_3
     * reset      PB_3
     * cs         PB_12
     */
    DEV::LCD lcd;

    static constexpr uintptr_t TMS_NODE_ID = 0x08;
    static constexpr uintptr_t BMS_NODE_ID = 0x05;
    static constexpr uintptr_t MC_NODE_ID = 0x01;
private:
    uint16_t dummyValue = 0;

    uint16_t totalVoltage = 0;

    uint16_t mcVoltage = 0;

    uint16_t thermTemps[4] = {};

    /** The status word provided by the MC node over CAN. Found in the first 16 bits of the 1st PDO coming from the MC. */
    uint16_t statusWord = 0;

    /** The torque actual value provided by the MC node over CAN. Found in the first 16 bits of the 4th PDO coming from the MC. */
    uint16_t torqueActual = 0;

    uint32_t actualPosition = 0;

    static constexpr uint16_t OBJECT_DICTIONARY_SIZE = 37;

    CO_OBJ_T objectDictionary[OBJECT_DICTIONARY_SIZE + 1] = {
        // Sync ID, defaults to 0x80
        {
            CO_KEY(0x1005, 0, CO_UNSIGNED32 | CO_OBJ_D__R_),
            nullptr,
            (uintptr_t) 0x80,
        },

        /**
         * Information about the hardware , hard coded sample values for now
         * 1: Vendor ID
         * 2: Product Code
         * 3: Revision Number
         * 4: Serial Number
         */
        {
            .Key = CO_KEY(0x1018, 1, CO_UNSIGNED32 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = (uintptr_t) 0x10,
        },
        {
            .Key = CO_KEY(0x1018, 2, CO_UNSIGNED32 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = (uintptr_t) 0x11,
        },
        {
            .Key = CO_KEY(0x1018, 3, CO_UNSIGNED32 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = (uintptr_t) 0x12,
        },
        {
            .Key = CO_KEY(0x1018, 4, CO_UNSIGNED32 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = (uintptr_t) 0x13,
        },

        /**
         * SDO CAN message IDS.
         * 1: Client -> Server ID, default is 0x600 + NODE_ID
         * 2: Server -> Client ID, default is 0x580 + NODE_ID
         */
        {
            .Key = CO_KEY(0x1200, 1, CO_UNSIGNED32 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = (uintptr_t) 0x600 + NODE_ID,
        },
        {
            .Key = CO_KEY(0x1200, 2, CO_UNSIGNED32 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = (uintptr_t) 0x580 + NODE_ID,
        },

        /**
         * *** START RPDO SETTINGS ***
         */
        /**
         * TMS RPDO 0
         * 0: RPDO number in index and total number of sub indexes.
         * 1: The COB-ID to receive PDOs from.
         * 2: transmission trigger
         */
        {
            .Key = CO_KEY(0x1400, 0, CO_UNSIGNED8 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = (uintptr_t) 3,
        },
        {
            .Key = CO_KEY(0x1400, 1, CO_UNSIGNED32 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = (uintptr_t) CO_COBID_TPDO_DEFAULT(0) + TMS_NODE_ID,
        },
        {
            .Key = CO_KEY(0x1400, 2, CO_UNSIGNED8 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = (uintptr_t) 0xFE,
        },
        /**
         * Motor Controller RPDO 0
         * 0: RPDO number in index and total number of sub indexes.
         * 1: The COB-ID to receive PDOs from.
         * 2: transmission trigger
         */
        {
            .Key = CO_KEY(0x1401, 0, CO_UNSIGNED8 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = (uintptr_t) 3,
        },
        {
            .Key = CO_KEY(0x1401, 1, CO_UNSIGNED32 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = (uintptr_t) CO_COBID_TPDO_DEFAULT(0) + MC_NODE_ID,
        },
        {
            .Key = CO_KEY(0x1401, 2, CO_UNSIGNED8 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = (uintptr_t) 0xFE,
        },
        /**
         *  Motor Controller RPDO 1
         * 0: RPDO number in index and total number of sub indexes.
         * 1: The COB-ID to receive PDOs from.
         * 2: transmission trigger
         */
        {
            .Key = CO_KEY(0x1402, 0, CO_UNSIGNED8 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = (uintptr_t) 3,
        },
        {
            .Key = CO_KEY(0x1402, 1, CO_UNSIGNED32 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = (uintptr_t) CO_COBID_TPDO_DEFAULT(1) + MC_NODE_ID,
        },
        {
            .Key = CO_KEY(0x1402, 2, CO_UNSIGNED8 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = (uintptr_t) 0xFE,
        },
        /**
         * TMS RPDO 0
         * Determines the PDO messages to receive when RPDO0 is triggered
         * 0: The number of PDO message associated with the RPDO
         * 1: tempOne
         * 2: tempTwo
         * 3: tempThree
         * 4: tempFour
         */
        {
            .Key = CO_KEY(0x1600, 0, CO_UNSIGNED8 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = (uintptr_t) 4,
        },
        {
            .Key = CO_KEY(0x1600, 1, CO_UNSIGNED32 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = CO_LINK(0x2100, 0, 16), // Temperature One
        },
        {
            .Key = CO_KEY(0x1600, 2, CO_UNSIGNED32 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = CO_LINK(0x2100, 1, 16), // Temperature Two
        },
        {
            .Key = CO_KEY(0x1600, 3, CO_UNSIGNED32 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = CO_LINK(0x2100, 2, 16), // Temperature Three
        },
        {
            .Key = CO_KEY(0x1600, 4, CO_UNSIGNED32 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = CO_LINK(0x2100, 3, 16), // Temperature Four
        },
        /**
         * Motor Controller RPDO 1
         * Determines the PDO messages to receive when RPDO3 is triggered
         * 0: The number of PDO message associated with the RPDO
         * 1: statusWord
         * 2: positionActual
         * 3: torqueActual
         */
        {
            .Key = CO_KEY(0x1601, 0, CO_UNSIGNED8 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = (uintptr_t) 3,
        },
        {
            .Key = CO_KEY(0x1601, 1, CO_UNSIGNED32 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = CO_LINK(0x2101, 0, 16),
        },
        {
            .Key = CO_KEY(0x1601, 2, CO_UNSIGNED32 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = CO_LINK(0x2101, 1, 32),
        },
        {
            .Key = CO_KEY(0x1601, 3, CO_UNSIGNED32 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = CO_LINK(0x2101, 2, 16),
        },
        /**
         * Motor Controller RPDO 2
         * Determines the PDO messages to receive when RPDO4 is triggered
         * 0: The number of PDO message associated with the RPDO
         * 1: Dummy Value
         * 2: Battery Voltage
         */
        {
            .Key = CO_KEY(0x1602, 0, CO_UNSIGNED8 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = (uintptr_t) 2,
        },
        {
            .Key = CO_KEY(0x1602, 1, CO_UNSIGNED32 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = CO_LINK(0x2102, 0, 16),
        },
        {
            .Key = CO_KEY(0x1602, 2, CO_UNSIGNED32 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = CO_LINK(0x2102, 1, 16),
        },
        /**
         * User defined data. Put elements that can be accessed via SDO
         * and depending on the configuration PDO
         */
        /* These data values assign TMU RPDO 0 to the 4 thermal temperatures. */
        {
            .Key = CO_KEY(0x2100, 0, CO_UNSIGNED16 | CO_OBJ___PRW),
            .Type = nullptr,
            .Data = (uintptr_t) &thermTemps[0],
        },
        {
            .Key = CO_KEY(0x2100, 1, CO_UNSIGNED16 | CO_OBJ___PRW),
            .Type = nullptr,
            .Data = (uintptr_t) &thermTemps[1],
        },
        {
            .Key = CO_KEY(0x2100, 2, CO_UNSIGNED16 | CO_OBJ___PRW),
            .Type = nullptr,
            .Data = (uintptr_t) &thermTemps[2],
        },
        {
            .Key = CO_KEY(0x2100, 3, CO_UNSIGNED16 | CO_OBJ___PRW),
            .Type = nullptr,
            .Data = (uintptr_t) &thermTemps[3],
        },
        /* These data values assign Motor Controller RPDO 0 to Status Word, Actual Position, and Torque Actual. */
        {
            .Key = CO_KEY(0x2101, 0, CO_UNSIGNED16 | CO_OBJ___PRW),
            .Type = nullptr,
            .Data = (uintptr_t) &statusWord,
        },
        {
            .Key = CO_KEY(0x2101, 1, CO_UNSIGNED32 | CO_OBJ___PRW),
            .Type = nullptr,
            .Data = (uintptr_t) &actualPosition,
        },
        {
            .Key = CO_KEY(0x2101, 2, CO_UNSIGNED16 | CO_OBJ___PRW),
            .Type = nullptr,
            .Data = (uintptr_t) &torqueActual,
        },
        /* These data values assign Motor Controller RPDO 1 to the Total Voltage, and then a dummy value. */
        {
            .Key = CO_KEY(0x2102, 0, CO_UNSIGNED16 | CO_OBJ___PRW),
            .Type = nullptr,
            .Data = (uintptr_t) &dummyValue,
        },
        {
            .Key = CO_KEY(0x2102, 1, CO_UNSIGNED16 | CO_OBJ___PRW),
            .Type = nullptr,
            .Data = (uintptr_t) &totalVoltage,
        },

        CO_OBJ_DIR_ENDMARK,
    };

    static constexpr char* SECTION_TITLES[9]{
        "Voltage",
        "MPH",
        "RPM",
        "Temp 1",
        "Temp 2",
        "Temp 3",
        "MC Stat",
        "Position",
        "Torque"};
};

}// namespace HUDL

#endif