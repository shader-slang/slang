// slang-glslang-compiler.cpp
#include "slang-glslang-compiler.h"

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

// Enable calling through to `glslang` on
// all platforms.
#ifndef SLANG_ENABLE_GLSLANG_SUPPORT
#   define SLANG_ENABLE_GLSLANG_SUPPORT 1
#endif

#if SLANG_ENABLE_GLSLANG_SUPPORT
#   include "../slang-glslang/slang-glslang.h"
#endif

namespace Slang
{

#if SLANG_ENABLE_GLSLANG_SUPPORT

class GlslangDownstreamCompiler : public DownstreamCompiler
{
public:
    typedef DownstreamCompiler Super;

    // DownstreamCompiler
    virtual SlangResult compile(const CompileOptions& options, RefPtr<DownstreamCompileResult>& outResult) SLANG_OVERRIDE;
    virtual SlangResult disassemble(SlangCompileTarget sourceBlobTarget, const void* blob, size_t blobSize, ISlangBlob** out) SLANG_OVERRIDE;
    virtual bool isFileBased() SLANG_OVERRIDE { return false; }

        /// Must be called before use
    SlangResult init(ISlangSharedLibrary* library);

    GlslangDownstreamCompiler() {}
    
protected:

    SlangResult _invoke(glslang_CompileRequest_1_1& request);

    glslang_CompileFunc_1_0 m_compile_1_0 = nullptr; 
    glslang_CompileFunc_1_1 m_compile_1_1 = nullptr; 
    
    ComPtr<ISlangSharedLibrary> m_sharedLibrary;  
};

SlangResult GlslangDownstreamCompiler::init(ISlangSharedLibrary* library)
{
    m_compile_1_0 = (glslang_CompileFunc_1_0)library->findFuncByName("glslang_compile");
    m_compile_1_1 = (glslang_CompileFunc_1_1)library->findFuncByName("glslang_compile_1_1");

    if (m_compile_1_0 == nullptr && m_compile_1_1 == nullptr)
    {
        return SLANG_FAIL;
    }

    m_sharedLibrary = library;

    // It's not clear how to query for a version, but we can get a version number from the header
    m_desc = Desc(SLANG_PASS_THROUGH_GLSLANG);

    return SLANG_OK;
}

SlangResult GlslangDownstreamCompiler::_invoke(glslang_CompileRequest_1_1& request)
{
    int err = 1;
    if (m_compile_1_1)
    {
        err = m_compile_1_1(&request);
    }
    else if (m_compile_1_0)
    {
        glslang_CompileRequest_1_0 request_1_0;
        request_1_0.set(request);
        err = m_compile_1_0(&request_1_0);
    }

    return err ? SLANG_FAIL : SLANG_OK;
}

static SlangResult _parseDiagnosticLine(const UnownedStringSlice& line, List<UnownedStringSlice>& lineSlices, DownstreamDiagnostic& outDiagnostic)
{
    /* ERROR: tests/diagnostics/syntax-error-intrinsic.slang:13: '@' : unexpected token */

    if (lineSlices.getCount() < 4)
    {
        return SLANG_FAIL;
    }
    {
        const UnownedStringSlice severitySlice = lineSlices[0].trim();

        outDiagnostic.severity = DownstreamDiagnostic::Severity::Error;
        if (severitySlice.caseInsensitiveEquals(UnownedStringSlice::fromLiteral("warning")))
        {
            outDiagnostic.severity = DownstreamDiagnostic::Severity::Warning;
        }
    }

    outDiagnostic.filePath = lineSlices[1];

    SLANG_RETURN_ON_FAIL(StringUtil::parseInt(lineSlices[2], outDiagnostic.fileLine));
    outDiagnostic.text = UnownedStringSlice(lineSlices[3].begin(), line.end());
    return SLANG_OK;
}



SlangResult GlslangDownstreamCompiler::compile(const CompileOptions& options, RefPtr<DownstreamCompileResult>& outResult)
{
    // This compiler doesn't read files, they should be read externally and stored in sourceContents/sourceContentsPath
    if (options.sourceFiles.getCount() > 0)
    {
        return SLANG_FAIL;
    }

    if (options.sourceLanguage != SLANG_SOURCE_LANGUAGE_GLSL || options.targetType != SLANG_SPIRV)
    {
        SLANG_ASSERT(!"Can only compile GLSL to SPIR-V");
        return SLANG_FAIL;
    }

    StringBuilder diagnosticOutput;
    auto diagnosticOutputFunc = [](void const* data, size_t size, void* userData)
    {
        (*(StringBuilder*)userData).append((char const*)data, (char const*)data + size);
    };
    List<uint8_t> spirv;
    auto outputFunc = [](void const* data, size_t size, void* userData)
    {
        ((List<uint8_t>*)userData)->addRange((uint8_t*)data, size);
    };

    
    glslang_CompileRequest_1_1 request;
    memset(&request, 0, sizeof(request));
    request.sizeInBytes = sizeof(request);

    request.action = GLSLANG_ACTION_COMPILE_GLSL_TO_SPIRV;
    request.sourcePath = options.sourceContentsPath.getBuffer();

    request.slangStage = options.stage;

    request.inputBegin = options.sourceContents.begin();
    request.inputEnd = options.sourceContents.end();

    // Find the SPIR-V version if set
    SemanticVersion spirvVersion;
    for (const auto& capabilityVersion : options.requiredCapabilityVersions)
    {
        if (capabilityVersion.kind == DownstreamCompiler::CapabilityVersion::Kind::SPIRV)
        {
            if (capabilityVersion.version > spirvVersion)
            {
                spirvVersion = capabilityVersion.version;
            }
        }
    }

    request.spirvVersion.major = spirvVersion.m_major;
    request.spirvVersion.minor = spirvVersion.m_minor;
    request.spirvVersion.patch = spirvVersion.m_patch;

    request.outputFunc = outputFunc;
    request.outputUserData = &spirv;

    request.diagnosticFunc = diagnosticOutputFunc;
    request.diagnosticUserData = &diagnosticOutput;

    request.optimizationLevel = (unsigned)options.optimizationLevel;
    request.debugInfoType = (unsigned)options.debugInfoType;

    const SlangResult invokeResult = _invoke(request);

    DownstreamDiagnostics diagnostics;

    // Set the diagnostics result
    diagnostics.result = invokeResult;

    if (SLANG_FAILED(invokeResult))
    {
        diagnostics.rawDiagnostics = diagnosticOutput;

        SlangResult diagnosticParseRes = DownstreamDiagnostic::parseColonDelimitedDiagnostics(diagnosticOutput.getUnownedSlice(), 1, _parseDiagnosticLine, diagnostics.diagnostics);
        SLANG_UNUSED(diagnosticParseRes);

        diagnostics.requireErrorDiagnostic();

        outResult = new BlobDownstreamCompileResult(diagnostics, nullptr);
        return SLANG_OK;
    }

    RefPtr<ListBlob> spirvBlob = ListBlob::moveCreate(spirv);
    outResult = new BlobDownstreamCompileResult(diagnostics, spirvBlob);

    return SLANG_OK;
}

SlangResult GlslangDownstreamCompiler::disassemble(SlangCompileTarget sourceBlobTarget, const void* blob, size_t blobSize, ISlangBlob** out)
{
    // Can only disassemble blobs that are DXBC
    if (sourceBlobTarget != SLANG_SPIRV)
    {
        return SLANG_FAIL;
    }

    StringBuilder builder;
    
    auto outputFunc = [](void const* data, size_t size, void* userData)
    {
        (*(StringBuilder*)userData).append((char const*)data, (char const*)data + size);
    };

    glslang_CompileRequest_1_1 request;
    memset(&request, 0, sizeof(request));
    request.sizeInBytes = sizeof(request);

    request.action = GLSLANG_ACTION_DISSASSEMBLE_SPIRV;

    request.sourcePath = nullptr;

    request.inputBegin = blob;
    request.inputEnd = (char*)blob + blobSize;

    request.outputFunc = outputFunc;
    request.outputUserData = &builder;

    SLANG_RETURN_ON_FAIL(_invoke(request));

    ComPtr<ISlangBlob> disassemblyBlob = StringUtil::createStringBlob(builder);
    *out = disassemblyBlob.detach();

    return SLANG_OK;
}

/* static */SlangResult GlslangDownstreamCompilerUtil::locateCompilers(const String& path, ISlangSharedLibraryLoader* loader, DownstreamCompilerSet* set)
{
    ComPtr<ISlangSharedLibrary> library;

#if SLANG_UNIX_FAMILY
    // On unix systems we need to ensure pthread is loaded first.
    // TODO(JS):
    // There is an argument that this should be performed through the loader....
    // NOTE! We don't currently load through a dependent library, as it is *assumed* something as core as 'ptheads'
    // isn't going to be distributed with the shader compiler. 
    ComPtr<ISlangSharedLibrary> pthreadLibrary;
    DefaultSharedLibraryLoader::load(loader, path, "pthread", pthreadLibrary.writeRef());
    if (!pthreadLibrary.get())
    {
        DefaultSharedLibraryLoader::load(loader, path, "libpthread.so.0", pthreadLibrary.writeRef());
    }

#endif

    SLANG_RETURN_ON_FAIL(DownstreamCompilerUtil::loadSharedLibrary(path, loader, nullptr, "slang-glslang", library));

    SLANG_ASSERT(library);
    if (!library)
    {
        return SLANG_FAIL;
    }

    RefPtr<GlslangDownstreamCompiler> compiler(new GlslangDownstreamCompiler);
    SLANG_RETURN_ON_FAIL(compiler->init(library));

    set->addCompiler(compiler);
    return SLANG_OK;
}

#else // SLANG_ENABLE_GLSLANG_SUPPORT

/* static */SlangResult GlslangDownstreamCompilerUtil::locateCompilers(const String& path, ISlangSharedLibraryLoader* loader, DownstreamCompilerSet* set)
{
    SLANG_UNUSED(path);
    SLANG_UNUSED(loader);
    SLANG_UNUSED(set);
    return SLANG_E_NOT_AVAILABLE;
}

#endif // SLANG_ENABLE_GLSLANG_SUPPORT

}
