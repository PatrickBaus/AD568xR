cmake_minimum_required(VERSION 3.14)
project(ad568xr_tests CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# The library uses binary literals, which are a GNU extension in C++11 (Arduino builds with gnu++11 or newer)
set(CMAKE_CXX_EXTENSIONS ON)

include(FetchContent)
FetchContent_Declare(
  doctest
  GIT_REPOSITORY https://github.com/doctest/doctest.git
  GIT_TAG v2.4.12
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(doctest)

# The library under test, built against the fakes in mocks/ and instrumented for coverage
add_library(ad568xr STATIC ../src/ad568xr.cpp)
target_include_directories(ad568xr PUBLIC ../src mocks)
target_compile_options(ad568xr PRIVATE -Wall -Wextra -Werror PUBLIC --coverage -O0)
target_link_options(ad568xr PUBLIC --coverage)

add_executable(ad568xr_tests test_ad568xr.cpp mocks/mocks.cpp)
target_compile_options(ad568xr_tests PRIVATE -Wall -Wextra)
target_link_libraries(ad568xr_tests PRIVATE ad568xr doctest::doctest)

enable_testing()
add_test(NAME ad568xr_tests COMMAND ad568xr_tests)
