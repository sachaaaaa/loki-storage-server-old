cmake_minimum_required (VERSION 3.1)

add_executable (Test main.cpp storage.cpp pow.cpp)

set_property(TARGET Test PROPERTY CXX_STANDARD 11)

# library under test
add_subdirectory(../storage storage)
add_subdirectory(../pow pow)
target_link_libraries(Test PRIVATE sn_storage pow)

# boost
find_package(Boost
REQUIRED
system
filesystem
chrono
thread
unit_test_framework
)
# add_definitions (-DBOOST_TEST_DYN_LINK)
target_include_directories(Test PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(Test PRIVATE ${Boost_LIBRARIES})