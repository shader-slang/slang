#include "core/slang-array-view.h"
#include "core/slang-basic.h"
#include "core/slang-blob.h"
#include "core/slang-riff.h"

#ifdef SLANG_EMBED_CORE_MODULE

alignas(Slang::RIFF::Chunk::kChunkAlignment) static const uint8_t g_coreModule[] = {
#include "slang-core-module-generated.h"
};

static Slang::StaticBlob g_coreModuleBlob((const void*)g_coreModule, sizeof(g_coreModule));

SLANG_API ISlangBlob* slang_getEmbeddedCoreModule()
{
    return &g_coreModuleBlob;
}

#else

SLANG_API ISlangBlob* slang_getEmbeddedCoreModule()
{
    return nullptr;
}

#endif
