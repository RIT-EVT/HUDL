/**
 * This is a basic sample of using the UART module. The program provides a
 * basic echo functionality where the uart will write back whatever the user
 * enters.
 */
#include <stdint.h>

#include <EVT/io/CANopen.hpp>
#include <EVT/io/GPIO.hpp>
#include <EVT/io/UART.hpp>
#include <EVT/io/manager.hpp>
#include <EVT/io/pin.hpp>
#include <EVT/utils/time.hpp>
#include <HUDL/HUDL.hpp>
#include <EVT/dev/Timer.hpp>
#include <HALf3/stm32f3xx_hal_tim.h>
#include <EVT/dev/platform/f3xx/f302x8/Timerf302x8.hpp>


namespace IO = EVT::core::IO;
namespace DEV = EVT::core::DEV;
namespace time = EVT::core::time;
using namespace std;

const uint32_t SPI_SPEED = SPI_SPEED_4MHZ;// 4MHz
const uint8_t deviceCount = 1;

/**
 * This struct is a catchall for data that is needed by the CAN interrupt
 * handler. An instance of this struct will be provided as the parameter
 * to the interrupt handler.
 */
struct CANInterruptParams {
    EVT::core::types::FixedQueue<CANOPEN_QUEUE_SIZE, IO::CANMessage>* queue;
};

/**
 * Interrupt handler for incoming CAN messages.
 *
 * @param priv[in] The private data (FixedQueue<CANOPEN_QUEUE_SIZE, CANMessage>)
 */
void canInterruptHandler(IO::CANMessage& message, void* priv) {
    struct CANInterruptParams* params = (CANInterruptParams*) priv;

    EVT::core::types::FixedQueue<CANOPEN_QUEUE_SIZE, IO::CANMessage>* queue =
            params->queue;

    if (queue == nullptr)
        return;
    if (!message.isCANExtended())
        queue->append(message);
}

int main() {
    IO::GPIO* devices[deviceCount];
    auto& reg_select = IO::getGPIO<IO::Pin::PA_3>(IO::GPIO::Direction::OUTPUT);
    reg_select.writePin(IO::GPIO::State::LOW);

    auto& reset = IO::getGPIO<IO::Pin::PB_3>(IO::GPIO::Direction::OUTPUT);
    reset.writePin(IO::GPIO::State::LOW);
    time::wait(100);
    reset.writePin(IO::GPIO::State::HIGH);
    time::wait(100);

    auto& cs = IO::getGPIO<IO::Pin::PB_12>(IO::GPIO::Direction::OUTPUT);
    cs.writePin(IO::GPIO::State::HIGH);

    auto& hudl_spi = IO::getSPI<IO::Pin::PB_13, IO::Pin::PB_15, IO::Pin::PC_11>(
        devices, deviceCount);
    hudl_spi.configureSPI(SPI_SPEED, SPI_MODE3, SPI_MSB_FIRST);

    auto board = HUDL::HUDL(reg_select, reset, cs, hudl_spi);

    // Setup UART
    IO::UART& uart = IO::getUART<IO::Pin::UART_TX, IO::Pin::UART_RX>(9600);

    // Setup CAN


    IO::CAN& can = IO::getCAN<IO::Pin::PA_12, IO::Pin::PA_11>(); // TODO: Figure out CAN pins
    // TODO: Need a CAN interrupt handler


    IO::CAN::CANStatus result = can.connect();

    // Setup CAN Drivers
    CO_IF_DRV canStackDriver;
    CO_IF_CAN_DRV canDriver;
    CO_IF_TIMER_DRV timerDriver;
    CO_IF_NVM_DRV nvmDriver;
    EVT::core::types::FixedQueue<CANOPEN_QUEUE_SIZE, IO::CANMessage> canOpenQueue;
    DEV::Timerf302x8 timer(TIM2, 100);
    IO::getCANopenCANDriver(&can, &canOpenQueue, &canDriver);
    IO::getCANopenTimerDriver(&timer, &timerDriver);
    IO::getCANopenNVMDriver(&nvmDriver);



    uint16_t tempOne;
    uint16_t voltageOne;

    while (1) {
        // Initialized LCD
        board.initLCD();


        board.drivePixel(1, 1, 1, 255);
        time::wait(10000);

        // echoes command back
        uart.printf("Temp One: %d\n", tempOne);
        uart.printf("Temp One: %d\n", voltageOne);

    }
}
