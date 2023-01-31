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

    char* titles[9] = {
        "B Voltage",
        "Speed",
        "RPM",
        "Temp 1",
        "Temp 2",
        "Temp 3",
        "Status 1",
        "Pre Stat",
        "Torque",
    };

    setDefaultSections(titles);
    displaySectionHeaders();
}

CO_OBJ_T* HUDL::getObjectDictionary() {
    return &objectDictionary[0];
}

uint16_t HUDL::getObjectDictionarySize() const {
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

    setTextForSection(0, voltage);
    //    setTextForSection(2, "25 MPH");
    //    setTextForSection(2, "3000");
    setTextForSection(3, tempOne);
    setTextForSection(4, tempTwo);
    setTextForSection(5, tempThree);
    //    setTextForSection(6, "ON");
    //    setTextForSection(7, "Ready");
    //    setTextForSection(8, "100 NM");
}

void HUDL::setTextForSection(uint8_t section, const char* text) {
    lcd.setTextForSection(section, text);
}

void HUDL::displaySectionHeaders() {
    lcd.displaySectionHeaders();
}

void HUDL::setDefaultSections(char** newSectionTitles) {
    lcd.setDefaultSections(newSectionTitles);
}

void HUDL::writeText(const char* text, uint8_t page, uint8_t column, bool wrapText) {
    lcd.writeText(text, page, column, wrapText);
}

void HUDL::displayBitMapInArea(uint8_t* bitMap, uint8_t bitMapWidth, uint8_t bitMapHeight, uint8_t page, uint8_t column) {
    lcd.displayBitMapInArea(bitMap, bitMapWidth, bitMapHeight, page, column);
}

void HUDL::setEntireScreenBitMap(const uint8_t* bitMap) {
    lcd.setEntireScreenBitMap(bitMap);
}

void HUDL::clearArea(uint8_t width, uint8_t height, uint8_t page, uint8_t column) {
    lcd.clearArea(width, height, page, column);
}

void HUDL::clearLCD() {
    lcd.clearLCD();
}

void HUDL::driveColumn(uint8_t page, uint8_t colUp, uint8_t colLow, uint8_t data) {
    lcd.driveColumn(page, colUp, colLow, data);
}
}// namespace HUDL
