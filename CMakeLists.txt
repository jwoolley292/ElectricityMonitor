cmake_minimum_required(VERSION 3.10)
set(QEMU_EXTRA_FLAGS -s)

include($ENV{ZEPHYR_BASE}/cmake/app/boilerplate.cmake NO_POLICY_SCOPE)
project(NONE)

zephyr_include_directories_ifdef(CONFIG_BOARD_BBC_MICROBIT
   $ENV{ZEPHYR_BASE}/boards/arm/bbc_microbit)

FILE(GLOB app_sources src/*.c)
target_sources(app PRIVATE ${app_sources})
