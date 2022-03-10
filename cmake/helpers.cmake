#set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(GTL_IDE_FOLDER phmap)

# -------------------------------------------------------------
# gtl_cc_test(NAME awesome_test
#             SRCS "awesome_test.cpp"
#             DEPS phmap::awesome gmock gtest_main)
# -------------------------------------------------------------
function(gtl_cc_test)
  cmake_parse_arguments(GTL_CC_TEST
    ""
    "NAME"
    "SRCS;COPTS;CWOPTS;CLOPTS;DEFINES;LINKOPTS;DEPS"
    ${ARGN}
  )

  set(_NAME "test_${GTL_CC_TEST_NAME}")
  add_executable(${_NAME} "")
  target_sources(${_NAME} PRIVATE ${GTL_CC_TEST_SRCS})
  target_include_directories(${_NAME}
    PUBLIC ${GTL_COMMON_INCLUDE_DIRS}
    PRIVATE ${GMOCK_INCLUDE_DIRS} ${GTEST_INCLUDE_DIRS}
  )
  target_compile_definitions(${_NAME}
    PUBLIC ${GTL_CC_TEST_DEFINES}
  )
if(MSVC)
  target_compile_options(${_NAME}
    PRIVATE ${GTL_CC_TEST_CWOPTS} /W4 /Zc:__cplusplus
  )
else()
  target_compile_options(${_NAME}
    PRIVATE ${GTL_CC_TEST_CLOPTS}
  )
endif()
  target_compile_options(${_NAME}
    PRIVATE ${GTL_CC_TEST_COPTS}
  )
  target_link_libraries(${_NAME}
    PUBLIC ${GTL_CC_TEST_DEPS}
    PRIVATE ${GTL_CC_TEST_LINKOPTS}
  )
  # Add all Abseil targets to a a folder in the IDE for organization.
  set_property(TARGET ${_NAME} PROPERTY FOLDER ${GTL_IDE_FOLDER}/test)

  add_test(NAME ${_NAME} COMMAND ${_NAME})
endfunction()

# -------------------------------------------------------------
function(gtl_check_target my_target)
  if(NOT TARGET ${my_target})
    message(FATAL_ERROR " GTL: compiling phmap tests requires a ${my_target} CMake target in your project,
                   see CMake/README.md for more details")
  endif(NOT TARGET ${my_target})
endfunction()


