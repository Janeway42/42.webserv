cmake_minimum_required(VERSION 3.20)
project(http-parser)
project(config-file-parser)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++98")
set(CMAKE_CXX_STANDARD 98)
SET(GCC_CODAM_COMPILE_FLAGS "-Wall -Wextra -Werror -Wshadow -pedantic -std=c++98")
SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_CODAM_COMPILE_FLAGS}")

include_directories(.)

add_executable(http-parser
        includes/Parser.hpp
        includes/RequestParser.hpp
        includes/RequestData.hpp
        srcs/mainHttpRequest.cpp
        srcs/Parser.cpp
        srcs/RequestData.cpp
        srcs/RequestParser.cpp)

add_executable(config-file-parser
        includes/ConfigFileParser.hpp
        includes/LocationData.hpp
        includes/Parser.hpp
        includes/ServerData.hpp
        srcs/mainConfigFile.cpp
        srcs/ConfigFileParser.cpp
        srcs/LocationData.cpp
        srcs/Parser.cpp
        srcs/ServerData.cpp)
