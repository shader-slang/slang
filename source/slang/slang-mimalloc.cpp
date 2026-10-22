#if defined(SLANG_ENABLE_MIMALLOC)

// Override global C++ allocation in slang-compiler with mimalloc.
//
// This file includes only mimalloc headers. It is compiled with /Zc:__cplusplus on MSVC, so a Slang
// header such as slang-common.h would expand differently here than in the rest of slang-compiler.

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4559)
#endif
#include "mimalloc-new-delete.h"
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

// We repeat the condition that mimalloc-new-delete.h places around its aligned overloads, so that a
// build in which over-aligned allocations would silently bypass mimalloc fails instead. A change to
// that condition in a later mimalloc has to be mirrored here.
#if !(__cplusplus > 201402L && defined(__cpp_aligned_new))
#error "aligned new/delete need __cplusplus > 201402L and __cpp_aligned_new; MSVC: /Zc:__cplusplus"
#endif

#endif
