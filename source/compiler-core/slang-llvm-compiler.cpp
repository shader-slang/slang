// slang-llvm-compiler.cpp
#include "slang-llvm-compiler.h"

#include "../core/slang-common.h"

namespace Slang
{

/* static */SlangResult LLVMDownstreamCompilerUtil::locateCompilers(const String& path, ISlangSharedLibraryLoader* loader, DownstreamCompilerSet* set)
{
    ComPtr<ISlangSharedLibrary> library;

    SLANG_RETURN_ON_FAIL(DownstreamCompilerUtil::loadSharedLibrary(path, loader, nullptr, "slang-llvm", library));

    SLANG_ASSERT(library);
    if (!library)
    {
        return SLANG_FAIL;
    }

    typedef SlangResult(*CreateDownstreamCompilerFunc)(const Guid& intf, IDownstreamCompiler** outCompiler);

    auto fn = (CreateDownstreamCompilerFunc)library->findFuncByName("createLLVMDownstreamCompiler_V2");
    if (!fn)
    {
        return SLANG_FAIL;
    }

    ComPtr<IDownstreamCompiler> downstreamCompiler;

    SLANG_RETURN_ON_FAIL(fn(IDownstreamCompiler::getTypeGuid(), downstreamCompiler.writeRef()));

    set->addSharedLibrary(library);
    set->addCompiler(downstreamCompiler);
    return SLANG_OK;
}

}
