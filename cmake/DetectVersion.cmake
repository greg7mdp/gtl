
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/include/gtl/gtl_config.hpp" _GTL_H_CONTENTS)
string(REGEX REPLACE ".*#define GTL_VERSION_MAJOR ([0-9]+).*" "\\1" DETECTED_GTL_VERSION_MAJOR "${_GTL_H_CONTENTS}")
string(REGEX REPLACE ".*#define GTL_VERSION_MINOR ([0-9]+).*" "\\1" DETECTED_GTL_VERSION_MINOR "${_GTL_H_CONTENTS}")
string(REGEX REPLACE ".*#define GTL_VERSION_PATCH ([0-9]+).*" "\\1" DETECTED_GTL_VERSION_PATCH "${_GTL_H_CONTENTS}")
set(DETECTED_GTL_VERSION "${DETECTED_GTL_VERSION_MAJOR}.${DETECTED_GTL_VERSION_MINOR}.${DETECTED_GTL_VERSION_PATCH}")

message(STATUS "Detected GTL Version - ${DETECTED_GTL_VERSION}")
