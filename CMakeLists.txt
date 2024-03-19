###############################################################################
# Top level CMakeList for building the EVT HUDL source code
###############################################################################
if(EVT_CORE_LOG_ENABLE)
    add_compile_definitions(EVT_CORE_LOG_ENABLE)
endif()
cmake_minimum_required(VERSION 3.15)

# Increases the number of RPDOs that CAN Open can handle.
# Also able to change the number of TPDOs if RPDO in the definition
# is changed to TPDO (CO_TPDO_N=50)
add_compile_definitions(CO_RPDO_N=50)

add_compile_definitions(USE_HAL_DRIVER)

set(EVT_CORE_DIR      ${CMAKE_SOURCE_DIR}/libs/EVT-core)
set(TARGET_DEV        "STM32F302x8")

# Link to the EVT-core library
add_subdirectory(libs/EVT-core/)

include(CMakeDependentOption)
include(${EVT_CORE_DIR}/cmake/evt-core_compiler.cmake)
include(${EVT_CORE_DIR}/cmake/evt-core_install.cmake)


###############################################################################
# Project Setup
###############################################################################
set(BOARD_LIB_NAME   HUDL)
if("${BOARD_LIB_NAME}" STREQUAL CHANGE_ME)
    message(FATAL_ERROR
            "You must set the template project name in CmakeLists.txt")
endif()

file(STRINGS version.txt BOARD_VERSION)

project(${BOARD_LIB_NAME}
        VERSION ${BOARD_VERSION}
        LANGUAGES CXX C
        )

add_library(${PROJECT_NAME} STATIC)

# Add sources
target_sources(${PROJECT_NAME} PRIVATE
        src/HUDL.cpp
        )

###############################################################################
# Handle dependencies
###############################################################################

# TODO: This should be set by the user of this library
add_compile_definitions(STM32F302x8)


target_link_libraries(${PROJECT_NAME}
        PUBLIC EVT
        )

###############################################################################
# Install and expose library
###############################################################################
install_and_expose(${PROJECT_NAME})

###############################################################################
# Build Target Code
###############################################################################
add_subdirectory(targets)
