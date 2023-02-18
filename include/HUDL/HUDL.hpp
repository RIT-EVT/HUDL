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
    uint16_t getObjectDictionarySize();

    /**
     * Updates the LCD display with values received from the CAN network
     */
    void updateLCD();
private:
    /**
     * reg_select PA_3
     * reset      PB_3
     * cs         PB_12
     */
    DEV::LCD lcd;
    uint32_t totalVoltage = 0;

    uint32_t thermTemps[4] = {};

    /** The status word provided by the MC node over CAN. Found in the first 16 bits of the 1st PDO coming from the MC. */
    uint16_t statusWord = 0;
    /** The position actual value provided by the MC node over CAN. Found in the middle 32 bits of the 1st PDO coming from the MC. */
    uint32_t positionActual = 0;
    /** The torque actual value provided by the MC node over CAN. Found in the last 16 bits of the 1st PDO coming from the MC. */
    uint16_t torqueActual = 0;

    uint16_t velocityActual = 0;

    uint32_t rpdo4First32BitsDummyData = 0;

    static constexpr uint16_t OBJECT_DICTIONARY_SIZE = 47;
    static constexpr uintptr_t TMS_NODE_ID = 0x08;
    static constexpr uintptr_t BMS_NODE_ID = 0x05;
    static constexpr uintptr_t MC_NODE_ID = 0x01;

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
         * RPDO0 settings
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
         * RPDO1 settings
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
            .Data = CO_COBID_TPDO_DEFAULT(1) + TMS_NODE_ID,
        },
        {
            .Key = CO_KEY(0x1401, 2, CO_UNSIGNED8 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = (uintptr_t) 0xFE,
        },

        /**
         * RPDO2 settings
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
            .Data = (uintptr_t) CO_COBID_TPDO_DEFAULT(0) + BMS_NODE_ID,
        },
        {
            .Key = CO_KEY(0x1402, 2, CO_UNSIGNED8 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = (uintptr_t) 0xFE,
        },

        /**
         * RPDO3 settings
         * 0: RPDO number in index and total number of sub indexes.
         * 1: The COB-ID to receive PDOs from.
         * 2: transmission trigger
         */
        {
            .Key = CO_KEY(0x1403, 0, CO_UNSIGNED8 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = (uintptr_t) 3,
        },
        {
            .Key = CO_KEY(0x1403, 1, CO_UNSIGNED32 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = (uintptr_t) CO_COBID_TPDO_DEFAULT(0) + MC_NODE_ID,
        },
        {
            .Key = CO_KEY(0x1403, 2, CO_UNSIGNED8 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = (uintptr_t) 0xFE,
        },
        /**
         * RPDO4 settings
         * 0: RPDO number in index and total number of sub indexes.
         * 1: The COB-ID to receive PDOs from.
         * 2: transmission trigger
         */
        {
            .Key = CO_KEY(0x1404, 0, CO_UNSIGNED8 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = (uintptr_t) 3,
        },
        {
            .Key = CO_KEY(0x1404, 1, CO_UNSIGNED32 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = (uintptr_t) CO_COBID_TPDO_DEFAULT(3) + MC_NODE_ID,
        },
        {
            .Key = CO_KEY(0x1404, 2, CO_UNSIGNED8 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = (uintptr_t) 0xFE,
        },

        /**
         * RPDO0 mapping, determines the PDO messages to receive when RPDO0 is triggered
         * 0: The number of PDO message associated with the RPDO
         * 1: Link to the first PDO message - tempOne
         * 2: Link to the second PDO message - tempTwo
         */
        {
            .Key = CO_KEY(0x1600, 0, CO_UNSIGNED8 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = (uintptr_t) 2,
        },
        {
            .Key = CO_KEY(0x1600, 1, CO_UNSIGNED32 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = CO_LINK(0x2100, 0, 32),
        },
        {
            .Key = CO_KEY(0x1600, 2, CO_UNSIGNED32 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = CO_LINK(0x2100, 1, 32),
        },

        /**
         * RPDO1 mapping, determines the PDO messages to receive when RPDO1 is triggered
         * 0: The number of PDO message associated with the RPDO
         * 1: Link to the first PDO message - tempThree
         * 2: Link to the second PDO message - tempFour
         */
        {
            .Key = CO_KEY(0x1601, 0, CO_UNSIGNED8 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = (uintptr_t) 2,
        },
        {
            .Key = CO_KEY(0x1601, 1, CO_UNSIGNED32 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = CO_LINK(0x2100, 2, 32),
        },
        {
            .Key = CO_KEY(0x1601, 2, CO_UNSIGNED32 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = CO_LINK(0x2100, 3, 32),
        },

        /**
         * RPDO2 mapping, determines the PDO messages to receive when RPDO1 is triggered
         * 0: The number of PDO message associated with the RPDO
         * 1: Link to the first PDO message - totalVoltage
         */
        {
            .Key = CO_KEY(0x1602, 0, CO_UNSIGNED8 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = (uintptr_t) 1,
        },
        {
            .Key = CO_KEY(0x1602, 1, CO_UNSIGNED32 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = CO_LINK(0x2101, 0, 32),
        },

        /**
         * RPDO3 mapping, determines the PDO messages to receive when RPDO3 is triggered
         * 0: The number of PDO message associated with the RPDO
         * 1: Link to the first PDO message - statusWord
         * 2: Link to the second PD0 message - positionActual
         * 2: Link to the third PD0 message - torqueActual
         */
        {
            .Key = CO_KEY(0x1603, 0, CO_UNSIGNED8 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = (uintptr_t) 3,
        },
        {
            .Key = CO_KEY(0x1603, 1, CO_UNSIGNED32 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = CO_LINK(0x2103, 0, 16),
        },
        {
            .Key = CO_KEY(0x1603, 2, CO_UNSIGNED32 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = CO_LINK(0x2103, 1, 32),
        },
        {
            .Key = CO_KEY(0x1603, 3, CO_UNSIGNED32 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = CO_LINK(0x2103, 2, 16),
        },

        /**
         * RPDO4 mapping, determines the PDO messages to receive when RPDO4 is triggered
         * 0: The number of PDO message associated with the RPDO
         * 1: Link to the first PDO message - empty value
         * 2: Link to the second PDO message - velocityActual
         */
        {
            .Key = CO_KEY(0x1604, 0, CO_UNSIGNED8 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = (uintptr_t) 2,
        },
        {
            .Key = CO_KEY(0x1604, 1, CO_UNSIGNED32 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = CO_LINK(0x2104, 0, 32),
        },
        {
            .Key = CO_KEY(0x1604, 2, CO_UNSIGNED32 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = CO_LINK(0x2104, 1, 32),
        },
        /**
         * User defined data. Put elements that can be accessed via SDO
         * and depending on the configuration PDO
         */
        {
            .Key = CO_KEY(0x2100, 0, CO_UNSIGNED32 | CO_OBJ___PRW),
            .Type = nullptr,
            .Data = (uintptr_t) &thermTemps[0],
        },
        {
            .Key = CO_KEY(0x2100, 1, CO_UNSIGNED32 | CO_OBJ___PRW),
            .Type = nullptr,
            .Data = (uintptr_t) &thermTemps[1],
        },
        {
            .Key = CO_KEY(0x2100, 2, CO_UNSIGNED32 | CO_OBJ___PRW),
            .Type = nullptr,
            .Data = (uintptr_t) &thermTemps[2],
        },
        {
            .Key = CO_KEY(0x2100, 3, CO_UNSIGNED32 | CO_OBJ___PRW),
            .Type = nullptr,
            .Data = (uintptr_t) &thermTemps[3],
        },
        {
            .Key = CO_KEY(0x2101, 0, CO_UNSIGNED32 | CO_OBJ___PRW),
            .Type = nullptr,
            .Data = (uintptr_t) &totalVoltage,
        },
        {
            .Key = CO_KEY(0x2103, 0, CO_UNSIGNED16 | CO_OBJ___PRW),
            .Type = nullptr,
            .Data = (uintptr_t) &statusWord,
        },
        {
            .Key = CO_KEY(0x2103, 1, CO_UNSIGNED32 | CO_OBJ___PRW),
            .Type = nullptr,
            .Data = (uintptr_t) &positionActual,
        },
        {
            .Key = CO_KEY(0x2103, 2, CO_UNSIGNED16 | CO_OBJ___PRW),
            .Type = nullptr,
            .Data = (uintptr_t) &torqueActual,
        },
        {
            .Key = CO_KEY(0x2104, 0, CO_UNSIGNED32 | CO_OBJ___PRW),
            .Type = nullptr,
            .Data = (uintptr_t) &rpdo4First32BitsDummyData,
        },
        {
            .Key = CO_KEY(0x2104, 1, CO_UNSIGNED32 | CO_OBJ___PRW),
            .Type = nullptr,
            .Data = (uintptr_t) &velocityActual,
        },
        CO_OBJ_DIR_ENDMARK,
    };

    static constexpr char* SECTION_TITLES[9]{
        "B Voltage",
        "Velocity",
        "RPM",
        "Temp 1",
        "Temp 2",
        "Temp 3",
        "MC Stat",
        "Position",
        "Torque"
    };
};

}// namespace HUDL

#endif
