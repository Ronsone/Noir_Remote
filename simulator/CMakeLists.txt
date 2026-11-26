cmake_minimum_required(VERSION 3.16)
project(noir_simulator C CXX)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 17)

# LVGL 9.5 (dezelfde versie als de firmware). Met -DLVGL_DIR=<pad> een eigen kopie gebruiken, anders van GitHub.
set(LVGL_DIR "" CACHE PATH "Pad naar een LVGL 9.5-map (leeg = downloaden)")
if(LVGL_DIR STREQUAL "")
  include(FetchContent)
  # SOURCE_SUBDIR zonder CMakeLists: alleen downloaden, de bronnen bouwen we hieronder zelf
  FetchContent_Declare(lvgl GIT_REPOSITORY https://github.com/lvgl/lvgl.git GIT_TAG v9.5.0 GIT_SHALLOW TRUE
                       SOURCE_SUBDIR geen_cmake)
  FetchContent_MakeAvailable(lvgl)
  set(LVGL_DIR ${lvgl_SOURCE_DIR})
endif()

find_package(SDL2 REQUIRED)

file(GLOB_RECURSE LVGL_SRC ${LVGL_DIR}/src/*.c)
add_library(lvgl STATIC ${LVGL_SRC})
target_include_directories(lvgl PUBLIC ${LVGL_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${SDL2_INCLUDE_DIRS})
target_compile_definitions(lvgl PUBLIC LV_CONF_INCLUDE_SIMPLE)
target_link_libraries(lvgl PUBLIC ${SDL2_LIBRARIES})

file(GLOB FONT_SRC ${CMAKE_CURRENT_SOURCE_DIR}/fonts/*.c)
add_executable(noir-simulator src/main.cpp ${FONT_SRC})
# noir-ui.h comes from ../firmware/core: the exact same screen code as the firmware
target_include_directories(noir-simulator PRIVATE src inc assets ${CMAKE_CURRENT_SOURCE_DIR}/../firmware/core)
target_link_libraries(noir-simulator PRIVATE lvgl)
if(MINGW)
  target_link_libraries(noir-simulator PRIVATE mingw32)
  target_link_options(noir-simulator PRIVATE -mconsole)
endif()
