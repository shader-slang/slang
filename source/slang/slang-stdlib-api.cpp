// slang-stdlib-api.cpp

#include "../core/slang-basic.h"
#include "../core/slang-array-view.h"

#include "../core/slang-blob.h"

#ifdef SLANG_WITHOUT_EMBEDDED_STD_LIB

SLANG_API ISlangBlob* slang_getEmbeddedStdLib()
{
    return nullptr;
}

#else

static const uint8_t g_stdLib[] =
{
#   include "slang-stdlib-generated.h"
};

static Slang::StaticBlob g_stdLibBlob((const void*)g_stdLib, sizeof(g_stdLib));

SLANG_API ISlangBlob* slang_getEmbeddedStdLib()
{
    return &g_stdLibBlob;
}

#endif
