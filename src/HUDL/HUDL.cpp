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
HUDL::HUDL(IO::GPIO& reg_select, IO::GPIO& reset, IO::GPIO& cs, IO::SPI& spi)
    : reg_select(reg_select), reset(reset), cs(cs), spi(spi) {}


}// namespace HUDL