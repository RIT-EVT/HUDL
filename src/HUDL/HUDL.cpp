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
#include <cstring>
#include "EVT/dev/BitmapFonts.hpp"
// clang-format on

namespace IO = EVT::core::IO;
namespace DEV = EVT::core::DEV;
namespace log = EVT::core::log;

namespace HUDL {
HUDL::HUDL(IO::GPIO& reg_select, IO::GPIO& reset, IO::SPI& spi) : lcd(DEV::LCD(reg_select, reset, spi)) {}

void HUDL::initLCD() {
    lcd.initLCD();
    lcd.clearLCD();
    lcd.setEntireScreenBitMap(evtBitMap);
    EVT::core::time::wait(2000);
    lcd.clearLCD();
    //    lcd.setDefaultSections(SECTION_TITLES);
    //    lcd.displaySectionHeaders();
}

CO_OBJ_T* HUDL::getObjectDictionary() {
    return &objectDictionary[0];
}

uint16_t HUDL::getObjectDictionarySize() const {
    return OBJECT_DICTIONARY_SIZE;
}

void HUDL::updateLCD() {
    switch (currentHUDLScreen) {
    case PAGE_1: {// Set a new scope for this case
        if (!setHeaders) {
            lcd.clearLCD();
            headerForCorner(TOP_LEFT, "Battery");
            headerForCorner(TOP_RIGHT, "HI Temp");
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
        std::sprintf(temp, "%hu.%hu C", thermTemps[0] / 100, thermTemps[0] % 100);

        dataForCorner(TOP_RIGHT, temp);

        // Set the rpm
        char rpm[8];
        std::sprintf(rpm, "%lu", actualPosition);
        dataForCorner(BOTTOM_LEFT, rpm);

        // Set the status word
        char status[8];

        if (statusWord == 0x21) {
            std::sprintf(status, "STOP");
        } else if (statusWord == 0x27) {
            std::sprintf(status, "GO");
        } else {
            std::sprintf(status, "UNKNOWN");
//            std::sprintf(errorString, "There was an error with the Motor Controller: Unknown Error %x", statusWord);
//            currentHUDLScreen = ERROR_PAGE;
        }

        dataForCorner(BOTTOM_RIGHT, status);
    } break;
    case ERROR_PAGE: {// Set a new scope for this page
        if (!setHeaders) {
            lcd.clearLCD();
        }
        writeError(errorString);
        break;
    }
    }
}

void HUDL::headerForCorner(HUDL::Corner corner, const char* text) {
    // Clear the sections area so text is not written over old text.
    uint8_t sectionColumn = columnForCorner(corner);
    uint8_t sectionPage = pageForCorner(corner);

    lcd.clearArea(64, 2, sectionPage, sectionColumn);

    // Calculate the padding to center the text in the section
    uint8_t length = strlen(text) * 8;
    uint8_t padding = (64 - length) / 2;

    sectionColumn += padding;

    // Write the text to the screen under the section header.
    lcd.writeLargeText(text, sectionPage, sectionColumn, false);
}

void HUDL::dataForCorner(HUDL::Corner corner, const char* text) {
    // Clear the sections area so text is not written over old text.
    uint8_t sectionColumn = columnForCorner(corner);
    uint8_t sectionPage = pageForCorner(corner) + 2;

    lcd.clearArea(64, 2, sectionPage, sectionColumn);

    // Calculate the padding to center the text in the section
    uint8_t length = strlen(text) * 8;
    uint8_t padding = (64 - length) / 2;

    sectionColumn += padding;

    // Write the text to the screen under the section header.
    lcd.writeLargeText(text, sectionPage, sectionColumn, false);
}

void HUDL::writeError(const char* text) {
    // Calculate the padding to center the text in the section
    uint8_t length = strlen("!!! Error !!!") * 8;
    uint8_t padding = (128 - length) / 2;

    // Write the Error header
    lcd.writeLargeText("!!! Error !!!", 0, padding, false);

    // Write the error message
    lcd.writeSmallText(text, 2, 0, true);
}

uint8_t HUDL::columnForCorner(HUDL::Corner corner) {
    switch (corner) {
    case TOP_LEFT:
    case BOTTOM_LEFT:
        return 0;
    case TOP_RIGHT:
    case BOTTOM_RIGHT:
        return 64;
    }
}

uint8_t HUDL::wrapForCorner(HUDL::Corner corner) {
    switch (corner) {
    case TOP_LEFT:
    case BOTTOM_LEFT:
        return 64;
    case TOP_RIGHT:
    case BOTTOM_RIGHT:
        return 128;
    }
}

uint8_t HUDL::pageForCorner(HUDL::Corner corner) {
    switch (corner) {
    case TOP_LEFT:
    case TOP_RIGHT:
        return 0;
    case BOTTOM_LEFT:
    case BOTTOM_RIGHT:
        return 4;
    }
}
}// namespace HUDL
