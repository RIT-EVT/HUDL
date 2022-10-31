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
#include <EVT/dev/platform/f3xx/f302x8/Timerf302x8.hpp>
#include "Canopen/co_core.h"
#include "Canopen/co_tmr.h"

extern "C" void CONodeFatalError(void) {
}

extern "C" void COIfCanReceive(CO_IF_FRM* frm) {}

extern "C" int16_t COLssStore(uint32_t baudrate, uint8_t nodeId) { return 0; }

extern "C" int16_t COLssLoad(uint32_t* baudrate, uint8_t* nodeId) { return 0; }

extern "C" void CONmtModeChange(CO_NMT* nmt, CO_MODE mode) {}

extern "C" void CONmtHbConsEvent(CO_NMT* nmt, uint8_t nodeId) {}

extern "C" void CONmtHbConsChange(CO_NMT* nmt, uint8_t nodeId, CO_MODE mode) {}

extern "C" int16_t COParaDefault(CO_PARA* pg) { return 0; }

extern "C" void COPdoTransmit(CO_IF_FRM* frm) {}

extern "C" int16_t COPdoReceive(CO_IF_FRM* frm) { return 0; }

extern "C" void COPdoSyncUpdate(CO_RPDO* pdo) {}

extern "C" void COTmrLock(void) {}

extern "C" void COTmrUnlock(void) {}


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
    EVT::core::types::FixedQueue<CANOPEN_QUEUE_SIZE, IO::CANMessage> *queue;
};

/**
 * Interrupt handler for incoming CAN messages.
 *
 * @param priv[in] The private data (FixedQueue<CANOPEN_QUEUE_SIZE, CANMessage>)
 */
void canInterruptHandler(IO::CANMessage &message, void *priv) {
    struct CANInterruptParams *params = (CANInterruptParams *) priv;

    EVT::core::types::FixedQueue<CANOPEN_QUEUE_SIZE, IO::CANMessage> *queue =
            params->queue;

    if (queue == nullptr)
        return;
    if (!message.isCANExtended())
        queue->append(message);
}

int main() {
    IO::GPIO *devices[deviceCount];
    auto &reg_select = IO::getGPIO<IO::Pin::PA_3>(IO::GPIO::Direction::OUTPUT);
    reg_select.writePin(IO::GPIO::State::LOW);

    auto &reset = IO::getGPIO<IO::Pin::PB_3>(IO::GPIO::Direction::OUTPUT);
    reset.writePin(IO::GPIO::State::LOW);
    time::wait(100);
    reset.writePin(IO::GPIO::State::HIGH);
    time::wait(100);

    auto &cs = IO::getGPIO<IO::Pin::PB_12>(IO::GPIO::Direction::OUTPUT);
    cs.writePin(IO::GPIO::State::HIGH);

    auto &hudl_spi = IO::getSPI<IO::Pin::PB_13, IO::Pin::PB_15, IO::Pin::PC_11>(
            devices, deviceCount);
    hudl_spi.configureSPI(SPI_SPEED, SPI_MODE3, SPI_MSB_FIRST);

    auto hudl = HUDL::HUDL(reg_select, reset, cs, hudl_spi);

    // Setup UART
    IO::UART const &uart = IO::getUART<IO::Pin::UART_TX, IO::Pin::UART_RX>(9600);

    // Setup CAN
    IO::CAN &can = IO::getCAN<IO::Pin::PA_12, IO::Pin::PA_11>(); // TODO: Figure out CAN pins
    EVT::core::types::FixedQueue<CANOPEN_QUEUE_SIZE, IO::CANMessage> canOpenQueue;
    DEV::Timerf302x8 timer(TIM2, 100);
    uint8_t sdoBuffer[1][CO_SDO_BUF_BYTE];
    CO_TMR_MEM appTmrMem[4];
    can.addIRQHandler(canInterruptHandler, reinterpret_cast<void *>(&canOpenQueue));

    // Initialize the CANopen drivers
    CO_IF_DRV canStackDriver;
    CO_IF_CAN_DRV canDriver;
    CO_IF_TIMER_DRV timerDriver;
    CO_IF_NVM_DRV nvmDriver;
    IO::getCANopenCANDriver(&can, &canOpenQueue, &canDriver);
    IO::getCANopenTimerDriver(&timer, &timerDriver);
    IO::getCANopenNVMDriver(&nvmDriver);

    // Attach the CANopen drivers
    canStackDriver.Can = &canDriver;
    canStackDriver.Timer = &timerDriver;
    canStackDriver.Nvm = &nvmDriver;

    CO_NODE_SPEC canSpec = {
            .NodeId = HUDL::HUDL::NODE_ID,
            .Baudrate = IO::CAN::DEFAULT_BAUD,
            .Dict = hudl.getObjectDictionary(),
            .DictLen = hudl.getObjectDictionarySize(),
            .EmcyCode = NULL,
            .TmrMem = appTmrMem,
            .TmrNum = 16,
            .TmrFreq = 100,
            .Drv = &canStackDriver,
            .SdoBuf = reinterpret_cast<uint8_t*>(&sdoBuffer[0]),
    };

    // Intialize CANopen logic
    CO_NODE canNode;
    CONodeInit(&canNode, &canSpec);
    CONodeStart(&canNode);
    CONmtSetMode(&canNode.Nmt, CO_OPERATIONAL);
    time::wait(500);

    // Join the CANopen network
    can.connect();

    // Main processing loop, contains the following logic
    // 1. Update CANopen logic and processing incomming messages
    // 2. Display current data from other devices on the bus
    // 3. Wait for new data to come in
    hudl.initLCD();

    while (1) {
        // Process incoming CAN messages
        CONodeProcess(&canNode);
        // Update the state of timer based events
        COTmrService(&canNode.Tmr);
        // Handle executing timer events that have elapsed
        COTmrProcess(&canNode.Tmr);
        // Update the state of the BMS
        // Wait for new data to come in
        time::wait(10);

        hudl.drivePixel(1, 1, 1, 255);
        time::wait(10000);

        // TODO: For now should echo values that it pulls. In the future it should write values to the displa
//        uart.printf("Temp One: %d\n", tempOne);
//        uart.printf("BMS Voltage: %d\n", voltageOne);
    }

}
