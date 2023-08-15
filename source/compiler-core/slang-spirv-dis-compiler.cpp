#include "slang-spirv-dis-compiler.h"

#include "../core/slang-common.h"
#include "../core/slang-string-util.h"
#include "../core/slang-string.h"
#include "slang-artifact-desc-util.h"
#include "slang-artifact-representation.h"
#include "slang-artifact-util.h"
#include "slang-artifact-representation-impl.h"

namespace Slang
{

SlangResult SPIRVDisDownstreamCompilerUtil::locateCompilers(
    const String&,
    ISlangSharedLibraryLoader*,
    DownstreamCompilerSet* set)
{
    // TODO: We could check that the compiler is actually present in PATH (or
    // explicitly given)

    ComPtr<IDownstreamCompiler> com(
        new SPIRVDisDownstreamCompiler(DownstreamCompilerDesc(SLANG_PASS_THROUGH_SPIRV_DIS)));
    set->addCompiler(com);

    return SLANG_OK;
}

SlangResult SLANG_MCALL SPIRVDisDownstreamCompiler::convert(
    IArtifact* from,
    const ArtifactDesc& to,
    IArtifact** outArtifact) noexcept
{
    const auto& fromDesc = from->getDesc();
    if(to.kind != ArtifactKind::Assembly ||
       to.payload != ArtifactPayload::SPIRV ||
       to.flags != 0 ||
       fromDesc.kind != ArtifactKind::Executable ||
       fromDesc.payload != ArtifactPayload::SPIRV ||
       fromDesc.flags != 0)
        return SLANG_E_INVALID_ARG;

    ISlangBlob* fromBlob;
    SLANG_RETURN_ON_FAIL(from->loadBlob(ArtifactKeep::No, &fromBlob));

    ComPtr<IOSFileArtifactRepresentation> fromFile;
    SLANG_RETURN_ON_FAIL(from->requireFile(ArtifactKeep::No, fromFile.writeRef()));

    String toFile;
    File::generateTemporary(UnownedStringSlice("spv-asm"), toFile);

    // Set up our process
    CommandLine commandLine;
    commandLine.m_executableLocation.setName("spirv-dis");
    commandLine.addArg("--comment");
    commandLine.addArg(fromFile->getPath());
    commandLine.addArg("-o");
    commandLine.addArg(toFile);
    RefPtr<Process> p;
    SLANG_RETURN_ON_FAIL(Process::create(commandLine, 0, p));
    const auto err = p->getStream(StdStreamType::ErrorOut);

    // Wait for it to finish
    if(!p->waitForTermination(1000))
        return SLANG_FAIL;

    // TODO: allow inheriting stderr in Process
    List<Byte> errData;
    SLANG_RETURN_ON_FAIL(StreamUtil::readAll(err, 0, errData));
    fwrite(errData.getBuffer(), errData.getCount(), 1, stderr);

    const auto ret = p->getReturnValue();
    if(ret != 0)
        return SLANG_FAIL;

    auto fileRep = OSFileArtifactRepresentation::create(
        IOSFileArtifactRepresentation::Kind::Owned,
        toFile.getUnownedSlice(),
        nullptr
    );
    auto artifact = ArtifactUtil::createArtifact(to);
    artifact->addRepresentation(fileRep.detach());
    *outArtifact = artifact.detach();

    return SLANG_OK;
}

SlangResult SLANG_MCALL SPIRVDisDownstreamCompiler::compile(
    const CompileOptions&,
    IArtifact**) noexcept
{
    SLANG_UNIMPLEMENTED_X(__func__);
}

}
