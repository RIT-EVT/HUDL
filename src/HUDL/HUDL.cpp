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

uint16_t HUDL::getObjectDictionarySize() {
    return OBJECT_DICTIONARY_SIZE;
}

void HUDL::updateLCD() {
    char tempOne[32];
    std::sprintf(tempOne, "%lu C", *(this->thermTemps + 0));

    char tempTwo[32];
    std::sprintf(tempTwo, "%lu C", *(this->thermTemps + 1));

    char tempThree[32];
    std::sprintf(tempThree, "%lu C", *(this->thermTemps + 2));

    char tempFour[32];
    std::sprintf(tempFour, "%lu C", *(this->thermTemps + 3));

    char voltage[32];
    std::sprintf(voltage, "%lu v", totalVoltage);

    lcd.setTextForSection(0, voltage);
    //    setTextForSection(2g "25 MPH");
    //    setTextForSection(2, "3000");
    lcd.setTextForSection(3, tempOne);
    lcd.setTextForSection(4, tempTwo);
    lcd.setTextForSection(5, tempThree);
    //    setTextForSection(6, "ON");
    //    setTextForSection(7, "Ready");
    //    setTextForSection(8, "100 NM");
    log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Status Word: 0x%X\n\r", statusWord);
    log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Position Actual: 0x%X\n\r", positionActual);
    log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Torque Actual: 0x%X\n\r", torqueActual);
    log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Velocity Value: 0x%X\n\r", velocityActual);
    log::LOGGER.log(log::Logger::LogLevel::DEBUG, "Dummy Data: 0x%X\n\r", rpdo4First32BitsDummyData);
}

}// namespace HUDL
