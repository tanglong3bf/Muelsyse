cmake_minimum_required(VERSION 3.5)
project(MuelsyseTestClient CXX)

include(CheckIncludeFileCXX)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(${PROJECT_NAME} main.cc)

# ##############################################################################
# If you include the drogon source code locally in your project, use this method
# to add drogon 
# add_subdirectory(drogon) 
# target_link_libraries(${PROJECT_NAME} PRIVATE drogon)
#
# and comment out the following lines
find_package(Drogon CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE Drogon::Drogon)

# ##############################################################################

message(STATUS "use c++20")

aux_source_directory(unittests TEST_DIR)
aux_source_directory(../../src PLUGIN_SRC)

target_sources(${PROJECT_NAME}
               PRIVATE
               ${TEST_DIR}
               ${PLUGIN_SRC})

target_link_libraries(${PROJECT_NAME} PRIVATE gtest)
SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fconcepts -fprofile-arcs -ftest-coverage -fno-inline -g3 -O0 -fno-omit-frame-pointer -fno-optimize-sibling-calls")

file(COPY config.yaml DESTINATION ${CMAKE_BINARY_DIR})
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/logs)
