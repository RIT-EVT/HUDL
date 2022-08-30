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

constexpr uint32_t SPI_SPEED = SPI_SPEED_4MHZ; // 4MHz
constexpr uint8_t deviceCount = 1;

IO::GPIO *devices[deviceCount];




int main() {

    // HUDL board = new HUDL();
    auto board = HUDL::HUDL();
    // register select set
    board.reg_select =
            IO::getGPIO<IO::Pin::PA_3>(EVT::core::IO::GPIO::Direction::OUTPUT);
    board.reg_select.writePin(EVT::core::IO::GPIO::State::LOW);

    // reset set
    board.reset =
            IO::getGPIO<IO::Pin::PB_3>(EVT::core::IO::GPIO::Direction::OUTPUT);

    // reset the board
    board.reset.writePin(EVT::core::IO::GPIO::State::LOW);
    time::wait(100);
    board.reset.writePin(EVT::core::IO::GPIO::State::HIGH);
    time::wait(100);

    // CS
    board.CS =
            IO::getGPIO<IO::Pin::PB_12>(EVT::core::IO::GPIO::Direction::OUTPUT);
    board.CS.writePin(EVT::core::IO::GPIO::State::HIGH);

    // Setup spi
    board.spi.configureSPI(SPI_SPEED, SPI_MODE3, SPI_MSB_FIRST);

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
