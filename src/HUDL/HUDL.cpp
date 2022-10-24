/**
 * This is a basic sample of using the UART module. The program provides a
 * basic echo functionality where the uart will write back whatever the user
 * enters.
 */

// clang-format off
#include "EVT/dev/LCD.hpp"
#include "EVT/io/GPIO.hpp"
#include "EVT/io/SPI.hpp"
#include "HUDL/HUDL.hpp"
// clang-format on

namespace IO = EVT::core::IO;

namespace HUDL {
HUDL::HUDL(IO::GPIO& reg_select, IO::GPIO& reset, IO::GPIO& cs, IO::SPI& spi) : lcd(DEV::LCD(reg_select, reset, spi)) {


}

void HUDL::data_write(unsigned char data) {

}

void HUDL::comm_write(unsigned char data) {
}

void HUDL::drive_pixel(unsigned char page, unsigned char col_up,
                       unsigned char col_low, unsigned char data) {

}

void HUDL::clear_lcd(unsigned char* lcd_string) {

}

void HUDL::init_LCD() {

}

}// namespace HUDL