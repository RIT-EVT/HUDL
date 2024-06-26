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
        headerForCorner(TOP_LEFT, "Bat %");
        headerForCorner(TOP_RIGHT, "Temp");
        headerForCorner(BOTTOM_LEFT, "RPM");
        headerForCorner(BOTTOM_RIGHT, "MC Stat");
        setHeaders = true;
    }

    // Set the battery voltage
    char voltage[9];
    std::sprintf(voltage, "%hu.%hu v", totalVoltage / 10, totalVoltage % 10);
    dataForCorner(TOP_LEFT, voltage);

    // Set the highest temp
    uint16_t highestTemp = thermTemps[0];
    for (uint16_t temp : thermTemps) {
        if (temp > highestTemp) {
            highestTemp = temp;
        }
    }

    char temp[9];
    std::sprintf(temp, "%hu.%hu C", highestTemp / 100, highestTemp % 100);
    dataForCorner(TOP_RIGHT, temp);

    // Set the rpm
    char rpm[8];
    std::sprintf(rpm, "%d", actualPosition);
    dataForCorner(BOTTOM_LEFT, rpm);

    // Set the motor controller status word
    char status[8];

    if (statusWord == 0x21) {
        std::sprintf(status, "STOP");
    } else if (statusWord == 0x27) {
        std::sprintf(status, "GO");
    } else {
        std::sprintf(status, "%x", statusWord);
    }

    dataForCorner(BOTTOM_RIGHT, status);
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
