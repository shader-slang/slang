// slang-llvm-compiler.cpp
#include "slang-llvm-compiler.h"

#include "../core/slang-common.h"
#include "../../slang-com-helper.h"

#include "../core/slang-blob.h"

#include "../core/slang-string-util.h"
#include "../core/slang-string-slice-pool.h"

#include "../core/slang-io.h"
#include "../core/slang-shared-library.h"
#include "../core/slang-semantic-version.h"
#include "../core/slang-char-util.h"

#include "slang-include-system.h"
#include "slang-source-loc.h"

#include "../core/slang-shared-library.h"

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

    typedef SlangResult(*CreateDownstreamCompilerFunc)(RefPtr<DownstreamCompiler>& out);

    auto fn = (CreateDownstreamCompilerFunc)library->findFuncByName("createLLVMDownstreamCompiler");
    if (!fn)
    {
        return SLANG_FAIL;
    }

    RefPtr<DownstreamCompiler> downstreamCompiler;

    SLANG_RETURN_ON_FAIL(fn(downstreamCompiler));

    set->addSharedLibrary(library);
    set->addCompiler(downstreamCompiler);
    return SLANG_OK;
}

}
