namespace DEV = EVT::core::DEV;
namespace IO = EVT::core::IO;

namespace HUDL {
    class HUDL {
    public:
        IO::GPIO &reg_select; // PA_3
        IO::GPIO &reset;      // PB_3
        IO::GPIO &CS;         // PB_12

        // default constructor for HUDL class
        HUDL();

        // writes data to the LCD to show on the screen
        void data_write(unsigned char d);

        // writes commands to the LCD to conrol the ST7565
        void comm_write(unsigned char d);

        // drives a single pixel on the LCD
        void drive_pixel(unsigned char page, unsigned char col_up,
                         unsigned char col_low, unsigned char data);

        // clears the LCD
        void ClearLCD(unsigned char *lcd_string);

        // calls a set of commands to initialize LCD
        void init_LCD();




    };
}
