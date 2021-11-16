// unit-test-process.cpp

#include "../../source/core/slang-string-util.h"
#include "../../source/core/slang-process-util.h"
#include "../../source/core/slang-io.h"
#include "../../source/core/slang-http.h"
#include "../../source/core/slang-random-generator.h"

#include "tools/unit-test/slang-unit-test.h"

using namespace Slang;

static SlangResult _httpReflectTest(UnitTestContext* context)
{
    SlangResult finalRes = SLANG_OK;

    RefPtr<Process> process;

    {
        CommandLine cmdLine;
        cmdLine.setExecutable(context->executableDirectory, "test-proxy");
        cmdLine.addArg("http-reflect");
        SLANG_RETURN_ON_FAIL(Process::create(cmdLine, Process::Flag::AttachDebugger, process));
    }

    Stream* writeStream = process->getStream(Process::StreamType::StdIn);
    RefPtr<BufferedReadStream> readStream( new BufferedReadStream(process->getStream(Process::StreamType::StdOut)));

    RefPtr<HTTPPacketConnection> connection = new HTTPPacketConnection(readStream, writeStream);

    RefPtr<RandomGenerator> rand = RandomGenerator::create(10000);

    for (Index i = 0; i < 100; i++)
    {
        if (process->isTerminated())
        {
            return SLANG_FAIL;
        }

        const Index size = Index(rand->nextInt32UpTo(8192));

        List<Byte> buf;
        buf.setCount(size);
        // Build up a buffer
        rand->nextData(buf.getBuffer(), size_t(size));

        // Write the data
        SLANG_RETURN_ON_FAIL(connection->write(buf.getBuffer(), size_t(size)));

        // Wait for the response
        SLANG_RETURN_ON_FAIL(connection->waitForContent());

        auto readContent = connection->getContent();

        if (readContent != buf.getArrayView())
        {
            finalRes = SLANG_FAIL;
            break;
        }

        // Consume that packet
        connection->consumeContent();
    }

    // Send the end
    {
        const char end[] = "end";
        SLANG_RETURN_ON_FAIL(connection->write(end, SLANG_COUNT_OF(end) - 1));
    }

    process->waitForTermination();
    return finalRes;
}

static SlangResult _countTest(UnitTestContext* context, Index size, Index crashIndex = -1)
{
    RefPtr<Process> process;

    /* Here we are trying to test what happens if the server produces a large amount of data, and
    we just wait for termination. Do we receive all of the data irrespective of how much there is? */

    {
        CommandLine cmdLine;
        
        cmdLine.setExecutable(context->executableDirectory, "test-proxy");
        cmdLine.addArg("count");
        
        StringBuilder buf;
        buf << size;

        cmdLine.addArg(buf);

        if (crashIndex >= 0)
        {
            buf.Clear();
            buf << crashIndex;
            cmdLine.addArg(buf);
        }

        SLANG_RETURN_ON_FAIL(Process::create(cmdLine, Process::Flag::AttachDebugger, process));
    }

    ExecuteResult exeRes;

#if 0
    /* It does block on ~4k of data which matches up with the buffer size, using this mechanism only works up to 4k on windows
    which matches the default pipe buffer size */
    process->waitForTermination();
#endif

    SLANG_RETURN_ON_FAIL(ProcessUtil::readUntilTermination(process, exeRes));

    Index v = 0;
    for (auto line : LineParser(exeRes.standardOutput.getUnownedSlice()))
    {
        if (line.getLength() == 0)
        {
            continue;
        }

        Index value;
        StringUtil::parseInt(line, value);

        if (value != v)
        {
            return SLANG_FAIL;
        }

        v++;
    }

    const Index endIndex = (crashIndex >= 0) ? (crashIndex + 1) : size;

    return v == endIndex ? SLANG_OK : SLANG_FAIL;
}

static SlangResult _countTests(UnitTestContext* context)
{
    const Index sizes[] = { 1, 10, 1000, 1000, 10000, 100000 };
    for (auto size : sizes)
    {
        SLANG_RETURN_ON_FAIL(_countTest(context, size));
        SLANG_RETURN_ON_FAIL(_countTest(context, size, size / 2));
    }

    return SLANG_OK;
}

static SlangResult _reflectTest(UnitTestContext* context)
{
    RefPtr<Process> process;
    {
        CommandLine cmdLine;
        cmdLine.setExecutable(context->executableDirectory, "test-proxy");
        cmdLine.addArg("reflect");
        SLANG_RETURN_ON_FAIL(Process::create(cmdLine, Process::Flag::AttachDebugger, process));
    }

    // Write a bunch of stuff to the stream
    Stream* readStream = process->getStream(Process::StreamType::StdOut);
    Stream* writeStream = process->getStream(Process::StreamType::StdIn);

    List<Byte> readBuffer;

    for (Index i = 0; i < 10000; i++)
    {
        SLANG_RETURN_ON_FAIL(StreamUtil::read(readStream, 0, readBuffer));

        StringBuilder buf;

        buf << i << " Hello " << i << "\n";
        SLANG_RETURN_ON_FAIL(writeStream->write(buf.getBuffer(), buf.getLength()));
    }

    const char end[] = "end\n";
    SLANG_RETURN_ON_FAIL(writeStream->write(end, SLANG_COUNT_OF(end) - 1));
    writeStream->flush();

    SLANG_RETURN_ON_FAIL(StreamUtil::readAll(readStream, 0, readBuffer));

    return SLANG_OK;
}

SLANG_UNIT_TEST(CommandLineProcess)
{
    SLANG_CHECK(SLANG_SUCCEEDED(_countTests(unitTestContext)));

    SLANG_CHECK(SLANG_SUCCEEDED(_reflectTest(unitTestContext)));

    SLANG_CHECK(SLANG_SUCCEEDED(_httpReflectTest(unitTestContext)));
}
