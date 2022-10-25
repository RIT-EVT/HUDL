#ifndef _HUDL_
#define _HUDL_

#include "EVT/dev/LCD.hpp"
#include "EVT/io/GPIO.hpp"
#include "EVT/io/SPI.hpp"
#include "HUDL/HUDL.hpp"

namespace IO = EVT::core::IO;
namespace DEV = EVT::core::DEV;


namespace HUDL {
    class HUDL {
    public:
        /**
        * Default Constructor for the HUDL class
         *
        * @param[in] reg_select is the register select pin
        * @param[in] reset is the reset pin
        * @param[in] cs is the chip select pin
        * @param[in] spi is the SPI instance
        */
        HUDL(IO::GPIO& reg_select, IO::GPIO& reset, IO::GPIO& cs, IO::SPI& spi);

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
        * @param[in] lcd_string
        */
        void clearLCD(const uint8_t* lcd_string);

        /**
         * Initializes LCD for use
         */
        void initLCD();

    private:
        // TODO: Remove later. Commented out since it is used for LCD now
//    IO::GPIO& reg_select;// PA_3
//    IO::GPIO& reset;     // PB_3
//    IO::GPIO& cs;        // PB_12
//    IO::SPI& spi;

        DEV::LCD lcd;

    };
}// namespace HUDL

#endif
