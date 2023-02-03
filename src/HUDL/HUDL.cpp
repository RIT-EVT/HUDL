/**
 * This is a basic sample of using the UART module. The program provides a
 * basic echo functionality where the uart will write back whatever the user
 * enters.
 */

// clang-format off
#include <EVT/dev/LCD.hpp>
#include <EVT/io/GPIO.hpp>
#include <EVT/io/SPI.hpp>
#include <EVT/utils/log.hpp>
#include <HUDL/HUDL.hpp>
// clang-format on

namespace IO = EVT::core::IO;
namespace DEV = EVT::core::DEV;
namespace log = EVT::core::log;

namespace HUDL {
HUDL::HUDL(IO::GPIO& reg_select, IO::GPIO& reset, IO::SPI& spi) : lcd(DEV::LCD(reg_select, reset, spi)) {}

void HUDL::dataWrite(uint8_t data) {
    lcd.dataWrite(data);
}

void HUDL::commWrite(uint8_t data) {
    lcd.commandWrite(data);
}

void HUDL::drivePixel(uint8_t page, uint8_t colUp, uint8_t colLow, uint8_t data) {
    lcd.drivePixel(page, colUp, colLow, data);
}

void HUDL::clearLCD(const uint8_t* bitmap) {
    lcd.clearLCD(bitmap);
}

void HUDL::initLCD() {
    lcd.initLCD();
}

CO_OBJ_T* HUDL::getObjectDictionary() {
    return &objectDictionary[0];
}

uint16_t HUDL::getObjectDictionarySize() const {
    return OBJECT_DICTIONARY_SIZE;
}

void HUDL::updateLCD() const {
    for (int i = 0; i < 4; i++) {
        log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Temp %d: %d\n\r", i, *(this->thermTemps + i));
    }
    log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Total Voltage: %d\n\r", totalVoltage);
    log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Status Word: 0x%X\n\r", statusWord);
    log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Position Actual: %d\n\r", positionActual);
    log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Torque Actual: %d\n\r", torqueActual);
    log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Velocity Value: %d\n\r", velocityValue);
}

void HUDL::displayMap(uint8_t* bitmap) {
    lcd.displayMap(bitmap);
}

}// namespace HUDL
