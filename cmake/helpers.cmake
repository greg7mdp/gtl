#set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(GTL_IDE_FOLDER phmap)

# -------------------------------------------------------------
function(gtl_set_target_options my_target)
  target_compile_options(${my_target} PRIVATE
    $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:-pedantic -Wall -Wextra -Wcast-align -Wcast-qual -Wdisabled-optimization -Winit-self -Wmissing-include-dirs -Woverloaded-virtual -Wredundant-decls -Wshadow -Wswitch-default -Wunused -Wno-gnu-zero-variadic-macro-arguments>
    $<$<CXX_COMPILER_ID:GNU>:-pedantic -Wall -Wextra -Wcast-align -Wcast-qual -Wdisabled-optimization -Winit-self -Wmissing-include-dirs -Woverloaded-virtual -Wredundant-decls -Wshadow -Wswitch-default -Wunused -Wno-interference-size>
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /Zc:__cplusplus /bigobj>
  )
endfunction()

# -------------------------------------------------------------
function(gtl_cc_app my_target)
  cmake_parse_arguments(GTL_CC_APP
    ""
    ""
    "SRCS;LIBS"
    ${ARGN}
  )
  add_executable(${my_target} ${GTL_CC_APP_SRCS})
  target_link_libraries(${my_target} PRIVATE ${PROJECT_NAME} ${GTL_CC_APP_LIBS})
  gtl_set_target_options(${my_target})
endfunction()

# -------------------------------------------------------------
# gtl_cc_test(NAME awesome_test
#             SRCS "awesome_test.cpp"
#             DEPS phmap::awesome gmock gtest_main)
# -------------------------------------------------------------
function(gtl_cc_test)
  cmake_parse_arguments(GTL_CC_TEST
    ""
    "NAME"
    "SRCS;DEPS"
    ${ARGN}
  )

  set(_NAME "test_${GTL_CC_TEST_NAME}")
  add_executable(${_NAME} ${GTL_CC_TEST_SRCS})
  target_link_libraries(${_NAME} PRIVATE ${PROJECT_NAME} ${GTL_CC_TEST_DEPS})
  gtl_set_target_options(${_NAME})

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
