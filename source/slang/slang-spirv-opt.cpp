#include "slang-spirv-opt.h"
#include "slang-spirv-val.h"

namespace Slang
{

    struct RemoveFileRAII
    {
        String fileName;

        RemoveFileRAII(String inFileName)
            :fileName(inFileName)
        {}

        ~RemoveFileRAII()
        {
            File::remove(fileName);
        }
    };

SlangResult optimizeSPIRV(const List<uint8_t>& spirv, String& outErr, List<uint8_t>& outSpv)
{
    // Set up our process
    CommandLine commandLine;
    commandLine.m_executableLocation.setName("spirv-opt");
    commandLine.addArg("--merge-return");
    commandLine.addArg("--inline-entry-points-exhaustive");
    commandLine.addArg("--eliminate-dead-functions");
    commandLine.addArg("--eliminate-local-single-block");
    commandLine.addArg("--eliminate-local-single-store");
    commandLine.addArg("--eliminate-dead-code-aggressive");

    commandLine.addArg("-o");
    String outFileName;
    File::generateTemporary(UnownedStringSlice("out_spv"), outFileName);
    RemoveFileRAII removeFile(outFileName);

    commandLine.addArg(outFileName);

    RefPtr<Process> p;

    // If we failed to even start the process, then spirv-opt isn't available
    SLANG_RETURN_ON_FAIL(Process::create(commandLine, 0, p));
    const auto in = p->getStream(StdStreamType::In);
    const auto out = p->getStream(StdStreamType::Out);
    const auto err = p->getStream(StdStreamType::ErrorOut);

    List<Byte> outErrData;
    SLANG_RETURN_ON_FAIL(StreamUtil::readAndWrite(in, spirv.getArrayView(), out, outSpv, err, outErrData));

    outSpv.clear();
    File::readAllBytes(outFileName, outSpv);

    SLANG_RETURN_ON_FAIL(p->waitForTermination(3600000));

    outErr = String(
        reinterpret_cast<const char*>(outErrData.begin()),
        reinterpret_cast<const char*>(outErrData.end())
    );

    const auto ret = p->getReturnValue();
    if (ret != 0)
        return SLANG_FAIL;

    return debugValidateSPIRV(outSpv);
}

}
