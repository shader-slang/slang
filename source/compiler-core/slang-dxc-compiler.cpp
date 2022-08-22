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

#include "slang-artifact-associated-impl.h"
#include "slang-artifact-util.h"
#include "slang-artifact-diagnostic-util.h"
#include "slang-artifact-desc-util.h"

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

static UnownedStringSlice _addName(const UnownedStringSlice& inSlice, StringSlicePool& pool)
{
    UnownedStringSlice slice = inSlice;
    if (slice.getLength() == 0)
    {
        slice = UnownedStringSlice::fromLiteral("unnamed");
    }

    StringBuilder buf;
    const Index length = slice.getLength();
    buf << slice;

    for (Index i = 0; ; ++i)
    {
        buf.reduceLength(length);
    
        if (i > 0)
        {
            buf << "_" << i;
        }

        StringSlicePool::Handle handle;
        if (!pool.findOrAdd(buf.getUnownedSlice(), handle))
        {
            return pool.getSlice(handle);
        }
    }
}

static UnownedStringSlice _addName(IArtifact* artifact, StringSlicePool& pool)
{
    return _addName(ArtifactUtil::getBaseName(artifact).getUnownedSlice(), pool);
}

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

class DXCDownstreamCompiler : public DownstreamCompilerBase
{
public:
    typedef DownstreamCompilerBase Super;

    // IDownstreamCompiler
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL compile(const CompileOptions& options, IArtifact** outArtifact) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW bool SLANG_MCALL canConvert(const ArtifactDesc& from, const ArtifactDesc& to) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL convert(IArtifact* from, const ArtifactDesc& to, IArtifact** outArtifact) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW bool SLANG_MCALL isFileBased() SLANG_OVERRIDE { return false; }

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

static SlangResult _parseDiagnosticLine(CharSliceAllocator& allocator, const UnownedStringSlice& line, List<UnownedStringSlice>& lineSlices, IArtifactDiagnostics::Diagnostic& outDiagnostic)
{
    /* tests/diagnostics/syntax-error-intrinsic.slang:14:2: error: expected expression */
    if (lineSlices.getCount() < 5)
    {
        return SLANG_FAIL;
    }

    outDiagnostic.filePath = allocator.allocate(lineSlices[0]);

    SLANG_RETURN_ON_FAIL(StringUtil::parseInt(lineSlices[1], outDiagnostic.location.line));

    //Int lineCol;
    //SLANG_RETURN_ON_FAIL(StringUtil::parseInt(lineSlices[2], lineCol));

    UnownedStringSlice severitySlice = lineSlices[3].trim();

    outDiagnostic.severity = ArtifactDiagnostic::Severity::Error;
    if (severitySlice == UnownedStringSlice::fromLiteral("warning"))
    {
        outDiagnostic.severity = ArtifactDiagnostic::Severity::Warning;
    }

    // The rest of the line
    outDiagnostic.text = allocator.allocate(lineSlices[4].begin(), line.end());
    return SLANG_OK;
}

static SlangResult _handleOperationResult(IDxcOperationResult* dxcResult, IArtifactDiagnostics* diagnostics, ComPtr<IDxcBlob>& outBlob)
{
    // Retrieve result.
    HRESULT resultCode = S_OK;
    SLANG_RETURN_ON_FAIL(dxcResult->GetStatus(&resultCode));

    // Note: it seems like the dxcompiler interface
    // doesn't support querying diagnostic output
    // *unless* the compile failed (no way to get
    // warnings out!?).

    if (SLANG_SUCCEEDED(diagnostics->getResult()))
    {
        diagnostics->setResult(resultCode);
    }

    // Try getting the error/diagnostics blob
    ComPtr<IDxcBlobEncoding> dxcErrorBlob;
    dxcResult->GetErrorBuffer(dxcErrorBlob.writeRef());

    if (dxcErrorBlob)
    {
        const UnownedStringSlice diagnosticsSlice = _getSlice(dxcErrorBlob);
        if (diagnosticsSlice.getLength())
        {
            diagnostics->appendRaw(asCharSlice(diagnosticsSlice));

            CharSliceAllocator allocator;
            List<IArtifactDiagnostics::Diagnostic> parsedDiagnostics;
            SlangResult diagnosticParseRes = ArtifactDiagnosticUtil::parseColonDelimitedDiagnostics(allocator, diagnosticsSlice, 0, _parseDiagnosticLine, diagnostics);

            SLANG_UNUSED(diagnosticParseRes);
            SLANG_ASSERT(SLANG_SUCCEEDED(diagnosticParseRes));
        }
    }

    // If it failed, make sure we have an error in the diagnostics
    if (SLANG_FAILED(resultCode))
    {
        // In case the parsing failed, we still have an error -> so require there is one in the diagnostics
        diagnostics->requireErrorDiagnostic();
    }
    else
    {
        // Okay, the compile supposedly succeeded, so we
        // just need to grab the buffer with the output DXIL.
        SLANG_RETURN_ON_FAIL(dxcResult->GetResult(outBlob.writeRef()));
    }

    return SLANG_OK;
}

SlangResult DXCDownstreamCompiler::compile(const CompileOptions& options, IArtifact** outArtifact)
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

    // Find all of the libraries
    List<IArtifact*> libraries;
    for (IArtifact* library : options.libraries)
    {
        const auto desc = library->getDesc();

        if (desc.kind == ArtifactKind::Library && desc.payload == ArtifactPayload::DXIL)
        {
            // Make sure they all have blobs
            ComPtr<ISlangBlob> libraryBlob;
            SLANG_RETURN_ON_FAIL(library->loadBlob(ArtifactKeep::Yes, libraryBlob.writeRef()));

            libraries.add(library);
        }
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

    String profileName = options.profileName;
    // If we are going to link we have to compile in the lib profile style
    if (libraries.getCount())
    {
        if (!profileName.startsWith("lib"))
        {
            const Index index = profileName.indexOf('_');
            if (index < 0)
            {
                profileName = "lib_6_3";
            }
            else
            {
                StringBuilder buf;
                buf << "lib" << profileName.getUnownedSlice().tail(index);
                profileName = buf;
            }
        }
    }

    OSString wideEntryPointName = options.entryPointName.toWString();
    OSString wideProfileName = profileName.toWString();

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

    auto diagnostics = ArtifactDiagnostics::create();
    
    ComPtr<IDxcBlob> dxcResultBlob;

    SLANG_RETURN_ON_FAIL(_handleOperationResult(dxcResult, diagnostics, dxcResultBlob));

    // If we have libraries then we need to link...
    if (libraries.getCount())
    {
        ComPtr<IDxcLinker> linker;
        SLANG_RETURN_ON_FAIL(m_createInstance(CLSID_DxcLinker, __uuidof(linker), (void**)linker.writeRef()));

        StringSlicePool pool(StringSlicePool::Style::Default);

        List<ComPtr<ISlangBlob>> libraryBlobs;
        List<OSString> libraryNames;

        for (IArtifact* library : libraries)
        {
            ComPtr<ISlangBlob> blob;
            SLANG_RETURN_ON_FAIL(library->loadBlob(ArtifactKeep::Yes, blob.writeRef()));

            libraryBlobs.add(blob);
            libraryNames.add(String(_addName(library, pool)).toWString());
        }

        // Add the compiled blob name
        String name;
        if (options.modulePath.getLength())
        {
            name = Path::getFileNameWithoutExt(options.modulePath);
        }
        else if (options.sourceContentsPath.getLength())
        {
            name = Path::getFileNameWithoutExt(options.sourceContentsPath);
        }

        // Add the blob with name
        {
            auto blob = (ISlangBlob*)dxcResultBlob.get();
            libraryBlobs.add(ComPtr<ISlangBlob>(blob));
            libraryNames.add(String(_addName(name.getUnownedSlice(), pool)).toWString());
        }

        const Index librariesCount = libraryNames.getCount();
        SLANG_ASSERT(libraryBlobs.getCount() == librariesCount);

        List<const wchar_t*> linkLibraryNames;
        
        linkLibraryNames.setCount(librariesCount);
        
        for (Index i = 0; i < librariesCount; ++i)
        {
            linkLibraryNames[i] = libraryNames[i].begin();

            // Register the library
            SLANG_RETURN_ON_FAIL(linker->RegisterLibrary(linkLibraryNames[i], (IDxcBlob*)libraryBlobs[i].get()));
        }

        // Use the original profile name
        wideProfileName = options.profileName.toWString();

        ComPtr<IDxcOperationResult> linkDxcResult;
        SLANG_RETURN_ON_FAIL(linker->Link(wideEntryPointName.begin(), wideProfileName.begin(), linkLibraryNames.getBuffer(), UINT32(librariesCount), nullptr, 0, linkDxcResult.writeRef()));

        ComPtr<IDxcBlob> linkedBlob;
        SLANG_RETURN_ON_FAIL(_handleOperationResult(linkDxcResult, diagnostics, linkedBlob));

        dxcResultBlob = linkedBlob;
    }

    auto artifact = ArtifactUtil::createArtifactForCompileTarget(options.targetType);
    artifact->addAssociated(diagnostics);
    if (dxcResultBlob)
    {
        artifact->addRepresentationUnknown((ISlangBlob*)dxcResultBlob.get());
    }

    *outArtifact = artifact.detach();
    return SLANG_OK;
}

bool DXCDownstreamCompiler::canConvert(const ArtifactDesc& from, const ArtifactDesc& to)
{
    return ArtifactDescUtil::isDissassembly(from, to) && from.payload == ArtifactPayload::DXIL;
}

SlangResult DXCDownstreamCompiler::convert(IArtifact* from, const ArtifactDesc& to, IArtifact** outArtifact) 
{
    // Can only disassemble blobs that are DXIL
    if (!canConvert(from->getDesc(), to))
    {
        return SLANG_FAIL;
    }

    ComPtr<ISlangBlob> dxilBlob;
    SLANG_RETURN_ON_FAIL(from->loadBlob(ArtifactKeep::No, dxilBlob.writeRef()));

    ComPtr<IDxcCompiler> dxcCompiler;
    SLANG_RETURN_ON_FAIL(m_createInstance(CLSID_DxcCompiler, __uuidof(dxcCompiler), (LPVOID*)dxcCompiler.writeRef()));
    ComPtr<IDxcLibrary> dxcLibrary;
    SLANG_RETURN_ON_FAIL(m_createInstance(CLSID_DxcLibrary, __uuidof(dxcLibrary), (LPVOID*)dxcLibrary.writeRef()));

    // Create blob from the input data
    ComPtr<IDxcBlobEncoding> dxcSourceBlob;
    SLANG_RETURN_ON_FAIL(dxcLibrary->CreateBlobWithEncodingFromPinned((LPBYTE)dxilBlob->getBufferPointer(), (UINT32)dxilBlob->getBufferSize(), 0, dxcSourceBlob.writeRef()));

    ComPtr<IDxcBlobEncoding> dxcResultBlob;
    SLANG_RETURN_ON_FAIL(dxcCompiler->Disassemble(dxcSourceBlob, dxcResultBlob.writeRef()));

    auto artifact = ArtifactUtil::createArtifact(to);

    // Is compatible with ISlangBlob
    ISlangBlob* disassemblyBlob = (ISlangBlob*)dxcResultBlob.get();
    artifact->addRepresentationUnknown(disassemblyBlob);

    *outArtifact = artifact.detach();
    return SLANG_OK;
}

/* static */SlangResult DXCDownstreamCompilerUtil::locateCompilers(const String& path, ISlangSharedLibraryLoader* loader, DownstreamCompilerSet* set)
{
    ComPtr<ISlangSharedLibrary> library;

    const char* dependentNames[] = {"dxil", nullptr } ;
    SLANG_RETURN_ON_FAIL(DownstreamCompilerUtil::loadSharedLibrary(path, loader, dependentNames, "dxcompiler", library));
 
    SLANG_ASSERT(library);
    if (!library)
    {
        return SLANG_FAIL;
    }

    auto compiler = new DXCDownstreamCompiler;
    ComPtr<IDownstreamCompiler> compilerIntf(compiler);
    SLANG_RETURN_ON_FAIL(compiler->init(library));

    set->addCompiler(compilerIntf);
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
