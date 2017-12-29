project(minesweeper)
add_definitions(-std=gnu++1y)
set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")
cmake_minimum_required(VERSION 2.8)
set(SRC_LIST
 src/main.cpp
 src/Minefield.cpp
 src/KBInput.cpp
)

find_library(CAFCORE NAMES caf_core_static caf_core)
find_library(CAFIO NAMES caf_io_static caf_io)
include_directories("/opt/build/static/include")
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.50 COMPONENTS system program_options thread REQUIRED)
list(APPEND LIBS ${Boost_LIBRARIES})
add_executable(${PROJECT_NAME} ${SRC_LIST})
list(APPEND LIBS ${CAFIO} ${CAFCORE}) 
if(WIN32)
  message(STATUS "Adding wsock32")
  list(APPEND LIBS -lpsapi -lws2_32 -liphlpapi)
endif()
message(STATUS "Libs=${LIBS}")
target_link_libraries(${PROJECT_NAME} ${LIBS})
