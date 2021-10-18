// main.cpp

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../source/core/slang-secure-crt.h"

#include "../../slang-com-helper.h"

#include "../../source/core/slang-string.h"
#include "../../source/core/slang-io.h"
#include "../../source/core/slang-writer.h"
#include "../../source/core/slang-string-util.h""

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

static SlangResult execute(int argc, const char* const* argv)
{
    typedef Slang::TestToolUtil::InnerMainFunc InnerMainFunc;

    if (argc < 2)
    {
        return SLANG_FAIL;
    }

    String exePath = Path::getParentDirectory(argv[0]);

    // The 'exeName' of the tool. 
    const String toolName = argv[1];

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

        auto testCount = testModule->getTestCount();
        SLANG_ASSERT(testIndex >= 0 && testIndex < testCount);

        UnitTestFunc testFunc = testModule->getTestFunc(testIndex);

        testFunc(&unitTestContext);

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
