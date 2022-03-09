cmake_minimum_required(VERSION 3.8)

list (APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(DetectVersion)

cmake_policy(SET CMP0048 NEW)                    ## set VERSION as documented by the project() command.
set(CMAKE_CXX_STANDARD 20)                       ## compile with C++20 support
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(gtl VERSION ${DETECTED_GTL_VERSION} LANGUAGES CXX)

set(GTL_DIR gtl)
set(GTL_HEADERS ${GTL_DIR}/phmap.hpp 
                ${GTL_DIR}/gtl_base.hpp 
                ${GTL_DIR}/bits.hpp 
                ${GTL_DIR}/phmap_utils.hpp 
                ${GTL_DIR}/gtl_config.hpp)

set(CMAKE_SUPPRESS_REGENERATION true)            ## suppress ZERO_CHECK project

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
include(helpers)
include_directories("${CMAKE_CURRENT_SOURCE_DIR}")

add_library(${PROJECT_NAME} INTERFACE)

target_sources(${PROJECT_NAME} INTERFACE ${GTL_HEADERS})

target_include_directories(
     ${PROJECT_NAME} INTERFACE
     $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
     $<INSTALL_INTERFACE:include>)

install(
    DIRECTORY ${PROJECT_SOURCE_DIR}/${GTL_DIR}/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${GTL_DIR})

install(TARGETS ${PROJECT_NAME}
        EXPORT ${PROJECT_NAME}-targets)

export(EXPORT ${PROJECT_NAME}-targets
       FILE "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")

## ------------------------- building tests and examples -------------
option(GTL_BUILD_TESTS      "Whether or not to build the tests"    OFF)
option(GTL_BUILD_EXAMPLES   "Whether or not to build the examples" OFF)
option(GTL_BUILD_BENCHMARKS "Whether or not to build the benchmarks" OFF)

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj")
endif()

if (GTL_BUILD_TESTS)

    include(cmake/DownloadGTest.cmake)
    include_directories(${PROJECT_SOURCE_DIR})

    check_target(gtest)
    check_target(gtest_main)
    check_target(gmock)

    enable_testing()

    ## ---------------- base hash components ----------------------------
    gtl_cc_test(NAME compressed_tuple SRCS "tests/phmap/compressed_tuple_test.cpp" DEPS gmock_main)
    gtl_cc_test(NAME container_memory SRCS "tests/phmap/container_memory_test.cpp" DEPS gmock_main)
    gtl_cc_test(NAME hash_policy_testing SRCS "tests/phmap/hash_policy_testing_test.cpp" DEPS gmock_main)
    gtl_cc_test(NAME node_hash_policy SRCS "tests/phmap/node_hash_policy_test.cpp" DEPS gmock_main)
    gtl_cc_test(NAME raw_hash_set SRCS "tests/phmap/raw_hash_set_test.cpp" DEPS gmock_main)
    gtl_cc_test(NAME raw_hash_set_allocator SRCS "tests/phmap/raw_hash_set_allocator_test.cpp" DEPS gmock_main)

    ## ---------------- regular hash maps ----------------------------
    gtl_cc_test(NAME flat_hash_set SRCS "tests/phmap/flat_hash_set_test.cpp"  DEPS gmock_main)
    gtl_cc_test(NAME flat_hash_map SRCS "tests/phmap/flat_hash_map_test.cpp" DEPS gmock_main)
    gtl_cc_test(NAME node_hash_map SRCS "tests/phmap/node_hash_map_test.cpp" DEPS gmock_main)
    gtl_cc_test(NAME node_hash_set SRCS "tests/phmap/node_hash_set_test.cpp" DEPS  gmock_main)

    ## --------------- parallel hash maps -----------------------------------------------
    gtl_cc_test(NAME parallel_flat_hash_map SRCS "tests/phmap/parallel_flat_hash_map_test.cpp" DEPS gmock_main)
    gtl_cc_test(NAME parallel_flat_hash_set SRCS "tests/phmap/parallel_flat_hash_set_test.cpp" DEPS gmock_main)
    gtl_cc_test(NAME parallel_node_hash_map SRCS "tests/phmap/parallel_node_hash_map_test.cpp" DEPS gmock_main)
    gtl_cc_test(NAME parallel_node_hash_set SRCS "tests/phmap/parallel_node_hash_set_test.cpp" DEPS  gmock_main)
    gtl_cc_test(NAME parallel_flat_hash_map_mutex SRCS "tests/phmap/parallel_flat_hash_map_mutex_test.cpp" DEPS gmock_main)
    gtl_cc_test(NAME dump_load SRCS "tests/phmap/dump_load_test.cpp" DEPS gmock_main)

    ## --------------- btree -----------------------------------------------
    gtl_cc_test(NAME btree SRCS "tests/btree/btree_test.cpp" CLOPTS "-w" DEPS gmock_main)

    ## --------------- misc -----------------------------------------------
    gtl_cc_test(NAME lru_cache SRCS "tests/misc/lru_cache.cpp" CLOPTS "-w" DEPS gmock_main)
    gtl_cc_test(NAME bit_vector SRCS "tests/misc/bitvector_test.cpp" CLOPTS "-w" DEPS gmock_main)


endif()

if (GTL_BUILD_EXAMPLES)
    if(NOT MSVC)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -Wall -Wextra -Wcast-align -Wcast-qual -Wdisabled-optimization -Winit-self -Wmissing-include-dirs -Woverloaded-virtual -Wredundant-decls -Wshadow -Wswitch-default -Wno-unused")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /Zc:__cplusplus")
    endif()

    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)

    add_executable(ex_btree examples/btree/btree.cpp gtl/debug_vis/gtl.natvis)

    add_executable(ex_insert_bench examples/phmap/insert_bench.cpp gtl/debug_vis/gtl.natvis)

if(MSVC)
    add_executable(ex_lazy_emplace_l examples/phmap/lazy_emplace_l.cpp gtl/debug_vis/gtl.natvis)
endif()

    add_executable(ex_allmaps examples/hmap/allmaps.cpp gtl/debug_vis/gtl.natvis)
    add_executable(ex_basic examples/hmap/basic.cpp gtl/debug_vis/gtl.natvis)
    add_executable(ex_bench examples/hmap/bench.cpp gtl/debug_vis/gtl.natvis)
    add_executable(ex_emplace examples/hmap/emplace.cpp gtl/debug_vis/gtl.natvis)

    add_executable(ex_serialize examples/hmap/serialize.cpp gtl/debug_vis/gtl.natvis)
    target_include_directories(ex_serialize PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../cereal/include>)

    add_executable(ex_hash_std examples/hmap/hash_std.cpp gtl/debug_vis/gtl.natvis)
    add_executable(ex_hash_value examples/hmap/hash_value.cpp gtl/debug_vis/gtl.natvis)
    add_executable(ex_two_files examples/hmap/f1.cpp examples/hmap/f2.cpp gtl/debug_vis/gtl.natvis)
    add_executable(ex_knucleotide examples/hmap/knucleotide.cpp gtl/debug_vis/gtl.natvis)
    add_executable(ex_dump_load examples/hmap/dump_load.cpp gtl/debug_vis/gtl.natvis)
    add_executable(ex_matt examples/hmap/matt.cpp gtl/debug_vis/gtl.natvis)

    add_executable(ex_soa examples/misc/soa.cpp gtl/debug_vis/gtl.natvis)
    add_executable(ex_vec_utils examples/misc/vec_utils.cpp gtl/debug_vis/gtl.natvis)
    add_executable(ex_bit_vector examples/misc/bit_vector.cpp gtl/debug_vis/gtl.natvis)

    target_link_libraries(ex_knucleotide Threads::Threads)
    target_link_libraries(ex_bench Threads::Threads)
endif()

if (GTL_BUILD_BENCHMARKS)
    add_executable(bench_bit_vector benchmarks/bitvector_bench.cpp gtl/debug_vis/gtl.natvis)
endif()
