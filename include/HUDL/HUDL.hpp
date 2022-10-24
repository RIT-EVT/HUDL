#ifndef _HUDL_
#define _HUDL_

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
        void dataWrite(unsigned char data);

        /**
        * Writes commands to the LCD to control the ST7565
         *
        * @param data being written for the command
        */
        void commWrite(unsigned char data);

        /**
        * Writes data to a single pixel
         *
        * @param[in] page is the page address to write data to
        * @param[in] colUp is the first four bits of the column write
        * @param[in] colLow is the last four bits of the column write
        * @param[in] data is the data value to write
        */
        void drivePixel(unsigned char page, unsigned char colUp,
                        unsigned char colLow, unsigned char data);

        /**
        * Clears the screen
         *
        * @param[in] lcd_string
        */
        void clearLCD(unsigned char* lcd_string);

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
