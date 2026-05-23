cmake_minimum_required(VERSION 3.15)

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR avr)

project(BoB_Jarvis C)

# AVR MCU
set(MCU atmega1284p)

# CPU Frequency
add_definitions(-DF_CPU=8000000UL)

# Compiler
set(CMAKE_C_COMPILER avr-gcc)

# AVR Flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mmcu=${MCU} -Os")

# Source files
add_executable(BoB_Jarvis.elf
        main.c
        MENU.c
        remote_keypad.c
)

# HEX generation
add_custom_command(TARGET BoB_Jarvis.elf POST_BUILD
        COMMAND avr-objcopy -O ihex -R .eeprom
        BoB_Jarvis.elf
        BoB_Jarvis.hex
)