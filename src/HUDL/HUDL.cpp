/**
 * This is a basic sample of using the UART module. The program provides a
 * basic echo functionality where the uart will write back whatever the user
 * enters.
 */

// clang-format off
#include "EVT/dev/LCD.hpp"
#include "EVT/io/GPIO.hpp"
#include "EVT/io/SPI.hpp"
#include "HUDL/HUDL.hpp"
// clang-format on

namespace IO = EVT::core::IO;
namespace DEV = EVT::core::DEV;

namespace HUDL {
HUDL::HUDL(IO::GPIO& reg_select, IO::GPIO& reset, IO::SPI& spi) : lcd(DEV::LCD(reg_select, reset, spi)) {
    this->sampleDataA = 0;
    this->sampleDataB = 0;
}

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

uint32_t* HUDL::getThermTemps() {
    return this->thermTemps;
}

uint16_t* HUDL::getVoltages() {
    return this->voltages;
}

void HUDL::displayMap(uint8_t* bitmap) {
    lcd.displayMap(bitmap);
}

}// namespace HUDL
