/**
 * This is a basic sample of using the UART module. The program provides a
 * basic echo functionality where the uart will write back whatever the user
 * enters.
 */
#include <stdint.h>

#include <EVT/io/GPIO.hpp>
#include <EVT/io/I2C.hpp>
#include <EVT/io/UART.hpp>
#include <EVT/io/manager.hpp>
#include <EVT/io/pin.hpp>
#include <EVT/utils/time.hpp>
#include <HUDL/HUDL.hpp>

namespace IO = EVT::core::IO;
namespace time = EVT::core::time;
using namespace std;

const uint32_t SPI_SPEED = SPI_SPEED_4MHZ; // 4MHz
const uint8_t deviceCount = 1;



int main() {
    IO::GPIO *devices[deviceCount];
    auto &reg_select = IO::getGPIO<IO::Pin::PA_3>(EVT::core::IO::GPIO::Direction::OUTPUT);
    reg_select.writePin(IO::GPIO::State::LOW);

    auto &reset = IO::getGPIO<IO::Pin::PB_3>(EVT::core::IO::GPIO::Direction::OUTPUT);
    reset.writePin(EVT::core::IO::GPIO::State::LOW);
    time::wait(100);
    reset.writePin(EVT::core::IO::GPIO::State::HIGH);
    time::wait(100);

    auto &cs = IO::getGPIO<IO::Pin::PB_12>(EVT::core::IO::GPIO::Direction::OUTPUT);
    cs.writePin(EVT::core::IO::GPIO::State::HIGH);

    auto &spi = IO::getSPI<IO::Pin::PB_13, EVT::core::IO::Pin::PB_15, IO::Pin::PC_11>(devices, deviceCount);
    spi.configureSPI(SPI_SPEED, SPI_MODE3, SPI_MSB_FIRST);

    auto board = HUDL::HUDL(reg_select, reset, cs, spi);


    // Setup UART
    IO::UART &uart = IO::getUART<IO::Pin::UART_TX, IO::Pin::UART_RX>(9600);

    // String to store user input
    char buf[100];

    while (1) {
        // Read user input
        uart.printf("Enter message: ");
        uart.gets(buf, 100);

        // Initialized LCD
        board.init_LCD();

        // Clear LCD Screen
        // ClearLCD();

        board.drive_pixel(1, 1, 1, 255);
        time::wait(10000);

        // echos command back
        uart.printf("\n\recho: %s\n\r", buf);
    }
}
