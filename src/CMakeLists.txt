cmake_minimum_required(VERSION 2.6)
project(Geminga) 

include(FindPkgConfig)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-write-strings -Wno-deprecated -g")

set(Boost_USE_STATIC_LIBS   OFF)
set(Boost_USE_MULTITHREADED ON)
find_package(Boost 1.38.0 COMPONENTS system thread filesystem unit_test_framework)

pkg_check_modules(MICROHTTPD "libmicrohttpd")
pkg_check_modules(AVAHI_CLIENT "avahi-client")

if(Boost_FOUND)
    include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${Boost_INCLUDE_DIRS}
        ${BoostTest_INCLUDE_DIRS}
        ${MICROHTTPD_INCLUDE_DIRS}
        ${AVAHI_CLIENT_INCLUDE_DIRS})

    link_directories(
        ${Boost_LIBRARY_DIRS}
        ${BoostTest_LIBRARY_DIRS}
        ${MICROHTTPD_LIBRARY_DIRS}
        ${AVAHI_CLIENT_LIBRARY_DIRS}
        ${CMAKE_BINARY_DIR})

    add_library(geminga SHARED
        Geminga.cpp
        Zeroconf.cpp)

    target_link_libraries(geminga
        ${Boost_SYSTEM_LIBRARY}
        ${Boost_THREAD_LIBRARY}
        ${Boost_FILESYSTEM_LIBRARY}
        -l${MICROHTTPD_LIBRARIES}
        -l${AVAHI_CLIENT_LIBRARIES})

    add_executable(gservice main.cpp)
    target_link_libraries(gservice -lgeminga)
endif()
