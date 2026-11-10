cmake_minimum_required(VERSION 3.24)
project(PosTerakhir LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

include(FetchContent)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES OFF CACHE BOOL "" FORCE)
FetchContent_Declare(raylib
    URL https://codeload.github.com/raysan5/raylib/tar.gz/refs/tags/5.5
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    TIMEOUT 600
    INACTIVITY_TIMEOUT 30
)
FetchContent_MakeAvailable(raylib)

add_executable(zombie_td src/main.c src/game.c src/view.c)
target_link_libraries(zombie_td PRIVATE raylib)
target_compile_options(zombie_td PRIVATE -Wall -Wextra -Wpedantic)
add_custom_command(TARGET zombie_td POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/assets $<TARGET_FILE_DIR:zombie_td>/assets
)

include(CTest)
if(BUILD_TESTING)
    add_executable(game_checks tests/game_checks.c src/game.c)
    target_include_directories(game_checks PRIVATE src)
    target_link_libraries(game_checks PRIVATE raylib)
    target_compile_options(game_checks PRIVATE -Wall -Wextra -Wpedantic)
    add_test(NAME game_rules COMMAND game_checks)
endif()
