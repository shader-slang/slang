#include "../core/slang-array-view.h"
#include "../core/slang-basic.h"
#include "../core/slang-blob.h"

#ifdef SLANG_EMBED_CORE_MODULE

static const uint8_t g_coreModule[] = {
#include "slang-core-module-generated.h"
};

static Slang::StaticBlob g_coreModuleBlob((const void*)g_coreModule, sizeof(g_coreModule));

static const uint8_t g_autodiffModule[] = {
#include "slang-autodiff-module-generated.h"
};

static Slang::StaticBlob g_autodiffModuleBlob(
    (const void*)g_autodiffModule,
    sizeof(g_autodiffModule));

SLANG_API ISlangBlob* slang_getEmbeddedCoreModule()
{
    return &g_coreModuleBlob;
}

SLANG_API ISlangBlob* slang_getEmbeddedAutodiffModule()
{
    return &g_autodiffModuleBlob;
}

#else

SLANG_API ISlangBlob* slang_getEmbeddedCoreModule()
{
    return nullptr;
}

SLANG_API ISlangBlob* slang_getEmbeddedAutodiffModule()
{
    return nullptr;
}

#endif
