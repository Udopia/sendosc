cmake_minimum_required(VERSION 3.0.0)
project(sendosc VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)

add_executable(sendosc Main.cc)

include(CTest)
if(BUILD_TESTING)
    add_executable(sendosc_cli_test tests/cli_test.cc)
    add_test(NAME sendosc_cli COMMAND sendosc_cli_test $<TARGET_FILE:sendosc>)
endif()
