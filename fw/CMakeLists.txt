cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(ringlight C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
pico_sdk_init()

add_executable(ringlight)

#Load SCPI Device library: https://github.com/j123b567/scpi-parser
add_compile_definitions(HAVE_STDBOOL)
set(SCPI_SRC_PATH "../scpi-parser/libscpi/src")
add_library(scpi STATIC
        ${SCPI_SRC_PATH}/error.c
        ${SCPI_SRC_PATH}/fifo.c
        ${SCPI_SRC_PATH}/ieee488.c
        ${SCPI_SRC_PATH}/minimal.c
        ${SCPI_SRC_PATH}/parser.c
        ${SCPI_SRC_PATH}/units.c
        ${SCPI_SRC_PATH}/utils.c
        ${SCPI_SRC_PATH}/lexer.c
        ${SCPI_SRC_PATH}/expression.c
)
target_include_directories(scpi PUBLIC "scpi-parser/libscpi/inc")

#Load USB
set(USB_SRC_PATH "../usb")
add_library(usbinst STATIC
        ${USB_SRC_PATH}/usb_descriptors.c
        ${USB_SRC_PATH}/usbtmc_app.c
)
target_include_directories(usbinst PUBLIC "usb")
target_link_libraries(usbinst PUBLIC pico_stdlib tinyusb_board tinyusb_device)

#Build main file
target_sources(ringlight PRIVATE
        main.cpp
        )

#pico_enable_stdio_usb(ringlight 1)
#pico_enable_stdio_uart(ringlight 0)
        
pico_add_extra_outputs(ringlight)
        
target_link_libraries(ringlight PRIVATE pico_stdlib hardware_pwm usbinst scpi)
