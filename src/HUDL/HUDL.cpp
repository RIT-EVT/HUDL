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
    char tempOne[16];
    std::sprintf(tempOne, "%hu.%hu C", thermTemps[0] / 100, thermTemps[0] % 100);

    char tempTwo[16];
    std::sprintf(tempTwo, "%hu.%hu C", thermTemps[1] / 100, thermTemps[1] % 100);

    char tempThree[16];
    std::sprintf(tempThree, "%hu.%hu C", thermTemps[2] / 100, thermTemps[2] % 100);

    // Calculate RPM
    actualPosition = ~actualPosition + 1; // Update the RPM to be correctly formatted

    uint32_t mph = actualPosition * (27 * 60 * 314) / 433 / 63360;

    char voltage[32];
    std::sprintf(voltage, "%hu v", totalVoltage);

    char rpm[32];
    std::sprintf(rpm, "%lu", actualPosition);

    char status[16];

    if (statusWord == 0x21) {
        std::sprintf(status, "STOP");
    } else if (statusWord == 0x27) {
        std::sprintf(status, "GO");
    } else {
        std::sprintf(status, "0x%x", statusWord);
    }

    char torque[16];
    std::sprintf(torque, "%hu", torqueActual);

    char mphText[32];
    std::sprintf(mphText, "%lu", mph);

    lcd.setTextForSection(0, voltage);
    lcd.setTextForSection(1, mphText);
    lcd.setTextForSection(2, rpm);
    lcd.setTextForSection(3, tempOne);
    lcd.setTextForSection(4, tempTwo);
    lcd.setTextForSection(5, tempThree);
    lcd.setTextForSection(6, status);
    lcd.setTextForSection(7, "NULL");
    lcd.setTextForSection(8, torque);

//    for (int i = 0; i < sizeof(thermTemps) / sizeof(uint32_t); i++) {
//        log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Temp %d: %d\n\r", i, thermTemps[i]);
//    }
//    log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Total Voltage: %d\n\r", totalVoltage);
//
//    log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Status Word: 0x%X\n\r", statusWord);
//    log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Position Actual: 0x%X\n\r", positionActual);
//    log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Torque Actual: 0x%X\n\r", torqueActual);
//    log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Velocity Value: 0x%X\n\r", velocityActual);
//    log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Dummy Data: 0x%X\n\r", rpdo4First32BitsDummyData);
}

}// namespace HUDL
