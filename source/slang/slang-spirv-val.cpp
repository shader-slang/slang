#include "slang-spirv-val.h"

namespace Slang
{

SlangResult disassembleSPIRV(const List<uint8_t>& spirv, String& outErr, String& outDis)
{
    // Set up our process
    CommandLine commandLine;
    commandLine.m_executableLocation.setName("spirv-dis");
    commandLine.addArg("--comment");
    commandLine.addArg("--color");
    RefPtr<Process> p;

    // If we failed to even start the process, then validation isn't available
    SLANG_RETURN_ON_FAIL(Process::create(commandLine, 0, p));
    const auto in = p->getStream(StdStreamType::In);
    const auto out = p->getStream(StdStreamType::Out);
    const auto err = p->getStream(StdStreamType::ErrorOut);

    List<Byte> outData;
    List<Byte> outErrData;
    SLANG_RETURN_ON_FAIL(
        StreamUtil::readAndWrite(in, spirv.getArrayView(), out, outData, err, outErrData));

    SLANG_RETURN_ON_FAIL(p->waitForTermination(10));

    outDis = String(
        reinterpret_cast<const char*>(outData.begin()),
        reinterpret_cast<const char*>(outData.end()));

    outErr = String(
        reinterpret_cast<const char*>(outErrData.begin()),
        reinterpret_cast<const char*>(outErrData.end()));

    const auto ret = p->getReturnValue();
    return ret == 0 ? SLANG_OK : SLANG_FAIL;
}


} // namespace Slang
