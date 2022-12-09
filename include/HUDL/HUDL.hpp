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
* @param[in] cs is the chip select piarrayn
* @param[in] spi is the SPI instance
*/
    HUDL(IO::GPIO& reg_select, IO::GPIO& reset, IO::SPI& spi);

    /**
* Writes data to the LCD to show on the screen
*
* @param[in] data being written to LCD
*/
    void dataWrite(uint8_t data);

    /**
* Writes commands to the LCD to control the ST7565
*
* @param data being written for the command
*/
    void commWrite(uint8_t data);

    /**
* Writes data to a single pixel
*
* @param[in] page is the page address to write data to
* @param[in] colUp is the first four bits of the column write
* @param[in] colLow is the last four bits of the column write
* @param[in] data is the data value to write
*/
    void drivePixel(uint8_t page, uint8_t colUp, uint8_t colLow, uint8_t data);

    /**
* Clears the screen
*
* @param[in] bitMap a pointer to the bitmap to be displayed
*/
    void clearLCD(const uint8_t* bitMap);

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
 * @return uint16_t size of the Object Dictionary
 */
    uint16_t getObjectDictionarySize() const;

    void displayMap(uint8_t* bitmap);

    /**
* Gets BMS Voltage values
*
* @return uint16_t* pointer to voltage values
 */
    uint16_t* getVoltages();

    /**
* Gets temperature values
*
* @return uint32_t* pointer of temperature values
*/
    uint32_t* getThermTemps();

private:
    /**
         * reg_select PA_3
         * reset      PB_3
         * cs         PB_12
         */
    DEV::LCD lcd;
    uint16_t voltages[1] = {};

    uint32_t thermTemps[5] = {0, 0, 0, 0};

    static constexpr uint16_t OBJECT_DICTIONARY_SIZE = 30;

    CO_OBJ_T objectDictionary[OBJECT_DICTIONARY_SIZE + 1] = {
        {CO_KEY(0x1005, 0, CO_UNSIGNED32 | CO_OBJ_D__R_), 0, (uintptr_t) 0x80},
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

        {
            .Key = CO_KEY(0x1200, 1, CO_UNSIGNED32 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = (uintptr_t) 0x600 + NODE_ID,// TODO: Check with TMS NODE ID
        },
        {
            .Key = CO_KEY(0x1200, 2, CO_UNSIGNED32 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = (uintptr_t) 0x580 + NODE_ID,
        },

        {
            .Key = CO_KEY(0x1400, 0, CO_UNSIGNED8 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = (uintptr_t) 3,
        },
        {
            .Key = CO_KEY(0x1400, 1, CO_UNSIGNED32 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = (uintptr_t) CO_COBID_TPDO_DEFAULT(0),
        },
        {
            .Key = CO_KEY(0x1400, 2, CO_UNSIGNED8 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = (uintptr_t) 0xFE,
        },

        {
            .Key = CO_KEY(0x1401, 0, CO_UNSIGNED8 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = (uintptr_t) 3,
        },
        {
            .Key = CO_KEY(0x1401, 1, CO_UNSIGNED32 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = (uintptr_t) CO_COBID_TPDO_DEFAULT(1),
        },
        {
            .Key = CO_KEY(0x1401, 2, CO_UNSIGNED8 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = (uintptr_t) 0xFE,
        },

        {
            .Key = CO_KEY(0x1600, 0, CO_UNSIGNED8 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = (uintptr_t) 2,
        },
        {
            .Key = CO_KEY(0x1600, 1, CO_UNSIGNED32 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = CO_LINK(0x2100, 0, 8),
        },
        {
            .Key = CO_KEY(0x1600, 2, CO_UNSIGNED32 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = CO_LINK(0x2100, 1, 8),
        },

        {
            .Key = CO_KEY(0x1601, 0, CO_UNSIGNED8 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = (uintptr_t) 2,
        },
        {
            .Key = CO_KEY(0x1601, 1, CO_UNSIGNED32 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = CO_LINK(0x2100, 2, 8),
        },
        {
            .Key = CO_KEY(0x1601, 2, CO_UNSIGNED32 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = CO_LINK(0x2100, 3, 8),
        },

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
        CO_OBJ_DIR_ENDMARK};
};

}// namespace HUDL

#endif
