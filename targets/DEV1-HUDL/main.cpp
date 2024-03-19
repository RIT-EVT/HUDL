/**
 * This is the main code running on the HUDL responsible for displaying
 * information that other boards that broadcast through the CAN network
 */
#include <cstdint>

#include <EVT/dev/Timer.hpp>
#include <EVT/io/CANopen.hpp>
#include <EVT/io/GPIO.hpp>
#include <EVT/io/pin.hpp>
#include <EVT/manager.hpp>
#include <EVT/utils/log.hpp>
#include <EVT/utils/time.hpp>

#include <HUDL.hpp>

namespace IO = EVT::core::IO;
namespace DEV = EVT::core::DEV;
namespace time = EVT::core::time;
namespace log = EVT::core::log;
namespace types = EVT::core::types;
using namespace std;

#define SPI_SPEED SPI_SPEED_500KHZ
#define DEVICE_COUNT 1

///////////////////////////////////////////////////////////////////////////////
// CANopen specific Callbacks. Need to be defined in some location
///////////////////////////////////////////////////////////////////////////////

/**
 * Interrupt handler to get CAN messages. A function pointer to this function
 * will be passed to the EVT-core CAN interface which will in turn call this
 * function each time a new CAN message comes in.
 *
 * NOTE: For this sample, every non-extended (so 11  bit CAN IDs) will be
 * assumed to be intended to be passed as a CANopen message.
 *
 * @param message[in] The passed in CAN message that was read.
 */
// create a can interrupt handler
void canInterrupt(IO::CANMessage& message, void* priv) {
    auto* queue = (types::FixedQueue<CANOPEN_QUEUE_SIZE, IO::CANMessage>*) priv;

    if (queue != nullptr) {
        queue->append(message);
    }
}

int main() {
    // Initialize system
    EVT::core::platform::init();

    IO::UART& uart = IO::getUART<IO::Pin::PB_9, IO::Pin::PB_8>(9600);
    log::LOGGER.setUART(&uart);
    log::LOGGER.setLogLevel(log::Logger::LogLevel::INFO);

    // Initialize the timer
    DEV::Timerf3xx timer(TIM2, 160);

    //create the RPDO node
    IO::GPIO* devices[DEVICE_COUNT];

    IO::GPIO& regSelect = IO::getGPIO<IO::Pin::PA_3>(IO::GPIO::Direction::OUTPUT);

    IO::GPIO& reset = IO::getGPIO<IO::Pin::PB_7>(IO::GPIO::Direction::OUTPUT);
    devices[0] = &IO::getGPIO<IO::Pin::PB_12>(IO::GPIO::Direction::OUTPUT);
    devices[0]->writePin(IO::GPIO::State::HIGH);

    auto& hudl_spi = IO::getSPI<IO::Pin::SPI_SCK, IO::Pin::SPI_MOSI>(devices, DEVICE_COUNT);

    IO::PWM& brightness = IO::getPWM<IO::Pin::PC_0>();
    brightness.setPeriod(1);
    brightness.setDutyCycle(100);

    hudl_spi.configureSPI(SPI_SPEED, SPI_MODE0, SPI_MSB_FIRST);

    HUDL::HUDL hudl(regSelect, reset, hudl_spi);

    ///////////////////////////////////////////////////////////////////////////
    // Setup CAN configuration, this handles making drivers, applying settings.
    // And generally creating the CANopen stack node which is the interface
    // between the application (the code we write) and the physical CAN network
    ///////////////////////////////////////////////////////////////////////////

    // Will store CANopen messages that will be populated by the EVT-core CAN
    // interrupt
    types::FixedQueue<CANOPEN_QUEUE_SIZE, IO::CANMessage> canOpenQueue;

    // Initialize CAN, add an IRQ which will add messages to the queue above
    IO::CAN& can = IO::getCAN<IO::Pin::PA_12, IO::Pin::PA_11>();
    can.addIRQHandler(canInterrupt, reinterpret_cast<void*>(&canOpenQueue));

    // Reserved memory for CANopen stack usage
    uint8_t sdoBuffer[CO_SSDO_N * CO_SDO_BUF_BYTE];
    CO_TMR_MEM appTmrMem[16];

    // Reserve CAN drivers
    CO_IF_DRV canStackDriver;

    CO_IF_CAN_DRV canDriver;
    CO_IF_TIMER_DRV timerDriver;
    CO_IF_NVM_DRV nvmDriver;

    // Reserve canNode
    CO_NODE canNode;

    // Adds CAN filtering to only allow messages from IDs 1, 5, and 8.
    can.addCANFilter(0x1, 0b00001111111, 0);
    can.addCANFilter(0x8, 0b00001111111, 1);
    can.addCANFilter(0x5, 0b00001111111, 2);

    // Attempt to join the CAN network
    IO::CAN::CANStatus result = can.connect();

    // test that the board is connected to the can network
    if (result != IO::CAN::CANStatus::OK) {
        log::LOGGER.log(log::Logger::LogLevel::ERROR, "Failed to connect to CAN network\r\n");
        return 1;
    } else {
        log::LOGGER.log(log::Logger::LogLevel::INFO, "Connected to CAN network\r\n");
    }

    // Initialize all the CANOpen drivers.
    IO::initializeCANopenDriver(&canOpenQueue, &can, &timer, &canStackDriver, &nvmDriver, &timerDriver, &canDriver);

    // Initialize the CANOpen node we are using.
    IO::initializeCANopenNode(&canNode, &hudl, &canStackDriver, sdoBuffer, appTmrMem);

    CONmtSetMode(&canNode.Nmt, CO_OPERATIONAL);

    time::wait(500);

    log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Error: %d\r\n", CONodeGetErr(&canNode));

    ///////////////////////////////////////////////////////////////////////////
    // Main loop
    ///////////////////////////////////////////////////////////////////////////

    hudl.initLCD();

    while (true) {
        hudl.process();
        IO::processCANopenNode(&canNode);
    }
}
