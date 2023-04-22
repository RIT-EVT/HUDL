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
    case PAGE_1:
    { // Set a new scope for this case
        if (!setHeaders) {
            lcd.clearLCD();
            headerForCorner(TOP_LEFT, "Battery");
            headerForCorner(TOP_RIGHT, "Torque");
            headerForCorner(BOTTOM_LEFT, "RPM");
            headerForCorner(BOTTOM_RIGHT, "MC Stat");
            setHeaders = true;
        }

        // Set the battery voltage
        char voltage[8];
        std::sprintf(voltage, "%hu v", totalVoltage * 10);
        dataForCorner(TOP_LEFT, voltage);

        // Set the torque
        char torque[8];
        std::sprintf(torque, "%hu", torqueActual);
        dataForCorner(TOP_RIGHT, torque);

        // Set the rpm
        char rpm[8];
        std::sprintf(rpm, "%lu", ~actualPosition);
        dataForCorner(BOTTOM_LEFT, rpm);

        // Set the status word
        char status[8];

        if (statusWord == 0x21) {
            std::sprintf(status, "STOP");
        } else if (statusWord == 0x27) {
            std::sprintf(status, "GO");
        } else {
            std::sprintf(status, "UNK %x", statusWord);
        }
        dataForCorner(BOTTOM_RIGHT, status);
    }
        break;
    case PAGE_2:
    { // Set a new scope for this case
        if (!setHeaders) {
            lcd.clearLCD();
            headerForCorner(TOP_LEFT, "Battery");
            headerForCorner(TOP_RIGHT, "Velocity");
            headerForCorner(BOTTOM_LEFT, "HI Temp");
            headerForCorner(BOTTOM_RIGHT, "Position");
            setHeaders = true;
        }

        // Set the battery voltage
        char voltage[8];
        std::sprintf(voltage, "%hu v", totalVoltage * 10);
        dataForCorner(TOP_LEFT, voltage);

        // Set the velocity
        uint32_t mph = actualPosition * (27 * 60 * 314) / 433 / 63360;
        char mphText[8];
        std::sprintf(mphText, "%lu", mph);
        dataForCorner(TOP_RIGHT, mphText);


        uint16_t highestTemp = thermTemps[0];
        for (uint16_t temp : thermTemps) {
            if (temp > highestTemp) {
                highestTemp = temp;
            }
        }
        char temp[8];
        std::sprintf(temp, "%hu.%hu C", thermTemps[0] / 100, thermTemps[0] % 100);

        dataForCorner(BOTTOM_LEFT, temp);


        char positionText[8];
        std::sprintf(positionText, "%lu", ~actualPosition);
        dataForCorner(BOTTOM_RIGHT, positionText);
    }
    break;
    case ERROR_PAGE:
    { // Set a new scope for this page
        if (!setHeaders) {
            lcd.clearLCD();
        }
        writeError("This is a really really really really really really long error that really really really broke the bike.");
        break;
    }
    }

    if (headerCounter == 255) {
        if (currentHUDLScreen == PAGE_1) {
            currentHUDLScreen = PAGE_2;
        } else if (currentHUDLScreen == PAGE_2) {
            currentHUDLScreen = PAGE_1;
        }
        setHeaders = false;
        headerCounter = 0;
    } else {
        headerCounter ++;
    }
}

void HUDL::headerForCorner(HUDL::Corner corner, const char* text) {
    // Clear the sections area so text is not written over old text.
    uint8_t sectionColumn = columnForCorner(corner);
    uint8_t sectionPage = pageForCorner(corner);

    lcd.clearArea(64, 16, sectionPage, sectionColumn);

    // Calculate the padding to center the text in the section
    uint8_t length = strlen(text) * 8;
    uint8_t padding = (64 - length) / 2;

    sectionColumn += padding;

    // Write the text to the screen under the section header.
    writeLargeText(text, sectionPage, sectionColumn, false);
}

void HUDL::dataForCorner(HUDL::Corner corner, const char* text) {
    // Clear the sections area so text is not written over old text.
    uint8_t sectionColumn = columnForCorner(corner);
    uint8_t sectionPage = pageForCorner(corner) + 2;

    lcd.clearArea(64, 8, sectionPage, sectionColumn);

    // Calculate the padding to center the text in the section
    uint8_t length = strlen(text) * 8;
    uint8_t padding = (64 - length) / 2;

    sectionColumn += padding;

    // Write the text to the screen under the section header.
    writeLargeText(text, sectionPage, sectionColumn, false);
}

uint8_t HUDL::columnForCorner(HUDL::Corner corner) {
    switch (corner) {
    case TOP_LEFT: return 0;
    case TOP_RIGHT: return 64;
    case BOTTOM_LEFT: return 0;
    case BOTTOM_RIGHT: return 64;
    }
}

uint8_t HUDL::wrapForCorner(HUDL::Corner corner) {
    switch (corner) {
    case TOP_LEFT: return 64;
    case TOP_RIGHT: return 128;
    case BOTTOM_LEFT: return 64;
    case BOTTOM_RIGHT: return 128;
    }
}

uint8_t HUDL::pageForCorner(HUDL::Corner corner) {
    switch (corner) {
    case TOP_LEFT: return 0;
    case TOP_RIGHT: return 0;
    case BOTTOM_LEFT: return 4;
    case BOTTOM_RIGHT: return 4;
    }
}

void HUDL::writeLargeText(const char* text, uint8_t page, uint8_t column, bool wrapText) {
    for (uint8_t x = 0; x < strlen(text); x++) {
        // Get the ASCII value of the character.
        uint8_t fontIndex = text[x];

        // Create the character that we need to write to the screen.
        unsigned char characterMap[16] = {
            BitmapFont::font6x13[fontIndex][0],
            BitmapFont::font6x13[fontIndex][1],
            BitmapFont::font6x13[fontIndex][2],
            BitmapFont::font6x13[fontIndex][3],
            BitmapFont::font6x13[fontIndex][4],
            BitmapFont::font6x13[fontIndex][5],
            BitmapFont::font6x13[fontIndex][6],
            BitmapFont::font6x13[fontIndex][7],
            BitmapFont::font6x13[fontIndex][8],
            BitmapFont::font6x13[fontIndex][9],
            BitmapFont::font6x13[fontIndex][10],
            BitmapFont::font6x13[fontIndex][11],
            BitmapFont::font6x13[fontIndex][12],
            0b00000000,
            0b00000000,
            0b00000000,
        };

        if (column >= 128) {
            return;
        }

        // Display the character bit map at the calculated page and column.
        lcd.displayBitMapInArea(characterMap, 8, 16, page, column, 2);
        column += 8;// Advance the column for the next character.

        // If we need to wrap text, move the page forward and the column to 0.
        if (wrapText && column >= 128) {
            page += 1;
            column = 0;
        }
    }
}

void HUDL::writeSmallText(const char* text, uint8_t page, uint8_t column, bool wrapText) {
    for (uint8_t x = 0; x < strlen(text); x++) {
        // Get the ASCII value of the character.
        uint8_t fontIndex = text[x];

        // Create the character that we need to write to the screen.
        unsigned char characterMap[4] = {
            BitmapFont::font4x6[fontIndex][0],
            BitmapFont::font4x6[fontIndex][1],
            BitmapFont::font4x6[fontIndex][2],
            BitmapFont::font4x6[fontIndex][3],
        };

        if (column >= 128) {
            return;
        }

        // Display the character bit map at the calculated page and column.
        lcd.displayBitMapInArea(characterMap, 4, 8, page, column, 1);
        column += 4;// Advance the column for the next character.

        // If we need to wrap text, move the page forward and the column to 0.
        if (wrapText && column >= 128) {
            page++;
            column = 0;
        }
    }

}
void HUDL::writeError(const char* text) {
     // Calculate the padding to center the text in the section
     uint8_t length = strlen("!!! Error !!!") * 8;
     uint8_t padding = (128 - length) / 2;

     // Write the Error header
     writeLargeText("!!! Error !!!", 0, padding, false);

     // Write the error message
     writeSmallText(text, 2, 0, true);
}

}// namespace HUDL
