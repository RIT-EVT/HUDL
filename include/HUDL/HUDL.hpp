#ifndef _HUDL_
#define _HUDL_

namespace IO = EVT::core::IO;

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


private:
    // TODO: These pins will go into an LCD class which will be a field instead
    IO::GPIO& reg_select;// PA_3
    IO::GPIO& reset;     // PB_3
    IO::GPIO& cs;        // PB_12
    IO::SPI& spi;
};
}// namespace HUDL

#endif
