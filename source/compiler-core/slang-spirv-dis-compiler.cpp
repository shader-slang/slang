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

    // Set up our process
    CommandLine commandLine;
    commandLine.m_executableLocation.setName("spirv-dis");
    commandLine.addArg("--comment");

    RefPtr<Process> p;
    SLANG_RETURN_ON_FAIL(Process::create(commandLine, 0, p));
    
    auto inputStream = p->getStream(StdStreamType::In);
    if (!inputStream)
        return SLANG_FAIL;

    auto outputStream = p->getStream(StdStreamType::Out);
    List<uint8_t> outBytes;
    const auto err = p->getStream(StdStreamType::ErrorOut);
    List<Byte> errData;

    SLANG_RETURN_ON_FAIL(StreamUtil::readAndWrite(
        inputStream,
        ArrayView<Byte>((Byte*)fromBlob->getBufferPointer(), (Index)fromBlob->getBufferSize()),
        outputStream,
        outBytes,
        err,
        errData));

    // Wait for it to finish
    if(!p->waitForTermination(1000))
        return SLANG_FAIL;

    if (errData.getCount())
        fwrite(errData.getBuffer(), 1, (size_t)errData.getCount(), stderr);

    StringBuilder sbOutput;
    List<UnownedStringSlice> lines;
    StringUtil::calcLines(StringUtil::trimEndOfLine(UnownedStringSlice((const char*)outBytes.getBuffer())), lines);
    for (auto line : lines)
    {
        sbOutput << StringUtil::trimEndOfLine(line) << "\n";
    }
    // If spirv-dis failed, we fail
    const auto ret = p->getReturnValue();
    if(ret != 0)
        return SLANG_FAIL;

    // Return as a file artifact
    
    auto disassemblyBlob = StringBlob::moveCreate(sbOutput);

    auto artifact = ArtifactUtil::createArtifact(to);
    artifact->addRepresentationUnknown(disassemblyBlob);

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
