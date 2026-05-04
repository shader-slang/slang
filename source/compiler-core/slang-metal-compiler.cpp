#include "slang-metal-compiler.h"

#include "../core/slang-io.h"
#include "slang-artifact-desc-util.h"
#include "slang-artifact-representation.h"
#include "slang-artifact-util.h"
#include "slang-gcc-compiler-util.h"

namespace Slang
{

// The `-metal-path` option names the directory that contains the `metal`
// executable.  On Windows, users commonly paste the full path to the
// executable (e.g. `C:\Program Files\Metal Developer Tools\bin\metal.exe`),
// which previously failed with `E52002: pass-through compiler not found`
// because the lookup joined `<path>/metal(.exe)`.  Strip a trailing
// `metal.exe` so both spellings resolve.  We intentionally do not strip a
// bare `metal` basename: a directory literally named `metal` containing a
// `metal` executable is a plausible real-world layout.
static String normalizeMetalToolsPath(const String& path)
{
    if (path.getLength() == 0)
        return path;
    String fileName = Path::getFileName(path);
    if (fileName.getUnownedSlice().caseInsensitiveEquals(toSlice("metal.exe")))
    {
        return Path::getParentDirectory(path);
    }
    return path;
}

class MetalDownstreamCompiler : public DownstreamCompilerBase
{
public:
    // Because the metal compiler shares the same commandline interface with clang,
    // we will use GccDownstreamCompilerUtil, which implements both gcc and clang,
    // to create the inner compiler and wrap it here.
    //
    ComPtr<IDownstreamCompiler> cppCompiler;
    String executablePath;

    MetalDownstreamCompiler(ComPtr<IDownstreamCompiler>& innerCompiler, String path)
        : DownstreamCompilerBase(innerCompiler->getDesc())
        , cppCompiler(innerCompiler)
        , executablePath(path)
    {
    }

    virtual SLANG_NO_THROW bool SLANG_MCALL isFileBased() override { return true; }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    compile(const CompileOptions& options, IArtifact** outArtifact) override
    {
        // All compile requests should be routed directly to the inner compiler.
        return cppCompiler->compile(options, outArtifact);
    }

    virtual SLANG_NO_THROW bool SLANG_MCALL
    canConvert(const ArtifactDesc& from, const ArtifactDesc& to) override
    {
        // Report that we can convert Metal IR to disassembly.
        return ArtifactDescUtil::isDisassembly(from, to) &&
               from.payload == ArtifactPayload::MetalAIR;
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    convert(IArtifact* from, const ArtifactDesc& to, IArtifact** outArtifact) override
    {
        // Use metal-objdump to disassemble the Metal IR.

        ExecutableLocation exeLocation(executablePath, "metal-objdump");
        CommandLine cmdLine;
        cmdLine.setExecutableLocation(exeLocation);
        cmdLine.addArg("--disassemble");
        ComPtr<IOSFileArtifactRepresentation> srcFile;
        SLANG_RETURN_ON_FAIL(from->requireFile(IArtifact::Keep::No, srcFile.writeRef()));
        cmdLine.addArg(String(srcFile->getPath()));

        ExecuteResult exeRes;
        SLANG_RETURN_ON_FAIL(ProcessUtil::execute(cmdLine, exeRes));
        auto artifact = ArtifactUtil::createArtifact(to);
        artifact->addRepresentationUnknown(StringBlob::create(exeRes.standardOutput));
        *outArtifact = artifact.detach();
        return SLANG_OK;
    }
};

static SlangResult locateMetalCompiler(const String& inPath, DownstreamCompilerSet* set)
{
    ComPtr<IDownstreamCompiler> innerCppCompiler;

    String path = normalizeMetalToolsPath(inPath);
    ExecutableLocation metalcLocation = ExecutableLocation(path, "metal");

    String metalSDKPath = path;

#if SLANG_APPLE_FAMILY
    // Use xcrun command to find the metal compiler.
    CommandLine xcrunCmdLine;
    ExecutableLocation xcrunLocation("xcrun");
    xcrunCmdLine.setExecutableLocation(xcrunLocation);
    xcrunCmdLine.addArg("--sdk");
    xcrunCmdLine.addArg("macosx");
    xcrunCmdLine.addArg("--find");
    xcrunCmdLine.addArg("metal");
    ExecuteResult exeRes;
    if (SLANG_SUCCEEDED(ProcessUtil::execute(xcrunCmdLine, exeRes)))
    {
        String metalPath = exeRes.standardOutput.trim();
        metalcLocation = ExecutableLocation(ExecutableLocation::Type::Path, metalPath);
        metalSDKPath = Path::getParentDirectory(metalcLocation.m_pathOrName);
    }
#endif

    SLANG_RETURN_ON_FAIL(
        GCCDownstreamCompilerUtil::createCompiler(metalcLocation, innerCppCompiler));

    ComPtr<IDownstreamCompiler> compiler =
        ComPtr<IDownstreamCompiler>(new MetalDownstreamCompiler(innerCppCompiler, metalSDKPath));
    set->addCompiler(compiler);
    return SLANG_OK;
}

SlangResult MetalDownstreamCompilerUtil::locateCompilers(
    const String& path,
    ISlangSharedLibraryLoader* loader,
    DownstreamCompilerSet* set)
{
    SLANG_UNUSED(loader);
    return locateMetalCompiler(path, set);
}

} // namespace Slang
