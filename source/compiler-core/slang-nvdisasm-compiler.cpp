#include "slang-nvdisasm-compiler.h"

#include "compiler-core/slang-artifact-associated-impl.h"
#include "compiler-core/slang-artifact-desc-util.h"
#include "compiler-core/slang-artifact-representation.h"
#include "compiler-core/slang-artifact-util.h"
#include "core/slang-blob.h"

namespace Slang
{
class NvdisasmDownstreamCompiler : public DownstreamCompilerBase
{
public:
    NvdisasmDownstreamCompiler(const ExecutableLocation& executable, const String& versionString)
        : DownstreamCompilerBase(Desc(SLANG_PASS_THROUGH_NVDISASM))
        , m_executable(executable)
        , m_versionString(StringBlob::create(versionString))
    {
    }

    SLANG_NO_THROW bool SLANG_MCALL isFileBased() SLANG_OVERRIDE { return true; }

    SLANG_NO_THROW SlangResult SLANG_MCALL getVersionString(ISlangBlob** outVersionString)
        SLANG_OVERRIDE
    {
        m_versionString->addRef();
        *outVersionString = m_versionString;
        return SLANG_OK;
    }

    SLANG_NO_THROW SlangResult SLANG_MCALL compile(const CompileOptions&, IArtifact**)
        SLANG_OVERRIDE
    {
        return SLANG_E_NOT_IMPLEMENTED;
    }

    SLANG_NO_THROW bool SLANG_MCALL canConvert(const ArtifactDesc& from, const ArtifactDesc& to)
        SLANG_OVERRIDE
    {
        return from.kind == ArtifactKind::ObjectCode && from.payload == ArtifactPayload::CuBin &&
               from.style == ArtifactStyle::Kernel && ArtifactDescUtil::isDisassembly(from, to);
    }

    SLANG_NO_THROW SlangResult SLANG_MCALL
    convert(IArtifact* from, const ArtifactDesc& to, IArtifact** outArtifact) SLANG_OVERRIDE
    {
        if (!canConvert(from->getDesc(), to))
            return SLANG_E_INVALID_ARG;

        auto artifact = ArtifactUtil::createArtifact(to);
        auto diagnostics = ArtifactDiagnostics::create();
        ArtifactUtil::addAssociated(artifact, diagnostics);

        // Keep the file representation alive until the process exits. A memory-only CUBIN
        // becomes a temporary file whose lifetime is owned by this representation.
        ComPtr<IOSFileArtifactRepresentation> file;
        SlangResult result = from->requireFile(IArtifact::Keep::No, file.writeRef());
        ExecuteResult processResult;
        processResult.init();
        processResult.resultCode = -1;
        if (SLANG_SUCCEEDED(result))
        {
            CommandLine command;
            command.setExecutableLocation(m_executable);
            // Basic disassembly must also work for NVRTC output without relocation data.
            command.addArg("-ndf");
            command.addArg(String(file->getPath()));
            result = ProcessUtil::execute(command, processResult);
            if (SLANG_SUCCEEDED(result) && processResult.resultCode != 0)
                result = SLANG_FAIL;
        }

        diagnostics->setRaw(SliceUtil::asCharSlice(processResult.standardError));
        if (SLANG_SUCCEEDED(result) && processResult.standardOutput.getLength() == 0)
            result = SLANG_FAIL;
        diagnostics->setResult(result);
        if (SLANG_FAILED(result))
        {
            StringBuilder message;
            message << "nvdisasm failed to disassemble CUBIN (exit code "
                    << processResult.resultCode << ").\n";
            diagnostics->appendRaw(SliceUtil::asCharSlice(message));
            diagnostics->requireErrorDiagnostic();
        }
        else
        {
            artifact->addRepresentationUnknown(StringBlob::create(processResult.standardOutput));
        }
        *outArtifact = artifact.detach();
        return result;
    }

private:
    ExecutableLocation m_executable;
    ComPtr<ISlangBlob> m_versionString;
};

SlangResult NvdisasmDownstreamCompilerUtil::locateCompilers(
    const String& path,
    ISlangSharedLibraryLoader* loader,
    DownstreamCompilerSet* set)
{
    SLANG_UNUSED(loader);
    List<ExecutableLocation> candidates;
    if (path.getLength())
    {
        // An explicit path disables fallback search, whether it names a directory or executable.
        SlangPathType pathType;
        const bool isDirectory = SLANG_SUCCEEDED(Path::getPathType(path, &pathType)) &&
                                 pathType == SLANG_PATH_TYPE_DIRECTORY;
        candidates.add(
            isDirectory ? ExecutableLocation(path, "nvdisasm") : ExecutableLocation(path));
    }
    else
    {
        for (const char* variable : {"CUDA_PATH", "CUDA_HOME"})
        {
            StringBuilder root;
            if (SLANG_SUCCEEDED(
                    PlatformUtil::getEnvironmentVariable(UnownedStringSlice(variable), root)) &&
                root.getLength())
            {
                candidates.add(ExecutableLocation(Path::combine(root, "bin"), "nvdisasm"));
            }
        }
        candidates.add(ExecutableLocation("nvdisasm"));
    }
    for (const auto& candidate : candidates)
    {
        CommandLine command;
        command.setExecutableLocation(candidate);
        command.addArg("--version");
        ExecuteResult result;
        if (SLANG_SUCCEEDED(ProcessUtil::execute(command, result)) && result.resultCode == 0)
        {
            ComPtr<IDownstreamCompiler> compiler(
                new NvdisasmDownstreamCompiler(candidate, result.standardOutput));
            set->addCompiler(compiler);
            return SLANG_OK;
        }
    }
    return SLANG_E_NOT_FOUND;
}
} // namespace Slang
