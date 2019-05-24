cmake_minimum_required(VERSION 2.8.0)

add_definitions(-DDISABLE_ENCRYPTION)

project(httpserver)

set(HEADER_FILES swarm.h
 serialization.h
 ../external/json.hpp
 connection_manager.h
 connection.h
 request_handler.h
 server.h
 router.h
 json_rpc_api.h
 client_api.h
 message_notifier.h
 types.h
 snode_api.h
 snode_push_api.h
 snode_test_api.h
)
set(SRC_FILES main.cpp
 swarm.cpp
 serialization.cpp
 connection_manager.cpp
 connection.cpp
 request_handler.cpp
 server.cpp
 router.cpp
 client_api.cpp
 json_rpc_api.cpp
 snode_api.cpp
 snode_push_api.cpp
 snode_test_api.cpp
)

add_executable(httpserver ${HEADER_FILES} ${SRC_FILES})

loki_add_subdirectory(../storage storage)
loki_add_subdirectory(../utils utils)
loki_add_subdirectory(../pow pow)
loki_add_subdirectory(../crypto crypto)
find_package(OpenSSL REQUIRED)

find_package(Boost
    REQUIRED
    log
    system
    filesystem
    program_options
)

target_link_libraries(httpserver PRIVATE OpenSSL::SSL)
target_link_libraries(httpserver PRIVATE storage)
target_link_libraries(httpserver PRIVATE utils)
target_link_libraries(httpserver PRIVATE pow)
target_link_libraries(httpserver PRIVATE crypto)

set_property(TARGET httpserver PROPERTY CXX_STANDARD 14)

target_include_directories(httpserver PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(httpserver PRIVATE ${Boost_LIBRARIES})
