// main.cpp

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../source/core/slang-secure-crt.h"

#include "../../slang-com-helper.h"

#include "../../source/core/slang-string.h"
#include "../../source/core/slang-io.h"
#include "../../source/core/slang-writer.h"
#include "../../source/core/slang-string-util.h"
#include "../../source/core/slang-process-util.h"

#include "../../source/core/slang-shared-library.h"

#include "../../source/core/slang-test-tool-util.h"

#include "tools/unit-test/slang-unit-test.h"

namespace TestProxy
{
using namespace Slang;

class TestReporter : public ITestReporter
{
public:
    // ITestReporter
    virtual SLANG_NO_THROW void SLANG_MCALL startTest(const char* testName) SLANG_OVERRIDE { }
    virtual SLANG_NO_THROW void SLANG_MCALL addResult(TestResult result)SLANG_OVERRIDE;
    virtual SLANG_NO_THROW void SLANG_MCALL addResultWithLocation(TestResult result, const char* testText, const char* file, int line) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW void SLANG_MCALL addResultWithLocation(bool testSucceeded, const char* testText, const char* file, int line) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW void SLANG_MCALL addExecutionTime(double time) SLANG_OVERRIDE { }
    virtual SLANG_NO_THROW void SLANG_MCALL message(TestMessageType type, const char* message) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW void SLANG_MCALL endTest() SLANG_OVERRIDE { }

    StringBuilder m_buf;
    Index m_failCount = 0;
    Index m_testCount = 0;
};


void TestReporter::message(TestMessageType type, const char* message)
{
    if (type == TestMessageType::RunError ||
        type == TestMessageType::TestFailure)
    {
        m_failCount++;
    }

    m_buf << message << "\n";
}

void TestReporter::addResultWithLocation(TestResult result, const char* testText, const char* file, int line)
{
    if (result == TestResult::Fail)
    {
        addResultWithLocation(false, testText, file, line);
    }
    else
    {
        m_testCount++;
    }
}

void TestReporter::addResultWithLocation(bool testSucceeded, const char* testText, const char* file, int line)
{
    m_testCount++;

    if (testSucceeded)
    {
        return;
    }

    m_buf << "[Failed]: " << testText << "\n";
    m_buf << file << ":" << line << "\n";

    m_failCount++;
}

void TestReporter::addResult(TestResult result)
{
    if (result == TestResult::Fail)
    {
        m_failCount++;
    }
}

static SlangResult _createSlangSession(const char* exePath, int argc, const char* const* argv, ComPtr<slang::IGlobalSession>& outSession)
{
    
    ComPtr<slang::IGlobalSession> session;

    // The sharedSession always has a pre-loaded stdlib, is sharedSession is not nullptr.
    // This differed test checks if the command line has an option to setup the stdlib.
    // If so we *don't* use the sharedSession, and create a new stdlib-less session just for this compilation. 
    if (TestToolUtil::hasDeferredStdLib(argc, argv))
    {
        SLANG_RETURN_ON_FAIL(slang_createGlobalSessionWithoutStdLib(SLANG_API_VERSION, session.writeRef()));
        TestToolUtil::setSessionDefaultPreludeFromExePath(exePath, session);
    }
    else
    {
        // Just create the global session in the regular way if there isn't one set
        SLANG_RETURN_ON_FAIL(slang_createGlobalSession(SLANG_API_VERSION, session.writeRef()));
        TestToolUtil::setSessionDefaultPreludeFromExePath(exePath, session);
    }

    outSession.swap(session);
    return SLANG_OK;
}

static SlangResult _appendBuffer(FILE* file, List<Byte>& ioDst)
{
    const size_t expandSize = 1024;

    const Index prevCount = ioDst.getCount();
    ioDst.setCount(prevCount + expandSize);

    const size_t readSize = fread(ioDst.getBuffer() + prevCount, 1, expandSize, file);

    ioDst.setCount(prevCount + Index(readSize));
}

static SlangResult _appendBuffer(Stream* stream, List<Byte>& ioDst)
{
    const size_t expandSize = 1024;

    const Index prevCount = ioDst.getCount();
    ioDst.setCount(prevCount + expandSize);

    size_t readSize;
    SLANG_RETURN_ON_FAIL(stream->read(ioDst.getBuffer() + prevCount, expandSize, readSize));

    ioDst.setCount(prevCount + Index(readSize));
    return SLANG_OK;
}

static SlangResult _outputCount(Index count)
{
    FILE* fileOut = stdout;

    StringBuilder buf;
    for (Index i = 0; i < count; ++i)
    {
        buf.Clear();
        buf << i << "\n";

        fwrite(buf.getBuffer(), 1, buf.getLength(), fileOut);
    }

    // NOTE! There is no flush here, we want to see if everything works if we just stream out.
    return SLANG_OK;
}

static SlangResult _outputReflect()
{
    // Read lines from std input.
    // When hit line with just 'end', terminate

    // Get in as Stream

    RefPtr<Stream> stdinStream;
    Process::getStdStream(Process::StreamType::StdIn, stdinStream);

    FILE* fileOut = stdout;

    //fprintf(fileOut, "end\n");
    //fflush(fileOut);
    //return SLANG_OK;

#if 1
    
    List<Byte> buffer;

    Index lineCount = 0;
    Index startIndex = 0; 

    while (true)
    {
        SLANG_RETURN_ON_FAIL(_appendBuffer(stdinStream, buffer));

        while (true)
        {
            UnownedStringSlice slice((const char*)buffer.begin() + startIndex, (const char*)buffer.end());
            UnownedStringSlice line;

            if (!StringUtil::extractLine(slice, line) || slice.begin() == nullptr)
            {
                break;
            }

            // Process the line
            if (line == UnownedStringSlice::fromLiteral("end"))
            {
                return SLANG_OK;
            }

            // Write the text to the output stream
            fwrite(line.begin(), 1, line.getLength(), fileOut);
            fputc('\n', fileOut);
            fflush(fileOut);

            lineCount++;

            // Move the start index forward
            const Index newStartIndex = slice.begin() ? Index(slice.begin() - (const char*)buffer.getBuffer()) : buffer.getCount();
            SLANG_ASSERT(newStartIndex > startIndex);
            startIndex = newStartIndex;
        }
    }
#endif
}

static SlangResult execute(int argc, const char*const* argv)
{
    typedef Slang::TestToolUtil::InnerMainFunc InnerMainFunc;

    if (argc < 2)
    {
        return SLANG_FAIL;
    }

    String exePath = Path::getParentDirectory(argv[0]);

    // The 'exeName' of the tool. 
    const String toolName = argv[1];

    if (toolName == "reflect")
    {
        return _outputReflect();
    }

    if (toolName == "count")
    {
        if (argc < 3)
        {
            return SLANG_FAIL;
        }
        Index value = StringToInt(argv[2]);
        return _outputCount(value);
    }

    {
        StringBuilder sharedLibToolBuilder;
        sharedLibToolBuilder.append(toolName);
        sharedLibToolBuilder.append("-tool");

        auto toolPath = Path::combine(exePath, sharedLibToolBuilder);

        ComPtr<ISlangSharedLibrary> sharedLibrary;
        if (SLANG_SUCCEEDED(DefaultSharedLibraryLoader::getSingleton()->loadSharedLibrary(toolPath.getBuffer(), sharedLibrary.writeRef())))
        {
            // Assume we will used the shared session
            ComPtr<slang::IGlobalSession> session;
            SLANG_RETURN_ON_FAIL(_createSlangSession(argv[0], argc - 2, argv + 2, session));

            auto func = (InnerMainFunc)sharedLibrary->findFuncByName("innerMain");
            if (!func)
            {
                return SLANG_FAIL;
            }

            // Work out the args sent to the shared library
            List<const char*> args;
            args.add(argv[0]);
            args.addRange(argv + 2, Index(argc - 2));

            RefPtr<StdWriters> stdWriters = StdWriters::createDefault();

            const SlangResult res = func(stdWriters, session, int(args.getCount()), args.begin());
            return res;
        }
    }

    // Okay lets assume it's a unit test
    {
        auto toolPath = Path::combine(exePath, toolName);

        ComPtr<ISlangSharedLibrary> sharedLibrary;
        SLANG_RETURN_ON_FAIL(SLANG_SUCCEEDED(DefaultSharedLibraryLoader::getSingleton()->loadSharedLibrary(toolPath.getBuffer(), sharedLibrary.writeRef())));

        // get the unit test export name

        UnitTestGetModuleFunc getModuleFunc = (UnitTestGetModuleFunc)sharedLibrary->findFuncByName("slangUnitTestGetModule");
        if (!getModuleFunc)
            return SLANG_FAIL;

        IUnitTestModule* testModule = getModuleFunc();
        if (!testModule)
            return SLANG_FAIL;

        // argv[0] exe,
        // argv[1] tool
        // argv[2] test name
        // argv[3] test index
        // argv[4] the enabled apis

        if (argc < 5)
        {
            return SLANG_FAIL;
        }

        TestReporter testReporter;

        testModule->setTestReporter(&testReporter);

        Int enabledApis;
        SLANG_RETURN_ON_FAIL(StringUtil::parseInt(UnownedStringSlice(argv[4]), enabledApis));

        Int testIndex;
        SLANG_RETURN_ON_FAIL(StringUtil::parseInt(UnownedStringSlice(argv[3]), testIndex));

        // Assume we will used the shared session
        ComPtr<slang::IGlobalSession> session;
        SLANG_RETURN_ON_FAIL(_createSlangSession(argv[0], argc - 5, argv + 5, session));

        UnitTestContext unitTestContext;
        unitTestContext.slangGlobalSession = session;
        unitTestContext.workDirectory = "";
        unitTestContext.enabledApis = RenderApiFlags(enabledApis);
        unitTestContext.executableDirectory = exePath.getBuffer();

        auto testCount = testModule->getTestCount();
        SLANG_ASSERT(testIndex >= 0 && testIndex < testCount);

        UnitTestFunc testFunc = testModule->getTestFunc(testIndex);

        try
        {
            testFunc(&unitTestContext);
        }
        catch (...)
        {
            testReporter.m_failCount++;
        }

        if (testReporter.m_failCount > 0)
        {
            // Write out to stderr...
            auto writers = StdWriters::createDefault();
            writers->getError().put(testReporter.m_buf.getUnownedSlice());
            return SLANG_FAIL;
        }

        if (testReporter.m_testCount == 0)
        {
            return SLANG_E_NOT_AVAILABLE;
        }

        return SLANG_OK;
    }

    return SLANG_E_NOT_AVAILABLE;
}

} // namespace TestProxy

int main(int argc, const char* const* argv)
{
    using namespace TestProxy;
    SlangResult res = execute(argc, argv);
    return (int)TestToolUtil::getReturnCode(res);
}
