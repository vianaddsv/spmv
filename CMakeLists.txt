cmake_minimum_required(VERSION 3.14)
project(spmv LANGUAGES CXX)

enable_testing()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(OpenMP REQUIRED)

option(BUILD_TESTS "Build unit tests (requires GTest)" OFF)

# ---------------------------------------------------------
# 1. Biblioteca Sequencial (Pura)
# ---------------------------------------------------------
add_library(csr_seq csr/csr_matrix.cpp csr/matrix_market_reader.cpp)
target_include_directories(csr_seq PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/csr)

# ---------------------------------------------------------
# 2. Biblioteca Paralela (Com OpenMP)
# ---------------------------------------------------------
add_library(csr_omp csr/csr_matrix.cpp csr/matrix_market_reader.cpp)
target_include_directories(csr_omp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/csr)
target_link_libraries(csr_omp PUBLIC OpenMP::OpenMP_CXX)

add_executable(spmv main.cpp)
target_link_libraries(spmv PRIVATE csr_seq)

add_executable(spmv_parallel main_parallel.cpp)
target_link_libraries(spmv_parallel PRIVATE csr_omp)

if(BUILD_TESTS)
    find_package(GTest REQUIRED)

    add_executable(test_runner test/test_csr_matrix.cpp)
    target_compile_definitions(test_runner PRIVATE PROJECT_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
    target_link_libraries(test_runner PRIVATE csr_seq GTest::gtest GTest::gtest_main)

    include(GoogleTest)
    gtest_discover_tests(test_runner)
endif()
