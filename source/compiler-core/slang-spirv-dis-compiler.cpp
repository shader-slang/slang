#include "slang-spirv-dis-compiler.h"

#include "../core/slang-common.h"
#include "slang-artifact-representation.h"
#include "slang-artifact-util.h"

namespace Slang
{

SlangResult SPIRVDisDownstreamCompilerUtil::locateCompilers(
    const String&,
    ISlangSharedLibraryLoader*,
    DownstreamCompilerSet* set)
{
    ComPtr<IDownstreamCompiler> com(
        new SPIRVDisDownstreamCompiler(DownstreamCompilerDesc(SLANG_PASS_THROUGH_SPIRV_DIS)));
    set->addCompiler(com);

    return SLANG_OK;
}

SlangResult SPIRVDisDownstreamCompiler::convert(IArtifact* from, const ArtifactDesc& to, IArtifact** outArtifact)
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

    // Set up our process
    CommandLine commandLine;
    commandLine.m_executableLocation.setName("spirv-dis");
    RefPtr<Process> p;
    SLANG_RETURN_ON_FAIL(Process::create(commandLine, 0, p));
    const auto in = p->getStream(StdStreamType::In);
    const auto out = p->getStream(StdStreamType::Out);
    const auto err = p->getStream(StdStreamType::ErrorOut);

    // Write the assembly
    SLANG_RETURN_ON_FAIL(in->write(fromBlob->getBufferPointer(), fromBlob->getBufferSize()));
    in->close();

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

    // Read the disassembly
    List<Byte> outData;
    SLANG_RETURN_ON_FAIL(StreamUtil::readAll(out, 0, outData));

    // Wobble it into an artifact
    ComPtr<ISlangBlob> outBlob = RawBlob::create(outData.getBuffer(), outData.getCount());
    auto artifact = ArtifactUtil::createArtifact(to);
    artifact->addRepresentationUnknown(outBlob.detach());
    *outArtifact = artifact.detach();

    return SLANG_OK;
}

SlangResult SPIRVDisDownstreamCompiler::compile(const CompileOptions&, IArtifact**)
{
    SLANG_UNIMPLEMENTED_X(__func__);
}

}
