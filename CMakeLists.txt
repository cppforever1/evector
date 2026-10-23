cmake_minimum_required(VERSION 3.10.0)
project(eunsortedvector VERSION 0.1.0 LANGUAGES C CXX)

add_executable(eunsortedvector main.cpp)
set_target_properties(eunsortedvector PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

include(CTest)
if(BUILD_TESTING)
    add_executable(eunsortedvector_tests tests/eunsortedvector_tests.cpp)
    target_include_directories(eunsortedvector_tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
    set_target_properties(eunsortedvector_tests PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
    )
    add_test(NAME eunsortedvector_tests COMMAND eunsortedvector_tests)
endif()
