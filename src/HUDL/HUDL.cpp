/**
 * This is a basic sample of using the UART module. The program provides a
 * basic echo functionality where the uart will write back whatever the user
 * enters.
 */

#include "EVT/io/GPIO.hpp"
#include "EVT/io/SPI.hpp"

namespace IO = EVT::core::IO;

namespace HUDL {
class HUDL {
public:
  // IO::GPIO SCL;  // PB_13 (PA5 on F334)
  // IO::GPIO SI;  // PB_15 (PA7 on F334)
  IO::GPIO &reg_select; // PA_3
  IO::GPIO &reset;      // PB_3
  IO::GPIO &cs;         // PB_12
  IO::SPI &spi;

  HUDL(IO::GPIO &reg_select, IO::GPIO &reset, IO::GPIO &cs, IO::SPI &spi)
      : reg_select(reg_select), reset(reset), cs(cs), spi(spi) {}

  void data_write(unsigned char d);

  void comm_write(unsigned char d);

  void drive_pixel(unsigned char page, unsigned char col_up,
                   unsigned char col_low, unsigned char data);

  void clear_lcd(unsigned char *lcd_string);

  void init_LCD();
};

// command write function
// @param: d : the data being written for the command
void HUDL::data_write(unsigned char d) // Data Output Serial Interface
{
  cs.writePin(EVT::core::IO::GPIO::State::LOW);
  reg_select.writePin(EVT::core::IO::GPIO::State::HIGH);
  spi.startTransmission(0);
  spi.write(&d, 1);
  spi.endTransmission(0);
  cs.writePin(EVT::core::IO::GPIO::State::HIGH);
}

// command write function
// @param: d : the data being written for the command
void HUDL::comm_write(unsigned char d) {
  cs.writePin(EVT::core::IO::GPIO::State::LOW);
  reg_select.writePin(EVT::core::IO::GPIO::State::LOW);
  spi.startTransmission(0);
  spi.write(&d, 1);
  spi.endTransmission(0);
  cs.writePin(EVT::core::IO::GPIO::State::HIGH);
}

// writes data to a single pixel
// @param: page : the page address to write data to
// @param: col_up : the first four bits of the column write
// @param: col_low : the last four bits of the column write
// @param: data : the data value to write
void HUDL::drive_pixel(unsigned char page, unsigned char col_up,
                       unsigned char col_low, unsigned char data) {
  comm_write(0x40); // line to start writing on (0 -> 64) moves set bits with it
  // DO NOT CHANGE
  comm_write(
      0xB0 +
      page); // writes the page address (4 bits, 8 rows selcted by values 0-7 )
  comm_write(
      0x10 +
      col_up); // writes the first 4 bits of the column select (out of 8 bits)
  comm_write(0x00 +
             col_low); // writes the second 4 bits of the column select (out)

  data_write(
      data); // writes 8 vertical bits based on value between 0-255 based on
             // bits set ex: 01001100 is       |WHITE|
  //                                                                                             |BLACK|
  //                                                                                             |WHITE|
  //                                                                                             |WHITE|
  //                                                                                             |BLACK|
  //                                                                                             |BLACK|
  //                                                                                             |WHITE|
  //                                                                                             |WHITE|
}

// clears the LCD screen
// @param: lcd_string :
void HUDL::clear_lcd(unsigned char *lcd_string) {
  unsigned char page = 0xB0;
  comm_write(0xAE); // Display OFF
  comm_write(0x40); // Display start address + 0x40
  for (int i = 0; i < 8;
       i++) {         // 64 pixel display / 8 pixels per page = 8 pages
    comm_write(page); // send page address
    comm_write(0x10); // column address upper 4 bits + 0x10
    comm_write(0x00); // column address lower 4 bits + 0x00
    for (int j = 0; j < 128; j++) { // 128 columns wide
      data_write(0x00);             // write clear pixels
      lcd_string++;
    }
    page++; // after 128 columns, go to next page
  }
  comm_write(0xAF);
}

// initializes LCD for use
void HUDL::init_LCD() {
  comm_write(0xA0); // ADC select
  comm_write(0xAE); // Display OFF
  comm_write(0xC8); // COM direction scan
  comm_write(0xA2); // LCD bias set
  comm_write(0x2F); // Power Control set
  comm_write(0x26); // Resistor Ratio Set
  comm_write(
      0x81); // Electronic Volume Command (set contrast) Double Btye: 1 of 2
  comm_write(
      0x11); // Electronic Volume value (contrast value) Double Byte: 2 of 2
  comm_write(0xAF); // Display ON
}
} // namespace HUDL