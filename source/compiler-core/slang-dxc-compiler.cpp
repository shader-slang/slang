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
#   include <Unknwn.h>
#   include "../../external/dxc/dxcapi.h"
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

static UnownedStringSlice _getSlice(IDxcBlob* blob) { return StringUtil::getSlice((ISlangBlob*)blob); }

// IDxcIncludeHandler
// 7f61fc7d-950d-467f-b3e3-3c02fb49187c
static const Guid IID_IDxcIncludeHandler = { 0x7f61fc7d, 0x950d, 0x467f, { 0x3c, 0x02, 0xfb, 0x49, 0x18, 0x7c } };

class DxcIncludeHandler : public IDxcIncludeHandler
{
public:
    // Implement IUnknown
    SLANG_NO_THROW HRESULT SLANG_MCALL QueryInterface(const IID& uuid, void** out)
    {
        ISlangUnknown* intf = getInterface(reinterpret_cast<const Guid&>(uuid));
        if (intf)
        {
            *out = intf;
            return SLANG_OK;
        }
        return SLANG_E_NO_INTERFACE;
    }
    SLANG_NO_THROW ULONG SLANG_MCALL AddRef() SLANG_OVERRIDE { return 1; }
    SLANG_NO_THROW ULONG SLANG_MCALL Release() SLANG_OVERRIDE { return 1; }

    // Implement IDxcIncludeHandler
    virtual HRESULT SLANG_MCALL LoadSource(LPCWSTR inFilename, IDxcBlob** outSource) SLANG_OVERRIDE
    {
        // Hmm DXC does something a bit odd - when it sees a path, it just passes that in with ./ in front!!
        // NOTE! It doesn't make any difference if it is "" or <> quoted.

        // So we just do a work around where we strip if we see a path starting with ./
        String filePath = String::fromWString(inFilename);

        // If it starts with ./ then attempt to strip it
        if (filePath.startsWith("./"))
        {
            String remaining(filePath.subString(2, filePath.getLength() - 2));

            // Okay if we strip ./ and what we have is absolute, then it's the absolute path that we care about,
            // otherwise we just leave as is.
            if (Path::isAbsolute(remaining))
            {
                filePath = remaining;
            }
        }

        ComPtr<ISlangBlob> blob;
        PathInfo pathInfo;
        SlangResult res = m_system.findAndLoadFile(filePath, String(), pathInfo, blob);

        // NOTE! This only works because ISlangBlob is *binary compatible* with IDxcBlob, if either
        // change things could go boom
        *outSource = (IDxcBlob*)blob.detach();
        return res;
    }

    DxcIncludeHandler(SearchDirectoryList* searchDirectories, ISlangFileSystemExt* fileSystemExt, SourceManager* sourceManager = nullptr) :
        m_system(searchDirectories, fileSystemExt, sourceManager)
    {
    }

protected:

    // Used by QueryInterface for casting
    ISlangUnknown* getInterface(const Guid& guid)
    {
        if (guid == ISlangUnknown::getTypeGuid() || guid == IID_IDxcIncludeHandler)
        {
            return (ISlangUnknown*)(static_cast<IDxcIncludeHandler*>(this));
        }
        return nullptr;
    }
    
    IncludeSystem m_system;
};

class DXCDownstreamCompiler : public DownstreamCompiler
{
public:
    typedef DownstreamCompiler Super;

    // DownstreamCompiler
    virtual SlangResult compile(const CompileOptions& options, RefPtr<DownstreamCompileResult>& outResult) SLANG_OVERRIDE;
    virtual SlangResult disassemble(SlangCompileTarget sourceBlobTarget, const void* blob, size_t blobSize, ISlangBlob** out) SLANG_OVERRIDE;
    virtual bool isFileBased() SLANG_OVERRIDE { return false; }

    /// Must be called before use
    SlangResult init(ISlangSharedLibrary* library);

    DXCDownstreamCompiler() {}

protected:

    DxcCreateInstanceProc m_createInstance = nullptr;
    ComPtr<ISlangSharedLibrary> m_sharedLibrary;
};

SlangResult DXCDownstreamCompiler::init(ISlangSharedLibrary* library)
{
    m_sharedLibrary = library;

    m_createInstance = (DxcCreateInstanceProc)library->findFuncByName("DxcCreateInstance");
    if (!m_createInstance)
    {
        return SLANG_FAIL;
    }

    m_desc = Desc(SLANG_PASS_THROUGH_DXC); 

    return SLANG_OK;
}

static SlangResult _parseDiagnosticLine(const UnownedStringSlice& line, List<UnownedStringSlice>& lineSlices, DownstreamDiagnostic& outDiagnostic)
{
    /* tests/diagnostics/syntax-error-intrinsic.slang:14:2: error: expected expression */
    if (lineSlices.getCount() < 5)
    {
        return SLANG_FAIL;
    }

    outDiagnostic.filePath = lineSlices[0];

    SLANG_RETURN_ON_FAIL(StringUtil::parseInt(lineSlices[1], outDiagnostic.fileLine));

    //Int lineCol;
    //SLANG_RETURN_ON_FAIL(StringUtil::parseInt(lineSlices[2], lineCol));

    UnownedStringSlice severitySlice = lineSlices[3].trim();

    outDiagnostic.severity = DownstreamDiagnostic::Severity::Error;
    if (severitySlice == UnownedStringSlice::fromLiteral("warning"))
    {
        outDiagnostic.severity = DownstreamDiagnostic::Severity::Warning;
    }

    // The rest of the line
    outDiagnostic.text = UnownedStringSlice(lineSlices[4].begin(), line.end());
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

    ComPtr<IDxcCompiler> dxcCompiler;
    SLANG_RETURN_ON_FAIL(m_createInstance(CLSID_DxcCompiler, __uuidof(dxcCompiler), (LPVOID*)dxcCompiler.writeRef()));
    ComPtr<IDxcLibrary> dxcLibrary;
    SLANG_RETURN_ON_FAIL(m_createInstance(CLSID_DxcLibrary, __uuidof(dxcLibrary), (LPVOID*)dxcLibrary.writeRef()));

    const auto& hlslSource = options.sourceContents;

    // Create blob from the string
    ComPtr<IDxcBlobEncoding> dxcSourceBlob;
    SLANG_RETURN_ON_FAIL(dxcLibrary->CreateBlobWithEncodingFromPinned(
        (LPBYTE)hlslSource.getBuffer(),
        (UINT32)hlslSource.getLength(),
        0,
        dxcSourceBlob.writeRef()));

    List<const WCHAR*> args;

    // Add all compiler specific options
    List<OSString> compilerSpecific;
    compilerSpecific.setCount(options.compilerSpecificArguments.getCount());

    for (Index i = 0; i < options.compilerSpecificArguments.getCount(); ++i)
    {
        compilerSpecific[i] = options.compilerSpecificArguments[i].toWString();
        args.add(compilerSpecific[i]);
    }

    // TODO: deal with
    bool treatWarningsAsErrors = false;
    if (treatWarningsAsErrors)
    {
        args.add(L"-WX");
    }

    switch (options.matrixLayout)
    {
        default:
            break;

        case SLANG_MATRIX_LAYOUT_ROW_MAJOR:
            args.add(L"-Zpr");
            break;
    }

    switch (options.floatingPointMode)
    {
        default:
            break;

        case FloatingPointMode::Precise:
            args.add(L"-Gis"); // "force IEEE strictness"
            break;
    }

    switch (options.optimizationLevel)
    {
        default:
            break;

        case OptimizationLevel::None:       args.add(L"-Od"); break;
        case OptimizationLevel::Default:    args.add(L"-O1"); break;
        case OptimizationLevel::High:       args.add(L"-O2"); break;
        case OptimizationLevel::Maximal:    args.add(L"-O3"); break;
    }

    switch (options.debugInfoType)
    {
        case DebugInfoType::None:
            break;

        default:
            args.add(L"-Zi");
            break;
    }

    // Slang strives to produce correct code, and by default
    // we do not show the user warnings produced by a downstream
    // compiler. When the downstream compiler *does* produce an
    // error, then we dump its entire diagnostic log, which can
    // include many distracting spurious warnings that have nothing
    // to do with the user's code, and just relate to the idiomatic
    // way that Slang outputs HLSL.
    //
    // It would be nice to use fine-grained flags to disable specific
    // warnings here, so that we keep ourselves honest (e.g., only
    // use `-Wno-parentheses` to eliminate that class of false positives),
    // but alas dxc doesn't support these options even though they
    // work on mainline Clang. Thus the only option we have available
    // is the big hammer of turning off *all* warnings coming from dxc.
    //
    args.add(L"-no-warnings");

    OSString wideEntryPointName = options.entryPointName.toWString();
    OSString wideProfileName = options.profileName.toWString();

    if (options.flags & CompileOptions::Flag::EnableFloat16)
    {
        args.add(L"-enable-16bit-types");
    }

    SearchDirectoryList searchDirectories;
    for (const auto& includePath : options.includePaths)
    {
        searchDirectories.searchDirectories.add(includePath);
    }

    OSString sourcePath = options.sourceContentsPath.toWString();

    DxcIncludeHandler includeHandler(&searchDirectories, options.fileSystemExt, options.sourceManager);

    ComPtr<IDxcOperationResult> dxcResult;
    SLANG_RETURN_ON_FAIL(dxcCompiler->Compile(dxcSourceBlob,
        sourcePath.begin(),
        wideEntryPointName.begin(),
        wideProfileName.begin(),
        args.getBuffer(),
        UINT32(args.getCount()),
        nullptr,            // `#define`s
        0,                  // `#define` count
        &includeHandler,    // `#include` handler
        dxcResult.writeRef()));

    // Retrieve result.
    HRESULT resultCode = S_OK;
    SLANG_RETURN_ON_FAIL(dxcResult->GetStatus(&resultCode));

    // Note: it seems like the dxcompiler interface
    // doesn't support querying diagnostic output
    // *unless* the compile failed (no way to get
    // warnings out!?).

    DownstreamDiagnostics diagnostics;
    diagnostics.result = resultCode;

    // Try getting the error/diagnostics blob
    ComPtr<IDxcBlobEncoding> dxcErrorBlob;
    dxcResult->GetErrorBuffer(dxcErrorBlob.writeRef());

    if (dxcErrorBlob)
    {
        const UnownedStringSlice diagnosticsSlice = _getSlice(dxcErrorBlob);
        if (diagnosticsSlice.getLength())
        {
            diagnostics.rawDiagnostics = String(diagnosticsSlice);

            SlangResult diagnosticParseRes = DownstreamDiagnostic::parseColonDelimitedDiagnostics(diagnosticsSlice, 0, _parseDiagnosticLine, diagnostics.diagnostics);

            SLANG_UNUSED(diagnosticParseRes);
            SLANG_ASSERT(SLANG_SUCCEEDED(diagnosticParseRes));
        }
    }

    ComPtr<IDxcBlob> dxcResultBlob;

    // If it failed, make sure we have an error in the diagnostics
    if (SLANG_FAILED(resultCode))
    {
        // In case the parsing failed, we still have an error -> so require there is one in the diagnostics
        diagnostics.requireErrorDiagnostic();
    }
    else
    {
        // Okay, the compile supposedly succeeded, so we
        // just need to grab the buffer with the output DXIL.
        SLANG_RETURN_ON_FAIL(dxcResult->GetResult(dxcResultBlob.writeRef()));
   }

    outResult = new BlobDownstreamCompileResult(diagnostics, (ISlangBlob*)dxcResultBlob.get());
    return SLANG_OK;
}

SlangResult DXCDownstreamCompiler::disassemble(SlangCompileTarget sourceBlobTarget, const void* blob, size_t blobSize, ISlangBlob** out)
{
    // Can only disassemble blobs that are DXIL
    if (sourceBlobTarget != SLANG_DXIL)
    {
        return SLANG_FAIL;
    }

    ComPtr<IDxcCompiler> dxcCompiler;
    SLANG_RETURN_ON_FAIL(m_createInstance(CLSID_DxcCompiler, __uuidof(dxcCompiler), (LPVOID*)dxcCompiler.writeRef()));
    ComPtr<IDxcLibrary> dxcLibrary;
    SLANG_RETURN_ON_FAIL(m_createInstance(CLSID_DxcLibrary, __uuidof(dxcLibrary), (LPVOID*)dxcLibrary.writeRef()));

    // Create blob from the input data
    ComPtr<IDxcBlobEncoding> dxcSourceBlob;
    SLANG_RETURN_ON_FAIL(dxcLibrary->CreateBlobWithEncodingFromPinned((LPBYTE)blob, (UINT32)blobSize, 0, dxcSourceBlob.writeRef()));

    ComPtr<IDxcBlobEncoding> dxcResultBlob;
    SLANG_RETURN_ON_FAIL(dxcCompiler->Disassemble(dxcSourceBlob, dxcResultBlob.writeRef()));

    // Is compatible with ISlangBlob
    *out = (ISlangBlob*)dxcResultBlob.detach();
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
