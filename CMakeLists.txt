cmake_minimum_required(VERSION 3.15)
project(anbUI C)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(PORTABILITY_LAYER pl_linux.c)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Android")
    set(PORTABILITY_LAYER pl_linux.c)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(PORTABILITY_LAYER pl_win32.c)
elseif(CMAKE_SYSTEM_NAME STREQUAL "DOS")
    set(PORTABILITY_LAYER pl_dos.c)
else()
    message(FATAL_ERROR "Unsupported system: ${CMAKE_SYSTEM_NAME}")
endif()

set(SOURCE_FILES anbui.c ad_obj.c ad_ui.c ad_text.c ${PORTABILITY_LAYER})

add_library(anbui STATIC ${SOURCE_FILES})
target_include_directories(anbui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(anbui_test ad_test.c)
target_link_libraries(anbui_test PRIVATE anbui)
