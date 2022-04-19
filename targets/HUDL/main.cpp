/**
 * This is a basic sample of using the UART module. The program provides a
 * basic echo functionality where the uart will write back whatever the user
 * enters.
 */
#include <stdint.h>

#include <EVT/io/CANopen.hpp>
#include <EVT/io/GPIO.hpp>
#include <EVT/io/I2C.hpp>
#include <EVT/io/manager.hpp>
#include <EVT/io/pin.hpp>
#include <EVT/io/UART.hpp>
#include <EVT/utils/time.hpp>

#include <HUDL/HUDL.hpp>
#include <HUDL/dev/LCD.hpp>

namespace IO = EVT::core::IO;
namespace DEV = EVT::core::DEV;
namespace time = EVT::core::time;
namespace log = EVT::core::log;

// Global CAN Node reference
CO_NODE canNode;

constexpr uint32_t SPI_SPEED = SPI_SPEED_4MHZ;// 4MHz
constexpr uint8_t deviceCount = 1;

IO::GPIO* devices[deviceCount];

void handleNMT(IO::CANMessage& message) {
    log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Network Management message recognized.");
    uint8_t* payload = message.getPayload();
    uint8_t targetID = payload[1];
    if (targetID == TMS::TMS::NODE_ID || targetID == 0x00) {
        CO_MODE mode;
        switch (payload[0]) {
        case 0x01:
            mode = CO_OPERATIONAL;
            break;
        case 0x80:
            mode = CO_PREOP;
            break;
        default:
            mode = CO_INVALID;
        }
        if (canNode.Nmt.Mode != mode)
            CONmtSetMode(&canNode.Nmt, mode);
    }
}

/**
 * Interrupt handler for incoming CAN messages.
 *
 * @param priv[in] The private data (FixedQueue<CANOPEN_QUEUE_SIZE, CANMessage>)
 */
void canInterruptHandler(IO::CANMessage& message, void* priv) {
    log::LOGGER.log(log::Logger::LogLevel::DEBUG, "CAN Message received.");

    // Handle NMT messages
    if (message.getId() == 0) {
        handleNMT(message);
        return;
    }

    EVT::core::types::FixedQueue<CANOPEN_QUEUE_SIZE, IO::CANMessage>* queue =
        (EVT::core::types::FixedQueue<CANOPEN_QUEUE_SIZE, IO::CANMessage>*) priv;
    if (queue == nullptr)
        return;
    if (!message.isCANExtended())
        queue->append(message);
}






///////////////////////////////////////////////////////////////////////////////
// CANopen specific Callbacks. Need to be defined in some location
///////////////////////////////////////////////////////////////////////////////
extern "C" void CONodeFatalError(void) {
    log::LOGGER.log(log::Logger::LogLevel::ERROR, "Fatal CANopen error");
}

extern "C" void COIfCanReceive(CO_IF_FRM* frm) {}

extern "C" int16_t COLssStore(uint32_t baudrate, uint8_t nodeId) { return 0; }

extern "C" int16_t COLssLoad(uint32_t* baudrate, uint8_t* nodeId) { return 0; }

extern "C" void CONmtModeChange(CO_NMT* nmt, CO_MODE mode) {
    log::LOGGER.log(log::Logger::LogLevel::INFO, "Network Management state changed.");
}

extern "C" void CONmtHbConsEvent(CO_NMT* nmt, uint8_t nodeId) {}

extern "C" void CONmtHbConsChange(CO_NMT* nmt, uint8_t nodeId, CO_MODE mode) {}

extern "C" int16_t COParaDefault(CO_PARA* pg) { return 0; }

extern "C" void COPdoTransmit(CO_IF_FRM* frm) {}

extern "C" int16_t COPdoReceive(CO_IF_FRM* frm) { return 0; }

extern "C" void COPdoSyncUpdate(CO_RPDO* pdo) {}

extern "C" void COTmrLock(void) {}

extern "C" void COTmrUnlock(void) {}



int main() {
    
    //HUDL board = new HUDL();
    HUDL *board = new HUDL();
     //register select set
    board->reg_select = IO::getGPIO<IO::Pin::PA_3>(EVT::core::IO::GPIO::Direction::OUTPUT);
    board->reg_select.writePin(EVT::core::IO::GPIO::State::LOW);

    // reset set
    board->reset = IO::getGPIO<IO::Pin::PB_3>(EVT::core::IO::GPIO::Direction::OUTPUT);

    // reset the board
    board->reset.writePin(EVT::core::IO::GPIO::State::LOW);          
    time::wait(100);                      
    board->reset.writePin(EVT::core::IO::GPIO::State::HIGH);    
    time::wait(100); 

    // CS
    board->CS = IO::getGPIO<IO::Pin::PB_12>(EVT::core::IO::GPIO::Direction::OUTPUT);
    board->CS.writePin(EVT::core::IO::GPIO::State::HIGH);

    // Setup spi
    board->spi.configureSPI(SPI_SPEED, SPI_MODE3, SPI_MSB_FIRST);

    // Setup UART
    IO::UART& uart = IO::getUART<IO::Pin::UART_TX, IO::Pin::UART_RX>(9600);

    // String to store user input
    char buf[100];

    while (1) {
        // Read user input
        uart.printf("Enter message: ");
        uart.gets(buf, 100);

        // Initialized LCD
        board->init_LCD();

        // Clear LCD Screen
        //ClearLCD();

        board->drive_pixel(1, 1, 1, 255);
        time::wait(10000); 

        //echos command back
        uart.printf("\n\recho: %s\n\r", buf);
    }
}
