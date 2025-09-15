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
#include <HUDL.hpp>
#include <cstdio>
#include <cstring>

#define DEV_1_RPM_MPH_RATIO 0.0148946657603435

// clang-format on

namespace IO = EVT::core::IO;
namespace DEV = EVT::core::DEV;
namespace log = EVT::core::log;

namespace HUDL {
typedef DEV::LCD::FontSize FontSize;

HUDL::HUDL(IO::GPIO& reg_select, IO::GPIO& reset, IO::SPI& spi) : lcd(DEV::LCD(reg_select, reset, spi)) {}

void HUDL::initLCD() {
    lcd.initLCD();
    lcd.clearLCD();
}

CO_OBJ_T* HUDL::getObjectDictionary() {
    return &objectDictionary[0];
}

uint8_t HUDL::getNumElements() {
    return OBJECT_DICTIONARY_SIZE;
}

uint8_t HUDL::getNodeID() {
    return NODE_ID;
}

void HUDL::updateLCD() {
    if (!setHeaders) {
        lcd.clearLCD();
        headerForCorner(TOP_LEFT, "X");
        headerForCorner(TOP_RIGHT, "Y");
        headerForCorner(BOTTOM_LEFT, "Z");
        headerForCorner(BOTTOM_RIGHT, "CAN?");
        setHeaders = true;
    }

    // Set the battery voltage
    char x[9];
    std::sprintf(x, "%hd", (int16_t)vectorXValues[1] / 16);
    dataForCorner(TOP_LEFT, x);

    char y[9];
    std::sprintf(y, "%hd", (int16_t)vectorYValues[1] / 16);
    dataForCorner(TOP_RIGHT, y);

    char z[9];
    std::sprintf(z, "%hd", (int16_t)vectorZValues[1] / 16);
    dataForCorner(BOTTOM_LEFT, z);

    log::LOGGER.log(log::Logger::LogLevel::INFO, "Euler Raw x: %d", (int16_t) vectorXValues[0] / 16);
    log::LOGGER.log(log::Logger::LogLevel::INFO, "Euler Raw y: %d", (int16_t) vectorYValues[0] / 16);
    log::LOGGER.log(log::Logger::LogLevel::INFO, "Euler Raw z: %d", (int16_t) vectorZValues[0] / 16);

    log::LOGGER.log(log::Logger::LogLevel::INFO, "Gyroscope Raw x: %d", (int16_t) vectorXValues[1] / 16);
    log::LOGGER.log(log::Logger::LogLevel::INFO, "Gyroscope Raw y: %d", (int16_t) vectorYValues[1] / 16);
    log::LOGGER.log(log::Logger::LogLevel::INFO, "Gyroscope Raw z: %d", (int16_t) vectorZValues[1] / 16);

    log::LOGGER.log(log::Logger::LogLevel::INFO, "Linear Acceleration Raw x: %d", (int16_t) vectorXValues[2] / 100);
    log::LOGGER.log(log::Logger::LogLevel::INFO, "Linear Acceleration Raw y: %d", (int16_t) vectorYValues[2] / 100);
    log::LOGGER.log(log::Logger::LogLevel::INFO, "Linear Acceleration Raw z: %d", (int16_t) vectorZValues[2] / 100);

    log::LOGGER.log(log::Logger::LogLevel::INFO, "Accelerometer Raw x: %d", (int16_t) vectorXValues[3] / 100);
    log::LOGGER.log(log::Logger::LogLevel::INFO, "Accelerometer Raw y: %d", (int16_t) vectorYValues[3] / 100);
    log::LOGGER.log(log::Logger::LogLevel::INFO, "Accelerometer Raw z: %d", (int16_t) vectorZValues[3] / 100);
}

void HUDL::headerForCorner(Corner corner, const char* text) {
    // Clear the sections area so text is not written over old text.
    uint8_t sectionColumn = columnForCorner(corner);
    uint8_t sectionPage = pageForCorner(corner);

    lcd.clearArea(64, 2, sectionPage, sectionColumn);

    // Calculate the padding to center the text in the section
    uint8_t length = strlen(text) * 8;
    uint8_t padding = (64 - length) / 2;

    sectionColumn += padding;

    // Write the text to the screen under the section header.
    lcd.writeText(text, sectionPage, sectionColumn, FontSize::LARGE, false);
}

void HUDL::dataForCorner(Corner corner, const char* text) {
    // Clear the sections area so text is not written over old text.
    uint8_t sectionColumn = columnForCorner(corner);
    uint8_t sectionPage = pageForCorner(corner) + 2;

    lcd.clearArea(64, 2, sectionPage, sectionColumn);

    // Calculate the padding to center the text in the section
    uint8_t length = strlen(text) * 8;
    uint8_t padding = (64 - length) / 2;

    sectionColumn += padding;

    // Write the text to the screen under the section header.
    lcd.writeText(text, sectionPage, sectionColumn, FontSize::LARGE, false);
}

uint8_t HUDL::columnForCorner(Corner corner) {
    switch (corner) {
    case TOP_LEFT:
    case BOTTOM_LEFT:
        return 0;
    case TOP_RIGHT:
    case BOTTOM_RIGHT:
        return 64;
    default:
        return -1;
    }
}

uint8_t HUDL::pageForCorner(Corner corner) {
    switch (corner) {
    case TOP_LEFT:
    case TOP_RIGHT:
        return 0;
    case BOTTOM_LEFT:
    case BOTTOM_RIGHT:
        return 4;
    default:
        return -1;
    }
}

void HUDL::process() {
    if (displayCounter >= REFRESH_RATE) {
        displayCounter = 0;

        updateLCD();
    } else {
        displayCounter++;
    }
}

}// namespace HUDL
