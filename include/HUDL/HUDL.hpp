#ifndef _HUDL_
#define _HUDL_

#include <EVT/dev/LCD.hpp>
#include <EVT/io/GPIO.hpp>
#include <EVT/io/CANopen.hpp>
#include <EVT/io/SPI.hpp>
#include <HUDL/HUDL.hpp>
#include <Canopen/co_core.h>


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
        HUDL(IO::GPIO &reg_select, IO::GPIO &reset, IO::SPI &spi);

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
        void clearLCD(const uint8_t *bitMap);

        /**
         * Initializes LCD for use
         */
        void initLCD();

        /**
         * Gets the object dictionary
         *
         * @return an object dictionary
         */
        CO_OBJ_T *getObjectDictionary();

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
        // TODO: Three voltages. Starting with one.
        uint16_t voltages[1] = {};

        // TODO: Four temps. Starting with three.
        uint32_t thermTemps[3] = {};

        static constexpr uint16_t OBJECT_DICTIONARY_SIZE = 30;

        CO_OBJ_T objectDictionary[OBJECT_DICTIONARY_SIZE + 1] = {
                // Sync ID, defaults to 0x80
                {CO_KEY(0x1005, 0, CO_UNSIGNED32 | CO_OBJ_D__R_), 0, (uintptr_t) 0x80},

                // Information about the hardware, hard coded sample values for now
                // 1: Vendor ID
                // 2: Product Code
                // 3: Revision Number
                // 4: Serial Number
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

                // SDO CAN message IDS.
                // 1: Client -> Server ID, default is 0x600 + NODE_ID
                // 2: Server -> Client ID, default is 0x580 + NODE_ID
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

                //RPDO settings
                // 0: RPDO number in index and total number of sub indexes.
                // 1: The COB-ID to receive PDOs from.
                // 2: transmission trigger
                {
                        .Key = CO_KEY(0x1400, 0, CO_UNSIGNED8 | CO_OBJ_D__R_),
                        .Type = nullptr,
                        .Data = (uintptr_t) 3},
                {// 180h+TPDO Node-ID
                        .Key = CO_KEY(0x1400, 1, CO_UNSIGNED32 | CO_OBJ_D__R_),
                        .Type = nullptr,
                        .Data = (uintptr_t) CO_COBID_TPDO_DEFAULT(0) + 1},
                {// asynchronous trigger
                        .Key = CO_KEY(0x1400, 2, CO_UNSIGNED8 | CO_OBJ_D__R_),
                        .Type = nullptr,
                        .Data = (uintptr_t) 0xFE},

                // 0: The number of PDO message associated with the RPDO
                // 1: Link to the first PDO message
                // n: Link to the nth PDO message
                {// maps two objects
                        .Key = CO_KEY(0x1600, 0, CO_UNSIGNED8 | CO_OBJ_D__R_),
                        .Type = nullptr,
                        .Data = (uintptr_t) 2},
                {// link the first byte to (0x2100, 0, 8) - tempOne
                        .Key = CO_KEY(0x1600, 1, CO_UNSIGNED32 | CO_OBJ_D__R_),
                        .Type = nullptr,
                        .Data = CO_LINK(0x2100, 0, 8)},
//                {// link the second byte to (0x2100, 1, 8) - tempTwo
//                        .Key = CO_KEY(0x1600, 2, CO_UNSIGNED32 | CO_OBJ_D__R_),
//                        .Type = nullptr,
//                        .Data = CO_LINK(0x2100, 1, 8)},
//                {// link the third byte to (0x2100, 2, 8) - tempThree
//                        .Key = CO_KEY(0x1600, 1, CO_UNSIGNED32 | CO_OBJ_D__R_),
//                        .Type = nullptr,
//                        .Data = CO_LINK(0x2100, 2, 8)},
//                {// link the fourth byte to (0x2100, 3, 8) - tempFour
//                        .Key = CO_KEY(0x1600, 2, CO_UNSIGNED32 | CO_OBJ_D__R_),
//                        .Type = nullptr,
//                        .Data = CO_LINK(0x2100, 3, 8)},

                // User defined data, this will be where we put elements that can be
                // accessed via SDO and depending on configuration PDO
                {
                        .Key = CO_KEY(0x2100, 0, CO_UNSIGNED32 | CO_OBJ___PR_),
                        .Type = nullptr,
                        .Data = (uintptr_t) &thermTemps[0],
                },
//                {
//                        .Key = CO_KEY(0x2100, 1, CO_UNSIGNED32 | CO_OBJ___PR_),
//                        .Type = nullptr,
//                        .Data = (uintptr_t) &thermTemps[1],
//                },
//                {
//                        .Key = CO_KEY(0x2100, 2, CO_UNSIGNED32 | CO_OBJ___PR_),
//                        .Type = nullptr,
//                        .Data = (uintptr_t) &thermTemps[2],
//                },
//                {
//                        .Key = CO_KEY(0x2100, 3, CO_UNSIGNED32 | CO_OBJ___PR_),
//                        .Type = nullptr,
//                        .Data = (uintptr_t) &tempFour,
//                },

                // End of dictionary marker
                CO_OBJ_DIR_ENDMARK};


    };

}// namespace HUDL

#endif
