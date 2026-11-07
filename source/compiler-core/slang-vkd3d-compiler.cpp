#include "slang-vkd3d-compiler.h"

#if SLANG_ENABLE_VKD3D_SHADER
#include "core/slang-blob.h"
#include "core/slang-string-util.h"
#include "slang-artifact-associated-impl.h"
#include "slang-artifact-desc-util.h"
#include "slang-artifact-diagnostic-util.h"
#include "slang-include-system.h"

#include <vkd3d_shader.h>
#endif

namespace Slang
{

#if SLANG_ENABLE_VKD3D_SHADER

struct Vkd3dIncludeHandler
{
    Vkd3dIncludeHandler(
        SearchDirectoryList* directories,
        ISlangFileSystemExt* fileSystem,
        SourceManager* sourceManager,
        const String& sourcePath)
        : system(directories, fileSystem, sourceManager), rootPath(sourcePath)
    {
    }

    static int open(
        const char* filename,
        bool local,
        const char* parentData,
        void* context,
        vkd3d_shader_code* out)
    {
        auto handler = static_cast<Vkd3dIncludeHandler*>(context);
        String parentPath = handler->rootPath;
        if (parentData)
        {
            auto sourceFile =
                handler->system.getSourceManager()->findSourceFileByContentRecursively(parentData);
            if (sourceFile)
                parentPath = sourceFile->getPathInfo().foundPath;
        }

        PathInfo pathInfo;
        ComPtr<ISlangBlob> blob;
        const auto mode = local ? IncludeSystem::Mode::Quote : IncludeSystem::Mode::System;
        if (SLANG_FAILED(
                handler->system.findAndLoadFile(filename, parentPath, pathInfo, blob, mode)))
            return VKD3D_ERROR;

        out->code = blob->getBufferPointer();
        out->size = blob->getBufferSize();
        return VKD3D_OK;
    }

    static void close(const vkd3d_shader_code* code, void* context)
    {
        SLANG_UNUSED(code);
        SLANG_UNUSED(context);
        // The source manager keeps included files alive
    }

    IncludeSystem system;
    String rootPath;
};

static SlangResult _parseDiagnosticLine(
    SliceAllocator& allocator,
    const UnownedStringSlice& line,
    List<UnownedStringSlice>& slices,
    ArtifactDiagnostic& diagnostic)
{
    Index codeIndex = -1;
    for (Index i = 0; i < slices.getCount(); ++i)
    {
        const auto code = slices[i].trim();
        Int codeNumber;
        if (code.getLength() == 5 && (code[0] == 'E' || code[0] == 'W') &&
            SLANG_SUCCEEDED(StringUtil::parseInt(code.tail(1), codeNumber)))
        {
            codeIndex = i;
            diagnostic.code = allocator.allocate(code);
            diagnostic.severity = code[0] == 'E' ? ArtifactDiagnostic::Severity::Error
                                                 : ArtifactDiagnostic::Severity::Warning;
            break;
        }
    }
    if (codeIndex < 0 || codeIndex + 1 >= slices.getCount())
        return SLANG_FAIL;

    if (codeIndex >= 3)
    {
        Int lineNumber, column;
        SLANG_RETURN_ON_FAIL(StringUtil::parseInt(slices[codeIndex - 2], lineNumber));
        SLANG_RETURN_ON_FAIL(StringUtil::parseInt(slices[codeIndex - 1], column));
        diagnostic.location.line = lineNumber;
        diagnostic.location.column = column;
        diagnostic.filePath =
            allocator.allocate(UnownedStringSlice(line.begin(), slices[codeIndex - 2].begin() - 1));
    }
    else if (codeIndex == 1)
    {
        diagnostic.filePath = allocator.allocate(slices[0]);
    }
    diagnostic.text =
        allocator.allocate(UnownedStringSlice(slices[codeIndex + 1].begin(), line.end()).trim());
    return SLANG_OK;
}

class Vkd3dDownstreamCompiler : public DownstreamCompilerBase
{
public:
    SlangResult init(ISlangSharedLibrary* library)
    {
        m_compile = reinterpret_cast<PFN_vkd3d_shader_compile>(
            library->findFuncByName("vkd3d_shader_compile"));
        m_freeCode = reinterpret_cast<PFN_vkd3d_shader_free_shader_code>(
            library->findFuncByName("vkd3d_shader_free_shader_code"));
        m_freeMessages = reinterpret_cast<PFN_vkd3d_shader_free_messages>(
            library->findFuncByName("vkd3d_shader_free_messages"));
        auto getVersion = reinterpret_cast<PFN_vkd3d_shader_get_version>(
            library->findFuncByName("vkd3d_shader_get_version"));
        if (!m_compile || !m_freeCode || !m_freeMessages || !getVersion)
            return SLANG_E_NOT_AVAILABLE;

        unsigned int major = 0, minor = 0;
        getVersion(&major, &minor);
        if (major < 2 || (major == 2 && minor < 1))
            return SLANG_E_NOT_AVAILABLE;

        auto getTargets = reinterpret_cast<PFN_vkd3d_shader_get_supported_target_types>(
            library->findFuncByName("vkd3d_shader_get_supported_target_types"));
        if (!getTargets ||
            !_supportsTarget(getTargets, VKD3D_SHADER_SOURCE_HLSL, VKD3D_SHADER_TARGET_DXBC_TPF) ||
            !_supportsTarget(getTargets, VKD3D_SHADER_SOURCE_DXBC_TPF, VKD3D_SHADER_TARGET_D3D_ASM))
            return SLANG_E_NOT_AVAILABLE;

        m_desc = Desc(SLANG_PASS_THROUGH_VKD3D, int(major), int(minor));
        m_library = library;
        return SLANG_OK;
    }

    SLANG_NO_THROW SlangResult SLANG_MCALL
    compile(const CompileOptions& inOptions, IArtifact** outArtifact) SLANG_OVERRIDE
    {
        if (!isVersionCompatible(inOptions))
            return SLANG_E_NOT_IMPLEMENTED;
        const auto options = getCompatibleVersion(&inOptions);
        if (options.sourceArtifacts.count != 1 ||
            options.sourceLanguage != SLANG_SOURCE_LANGUAGE_HLSL ||
            options.targetType != SLANG_DXBC)
            return SLANG_E_INVALID_ARG;

        if (options.optimizationLevel != CompileOptions::OptimizationLevel::Default ||
            options.debugInfoType != CompileOptions::DebugInfoType::None ||
            options.floatingPointMode != CompileOptions::FloatingPointMode::Default ||
            options.denormalModeFp16 != CompileOptions::FloatingPointDenormalMode::Any ||
            options.denormalModeFp32 != CompileOptions::FloatingPointDenormalMode::Any ||
            options.denormalModeFp64 != CompileOptions::FloatingPointDenormalMode::Any ||
            options.compilerSpecificArguments.count)
        {
            auto diagnostics = ArtifactDiagnostics::create();
            diagnostics->setResult(SLANG_E_NOT_AVAILABLE);
            diagnostics->setRaw(asCharSlice(toSlice(
                "vkd3d does not support optimization levels, debug info, floating point mode "
                "overrides or compiler specific arguments\n")));
            diagnostics->requireErrorDiagnostic();
            auto artifact = ArtifactUtil::createArtifactForCompileTarget(options.targetType);
            ArtifactUtil::addAssociated(artifact, diagnostics);
            *outArtifact = artifact.detach();
            return SLANG_OK;
        }

        auto source = options.sourceArtifacts[0];
        ComPtr<ISlangBlob> blob;
        SLANG_RETURN_ON_FAIL(source->loadBlob(ArtifactKeep::Yes, blob.writeRef()));
        String sourcePath(ArtifactUtil::findPath(source));

        SearchDirectoryList directories;
        for (const auto& path : options.includePaths)
            directories.searchDirectories.add(asString(path));
        Vkd3dIncludeHandler includes(
            &directories,
            options.fileSystemExt,
            options.sourceManager,
            sourcePath);

        List<vkd3d_shader_macro> macros;
        StringBuilder sourceText;
        for (const auto& define : options.defines)
        {
            if (asStringSlice(define.nameWithSig).indexOf('(') >= 0)
            {
                sourceText << "#define " << asStringSlice(define.nameWithSig) << " "
                           << asStringSlice(define.value) << "\n";
            }
            else
            {
                macros.add({define.nameWithSig, define.value});
            }
        }
        vkd3d_shader_code sourceCode = {blob->getBufferPointer(), blob->getBufferSize()};
        if (sourceText.getLength())
        {
            sourceText.append(
                static_cast<const char*>(blob->getBufferPointer()),
                blob->getBufferSize());
            sourceCode = {sourceText.getBuffer(), size_t(sourceText.getLength())};
        }

        vkd3d_shader_preprocess_info preprocess = {};
        preprocess.type = VKD3D_SHADER_STRUCTURE_TYPE_PREPROCESS_INFO;
        preprocess.macros = macros.getBuffer();
        preprocess.macro_count = unsigned(macros.getCount());
        if (options.fileSystemExt && options.sourceManager)
        {
            preprocess.pfn_open_include = Vkd3dIncludeHandler::open;
            preprocess.pfn_close_include = Vkd3dIncludeHandler::close;
            preprocess.include_context = &includes;
        }

        vkd3d_shader_hlsl_source_info hlsl = {};
        hlsl.type = VKD3D_SHADER_STRUCTURE_TYPE_HLSL_SOURCE_INFO;
        hlsl.next = &preprocess;
        hlsl.entry_point = options.entryPointName;
        hlsl.profile = options.profileName;

        const vkd3d_shader_compile_option compileOptions[] = {
            {VKD3D_SHADER_COMPILE_OPTION_API_VERSION, VKD3D_SHADER_API_VERSION_2_1},
            {VKD3D_SHADER_COMPILE_OPTION_PACK_MATRIX_ORDER,
             options.matrixLayout == SLANG_MATRIX_LAYOUT_COLUMN_MAJOR
                 ? VKD3D_SHADER_COMPILE_OPTION_PACK_MATRIX_COLUMN_MAJOR
                 : VKD3D_SHADER_COMPILE_OPTION_PACK_MATRIX_ROW_MAJOR},
        };
        vkd3d_shader_compile_info info = {};
        info.type = VKD3D_SHADER_STRUCTURE_TYPE_COMPILE_INFO;
        info.next = &hlsl;
        info.source = sourceCode;
        info.source_type = VKD3D_SHADER_SOURCE_HLSL;
        info.target_type = VKD3D_SHADER_TARGET_DXBC_TPF;
        info.options = compileOptions;
        info.option_count = SLANG_COUNT_OF(compileOptions);
        info.log_level = VKD3D_SHADER_LOG_WARNING;
        info.source_name = sourcePath.getBuffer();
        return _compile(info, ArtifactDescUtil::makeDescForCompileTarget(SLANG_DXBC), outArtifact);
    }

    SLANG_NO_THROW bool SLANG_MCALL canConvert(const ArtifactDesc& from, const ArtifactDesc& to)
        SLANG_OVERRIDE
    {
        return ArtifactDescUtil::isDisassembly(from, to) && from.payload == ArtifactPayload::DXBC;
    }

    SLANG_NO_THROW SlangResult SLANG_MCALL
    convert(IArtifact* from, const ArtifactDesc& to, IArtifact** outArtifact) SLANG_OVERRIDE
    {
        if (!canConvert(from->getDesc(), to))
            return SLANG_E_INVALID_ARG;
        ComPtr<ISlangBlob> blob;
        SLANG_RETURN_ON_FAIL(from->loadBlob(ArtifactKeep::No, blob.writeRef()));
        vkd3d_shader_compile_info info = {};
        info.type = VKD3D_SHADER_STRUCTURE_TYPE_COMPILE_INFO;
        info.source = {blob->getBufferPointer(), blob->getBufferSize()};
        info.source_type = VKD3D_SHADER_SOURCE_DXBC_TPF;
        info.target_type = VKD3D_SHADER_TARGET_D3D_ASM;
        info.log_level = VKD3D_SHADER_LOG_WARNING;
        return _compile(info, to, outArtifact);
    }

    SLANG_NO_THROW bool SLANG_MCALL isFileBased() SLANG_OVERRIDE { return false; }
    SLANG_NO_THROW SlangResult SLANG_MCALL getPath(slang::IBlob** outPath) SLANG_OVERRIDE
    {
        return getPathFromSymbol(reinterpret_cast<void*>(m_compile), outPath);
    }

private:
    static bool _supportsTarget(
        PFN_vkd3d_shader_get_supported_target_types getTargets,
        vkd3d_shader_source_type source,
        vkd3d_shader_target_type target)
    {
        unsigned int count = 0;
        const auto targets = getTargets(source, &count);
        for (unsigned int i = 0; i < count; ++i)
        {
            if (targets[i] == target)
                return true;
        }
        return false;
    }

    SlangResult _compile(
        const vkd3d_shader_compile_info& info,
        const ArtifactDesc& desc,
        IArtifact** outArtifact)
    {
        vkd3d_shader_code code = {};
        char* messages = nullptr;
        int result = m_compile(&info, &code, &messages);
        auto diagnostics = ArtifactDiagnostics::create();
        SlangResult slangResult = result >= 0 ? SLANG_OK : SLANG_FAIL;
        if (result == VKD3D_ERROR_OUT_OF_MEMORY)
            slangResult = SLANG_E_OUT_OF_MEMORY;
        else if (result == VKD3D_ERROR_NOT_IMPLEMENTED)
            slangResult = SLANG_E_NOT_IMPLEMENTED;
        diagnostics->setResult(slangResult);
        if (messages)
        {
            const UnownedStringSlice text(messages);
            diagnostics->setRaw(asCharSlice(text));
            SliceAllocator allocator;
            ArtifactDiagnosticUtil::parseColonDelimitedDiagnostics(
                allocator,
                text,
                0,
                _parseDiagnosticLine,
                diagnostics);
        }
        m_freeMessages(messages);
        if (SLANG_FAILED(slangResult))
            diagnostics->requireErrorDiagnostic();

        auto artifact = ArtifactUtil::createArtifact(desc);
        ArtifactUtil::addAssociated(artifact, diagnostics);
        ComPtr<ISlangBlob> blob;
        SlangResult copyResult = SLANG_OK;
        if (result >= 0)
        {
            copyResult = RawBlob::tryCreate(code.code, code.size, blob);
            if (SLANG_SUCCEEDED(copyResult))
                artifact->addRepresentationUnknown(blob);
        }
        m_freeCode(&code);
        SLANG_RETURN_ON_FAIL(copyResult);
        *outArtifact = artifact.detach();
        return SLANG_OK;
    }

    PFN_vkd3d_shader_compile m_compile = nullptr;
    PFN_vkd3d_shader_free_shader_code m_freeCode = nullptr;
    PFN_vkd3d_shader_free_messages m_freeMessages = nullptr;
    ComPtr<ISlangSharedLibrary> m_library;
};

SlangResult Vkd3dDownstreamCompilerUtil::locateCompilers(
    const String& path,
    ISlangSharedLibraryLoader* loader,
    DownstreamCompilerSet* set)
{
    ComPtr<ISlangSharedLibrary> library;
    SLANG_RETURN_ON_FAIL(
        DownstreamCompilerUtil::loadSharedLibrary(path, loader, nullptr, "vkd3d-shader", library));
    auto compiler = new Vkd3dDownstreamCompiler;
    ComPtr<IDownstreamCompiler> compilerInterface(compiler);
    SLANG_RETURN_ON_FAIL(compiler->init(library));
    set->addCompiler(compilerInterface);
    return SLANG_OK;
}

#else

SlangResult Vkd3dDownstreamCompilerUtil::locateCompilers(
    const String& path,
    ISlangSharedLibraryLoader* loader,
    DownstreamCompilerSet* set)
{
    SLANG_UNUSED(path);
    SLANG_UNUSED(loader);
    SLANG_UNUSED(set);
    return SLANG_E_NOT_AVAILABLE;
}

#endif

} // namespace Slang
