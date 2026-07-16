#if defined(SLANG_MIMALLOC_OVERRIDE_NEW_DELETE)

// Override global C++ allocation in slang-compiler with mimalloc.

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4559)
#endif
#include "mimalloc-new-delete.h"
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

#endif
