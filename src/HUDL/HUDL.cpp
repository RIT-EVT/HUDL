/**
 * This is a basic sample of using the UART module. The program provides a
 * basic echo functionality where the uart will write back whatever the user
 * enters.
 */

// clang-format off
#include "EVT/io/GPIO.hpp"
#include "EVT/io/SPI.hpp"
#include "HUDL/HUDL.hpp"
// clang-format on

namespace IO = EVT::core::IO;

namespace HUDL {
HUDL::HUDL(IO::GPIO &reg_select, IO::GPIO &reset, IO::GPIO &cs, IO::SPI &spi)
    : reg_select(reg_select), reset(reset), cs(cs), spi(spi) {}


void HUDL::data_write(unsigned char data) {
  cs.writePin(EVT::core::IO::GPIO::State::LOW);
  reg_select.writePin(EVT::core::IO::GPIO::State::HIGH);
  spi.startTransmission(0);
  spi.write(&data, 1);
  spi.endTransmission(0);
  cs.writePin(EVT::core::IO::GPIO::State::HIGH);
}


void HUDL::comm_write(unsigned char data) {
  cs.writePin(EVT::core::IO::GPIO::State::LOW);
  reg_select.writePin(EVT::core::IO::GPIO::State::LOW);
  spi.startTransmission(0);
  spi.write(&data, 1);
  spi.endTransmission(0);
  cs.writePin(EVT::core::IO::GPIO::State::HIGH);
}


void HUDL::drive_pixel(unsigned char page, unsigned char col_up,
                       unsigned char col_low, unsigned char data) {
  HUDL::comm_write(
      0x40); // line to start writing on (0 -> 64) moves set bits with it
  // DO NOT CHANGE
  comm_write(
      0xB0 +
      page); // writes the page address (4 bits, 8 rows selected by values 0-7 )
  comm_write(
      0x10 +
      col_up); // writes the first 4 bits of the column select (out of 8 bits)
  comm_write(0x00 +
             col_low); // writes the second 4 bits of the column select (out)

  data_write(data); // writes 8 vertical bits based on value between 0-255 based
                    // on bits set ex: 01001100 is       |WHITE|
                    //  |BLACK|
                    //  |WHITE|
                    //  |WHITE|
                    //  |BLACK|
                    //  |BLACK|
                    //  |WHITE|
                    //  |WHITE|
}


void HUDL::clear_lcd(unsigned char *lcd_string) {
  unsigned char page = 0xB0;
  comm_write(0xAE); // Display OFF
  comm_write(0x40); // Display start address + 0x40

  // 64 pixel display / 8 pixels per page = 8 pages
  for (int i = 0; i < 8; i++) {
    comm_write(page); // send page address
    comm_write(0x10); // column address upper 4 bits + 0x10
    comm_write(0x00); // column address lower 4 bits + 0x00

    // 128 columns wide
    for (int j = 0; j < 128; j++) {
      data_write(0x00); // write clear pixels
      lcd_string++;
    }

    page++; // after 128 columns, go to next page
  }

  comm_write(0xAF);
}


void HUDL::init_LCD() {
  HUDL::comm_write(0xA0); // ADC select
  HUDL::comm_write(0xAE); // Display OFF
  HUDL::comm_write(0xC8); // COM direction scan
  HUDL::comm_write(0xA2); // LCD bias set
  HUDL::comm_write(0x2F); // Power Control set
  HUDL::comm_write(0x26); // Resistor Ratio Set
  HUDL::comm_write(
      0x81); // Electronic Volume Command (set contrast) Double Byte: 1 of 2
  HUDL::comm_write(
      0x11); // Electronic Volume value (contrast value) Double Byte: 2 of 2
  HUDL::comm_write(0xAF); // Display ON
}

} // namespace HUDL