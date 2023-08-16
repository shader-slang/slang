#include "slang-spirv-val.h"

namespace Slang
{

static SlangResult disassembleSPIRV(const List<uint8_t>& spirv, String& outErr, String& outDis)
{
    // Set up our process
    CommandLine commandLine;
    commandLine.m_executableLocation.setName("spirv-dis");
    RefPtr<Process> p;

    // If we failed to even start the process, then validation isn't available
    SLANG_RETURN_ON_FAIL(Process::create(commandLine, 0, p));
    const auto in = p->getStream(StdStreamType::In);
    const auto out = p->getStream(StdStreamType::Out);
    const auto err = p->getStream(StdStreamType::ErrorOut);

    // Write the assembly
    SLANG_RETURN_ON_FAIL(in->write(spirv.getBuffer(), spirv.getCount()));
    in->close();

    // Wait for it to finish
    List<Byte> outData;
    List<Byte> outErrData;
    while (!out->isEnd() || !err->isEnd())
    {
        if (!out->isEnd())
            StreamUtil::readAll(out, 0, outData);
        if (!err->isEnd())
            StreamUtil::readAll(err, 0, outErrData);
    }
    SLANG_RETURN_ON_FAIL(p->waitForTermination(10));

    outDis = String(
        reinterpret_cast<const char*>(outData.begin()),
        reinterpret_cast<const char*>(outData.end())
    );

    outErr = String(
        reinterpret_cast<const char*>(outErrData.begin()),
        reinterpret_cast<const char*>(outErrData.end())
    );

    const auto ret = p->getReturnValue();
    return ret == 0 ? SLANG_OK : SLANG_FAIL;
}

SlangResult debugValidateSPIRV(const List<uint8_t>& spirv)
{
    // Set up our process
    CommandLine commandLine;
    commandLine.m_executableLocation.setName("spirv-val");
    RefPtr<Process> p;
    const auto createResult = Process::create(commandLine, 0, p);
    // If we failed to even start the process, then validation isn't available
    if(SLANG_FAILED(createResult))
        return SLANG_E_NOT_AVAILABLE;
    const auto in = p->getStream(StdStreamType::In);
    const auto out = p->getStream(StdStreamType::Out);
    const auto err = p->getStream(StdStreamType::ErrorOut);

    // Write the assembly
    SLANG_RETURN_ON_FAIL(in->write(spirv.getBuffer(), spirv.getCount()));
    in->close();

    // Wait for it to finish
    if(!p->waitForTermination(1000))
        return SLANG_FAIL;


    // TODO: allow inheriting stderr in Process
    List<Byte> outData;
    SLANG_RETURN_ON_FAIL(StreamUtil::readAll(out, 0, outData));
    fwrite(outData.getBuffer(), outData.getCount(), 1, stderr);
    outData.clear();
    SLANG_RETURN_ON_FAIL(StreamUtil::readAll(err, 0, outData));
    fwrite(outData.getBuffer(), outData.getCount(), 1, stderr);
    const auto ret = p->getReturnValue();

    if(ret != 0)
    {
        String spirvDisErr;
        String spirvDis;
        disassembleSPIRV(spirv, spirvDisErr, spirvDis);
        fwrite(spirvDisErr.getBuffer(), spirvDisErr.getLength(), 1, stderr);
        fwrite(spirvDis.getBuffer(), spirvDis.getLength(), 1, stderr);
    }

    return ret == 0 ? SLANG_OK : SLANG_FAIL;
}

}
