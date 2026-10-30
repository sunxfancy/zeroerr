// Include immediately after the library, before any other consumer includes.
// libstdc++ <memory> includes <ostream>/<string> for C++20 unique_ptr insertion.
// The core's smart-pointer traits need <memory>; the standalone String must
// remain free of these headers even in C++20. Other heavy headers stay forbidden.
#if !(defined(ZEROERR_TEST_CORE_USES_MEMORY) && defined(__GLIBCXX__) && __cplusplus >= 202002L)
#if defined(_STRING_) || defined(_XSTRING_) || defined(_OSTREAM_) || defined(_IOSTREAM_) || \
    defined(_GLIBCXX_STRING) || defined(_GLIBCXX_OSTREAM) || defined(_BASIC_STRING_H) || \
    defined(_LIBCPP_STRING) || defined(_LIBCPP_OSTREAM)
#error "Default zeroerr consumer introduced a forbidden string or stream header"
#endif
#endif

#if defined(_SSTREAM_) || defined(_CHRONO_) || defined(_REGEX_) || defined(_MUTEX_) || defined(_COMPLEX_) || \
    defined(_GLIBCXX_SSTREAM) || defined(_GLIBCXX_CHRONO) || defined(_GLIBCXX_REGEX) || \
    defined(_GLIBCXX_MUTEX) || defined(_GLIBCXX_COMPLEX) || defined(_LIBCPP_SSTREAM) || \
    defined(_LIBCPP_CHRONO) || defined(_LIBCPP_REGEX) || defined(_LIBCPP_MUTEX) || defined(_LIBCPP_COMPLEX)
#error "Default zeroerr consumer introduced a forbidden stream, clock, regex, mutex or complex header"
#endif

#if defined(_LIBCPP_REMOVE_TRANSITIVE_INCLUDES) && !defined(ZEROERR_TEST_TRANSITIVE_INCLUDES_PRESET)
#error "zeroerr leaked the libc++ transitive include setting"
#endif
