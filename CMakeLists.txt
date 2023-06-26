#[===================================================================[
    gtl library by Gregory Popovitch

    CMake projects that wish to use this library may do
    something like :

    include(FetchContent)
    FetchContent_Declare(
        gtl
        GIT_REPOSITORY https://github.com/greg7mdp/gtl.git
        GIT_TAG        v1.1.5 # adjust tag/branch/commit as needed
    )
    FetchContent_MakeAvailable(gtl)

    ...
    set(CMAKE_CXX_STANDARD 20)
    target_link_libraries (my_target PRIVATE gtl)

#]===================================================================]

cmake_minimum_required(VERSION 3.8)

list (APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(DetectVersion)

cmake_policy(SET CMP0048 NEW)              ## set VERSION as documented by the project() command.
cmake_policy(SET CMP0076 NEW)              ## accept new policy

if(NOT CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 20)             ## compile with C++20 support
endif()

if(NOT CMAKE_CXX_STANDARD_REQUIRED)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()

if(NOT DEFINED GTL_MASTER_PROJECT)
    set(GTL_MASTER_PROJECT OFF)
    if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
        set(GTL_MASTER_PROJECT ON)
    endif()
endif()

project(gtl VERSION ${DETECTED_GTL_VERSION} LANGUAGES CXX)

## ----------------------------- options -----------------------------
option(GTL_INSTALL "Enable installation" ${GTL_MASTER_PROJECT})


set(GTL_DIR gtl)
# See CMP0076
set(GTL_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include/${GTL_DIR}/phmap.hpp 
                ${CMAKE_CURRENT_SOURCE_DIR}/include/${GTL_DIR}/bits.hpp 
                ${CMAKE_CURRENT_SOURCE_DIR}/include/${GTL_DIR}/btree.hpp 
                ${CMAKE_CURRENT_SOURCE_DIR}/include/${GTL_DIR}/gtl_base.hpp 
                ${CMAKE_CURRENT_SOURCE_DIR}/include/${GTL_DIR}/gtl_config.hpp
                ${CMAKE_CURRENT_SOURCE_DIR}/include/${GTL_DIR}/intrusive.hpp 
                ${CMAKE_CURRENT_SOURCE_DIR}/include/${GTL_DIR}/lru_cache.hpp 
                ${CMAKE_CURRENT_SOURCE_DIR}/include/${GTL_DIR}/meminfo.hpp 
                ${CMAKE_CURRENT_SOURCE_DIR}/include/${GTL_DIR}/memoize.hpp 
                ${CMAKE_CURRENT_SOURCE_DIR}/include/${GTL_DIR}/bit_vector.hpp 
                ${CMAKE_CURRENT_SOURCE_DIR}/include/${GTL_DIR}/phmap_dump.hpp 
                ${CMAKE_CURRENT_SOURCE_DIR}/include/${GTL_DIR}/phmap_fwd_decl.hpp 
                ${CMAKE_CURRENT_SOURCE_DIR}/include/${GTL_DIR}/phmap_utils.hpp 
                ${CMAKE_CURRENT_SOURCE_DIR}/include/${GTL_DIR}/soa.hpp 
                ${CMAKE_CURRENT_SOURCE_DIR}/include/${GTL_DIR}/stopwatch.hpp 
                ${CMAKE_CURRENT_SOURCE_DIR}/include/${GTL_DIR}/utils.hpp
                ${CMAKE_CURRENT_SOURCE_DIR}/include/${GTL_DIR}/adv_utils.hpp)

include(helpers)

add_library(${PROJECT_NAME} INTERFACE)

target_sources(${PROJECT_NAME} INTERFACE ${GTL_HEADERS})

target_include_directories(
     ${PROJECT_NAME} INTERFACE
     $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
     $<INSTALL_INTERFACE:include>)

if(GTL_INSTALL)
    include(GNUInstallDirs)
    include(CMakePackageConfigHelpers)

    install(
        DIRECTORY ${PROJECT_SOURCE_DIR}/include/${GTL_DIR}/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${GTL_DIR})

    install(TARGETS ${PROJECT_NAME}
            EXPORT ${PROJECT_NAME}-targets)

    export(EXPORT ${PROJECT_NAME}-targets
           FILE "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")
endif()


## ------------------------- building tests and examples -------------
option(GTL_BUILD_TESTS      "Whether or not to build the tests"      ${GTL_MASTER_PROJECT})
option(GTL_BUILD_EXAMPLES   "Whether or not to build the examples"   ${GTL_MASTER_PROJECT})
option(GTL_BUILD_BENCHMARKS "Whether or not to build the benchmarks" ${GTL_MASTER_PROJECT})

if (GTL_BUILD_TESTS OR GTL_BUILD_EXAMPLES)
    include_directories(${PROJECT_SOURCE_DIR})
endif()

if (GTL_BUILD_TESTS)

    if (NOT GTL_GTEST_LIBS)
        include(cmake/DownloadGTest.cmake)

        gtl_check_target(gtest)
        gtl_check_target(gtest_main)
        gtl_check_target(gmock)
        set(GTL_GTEST_LIBS gmock_main)
    endif()

    enable_testing()

    ## ---------------- base hash components ----------------------------
    gtl_cc_test(NAME container_memory SRCS "tests/phmap/container_memory_test.cpp" DEPS ${GTL_GTEST_LIBS})
    gtl_cc_test(NAME hash_policy_testing SRCS "tests/phmap/hash_policy_testing_test.cpp" DEPS ${GTL_GTEST_LIBS})
    gtl_cc_test(NAME node_hash_policy SRCS "tests/phmap/node_hash_policy_test.cpp" DEPS ${GTL_GTEST_LIBS})
    gtl_cc_test(NAME raw_hash_set SRCS "tests/phmap/raw_hash_set_test.cpp" DEPS ${GTL_GTEST_LIBS})
    gtl_cc_test(NAME raw_hash_set_allocator SRCS "tests/phmap/raw_hash_set_allocator_test.cpp" DEPS ${GTL_GTEST_LIBS})

    ## ---------------- regular hash maps ----------------------------
    gtl_cc_test(NAME flat_hash_set SRCS "tests/phmap/flat_hash_set_test.cpp"  DEPS ${GTL_GTEST_LIBS})
    gtl_cc_test(NAME flat_hash_map SRCS "tests/phmap/flat_hash_map_test.cpp" DEPS ${GTL_GTEST_LIBS})
    gtl_cc_test(NAME node_hash_map SRCS "tests/phmap/node_hash_map_test.cpp" DEPS ${GTL_GTEST_LIBS})
    gtl_cc_test(NAME node_hash_set SRCS "tests/phmap/node_hash_set_test.cpp" DEPS ${GTL_GTEST_LIBS})

    ## --------------- parallel hash maps -----------------------------------------------
    gtl_cc_test(NAME parallel_flat_hash_map SRCS "tests/phmap/parallel_flat_hash_map_test.cpp" DEPS ${GTL_GTEST_LIBS})
    gtl_cc_test(NAME parallel_flat_hash_set SRCS "tests/phmap/parallel_flat_hash_set_test.cpp" DEPS ${GTL_GTEST_LIBS})
    gtl_cc_test(NAME parallel_node_hash_map SRCS "tests/phmap/parallel_node_hash_map_test.cpp" DEPS ${GTL_GTEST_LIBS})
    gtl_cc_test(NAME parallel_node_hash_set SRCS "tests/phmap/parallel_node_hash_set_test.cpp" DEPS ${GTL_GTEST_LIBS})
    gtl_cc_test(NAME parallel_flat_hash_map_mutex SRCS "tests/phmap/parallel_flat_hash_map_mutex_test.cpp" DEPS ${GTL_GTEST_LIBS})
    gtl_cc_test(NAME dump_load SRCS "tests/phmap/dump_load_test.cpp" DEPS ${GTL_GTEST_LIBS})
    gtl_cc_test(NAME erase_if SRCS "tests/phmap/erase_if_test.cpp" DEPS ${GTL_GTEST_LIBS})

    ## --------------- btree -----------------------------------------------
    gtl_cc_test(NAME btree SRCS "tests/btree/btree_test.cpp" DEPS ${GTL_GTEST_LIBS})

    ## --------------- misc -----------------------------------------------
    gtl_cc_test(NAME lru_cache SRCS "tests/misc/lru_cache_test.cpp" DEPS ${GTL_GTEST_LIBS})
    gtl_cc_test(NAME bit_vector SRCS "tests/misc/bitvector_test.cpp" DEPS ${GTL_GTEST_LIBS})
    gtl_cc_test(NAME vector SRCS "tests/misc/vector_test.cpp" DEPS ${GTL_GTEST_LIBS})
endif()

if (GTL_BUILD_EXAMPLES)
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)

    gtl_cc_app(ex_btree SRCS examples/btree/btree.cpp include/gtl/debug_vis/gtl.natvis)

    gtl_cc_app(ex_insert_bench SRCS examples/phmap/insert_bench.cpp include/gtl/debug_vis/gtl.natvis)
    gtl_cc_app(ex_mt_word_counter SRCS examples/phmap/mt_word_counter.cpp include/gtl/debug_vis/gtl.natvis)

if(MSVC)
    gtl_cc_app(ex_lazy_emplace_l SRCS examples/phmap/lazy_emplace_l.cpp include/gtl/debug_vis/gtl.natvis)
endif()

    gtl_cc_app(ex_allmaps SRCS examples/hmap/allmaps.cpp include/gtl/debug_vis/gtl.natvis)
    gtl_cc_app(ex_basic SRCS examples/hmap/basic.cpp include/gtl/debug_vis/gtl.natvis)
    gtl_cc_app(ex_bench SRCS examples/hmap/bench.cpp include/gtl/debug_vis/gtl.natvis LIBS Threads::Threads)
    gtl_cc_app(ex_emplace SRCS examples/hmap/emplace.cpp include/gtl/debug_vis/gtl.natvis)

    gtl_cc_app(ex_serialize SRCS examples/hmap/serialize.cpp include/gtl/debug_vis/gtl.natvis)
    #target_include_directories(ex_serialize PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../cereal/include>)

    gtl_cc_app(ex_hash SRCS examples/hmap/hash.cpp include/gtl/debug_vis/gtl.natvis)
    gtl_cc_app(ex_hash_std SRCS examples/hmap/hash_std.cpp include/gtl/debug_vis/gtl.natvis)
    gtl_cc_app(ex_hash_value SRCS examples/hmap/hash_value.cpp include/gtl/debug_vis/gtl.natvis)
    gtl_cc_app(ex_two_files SRCS examples/hmap/f1.cpp examples/hmap/f2.cpp include/gtl/debug_vis/gtl.natvis)
    gtl_cc_app(ex_knucleotide SRCS examples/hmap/knucleotide.cpp include/gtl/debug_vis/gtl.natvis LIBS Threads::Threads)
    gtl_cc_app(ex_dump_load SRCS examples/hmap/dump_load.cpp include/gtl/debug_vis/gtl.natvis)
    gtl_cc_app(ex_dump_nested SRCS examples/hmap/dump_nested.cpp include/gtl/debug_vis/gtl.natvis)
    gtl_cc_app(ex_matt SRCS examples/hmap/matt.cpp include/gtl/debug_vis/gtl.natvis)

    gtl_cc_app(ex_soa SRCS examples/misc/soa.cpp include/gtl/debug_vis/gtl.natvis)
    gtl_cc_app(ex_vec_utils SRCS examples/misc/vec_utils.cpp include/gtl/debug_vis/gtl.natvis)
    gtl_cc_app(ex_bit_vector SRCS examples/misc/bit_vector.cpp include/gtl/debug_vis/gtl.natvis)
    gtl_cc_app(ex_intrusive SRCS examples/misc/intrusive.cpp include/gtl/debug_vis/gtl.natvis)
    gtl_cc_app(ex_utils SRCS "examples/misc/utils.cpp" include/gtl/debug_vis/gtl.natvis)
    #gtl_cc_app(ex_adv_utils SRCS "examples/misc/adv_utils.cpp" include/gtl/debug_vis/gtl.natvis)
    
    ## cache/memoize
    gtl_cc_app(ex_memoize_fib SRCS examples/memoize/memoize_fib.cpp include/gtl/debug_vis/gtl.natvis)
    gtl_cc_app(ex_memoize_primes SRCS examples/memoize/memoize_primes.cpp include/gtl/debug_vis/gtl.natvis)
    gtl_cc_app(ex_mt_memoize SRCS examples/memoize/mt_memoize.cpp include/gtl/debug_vis/gtl.natvis LIBS Threads::Threads)
    gtl_cc_app(ex_mt_memoize_lru SRCS examples/memoize/mt_memoize_lru.cpp include/gtl/debug_vis/gtl.natvis LIBS Threads::Threads)
endif()

if (GTL_BUILD_BENCHMARKS)
    gtl_cc_app(bench_bit_vector SRCS benchmarks/bitvector_bench.cpp include/gtl/debug_vis/gtl.natvis)
    gtl_cc_app(bench_hash SRCS benchmarks/hash_bench.cpp include/gtl/debug_vis/gtl.natvis)
endif()
