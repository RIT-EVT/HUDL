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
#include <cstdio>
// clang-format on

namespace IO = EVT::core::IO;
namespace DEV = EVT::core::DEV;
namespace log = EVT::core::log;

namespace HUDL {
HUDL::HUDL(IO::GPIO& reg_select, IO::GPIO& reset, IO::SPI& spi) : lcd(DEV::LCD(reg_select, reset, spi)) {}

void HUDL::initLCD() {
    lcd.initLCD();
    lcd.clearLCD();
    lcd.setDefaultSections(SECTION_TITLES);
    lcd.displaySectionHeaders();
}

CO_OBJ_T* HUDL::getObjectDictionary() {
    return &objectDictionary[0];
}

uint16_t HUDL::getObjectDictionarySize() const {
    return OBJECT_DICTIONARY_SIZE;
}

void HUDL::updateLCD() {
    char temps[32][4];
    for(int i = 0; i < sizeof(thermTemps) / sizeof(uint32_t); i ++) {
        std::sprintf(temps[i], "%d C", thermTemps[i]);
    }

    char voltage[32];
    std::sprintf(voltage, "%d v", totalVoltage);

    char status[32];
    std::sprintf(status, "0x%X", statusWord);

    char position[32];
    std::sprintf(position, "0x%X", positionActual);

    char torque[32];
    std::sprintf(torque, "0x%X", torqueActual);

    char velocity[32];
    std::sprintf(velocity, "0x%X", velocityActual);

    lcd.setTextForSection(0, voltage);
    lcd.setTextForSection(1, velocity);
    lcd.setTextForSection(2, "3000");
    lcd.setTextForSection(3, temps[0]);
    lcd.setTextForSection(4, temps[1]);
    lcd.setTextForSection(5, temps[2]);
    lcd.setTextForSection(6, status);
    lcd.setTextForSection(7, position);
    lcd.setTextForSection(8, torque);

    for (int i = 0; i < sizeof(thermTemps) / sizeof(uint32_t); i++) {
        log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Temp %d: %d\n\r", i, thermTemps[i]);
    }
    log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Total Voltage: %d\n\r", totalVoltage);

    log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Status Word: 0x%X\n\r", statusWord);
    log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Position Actual: 0x%X\n\r", positionActual);
    log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Torque Actual: 0x%X\n\r", torqueActual);
    log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Velocity Value: 0x%X\n\r", velocityActual);
    log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Dummy Data: 0x%X\n\r", rpdo4First32BitsDummyData);
}

}// namespace HUDL
