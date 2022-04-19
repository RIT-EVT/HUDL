#ifndef HUDL_INCLUDE_HUDL_HUDL_HPP
#ifndef HUDL_INCLUDE_HUDL_HUDL_HPP

#include <Canopen/co_core.h>
#include <EVT/dev/lcd.hpp>
#include <EVT/utils/log.hpp>

namespace DEV = EVT::core::DEV;
namespace log = EVT::core::log;
namespace IO = EVT::core::IO;


namespace HUDL {

class HUDL {
public: 
    
    /**
    * Constructpr for the HUDL Class
    */
    HUDL(IO::Pin::GPIO& SI, IO::Pin::GPIO& SCK, IO::Pin::GPIO& CS ,IO::Pin::GPIO& REG_SEL, IO::Pin::GPIO& RESET);
    
    DisplayError();

    AdjustDigipot();

    /**
     * Get a pointer to the start of the CANopen object dictionary.
     *
     * @return Pointer to the start of the CANopen object dictionary.
     */
    CO_OBJ_T* getObjectDictionary();

    /**
     * Get the number of elements in the object dictionary.
     *
     * @return The number of elements in the object dictionary
     */
    uint16_t getObjectDictionarySize();
    
}

}// namespace this class

#endif // HUDL_INCLUDE_HUDL_HUDL_HPP