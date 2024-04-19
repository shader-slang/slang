#include "slang-metal-compiler.h"
#include "slang-gcc-compiler-util.h"
#include "slang-artifact-desc-util.h"
#include "slang-artifact-util.h"
#include "slang-artifact-representation.h"

namespace Slang
{

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

        virtual SLANG_NO_THROW SlangResult SLANG_MCALL compile(const CompileOptions& options, IArtifact** outArtifact)
        {
            // All compile requests should be routed directly to the inner compiler.
            return cppCompiler->compile(options, outArtifact);
        }

        virtual SLANG_NO_THROW bool SLANG_MCALL canConvert(const ArtifactDesc& from, const ArtifactDesc& to) override
        {
            // Report that we can convert Metal IR to disassembly.
            return ArtifactDescUtil::isDisassembly(from, to) && from.payload == ArtifactPayload::MetalAIR;
        }

        virtual SLANG_NO_THROW SlangResult SLANG_MCALL convert(IArtifact* from, const ArtifactDesc& to, IArtifact** outArtifact)
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

    static SlangResult locateMetalCompiler(const String& path, DownstreamCompilerSet* set)
    {
        ComPtr<IDownstreamCompiler> innerCppCompiler;
        ExecutableLocation exeLocation(path, "metal");
        SLANG_RETURN_ON_FAIL(GCCDownstreamCompilerUtil::createCompiler(exeLocation, innerCppCompiler));

        ComPtr<IDownstreamCompiler> compiler = ComPtr<IDownstreamCompiler>(
            new MetalDownstreamCompiler(innerCppCompiler, path));
        set->addCompiler(compiler);
        return SLANG_OK;
    }

    SlangResult MetalDownstreamCompilerUtil::locateCompilers(const String& path, ISlangSharedLibraryLoader* loader, DownstreamCompilerSet* set)
    {
        SLANG_UNUSED(loader);
        return locateMetalCompiler(path, set);
    }

}
