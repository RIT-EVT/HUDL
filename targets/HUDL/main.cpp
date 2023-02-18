/**
 * This is the main code running on the HUDL responsible for displaying
 * information that other boards that broadcast through the CAN network
 */
#include <stdint.h>

#include <Canopen/co_core.h>
#include <Canopen/co_tmr.h>
#include <EVT/dev/Timer.hpp>
#include <EVT/io/CANopen.hpp>
#include <EVT/io/GPIO.hpp>
#include <EVT/io/manager.hpp>
#include <EVT/io/pin.hpp>
#include <EVT/utils/log.hpp>
#include <EVT/utils/time.hpp>

#include <EVT/dev/platform/f3xx/f302x8/Timerf302x8.hpp>
#include <HUDL/HUDL.hpp>

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

// create a can interrupt handler
void canInterrupt(IO::CANMessage& message, void* priv) {
    EVT::core::types::FixedQueue<CANOPEN_QUEUE_SIZE, IO::CANMessage>* queue =
        (EVT::core::types::FixedQueue<CANOPEN_QUEUE_SIZE, IO::CANMessage>*) priv;

    // Log raw received data
    log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Got RAW message from %X of length %d with data: ", message.getId(), message.getDataLength());

    uint8_t* data = message.getPayload();
    for (int i = 0; i < message.getDataLength(); i++) {
        log::LOGGER.log(log::Logger::LogLevel::DEBUG, "%X ", *data);
        data++;
    }

    if (queue != nullptr)
        queue->append(message);
}

///////////////////////////////////////////////////////////////////////////////
// CANopen specific Callbacks. Need to be defined in some location
///////////////////////////////////////////////////////////////////////////////
extern "C" void CONodeFatalError(void) {}

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

int main() {
    // Initialize system
    IO::init();

    // Will store CANopen messages that will be populated by the EVT-core CAN
    // interrupt
    EVT::core::types::FixedQueue<CANOPEN_QUEUE_SIZE, IO::CANMessage> canOpenQueue;

    // Initialize CAN, add an IRQ which will add messages to the queue above
    IO::CAN& can = IO::getCAN<IO::Pin::PA_12, IO::Pin::PA_11>();
    can.addIRQHandler(canInterrupt, reinterpret_cast<void*>(&canOpenQueue));

    // Initialize the timer
    DEV::Timerf302x8 timer(TIM2, 100);
    timer.stopTimer();

    //create the RPDO node
    IO::GPIO* devices[deviceCount];

    IO::GPIO& regSelect = IO::getGPIO<IO::Pin::PA_3>(EVT::core::IO::GPIO::Direction::OUTPUT);
    IO::GPIO& reset = IO::getGPIO<IO::Pin::PB_3>(EVT::core::IO::GPIO::Direction::OUTPUT);
    devices[0] = &IO::getGPIO<IO::Pin::PB_12>(EVT::core::IO::GPIO::Direction::OUTPUT);
    devices[0]->writePin(IO::GPIO::State::HIGH);
    auto& hudl_spi = IO::getSPI<IO::Pin::SPI_SCK, IO::Pin::SPI_MOSI>(
        devices, deviceCount);

    hudl_spi.configureSPI(SPI_SPEED, SPI_MODE0, SPI_MSB_FIRST);

    HUDL::HUDL hudl(regSelect, reset, hudl_spi);

    // Reserved memory for CANopen stack usage
    uint8_t sdoBuffer[1][CO_SDO_BUF_BYTE];
    CO_TMR_MEM appTmrMem[4];

    // Attempt to join the CAN network
    IO::CAN::CANStatus result = can.connect();

    IO::UART& uart = IO::getUART<IO::Pin::UART_TX, IO::Pin::UART_RX>(9600);
    log::LOGGER.setUART(&uart);

    //test that the board is connected to the can network
    if (result != IO::CAN::CANStatus::OK) {
        log::LOGGER.log(log::Logger::LogLevel::ERROR, "Failed to connect to CAN network\r\n");
        return 1;
    } else {
        log::LOGGER.log(log::Logger::LogLevel::INFO, "Connected to CAN network\r\n");
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

    CO_NODE canNode;

    CONodeInit(&canNode, &canSpec);
    CONodeStart(&canNode);
    CONmtSetMode(&canNode.Nmt, CO_OPERATIONAL);

    time::wait(500);

    //print any CANopen errors
    CO_ERR err = CONodeGetErr(&canNode);
    switch (err) {
    case CO_ERR_NONE:
        log::LOGGER.log(log::Logger::LogLevel::DEBUG, "No Error: %d\r\n", err);
        break;
    case CO_ERR_BAD_ARG:
        log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Error: %d (Invalid Argument)\r\n", err);
        break;
    case CO_ERR_OBJ_NOT_FOUND:
        log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Error: %d (searched object not found in dictionary)\r\n", err);
        break;
    case CO_ERR_OBJ_READ:
        log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Error: %d (error during reading a object entry)\r\n", err);
        break;
    case CO_ERR_OBJ_WRITE:
        log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Error: %d (error during writing a object entry)\r\n", err);
        break;
    case CO_ERR_OBJ_SIZE:
        log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Error: %d (read/write with wrong size to object)\r\n", err);
        break;
    case CO_ERR_OBJ_MAP_LEN:
        log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Error: %d (invalid mapping length)\r\n", err);
        break;
    case CO_ERR_OBJ_MAP_TYPE:
        log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Error: %d (invalid mapping type)\r\n", err);
        break;
    case CO_ERR_OBJ_ACC:
        log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Error: %d (unsupported access)\r\n", err);
        break;
    case CO_ERR_OBJ_RANGE:
        log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Error: %d (value range of parameter exceeded)\r\n", err);
        break;
    case CO_ERR_OBJ_INCOMPATIBLE:
        log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Error: %d (incompatible parameter value)\r\n", err);
        break;
    case CO_ERR_PARA_IDX:
        log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Error: %d (wrong index for parameter type)\r\n", err);
        break;
    case CO_ERR_PARA_STORE:
        log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Error: %d (error during storing parameter)\r\n", err);
        break;
    case CO_ERR_PARA_RESTORE:
        log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Error: %d (error during restoring parameter)\r\n", err);
        break;
    case CO_ERR_PARA_LOAD:
        log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Error: %d (error during loading parameter)\r\n", err);
        break;
    case CO_ERR_LSS_STORE:
        log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Error: %d (error during storing LSS configuration)\r\n", err);
        break;
    case CO_ERR_LSS_LOAD:
        log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Error: %d (error during loading LSS configuration)\r\n", err);
        break;
    case CO_ERR_CFG_1001_0:
        log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Error: %d (entry 1001:0 is bad/not existing)\r\n", err);
        break;
    case CO_ERR_CFG_1003_0:
        log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Error: %d (entry 1003:0 is bad/not existing)\r\n", err);
        break;
    case CO_ERR_CFG_1003_1:
        log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Error: %d (entry 1003:1 is bad/not existing)\r\n", err);
        break;
    case CO_ERR_CFG_1005_0:
        log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Error: %d (entry 1005:0 is bad/not existing)\r\n", err);
        break;
    case CO_ERR_CFG_1010_0:
        log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Error: %d (entry 1010:0 is bad/not existing)\r\n", err);
        break;
    case CO_ERR_CFG_1011_0:
        log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Error: %d (entry 1011:0 is bad/not existing)\r\n", err);
        break;
    case CO_ERR_CFG_1014_0:
        log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Error: %d (entry 1014:0 is bad/not existing)\r\n", err);
        break;
    case CO_ERR_CFG_1017_0:
        log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Error: %d (entry 1017:0 is bad/not existing)\r\n", err);
        break;
    case CO_ERR_CFG_1016:
        log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Error: %d (entry in 1016 is bad/not existing)\r\n", err);
        break;
    case CO_ERR_CFG_1018:
        log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Error: %d (entry in 1018 is bad/not existing)\r\n", err);
        break;
    case CO_ERR_TMR_NO_ACT:
        log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Error: %d (no action available while creating)\r\n", err);
        break;
    case CO_ERR_TMR_INSERT:
        log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Error: %d (error during insert action in tmr-list)\r\n", err);
        break;
    case CO_ERR_TMR_CREATE:
        log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Error: %d (error during creating a timed action)\r\n", err);
        break;
    case CO_ERR_TMR_DELETE:
        log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Error: %d (error during deleting a timed action)\r\n", err);
        break;
    case CO_ERR_NMT_INIT:
        log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Error: %d (error during initializing NMT slave)\r\n", err);
        break;
    case CO_ERR_NMT_APP_RESET:
        log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Error: %d (error in resetting application)\r\n", err);
        break;
    case CO_ERR_NMT_COM_RESET:
        log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Error: %d (error in resetting communication)\r\n", err);
        break;
    case CO_ERR_NMT_MODE:
        log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Error: %d (action not allowed in current NMT mode)\r\n", err);
        break;
    case CO_ERR_EMCY_BAD_ROOT:
        log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Error: %d (error in emcy structure, member: Root)\r\n", err);
        break;
    case CO_ERR_TPDO_COM_OBJ:
        log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Error: %d (config error in TPDO communication)\r\n", err);
        break;
    case CO_ERR_TPDO_MAP_OBJ:
        log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Error: %d (config error in TPDO mapping)\r\n", err);
        break;
    case CO_ERR_TPDO_OBJ_TRIGGER:
        log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Error: %d (error during trigger via an object)\r\n", err);
        break;
    case CO_ERR_TPDO_NUM_TRIGGER:
        log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Error: %d (error during trigger via an PDO number)\r\n", err);
        break;
    case CO_ERR_TPDO_INHIBIT:
        log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Error: %d (error during inhibit timer creation)\r\n", err);
        break;
    case CO_ERR_TPDO_EVENT:
        log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Error: %d (error during event timer creation)\r\n", err);
        break;
    case CO_ERR_RPDO_COM_OBJ:
        log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Error: %d (config error in RPDO communication)\r\n", err);
        break;
    case CO_ERR_RPDO_MAP_OBJ:
        log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Error: %d (config error in RPDO mapping)\r\n", err);
        break;
    case CO_ERR_SDO_SILENT:
        log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Error: %d (no SDO response (e.g. block transfer))\r\n", err);
        break;
    case CO_ERR_SDO_ABORT:
        log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Error: %d (error in SDO request with ABORT resp.)\r\n", err);
        break;
    case CO_ERR_SDO_READ:
        log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Error: %d (error during in SDO block reading)\r\n", err);
        break;
    case CO_ERR_SDO_WRITE:
        log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Error: %d (error during in SDO block writing)\r\n", err);
        break;
    case CO_ERR_SYNC_MSG:
        log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Error: %d (error during receive synchronous PDO)\r\n", err);
        break;
    case CO_ERR_IF_CAN_INIT:
        log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Error: %d (error during initialization)\r\n", err);
        break;
    case CO_ERR_IF_CAN_ENABLE:
        log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Error: %d (error during enabling CAN interface)\r\n", err);
        break;
    case CO_ERR_IF_CAN_FLUSH_RX:
        log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Error: %d (error during flushing CAN RX interface)\r\n", err);
        break;
    case CO_ERR_IF_CAN_FLUSH_TX:
        log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Error: %d (error during flushing CAN TX interface)\r\n", err);
        break;
    case CO_ERR_IF_CAN_RESET:
        log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Error: %d (error during resetting CAN interface)\r\n", err);
        break;
    case CO_ERR_IF_CAN_CLOSE:
        log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Error: %d (error during closing the CAN interface)\r\n", err);
        break;
    case CO_ERR_IF_CAN_READ:
        log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Error: %d (error during reading from CAN interface)\r\n", err);
        break;
    case CO_ERR_IF_CAN_SEND:
        log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Error: %d (error during sending to CAN interface)\r\n", err);
        break;
    case CO_ERR_IF_TIMER_INIT:
        log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Error: %d (error during initializing timer)\r\n", err);
        break;
    case CO_ERR_IF_TIMER_UPDATE:
        log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Error: %d (error during updating timer)\r\n", err);
        break;
    case CO_ERR_IF_TIMER_RELOAD:
        log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Error: %d (error during reloading timer)\r\n", err);
        break;
    case CO_ERR_IF_TIMER_DELAY:
        log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Error: %d (error during getting remaining ticks)\r\n", err);
        break;
    case CO_ERR_IF_TIMER_STOP:
        log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Error: %d (error during stopping timer)\r\n", err);
        break;
    case CO_ERR_IF_TIMER_START:
        log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Error: %d (error during starting timer)\r\n", err);
        break;
    case CO_ERR_IF_NVM_INIT:
        log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Error: %d (error during initializing NVM)\r\n", err);
        break;
    case CO_ERR_IF_NVM_READ:
        log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Error: %d (error during reading NVM)\r\n", err);
        break;
    case CO_ERR_IF_NVM_WRITE:
        log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Error: %d (error during writing NVM)\r\n", err);
        break;
    case CO_ERR_SIG_INIT:
        log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Error: %d (error during initializing CAN signals)\r\n", err);
        break;
    case CO_ERR_SIG_CREATE:
        log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Error: %d (error during creating a needed signal)\r\n", err);
        break;
    case CO_ERR_MSG_INIT:
        log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Error: %d (error during message initialization)\r\n", err);
        break;
    case CO_ERR_MSG_CREATE:
        log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Error: %d (error during creating a message)\r\n", err);
        break;
    case CO_ERR_MSG_READ:
        log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Error: %d (error during reading a message)\r\n", err);
        break;
    case CO_ERR_TYPE_RD:
        log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Error: %d (error during reading type)\r\n", err);
        break;
    case CO_ERR_TYPE_CTRL:
        log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Error: %d (error during type control)\r\n", err);
        break;
    case CO_ERR_TYPE_WR:
        log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Error: %d (error during writing type)\r\n", err);
        break;
    default:
        log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Unknown Error: %d\r\n", err);
    }

    while (1) {
        hudl.updateLCD();

        CONodeProcess(&canNode);
        // Update the state of timer based events
        COTmrService(&canNode.Tmr);
        // Handle executing timer events that have elapsed
        COTmrProcess(&canNode.Tmr);
        // Wait for new data to come in
        time::wait(100);
    }
}
