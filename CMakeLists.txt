cmake_minimum_required(VERSION 3.24)
project(ProxyServer)

set(CMAKE_CXX_STANDARD 17)
add_compile_options(-Wall -Werror -Wextra -O3)

##for profiling
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg -g")
# set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pg")


add_executable(ProxyServer
        main.cpp
        src/Client.cpp
        src/Connection.cpp
	src/Logger.cpp
	src/ServerImpEpoll.cpp
)

