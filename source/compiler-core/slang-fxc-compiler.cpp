// slang-fxc-compiler.cpp
#include "slang-fxc-compiler.h"

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

// Enable calling through to `fxc` or `dxc` to
// generate code on Windows.
#ifdef _WIN32
#   define WIN32_LEAN_AND_MEAN
#   define NOMINMAX
#   include <Windows.h>
#   undef WIN32_LEAN_AND_MEAN
#   undef NOMINMAX
#   include <d3dcompiler.h>

#   ifndef SLANG_ENABLE_DXBC_SUPPORT
#       define SLANG_ENABLE_DXBC_SUPPORT 1
#   endif
#endif

#ifndef SLANG_ENABLE_DXBC_SUPPORT
#   define SLANG_ENABLE_DXBC_SUPPORT 0
#endif

#if SLANG_ENABLE_DXBC_SUPPORT
    // Some of the `D3DCOMPILE_*` constants aren't available in all
    // versions of `d3dcompiler.h`, so we define them here just in case
#   ifndef D3DCOMPILE_ENABLE_UNBOUNDED_DESCRIPTOR_TABLES
#       define D3DCOMPILE_ENABLE_UNBOUNDED_DESCRIPTOR_TABLES (1 << 20)
#   endif

#   ifndef D3DCOMPILE_ALL_RESOURCES_BOUND
#       define D3DCOMPILE_ALL_RESOURCES_BOUND (1 << 21)
#   endif
#endif

namespace Slang
{

#if SLANG_ENABLE_DXBC_SUPPORT

static UnownedStringSlice _getSlice(ID3DBlob* blob) { return StringUtil::getSlice((ISlangBlob*)blob);  }

struct FxcIncludeHandler : ID3DInclude
{

    STDMETHOD(Open)(D3D_INCLUDE_TYPE includeType, LPCSTR fileName, LPCVOID parentData, LPCVOID* outData, UINT* outSize) override
    {
        SLANG_UNUSED(includeType);
        // NOTE! The pParentData means the *text* of any previous include.
        // In order to work out what *path* that came from, we need to seach which source file it came from, and
        // use it's path

        // Assume the root pathInfo initially 
        PathInfo includedFromPathInfo = m_rootPathInfo;

        // Lets try and find the parent source if there is any
        if (parentData)
        {
            SourceFile* foundSourceFile = m_system.getSourceManager()->findSourceFileByContentRecursively((const char*)parentData);
            if (foundSourceFile)
            {
                includedFromPathInfo = foundSourceFile->getPathInfo();
            }
        }

        String path(fileName);
        PathInfo pathInfo;
        ComPtr<ISlangBlob> blob;

        SLANG_RETURN_ON_FAIL(m_system.findAndLoadFile(path, includedFromPathInfo.foundPath, pathInfo, blob));

        // Return the data
        *outData = blob->getBufferPointer();
        *outSize = (UINT)blob->getBufferSize();

        return S_OK;
    }

    STDMETHOD(Close)(LPCVOID pData) override
    {
        SLANG_UNUSED(pData);
        return S_OK;
    }
    FxcIncludeHandler(SearchDirectoryList* searchDirectories, ISlangFileSystemExt* fileSystemExt, SourceManager* sourceManager) :
        m_system(searchDirectories, fileSystemExt, sourceManager)
    {
    }

    PathInfo m_rootPathInfo;
    IncludeSystem m_system;
};

class FXCDownstreamCompiler : public DownstreamCompiler
{
public:
    typedef DownstreamCompiler Super;

    // DownstreamCompiler
    virtual SlangResult compile(const CompileOptions& options, RefPtr<DownstreamCompileResult>& outResult) SLANG_OVERRIDE;
    virtual SlangResult disassemble(SlangCompileTarget sourceBlobTarget, const void* blob, size_t blobSize, ISlangBlob** out) SLANG_OVERRIDE;
    virtual bool isFileBased() SLANG_OVERRIDE { return false; }

        /// Must be called before use
    SlangResult init(ISlangSharedLibrary* library);

    FXCDownstreamCompiler() {}
    
protected:

    pD3DCompile m_compile = nullptr;
    pD3DDisassemble m_disassemble = nullptr;

    ComPtr<ISlangSharedLibrary> m_sharedLibrary;  
};

SlangResult FXCDownstreamCompiler::init(ISlangSharedLibrary* library)
{
    m_compile = (pD3DCompile)library->findFuncByName("D3DCompile");
    m_disassemble = (pD3DDisassemble)library->findFuncByName("D3DDisassemble");

    if (!m_compile || !m_disassemble)
    {
        return SLANG_FAIL;
    }

    m_sharedLibrary = library;

    // It's not clear how to query for a version, but we can get a version number from the header
    m_desc = Desc(SLANG_PASS_THROUGH_FXC, D3D_COMPILER_VERSION);

    return SLANG_OK;
}

static SlangResult _parseDiagnosticLine(const UnownedStringSlice& line, List<UnownedStringSlice>& lineSlices, DownstreamDiagnostic& outDiagnostic)
{
    /* tests/diagnostics/syntax-error-intrinsic.slang(14,2): error X3000: syntax error: unexpected token '@' */
    if (lineSlices.getCount() < 3)
    {
        return SLANG_FAIL;
    }

    SLANG_RETURN_ON_FAIL(DownstreamDiagnostic::splitPathLocation(lineSlices[0], outDiagnostic));

    {
        const UnownedStringSlice severityAndCodeSlice = lineSlices[1].trim();
        const UnownedStringSlice severitySlice = StringUtil::getAtInSplit(severityAndCodeSlice, ' ', 0);

        outDiagnostic.code = StringUtil::getAtInSplit(severityAndCodeSlice, ' ', 1);

        outDiagnostic.severity = DownstreamDiagnostic::Severity::Error;
        if (severitySlice == "warning")
        {
            outDiagnostic.severity = DownstreamDiagnostic::Severity::Warning;
        }
    }

    outDiagnostic.text = UnownedStringSlice(lineSlices[2].begin(), line.end());
    return SLANG_OK;
}

SlangResult FXCDownstreamCompiler::compile(const CompileOptions& options, RefPtr<DownstreamCompileResult>& outResult)
{
    // This compiler doesn't read files, they should be read externally and stored in sourceContents/sourceContentsPath
    if (options.sourceFiles.getCount() > 0)
    {
        return SLANG_FAIL;
    }

    if (options.sourceLanguage != SLANG_SOURCE_LANGUAGE_HLSL || options.targetType != SLANG_DXBC)
    {
        SLANG_ASSERT(!"Can only compile HLSL to DXBC");
        return SLANG_FAIL;
    }

    // If we have been invoked in a pass-through mode, then we need to make sure
    // that the downstream compiler sees whatever options were passed to Slang
    // via the command line or API.
    //
    // TODO: more pieces of information should be added here as needed.
    //

    SearchDirectoryList searchDirectories;
    for (const auto& includePath : options.includePaths)
    {
        searchDirectories.searchDirectories.add(includePath);
    }

    // Use the default fileSystemExt is not set
    ID3DInclude* includeHandler = nullptr;

    FxcIncludeHandler fxcIncludeHandlerStorage(&searchDirectories, options.fileSystemExt, options.sourceManager);
    if (options.fileSystemExt)
    {
        if (options.sourceContentsPath.getLength() > 0)
        {
            fxcIncludeHandlerStorage.m_rootPathInfo = PathInfo::makePath(options.sourceContentsPath);
        }
        includeHandler = &fxcIncludeHandlerStorage;
    }

    List<D3D_SHADER_MACRO> dxMacrosStorage;
    D3D_SHADER_MACRO const* dxMacros = nullptr;

    if (options.defines.getCount() > 0)
    {
        for (const auto& define : options.defines)
        {
            D3D_SHADER_MACRO dxMacro;
            dxMacro.Name = define.nameWithSig.getBuffer();
            dxMacro.Definition = define.value.getBuffer();
            dxMacrosStorage.add(dxMacro);
        }
        D3D_SHADER_MACRO nullTerminator = { 0, 0 };
        dxMacrosStorage.add(nullTerminator);

        dxMacros = dxMacrosStorage.getBuffer();
    }

    DWORD flags = 0;

    switch (options.floatingPointMode)
    {
        default:
            break;

        case FloatingPointMode::Precise:
            flags |= D3DCOMPILE_IEEE_STRICTNESS;
            break;
    }

    flags |= D3DCOMPILE_ENABLE_STRICTNESS;
    flags |= D3DCOMPILE_ENABLE_UNBOUNDED_DESCRIPTOR_TABLES;

    switch (options.optimizationLevel)
    {
        default:
            break;

        case OptimizationLevel::None:       flags |= D3DCOMPILE_OPTIMIZATION_LEVEL0; break;
        case OptimizationLevel::Default:    flags |= D3DCOMPILE_OPTIMIZATION_LEVEL1; break;
        case OptimizationLevel::High:       flags |= D3DCOMPILE_OPTIMIZATION_LEVEL2; break;
        case OptimizationLevel::Maximal:    flags |= D3DCOMPILE_OPTIMIZATION_LEVEL3; break;
    }

    switch (options.debugInfoType)
    {
        case DebugInfoType::None:
            break;

        default:
            flags |= D3DCOMPILE_DEBUG;
            break;
    }

    ComPtr<ID3DBlob> codeBlob;
    ComPtr<ID3DBlob> diagnosticsBlob;
    HRESULT hr = m_compile(
        options.sourceContents.begin(),
        options.sourceContents.getLength(),
        options.sourceContentsPath.getBuffer(),
        dxMacros,
        includeHandler,
        options.entryPointName.getBuffer(),
        options.profileName.getBuffer(),
        flags,
        0, // unused: effect flags
        codeBlob.writeRef(),
        diagnosticsBlob.writeRef());

    DownstreamDiagnostics diagnostics;

    // HRESULT is compatible with SlangResult
    diagnostics.result = hr;

    if (diagnosticsBlob)
    {
        UnownedStringSlice diagnosticText = _getSlice(diagnosticsBlob);
        diagnostics.rawDiagnostics = diagnosticText;

        SlangResult diagnosticParseRes = DownstreamDiagnostic::parseColonDelimitedDiagnostics(diagnosticText, 0, _parseDiagnosticLine, diagnostics.diagnostics);
        SLANG_UNUSED(diagnosticParseRes);
        SLANG_ASSERT(SLANG_SUCCEEDED(diagnosticParseRes));
    }

    // ID3DBlob is compatible with ISlangBlob, so just cast away...
    ISlangBlob* slangCodeBlob = (ISlangBlob*)codeBlob.get();

    outResult = new BlobDownstreamCompileResult(diagnostics, slangCodeBlob);
    return SLANG_OK;
}

SlangResult FXCDownstreamCompiler::disassemble(SlangCompileTarget sourceBlobTarget, const void* blob, size_t blobSize, ISlangBlob** out)
{
    // Can only disassemble blobs that are DXBC
    if (sourceBlobTarget != SLANG_DXBC)
    {
        return SLANG_FAIL;
    }

    ComPtr<ID3DBlob> codeBlob;
    SLANG_RETURN_ON_FAIL(m_disassemble(blob, blobSize, 0, nullptr, codeBlob.writeRef()));

    // ISlangBlob is compatible with ID3DBlob
    *out = (ISlangBlob*)codeBlob.detach();
    return SLANG_OK;
}

/* static */SlangResult FXCDownstreamCompilerUtil::locateCompilers(const String& path, ISlangSharedLibraryLoader* loader, DownstreamCompilerSet* set)
{
    ComPtr<ISlangSharedLibrary> library;

    SLANG_RETURN_ON_FAIL(DownstreamCompilerUtil::loadSharedLibrary(path, loader, nullptr, "d3dcompiler_47", library));

    SLANG_ASSERT(library);
    if (!library)
    {
        return SLANG_FAIL;
    }

    RefPtr<FXCDownstreamCompiler> compiler(new FXCDownstreamCompiler);
    SLANG_RETURN_ON_FAIL(compiler->init(library));

    set->addCompiler(compiler);
    return SLANG_OK;
}

#else // SLANG_ENABLE_DXBC_SUPPORT

/* static */SlangResult FXCDownstreamCompilerUtil::locateCompilers(const String& path, ISlangSharedLibraryLoader* loader, DownstreamCompilerSet* set)
{
    SLANG_UNUSED(path);
    SLANG_UNUSED(loader);
    SLANG_UNUSED(set);
    return SLANG_E_NOT_AVAILABLE;
}

#endif // SLANG_ENABLE_DXBC_SUPPORT

}
