// slang-dxc-compiler.cpp
#include "slang-dxc-compiler.h"

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

// Enable calling through to  `dxc` to
// generate code on Windows.
#ifdef _WIN32
#   define WIN32_LEAN_AND_MEAN
#   define NOMINMAX
#   include <Windows.h>
#   undef WIN32_LEAN_AND_MEAN
#   undef NOMINMAX

#   ifndef SLANG_ENABLE_DXIL_SUPPORT
#       define SLANG_ENABLE_DXIL_SUPPORT 1
#   endif
#endif

#ifndef SLANG_ENABLE_DXIL_SUPPORT
#   define SLANG_ENABLE_DXIL_SUPPORT 0
#endif

namespace Slang
{

#if SLANG_ENABLE_DXIL_SUPPORT

class DXCDownstreamCompiler : public DownstreamCompiler
{
public:
    typedef DownstreamCompiler Super;

    // DownstreamCompiler
    virtual SlangResult compile(const CompileOptions& options, RefPtr<DownstreamCompileResult>& outResult) SLANG_OVERRIDE;
    virtual ISlangSharedLibrary* getSharedLibrary() SLANG_OVERRIDE { return m_sharedLibrary; }
    virtual SlangResult dissassemble(SlangCompileTarget sourceBlobTarget, const void* blob, size_t blobSize, ISlangBlob** out) SLANG_OVERRIDE;
    virtual bool isFileBased() SLANG_OVERRIDE { return false; }

    /// Must be called before use
    SlangResult init(ISlangSharedLibrary* library);

    DXCDownstreamCompiler() {}

protected:

    ComPtr<ISlangSharedLibrary> m_sharedLibrary;
};

SlangResult DXCDownstreamCompiler::init(ISlangSharedLibrary* library)
{
    m_sharedLibrary = library;

    m_desc = Desc(SLANG_PASS_THROUGH_DXC); 

    return SLANG_OK;
}


SlangResult DXCDownstreamCompiler::compile(const CompileOptions& options, RefPtr<DownstreamCompileResult>& outResult)
{
    // This compiler doesn't read files, they should be read externally and stored in sourceContents/sourceContentsPath
    if (options.sourceFiles.getCount() > 0)
    {
        return SLANG_FAIL;
    }

    if (options.sourceLanguage != SLANG_SOURCE_LANGUAGE_HLSL || options.targetType != SLANG_DXIL)
    {
        SLANG_ASSERT(!"Can only compile HLSL to DXIL");
        return SLANG_FAIL;
    }

    //outResult = new BlobDownstreamCompileResult(diagnostics, slangCodeBlob);
    return SLANG_OK;
}

SlangResult DXCDownstreamCompiler::dissassemble(SlangCompileTarget sourceBlobTarget, const void* blob, size_t blobSize, ISlangBlob** out)
{
    // Can only disassemble blobs that are DXBC
    if (sourceBlobTarget != SLANG_DXIL)
    {
        return SLANG_FAIL;
    }

    //ComPtr<ID3DBlob> codeBlob;
    //SLANG_RETURN_ON_FAIL(m_disassemble(blob, blobSize, 0, nullptr, codeBlob.writeRef()));

    // ISlangBlob is compatible with ID3DBlob
    //*out = (ISlangBlob*)codeBlob.detach();
    return SLANG_OK;
}

/* static */SlangResult DXCDownstreamCompilerUtil::locateCompilers(const String& path, ISlangSharedLibraryLoader* loader, DownstreamCompilerSet* set)
{
    ComPtr<ISlangSharedLibrary> library;

    // If the user supplies a path to their preferred version of DXC
    // we just use this.
    if (path.getLength() != 0)
    {
        // We *assume* path is the path to d3dcompiler.
        ComPtr<ISlangSharedLibrary> dxil;

        // Attempt to load dxil from same path that d3dcompiler is located
        const String parentPath = Path::getParentDirectory(path);
        if (parentPath.getLength())
        {
            String dxilPath = Path::combine(parentPath, "dxil");
            // Try to load dxil along this path first
            // If it fails - then DXC may load from a different place and thats ok.
            loader->loadSharedLibrary(dxilPath.getBuffer(), dxil.writeRef());
        }

        SLANG_RETURN_ON_FAIL(loader->loadSharedLibrary(path.getBuffer(), library.writeRef()));
    }
    else
    {
        SLANG_RETURN_ON_FAIL(loader->loadSharedLibrary("dxcompiler", library.writeRef()));
    }
     
    SLANG_ASSERT(library);
    if (!library)
    {
        return SLANG_FAIL;
    }

    RefPtr<DXCDownstreamCompiler> compiler(new DXCDownstreamCompiler);
    SLANG_RETURN_ON_FAIL(compiler->init(library));

    set->addCompiler(compiler);
    return SLANG_OK;
}

#else // SLANG_ENABLE_DXIL_SUPPORT

/* static */SlangResult DXCDownstreamCompilerUtil::locateCompilers(const String& path, ISlangSharedLibraryLoader* loader, DownstreamCompilerSet* set)
{
    SLANG_UNUSED(path);
    SLANG_UNUSED(loader);
    SLANG_UNUSED(set);
    return SLANG_E_NOT_AVAILABLE;
}

#endif // SLANG_ENABLE_DXIL_SUPPORT

}
