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
        * @param[in] cs is the chip select pin
        * @param[in] spi is the SPI instance
        */
        HUDL(IO::GPIO &reg_select, IO::GPIO &reset, IO::GPIO &cs, IO::SPI &spi);

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
         * @return an object dictionary
         */
        CO_OBJ_T *getObjectDictionary();

        /**
         * Gets the size of the Object Dictionary
         * @return uint16_t size of the Object Dictionary
         */
        uint16_t getObjectDictionarySize();

    private:
        /**
         * reg_select PA_3
         * reset      PB_3
         * cs         PB_12
         */

        DEV::LCD lcd;

        static constexpr uint16_t OBJECT_DICTIONARY_SIZE = 30;

        CO_OBJ_T objectDictionary[OBJECT_DICTIONARY_SIZE + 1] = {
                {
                        .Key = CO_KEY(0x1600, 0, CO_UNSIGNED8 | CO_OBJ_D__R_),
                        .Type = 0,
                        .Data = (uintptr_t) 2
                },
                {
                        .Key = CO_KEY(0x1600, 1, CO_UNSIGNED32 | CO_OBJ_D__R_),
                        .Type = 0,
                        .Data = CO_LINK(0x2100, 0, 8)// Link to 8bit sample data position in dictionary
                },
                {
                        .Key = CO_KEY(0x1600, 2, CO_UNSIGNED32 | CO_OBJ_D__R_),
                        .Type = 0,
                        .Data = CO_LINK(0x2100, 1, 16)// Link to 16bit sample data position in dictionary
                },


                // End of dictionary marker
                CO_OBJ_DIR_ENDMARK,
        };


    };

}// namespace HUDL

#endif
