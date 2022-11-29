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
#include "EVT/dev/LED.hpp"
#include "EVT/utils/log.hpp"

namespace IO = EVT::core::IO;
namespace DEV = EVT::core::DEV;
namespace time = EVT::core::time;
namespace log = EVT::core::log;
using namespace std;

const uint32_t SPI_SPEED = SPI_SPEED_500KHZ;
const uint8_t deviceCount = 1;


///////////////////////////////////////////////////////////////////////////////
// EVT-core CAN callback and CAN setup. This will include logic to set
// aside CANopen messages into a specific queue
///////////////////////////////////////////////////////////////////////////////

/**
 * Interrupt handler to get CAN messages. A function pointer to this function
 * will be passed to the EVT-core CAN interface which will in turn call this
 * function each time a new CAN message comes in.
 *
 * NOTE: For this sample, every non-extended (so 11 bit CAN IDs) will be
 * assumed to be intended to be passed as a CANopen message.
 *
 * @param message[in] The passed in CAN message that was read.
 */

IO::UART &uart = IO::getUART<IO::Pin::PB_10, IO::Pin::PB_11>(9600);

// create a can interrupt handler
void canInterrupt(IO::CANMessage &message, void *priv) {
    EVT::core::types::FixedQueue<CANOPEN_QUEUE_SIZE, IO::CANMessage> *queue =
            (EVT::core::types::FixedQueue<CANOPEN_QUEUE_SIZE, IO::CANMessage> *) priv;

    //print out raw received data
    uart.printf("Got RAW message from %X of length %d with data: ", message.getId(), message.getDataLength());
    uint8_t *data = message.getPayload();
    for (int i = 0; i < message.getDataLength(); i++) {
        uart.printf("%X ", *data);
        data++;
    }
    uart.printf("\r\n");

    if (queue != nullptr)
        queue->append(message);
}

///////////////////////////////////////////////////////////////////////////////
// CANopen specific Callbacks. Need to be defined in some location
///////////////////////////////////////////////////////////////////////////////
extern "C" void CONodeFatalError(void) {}

extern "C" void COIfCanReceive(CO_IF_FRM *frm) {}

extern "C" int16_t COLssStore(uint32_t baudrate, uint8_t nodeId) { return 0; }

extern "C" int16_t COLssLoad(uint32_t *baudrate, uint8_t *nodeId) { return 0; }

extern "C" void CONmtModeChange(CO_NMT *nmt, CO_MODE mode) {}

extern "C" void CONmtHbConsEvent(CO_NMT *nmt, uint8_t nodeId) {}

extern "C" void CONmtHbConsChange(CO_NMT *nmt, uint8_t nodeId, CO_MODE mode) {}

extern "C" int16_t COParaDefault(CO_PARA *pg) { return 0; }

extern "C" void COPdoTransmit(CO_IF_FRM *frm) {}

extern "C" int16_t COPdoReceive(CO_IF_FRM *frm) { return 0; }

extern "C" void COPdoSyncUpdate(CO_RPDO *pdo) {}

extern "C" void COTmrLock(void) {}

extern "C" void COTmrUnlock(void) {}

int main() {
    // Initialize system
    IO::init();

    // Will store CANopen messages that will be populated by the EVT-core CAN
    // interrupt
    EVT::core::types::FixedQueue<CANOPEN_QUEUE_SIZE, IO::CANMessage> canOpenQueue;

    // Initialize CAN, add an IRQ which will add messages to the queue above
    IO::CAN &can = IO::getCAN<IO::Pin::PA_12, IO::Pin::PA_11>();
    can.addIRQHandler(canInterrupt, reinterpret_cast<void *>(&canOpenQueue));

    // Initialize the timer
    DEV::Timerf302x8 timer(TIM2, 100);
    timer.stopTimer();

    //create the RPDO node
    IO::GPIO *devices[deviceCount];

    IO::GPIO &regSelect = IO::getGPIO<IO::Pin::PA_3>(EVT::core::IO::GPIO::Direction::OUTPUT);
    IO::GPIO &reset = IO::getGPIO<IO::Pin::PB_3>(EVT::core::IO::GPIO::Direction::OUTPUT);
    devices[0] = &IO::getGPIO<IO::Pin::PB_12>(EVT::core::IO::GPIO::Direction::OUTPUT);
    devices[0]->writePin(IO::GPIO::State::HIGH);
    auto &hudl_spi = IO::getSPI<IO::Pin::SPI_SCK, IO::Pin::SPI_MOSI>(
            devices, deviceCount);

    hudl_spi.configureSPI(SPI_SPEED, SPI_MODE0, SPI_MSB_FIRST);

    auto hudl = HUDL::HUDL(regSelect, reset, hudl_spi);

    // Reserved memory for CANopen stack usage
    uint8_t sdoBuffer[1][CO_SDO_BUF_BYTE];
    CO_TMR_MEM appTmrMem[4];

    // Attempt to join the CAN network
    IO::CAN::CANStatus result = can.connect();

    //test that the board is connected to the can network
    if (result != IO::CAN::CANStatus::OK) {
        uart.printf("Failed to connect to CAN network\r\n");
        return 1;
    }

    ///////////////////////////////////////////////////////////////////////////
    // Setup CAN configuration, this handles making drivers, applying settings.
    // And generally creating the CANopen stack node which is the interface
    // between the application (the code we write) and the physical CAN network
    ///////////////////////////////////////////////////////////////////////////
    // Make drivers
    CO_IF_DRV canStackDriver;

    CO_IF_CAN_DRV canDriver;
    CO_IF_TIMER_DRV timerDriver;
    CO_IF_NVM_DRV nvmDriver;

    IO::getCANopenCANDriver(&can, &canOpenQueue, &canDriver);
    IO::getCANopenTimerDriver(&timer, &timerDriver);
    IO::getCANopenNVMDriver(&nvmDriver);

    canStackDriver.Can = &canDriver;
    canStackDriver.Timer = &timerDriver;
    canStackDriver.Nvm = &nvmDriver;

    //setup CANopen Node
    CO_NODE_SPEC canSpec = {
            .NodeId = 0x02,
            .Baudrate = IO::CAN::DEFAULT_BAUD,
            .Dict = hudl.getObjectDictionary(),
            .DictLen = hudl.getObjectDictionarySize(),
            .EmcyCode = NULL,
            .TmrMem = appTmrMem,
            .TmrNum = 16,
            .TmrFreq = 100,
            .Drv = &canStackDriver,
            .SdoBuf = reinterpret_cast<uint8_t *>(&sdoBuffer[0]),
    };

    CO_NODE canNode;

    CONodeInit(&canNode, &canSpec);
    CONodeStart(&canNode);
    CONmtSetMode(&canNode.Nmt, CO_OPERATIONAL);

    time::wait(500);

    //print any CANopen errors
    uart.printf("Error: %d\r\n", CONodeGetErr(&canNode));
    while (1) {
        //Print new value when changed over CAN
        uint32_t *temps = hudl.getThermTemps();
        // Process incoming CAN messages
        for (int i = 0; i < 3; i++) {
            uart.printf("Temp %d: %d\n\r", i, *(temps + i));
        }


        CONodeProcess(&canNode);
        // Update the state of timer based events
        COTmrService(&canNode.Tmr);
        // Handle executing timer events that have elapsed
        COTmrProcess(&canNode.Tmr);
        // Wait for new data to come in
        time::wait(1000);
    }
}


