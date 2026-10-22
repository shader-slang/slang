#if defined(SLANG_ENABLE_MIMALLOC)

// Override global C++ allocation in slang-compiler with mimalloc.

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4559)
#endif
#include "mimalloc-new-delete.h"
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

// mimalloc-new-delete.h omits the aligned overloads unless this condition holds, and over-aligned
// allocations would then silently bypass mimalloc. On MSVC the condition depends on the
// /Zc:__cplusplus option that source/slang/CMakeLists.txt sets for this file.
#if defined(_MSC_VER) && !(__cplusplus > 201402L && defined(__cpp_aligned_new))
#error "mimalloc-new-delete.h did not define the aligned operator new/delete overloads"
#endif

#endif
