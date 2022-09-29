#ifndef _HUDL_
#define _HUDL_

namespace IO = EVT::core::IO;

namespace HUDL {
class HUDL {
public:
  IO::GPIO &reg_select; // PA_3
  IO::GPIO &reset;      // PB_3
  IO::GPIO &cs;         // PB_12
  IO::SPI &spi;

  // default constructor for HUDL class
  HUDL(IO::GPIO &reg_select, IO::GPIO &reset, IO::GPIO &cs, IO::SPI &spi);

    /**
   * Writes data to the LCD to show on the screen
   * @param data being written to LCD
   */
  void data_write(unsigned char data);

    /**
   * Writes commands to the LCD to control the ST7565
   * @param data being written for the command
   */
  void comm_write(unsigned char data);

    /**
   * Writes data to a single pixel
   * @param page is the page address to write data to
   * @param col_up is the first four bits of the column write
   * @param col_low is the last four bits of the column write
   * @param data is the data value to write
   */
  void drive_pixel(unsigned char page, unsigned char col_up,
                   unsigned char col_low, unsigned char data);

    /**
   * Clears the screen
   * @param lcd_string
   */
  void clear_lcd(unsigned char *lcd_string);

    /**
    * Initializes LCD for use
    */
  void init_LCD();
};
} // namespace HUDL

#endif
