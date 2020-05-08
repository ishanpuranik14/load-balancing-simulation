cmake_minimum_required(VERSION 3.0.0)
project(load-balancing-simulation)

include(CTest)
include(cmake/utils.cmake)
enable_testing()

find_package(spdlog REQUIRED)

add_executable(${PROJECT_NAME} src/simulator.cpp src/Poisson.cpp src/Generator.cpp src/Uniform.cpp src/Server.cpp
        src/Stats.cpp src/Request.cpp src/Clock.h src/Clock.cpp src/TracePlayer.cpp src/RequestSpec.h)
spdlog_enable_warnings(${PROJECT_NAME})
target_link_libraries(${PROJECT_NAME} PRIVATE spdlog::spdlog)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
