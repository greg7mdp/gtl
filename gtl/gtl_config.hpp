#if !defined(gtl_config_h_guard_)
#define gtl_config_h_guard_

// ---------------------------------------------------------------------------
// Copyright (c) 2019, Gregory Popovitch - greg7mdp@gmail.com
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Includes work from abseil-cpp (https://github.com/abseil/abseil-cpp)
// with modifications.
// 
// Copyright 2018 The Abseil Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// ---------------------------------------------------------------------------

#define GTL_VERSION_MAJOR 1
#define GTL_VERSION_MINOR 0
#define GTL_VERSION_PATCH 0

// Included for the __GLIBC__ macro (or similar macros on other systems).
#include <limits.h>

#ifdef __cplusplus
    // Included for __GLIBCXX__, _LIBCPP_VERSION
    #include <cstddef>
#endif  // __cplusplus

#if defined(__APPLE__)
    // Included for TARGET_OS_IPHONE, __IPHONE_OS_VERSION_MIN_REQUIRED,
    // __IPHONE_8_0.
    #include <Availability.h>
    #include <TargetConditionals.h>
#endif

#define GTL_XSTR(x) GTL_STR(x)
#define GTL_STR(x) #x
#define GTL_VAR_NAME_VALUE(var) #var "="  GTL_STR(var)

// -----------------------------------------------------------------------------
// Some sanity checks
// -----------------------------------------------------------------------------
#if defined(_MSC_FULL_VER) && _MSC_FULL_VER < 192930139 && !defined(__clang__)
    #error "gtl requires Visual Studio 2015 Update 2 or higher."
#endif

// We support gcc 10 and later.
#if defined(__GNUC__) && !defined(__clang__)
    #if __GNUC__ < 10
        #error "gtl requires gcc 10 or higher."
    #endif
#endif

// We support Apple Xcode clang 4.2.1 (version 421.11.65) and later.
// This corresponds to Apple Xcode version 4.5.
#if defined(__apple_build_version__) && __apple_build_version__ < 4211165
    #error "gtl requires __apple_build_version__ of 4211165 or higher."
#endif

// Enforce C++11 as the minimum. 
#if defined(__cplusplus) && !defined(_MSC_VER)
    #if __cplusplus < 201402L
        #error "C++ versions less than C++20 are not supported."
    #endif
#endif

// We have chosen glibc 2.12 as the minimum 
#if defined(__GLIBC__) && defined(__GLIBC_PREREQ)
    #if !__GLIBC_PREREQ(2, 12)
        #error "Minimum required version of glibc is 2.12."
    #endif
#endif

#if defined(_STLPORT_VERSION)
    #error "STLPort is not supported."
#endif

#if CHAR_BIT != 8
    #error "gtl assumes CHAR_BIT == 8."
#endif

// gtl currently assumes that an int is 4 bytes. 
#if INT_MAX < 2147483647
    #error "gtl assumes that int is at least 4 bytes. "
#endif



// -----------------------------------------------------------------------------
// Compiler Feature Checks
// -----------------------------------------------------------------------------

#ifdef __has_builtin
    #define GTL_HAVE_BUILTIN(x) __has_builtin(x)
#else
    #define GTL_HAVE_BUILTIN(x) 0
#endif

#define GTL_BRANCHLESS 1

#ifdef __has_feature
#define PHMAP_HAVE_FEATURE(f) __has_feature(f)
#else
#define PHMAP_HAVE_FEATURE(f) 0
#endif

// Portable check for GCC minimum version:
// https://gcc.gnu.org/onlinedocs/cpp/Common-Predefined-Macros.html
#if defined(__GNUC__) && defined(__GNUC_MINOR__)
    #define PHMAP_INTERNAL_HAVE_MIN_GNUC_VERSION(x, y) (__GNUC__ > (x) || __GNUC__ == (x) && __GNUC_MINOR__ >= (y))
#else
    #define PHMAP_INTERNAL_HAVE_MIN_GNUC_VERSION(x, y) 0
#endif

#if defined(__clang__) && defined(__clang_major__) && defined(__clang_minor__)
    #define PHMAP_INTERNAL_HAVE_MIN_CLANG_VERSION(x, y) (__clang_major__ > (x) || __clang_major__ == (x) && __clang_minor__ >= (y))
#else
    #define PHMAP_INTERNAL_HAVE_MIN_CLANG_VERSION(x, y) 0
#endif

// ------------------------------------------------------------
// Checks whether the __int128 compiler extension for a 128-bit 
// integral type is supported.
// ------------------------------------------------------------
#ifdef GTL_HAVE_INTRINSIC_INT128
    #error GTL_HAVE_INTRINSIC_INT128 cannot be directly set
#elif defined(__SIZEOF_INT128__)
    #if (defined(__clang__) && !defined(_WIN32) && !defined(__aarch64__)) || \
        (defined(__CUDACC__) && __CUDACC_VER_MAJOR__ >= 9) ||                \
        (defined(__GNUC__) && !defined(__clang__) && !defined(__CUDACC__))
        #define GTL_HAVE_INTRINSIC_INT128 1
    #elif defined(__CUDACC__)
        #if __CUDACC_VER__ >= 70000
            #define GTL_HAVE_INTRINSIC_INT128 1
        #endif  // __CUDACC_VER__ >= 70000
    #endif  // defined(__CUDACC__)
#endif

// -----------------------------------------------------------------------
// Checks the endianness of the platform.
// -----------------------------------------------------------------------
#if defined(GTL_IS_BIG_ENDIAN)
    #error "GTL_IS_BIG_ENDIAN cannot be directly set."
#endif

#if defined(GTL_IS_LITTLE_ENDIAN)
    #error "GTL_IS_LITTLE_ENDIAN cannot be directly set."
#endif

#if (defined(__BYTE_ORDER__) && defined(__ORDER_LITTLE_ENDIAN__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
    #define GTL_IS_LITTLE_ENDIAN 1
#elif defined(__BYTE_ORDER__) && defined(__ORDER_BIG_ENDIAN__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    #define GTL_IS_BIG_ENDIAN 1
#elif defined(_WIN32)
    #define GTL_IS_LITTLE_ENDIAN 1
#else
    #error "gtl endian detection needs to be set up for your compiler"
#endif

// ---------------------------------------------------------------------------
// Checks whether wchar_t is treated as a native type
// (MSVC: /Zc:wchar_t- treats wchar_t as unsigned short)
// ---------------------------------------------------------------------------
#if !defined(_MSC_VER) || defined(_NATIVE_WCHAR_T_DEFINED)
    #define GTL_HAS_NATIVE_WCHAR_T
#endif

// -----------------------------------------------------------------------------
// Sanitizer Attributes
// -----------------------------------------------------------------------------
//
// Sanitizer-related attributes are not "defined" in this file (and indeed
// are not defined as such in any file). To utilize the following
// sanitizer-related attributes within your builds, define the following macros
// within your build using a `-D` flag, along with the given value for
// `-fsanitize`:
//
//   * `ADDRESS_SANITIZER` + `-fsanitize=address` (Clang, GCC 4.8)
//   * `MEMORY_SANITIZER` + `-fsanitize=memory` (Clang-only)
//   * `THREAD_SANITIZER + `-fsanitize=thread` (Clang, GCC 4.8+)
//   * `UNDEFINED_BEHAVIOR_SANITIZER` + `-fsanitize=undefined` (Clang, GCC 4.9+)
//   * `CONTROL_FLOW_INTEGRITY` + -fsanitize=cfi (Clang-only)
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// A function-like feature checking macro that is a wrapper around
// `__has_attribute`, which is defined by GCC 5+ and Clang and evaluates to a
// nonzero constant integer if the attribute is supported or 0 if not.
//
// It evaluates to zero if `__has_attribute` is not defined by the compiler.
// -----------------------------------------------------------------------------
#ifdef __has_attribute
    #define GTL_HAVE_ATTRIBUTE(x) __has_attribute(x)
#else
    #define GTL_HAVE_ATTRIBUTE(x) 0
#endif

// -----------------------------------------------------------------------------
// A function-like feature checking macro that accepts C++11 style attributes.
// It's a wrapper around `__has_cpp_attribute`, defined by ISO C++ SD-6
// (https://en.cppreference.com/w/cpp/experimental/feature_test). If we don't
// find `__has_cpp_attribute`, will evaluate to 0.
// -----------------------------------------------------------------------------
#if defined(__cplusplus) && defined(__has_cpp_attribute)
    #define GTL_HAVE_CPP_ATTRIBUTE(x) __has_cpp_attribute(x)
#else
    #define GTL_HAVE_CPP_ATTRIBUTE(x) 0
#endif

// -----------------------------------------------------------------------------
// Function Attributes
// -----------------------------------------------------------------------------
#if GTL_HAVE_ATTRIBUTE(format) || (defined(__GNUC__) && !defined(__clang__))
    #define GTL_PRINTF_ATTRIBUTE(string_index, first_to_check) \
      __attribute__((__format__(__printf__, string_index, first_to_check)))
    #define GTL_SCANF_ATTRIBUTE(string_index, first_to_check) \
      __attribute__((__format__(__scanf__, string_index, first_to_check)))
#else
    #define GTL_PRINTF_ATTRIBUTE(string_index, first_to_check)
    #define GTL_SCANF_ATTRIBUTE(string_index, first_to_check)
#endif

#if GTL_HAVE_ATTRIBUTE(always_inline) || (defined(__GNUC__) && !defined(__clang__))
    #define GTL_ATTRIBUTE_ALWAYS_INLINE __attribute__((always_inline))
    #define GTL_HAVE_ATTRIBUTE_ALWAYS_INLINE 1
#else
    #define GTL_ATTRIBUTE_ALWAYS_INLINE
#endif

#if !defined(__INTEL_COMPILER) && (GTL_HAVE_ATTRIBUTE(noinline) || (defined(__GNUC__) && !defined(__clang__)))
    #define GTL_ATTRIBUTE_NOINLINE __attribute__((noinline))
    #define GTL_HAVE_ATTRIBUTE_NOINLINE 1
#else
    #define GTL_ATTRIBUTE_NOINLINE
#endif

#if GTL_HAVE_ATTRIBUTE(disable_tail_calls)
    #define GTL_HAVE_ATTRIBUTE_NO_TAIL_CALL 1
    #define GTL_ATTRIBUTE_NO_TAIL_CALL __attribute__((disable_tail_calls))
#elif defined(__GNUC__) && !defined(__clang__)
    #define GTL_HAVE_ATTRIBUTE_NO_TAIL_CALL 1
    #define GTL_ATTRIBUTE_NO_TAIL_CALL \
      __attribute__((optimize("no-optimize-sibling-calls")))
#else
    #define GTL_ATTRIBUTE_NO_TAIL_CALL
    #define GTL_HAVE_ATTRIBUTE_NO_TAIL_CALL 0
#endif

#if (GTL_HAVE_ATTRIBUTE(weak) || (defined(__GNUC__) && !defined(__clang__))) && \
    !(defined(__llvm__) && defined(_WIN32))
    #undef GTL_ATTRIBUTE_WEAK
    #define GTL_ATTRIBUTE_WEAK __attribute__((weak))
    #define GTL_HAVE_ATTRIBUTE_WEAK 1
#else
    #define GTL_ATTRIBUTE_WEAK
    #define GTL_HAVE_ATTRIBUTE_WEAK 0
#endif

#if GTL_HAVE_ATTRIBUTE(nonnull) || (defined(__GNUC__) && !defined(__clang__))
    #define GTL_ATTRIBUTE_NONNULL(arg_index) __attribute__((nonnull(arg_index)))
#else
    #define GTL_ATTRIBUTE_NONNULL(...)
#endif

#if GTL_HAVE_ATTRIBUTE(noreturn) || (defined(__GNUC__) && !defined(__clang__))
    #define GTL_ATTRIBUTE_NORETURN __attribute__((noreturn))
#elif defined(_MSC_VER)
    #define GTL_ATTRIBUTE_NORETURN __declspec(noreturn)
#else
    #define GTL_ATTRIBUTE_NORETURN
#endif

#if defined(__GNUC__) && defined(ADDRESS_SANITIZER)
    #define GTL_ATTRIBUTE_NO_SANITIZE_ADDRESS __attribute__((no_sanitize_address))
#else
    #define GTL_ATTRIBUTE_NO_SANITIZE_ADDRESS
#endif

#if defined(__GNUC__) && defined(MEMORY_SANITIZER)
    #define GTL_ATTRIBUTE_NO_SANITIZE_MEMORY __attribute__((no_sanitize_memory))
#else
    #define GTL_ATTRIBUTE_NO_SANITIZE_MEMORY
#endif

#if defined(__GNUC__) && defined(THREAD_SANITIZER)
    #define GTL_ATTRIBUTE_NO_SANITIZE_THREAD __attribute__((no_sanitize_thread))
#else
    #define GTL_ATTRIBUTE_NO_SANITIZE_THREAD
#endif

#if defined(__GNUC__) && (defined(UNDEFINED_BEHAVIOR_SANITIZER) || defined(ADDRESS_SANITIZER))
    #define GTL_ATTRIBUTE_NO_SANITIZE_UNDEFINED __attribute__((no_sanitize("undefined")))
#else
    #define GTL_ATTRIBUTE_NO_SANITIZE_UNDEFINED
#endif

#if defined(__GNUC__) && defined(CONTROL_FLOW_INTEGRITY)
    #define GTL_ATTRIBUTE_NO_SANITIZE_CFI __attribute__((no_sanitize("cfi")))
#else
    #define GTL_ATTRIBUTE_NO_SANITIZE_CFI
#endif

#if defined(__GNUC__) && defined(SAFESTACK_SANITIZER)
    #define GTL_ATTRIBUTE_NO_SANITIZE_SAFESTACK __attribute__((no_sanitize("safe-stack")))
#else
    #define GTL_ATTRIBUTE_NO_SANITIZE_SAFESTACK
#endif

#if GTL_HAVE_ATTRIBUTE(returns_nonnull) || (defined(__GNUC__)  && !defined(__clang__))
    #define GTL_ATTRIBUTE_RETURNS_NONNULL __attribute__((returns_nonnull))
#else
    #define GTL_ATTRIBUTE_RETURNS_NONNULL
#endif

#if GTL_HAVE_ATTRIBUTE(nodiscard)
    #define GTL_MUST_USE_RESULT [[nodiscard]]
#elif defined(__clang__) && GTL_HAVE_ATTRIBUTE(warn_unused_result)
    #define GTL_MUST_USE_RESULT __attribute__((warn_unused_result))
#else
    #define GTL_MUST_USE_RESULT
#endif

#if GTL_HAVE_ATTRIBUTE(hot) || (defined(__GNUC__) && !defined(__clang__))
    #define GTL_ATTRIBUTE_HOT __attribute__((hot))
#else
    #define GTL_ATTRIBUTE_HOT
#endif

#if GTL_HAVE_ATTRIBUTE(cold) || (defined(__GNUC__) && !defined(__clang__))
    #define GTL_ATTRIBUTE_COLD __attribute__((cold))
#else
    #define GTL_ATTRIBUTE_COLD
#endif

#if defined(__clang__)
    #if GTL_HAVE_CPP_ATTRIBUTE(clang::reinitializes)
        #define GTL_ATTRIBUTE_REINITIALIZES [[clang::reinitializes]]
    #else
        #define GTL_ATTRIBUTE_REINITIALIZES
    #endif
#else
    #define GTL_ATTRIBUTE_REINITIALIZES
#endif

#if GTL_HAVE_ATTRIBUTE(unused) || (defined(__GNUC__) && !defined(__clang__))
    #undef GTL_ATTRIBUTE_UNUSED
    #define GTL_ATTRIBUTE_UNUSED __attribute__((__unused__))
#else
    #define GTL_ATTRIBUTE_UNUSED
#endif

#if GTL_HAVE_ATTRIBUTE(tls_model) || (defined(__GNUC__) && !defined(__clang__))
    #define GTL_ATTRIBUTE_INITIAL_EXEC __attribute__((tls_model("initial-exec")))
#else
    #define GTL_ATTRIBUTE_INITIAL_EXEC
#endif

#if GTL_HAVE_ATTRIBUTE(packed) || (defined(__GNUC__) && !defined(__clang__))
    #define GTL_ATTRIBUTE_PACKED __attribute__((__packed__))
#else
    #define GTL_ATTRIBUTE_PACKED
#endif

#if GTL_HAVE_ATTRIBUTE(aligned) || (defined(__GNUC__) && !defined(__clang__))
    #define GTL_ATTRIBUTE_FUNC_ALIGN(bytes) __attribute__((aligned(bytes)))
#else
    #define GTL_ATTRIBUTE_FUNC_ALIGN(bytes)
#endif

// ----------------------------------------------------------------------
// Figure out SSE support
// ----------------------------------------------------------------------
#ifndef GTL_HAVE_SSE2
    #if defined(__SSE2__) ||  \
        (defined(_MSC_VER) && \
         (defined(_M_X64) || (defined(_M_IX86) && _M_IX86_FP >= 2)))
        #define GTL_HAVE_SSE2 1
    #else
        #define GTL_HAVE_SSE2 0
    #endif
#endif

#ifndef GTL_HAVE_SSSE3
    #if defined(__SSSE3__) || defined(__AVX2__)
        #define GTL_HAVE_SSSE3 1
    #else
        #define GTL_HAVE_SSSE3 0
    #endif
#endif

#if GTL_HAVE_SSSE3 && !GTL_HAVE_SSE2
    #error "Bad configuration!"
#endif

#if GTL_HAVE_SSE2
    #include <emmintrin.h>
#endif

#if GTL_HAVE_SSSE3
    #include <tmmintrin.h>
#endif


// ----------------------------------------------------------------------
// base/macros.h
// ----------------------------------------------------------------------

// GTL_ARRAYSIZE()
//
// Returns the number of elements in an array as a compile-time constant, which
// can be used in defining new arrays. If you use this macro on a pointer by
// mistake, you will get a compile-time error.
// ----------------------------------------------------------------------------
#define GTL_ARRAYSIZE(array) (sizeof(::gtl::macros_internal::ArraySizeHelper(array)))

namespace gtl {
    namespace macros_internal {
        // Note: this internal template function declaration is used by GTL_ARRAYSIZE.
        // The function doesn't need a definition, as we only use its type.
        // ---------------------------------------------------------------------------
        template <typename T, size_t N>
        auto ArraySizeHelper(const T (&array)[N]) -> char (&)[N];
    } 
}

#endif // gtl_config_h_guard_
