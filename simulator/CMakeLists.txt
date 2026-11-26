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

if(EMSCRIPTEN)
  # Browser build (phone proof of concept): emcmake cmake -S simulator -B build-web && cmake --build build-web
  # SDL2 comes from Emscripten's own port; the fonts are packed into the .data file.
  set(SDL_FLAGS "-sUSE_SDL=2")
  add_compile_options(${SDL_FLAGS})
  add_link_options(${SDL_FLAGS})
else()
  find_package(SDL2 REQUIRED)
endif()

file(GLOB_RECURSE LVGL_SRC ${LVGL_DIR}/src/*.c)
add_library(lvgl STATIC ${LVGL_SRC})
target_include_directories(lvgl PUBLIC ${LVGL_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${SDL2_INCLUDE_DIRS})
target_compile_definitions(lvgl PUBLIC LV_CONF_INCLUDE_SIMPLE)
if(NOT EMSCRIPTEN)
  target_link_libraries(lvgl PUBLIC ${SDL2_LIBRARIES})
endif()

file(GLOB FONT_SRC ${CMAKE_CURRENT_SOURCE_DIR}/fonts/*.c)
add_executable(noir-simulator src/main.cpp ${FONT_SRC})
# noir-ui.h comes from ../firmware/core: the exact same screen code as the firmware
target_include_directories(noir-simulator PRIVATE src inc assets ${CMAKE_CURRENT_SOURCE_DIR}/../firmware/core)
target_link_libraries(noir-simulator PRIVATE lvgl)
# bigger screens (--size) render Inter and the icons from the firmware's TTF files at their own size
if(EMSCRIPTEN)
  target_compile_definitions(noir-simulator PRIVATE NOIR_FONT_DIR="/fonts")
  set_target_properties(noir-simulator PROPERTIES SUFFIX ".js")
  target_link_options(noir-simulator PRIVATE -sALLOW_MEMORY_GROWTH=1 -sINITIAL_MEMORY=67108864 -sEXPORTED_RUNTIME_METHODS=ccall,cwrap
    -sEXPORTED_FUNCTIONS=_main,_noir_web_config,_noir_web_states -sMODULARIZE=1 -sEXPORT_NAME=NoirSim
    "SHELL:--preload-file ${CMAKE_CURRENT_SOURCE_DIR}/../firmware/fonts/Inter-Regular.ttf@/fonts/Inter-Regular.ttf"
    "SHELL:--preload-file ${CMAKE_CURRENT_SOURCE_DIR}/fonts/mdi-web.ttf@/fonts/mdi-subset.ttf")   # only the icons NOIR uses (tools/make_sim_font.py)
else()
  target_compile_definitions(noir-simulator PRIVATE NOIR_FONT_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../firmware/fonts")
endif()
if(MINGW)
  target_link_libraries(noir-simulator PRIVATE mingw32)
  target_link_options(noir-simulator PRIVATE -mconsole)
endif()
