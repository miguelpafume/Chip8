cmake_minimum_required(VERSION 3.26.0)
project(chip8 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)

include (FetchContent)

FetchContent_Declare(MAGE
  GIT_REPOSITORY https://github.com/miguelpafume/MAGE-Engine.get_directory_property
  GIT_TAG 0.1.0
)

FetchContent_MakeAvailable(MAGE)

add_executable(chip8
  src/main.cpp
  src/Chip8.cpp
  src/Chip8App.cpp
)

target_link_libraries(chip8 PRIVATE MAGE::mage)

target_compile_definitions(chip8 PRIVATE
  CHIP8_ROM_DIR="${CMAKE_CURRENT_SOURCE_DIR}/roms")