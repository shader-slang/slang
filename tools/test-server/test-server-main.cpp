// test-server.cpp

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
#include "../../source/core/slang-http.h"

#include "../../source/compiler-core/slang-source-loc.h"
#include "../../source/compiler-core/slang-diagnostic-sink.h"

#include "../../source/compiler-core/slang-json-parser.h"
#include "../../source/compiler-core/slang-json-rpc.h"
#include "../../source/compiler-core/slang-json-value.h"

#include "tools/unit-test/slang-unit-test.h"

namespace TestServer
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

class TestServer
{
public:
    typedef Slang::TestToolUtil::InnerMainFunc InnerMainFunc;

    SlangResult init(int argc, const char* const* argv);

        /// Can return nullptr if cannot create the session
    slang::IGlobalSession* getGlobalSession();
        /// Can return nullptr if cannot load the tool
    ISlangSharedLibrary* loadSharedLibrary(const String& name);

        /// Get a unit test module. Returns nullptr if not found.
    IUnitTestModule* getUnitTestModule(const String& name);

        /// Given a tool name return it's function pointer. Or nullptr on failure.
    InnerMainFunc getToolFunction(const String& name);

        /// Execute the server
    SlangResult execute();

        /// Dtor
    ~TestServer();

protected:
    SlangResult _executeSingle();
    SlangResult _executeUnitTest(JSONContainer* container, const JSONValue& root);
    SlangResult _executeTool(JSONContainer* container, const JSONValue& root);

    bool m_quit = false;

    ComPtr<slang::IGlobalSession> m_session;

    Dictionary<String, ComPtr<ISlangSharedLibrary>> m_sharedLibraryMap;      ///< Maps tool names to the dll
    Dictionary<String, IUnitTestModule*> m_unitTestModules;

    String m_exePath;                                               ///< Path to executable

    DiagnosticSink m_diagnosticSink;
    SourceManager m_sourceManager;

    RefPtr<HTTPPacketConnection> m_connection;
};

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!! TestServer !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

SlangResult TestServer::init(int argc, const char* const* argv)
{
    m_exePath = Path::getParentDirectory(argv[0]);

    RefPtr<Stream> stdinStream, stdoutStream;

    Process::getStdStream(Process::StreamType::StdIn, stdinStream);
    Process::getStdStream(Process::StreamType::StdOut, stdoutStream);

    RefPtr<BufferedReadStream> readStream(new BufferedReadStream(stdinStream));

    m_connection = new HTTPPacketConnection(readStream, stdoutStream);

    m_sourceManager.initialize(nullptr, nullptr);
    m_diagnosticSink.init(&m_sourceManager, &JSONLexer::calcLexemeLocation);

    return SLANG_OK;
}

TestServer::~TestServer()
{
    for (auto& pair : m_unitTestModules)
    {
        pair.Value->destroy();
    }
}

slang::IGlobalSession* TestServer::getGlobalSession()
{
    if (!m_session)
    {
        // Just create the global session in the regular way if there isn't one set
        if (SLANG_FAILED(slang_createGlobalSession(SLANG_API_VERSION, m_session.writeRef())))
        {
            return nullptr;
        }
        TestToolUtil::setSessionDefaultPreludeFromExePath(m_exePath.getBuffer(), m_session);
    }

    return m_session;
}

ISlangSharedLibrary* TestServer::loadSharedLibrary(const String& name)
{
    auto keyPtr = m_sharedLibraryMap.TryGetValue(name);
    if (keyPtr)
    {
        return *keyPtr;
    }

    auto loader = DefaultSharedLibraryLoader::getSingleton();

    auto toolPath = Path::combine(m_exePath, name);

    ComPtr<ISlangSharedLibrary> sharedLibrary;
    if (SLANG_FAILED(loader->loadSharedLibrary(toolPath.getBuffer(), sharedLibrary.writeRef())))
    {
        return nullptr;
    }

    m_sharedLibraryMap.Add(name, sharedLibrary);
    return sharedLibrary;
}

IUnitTestModule* TestServer::getUnitTestModule(const String& name)
{
    auto unitTestModulePtr = m_unitTestModules.TryGetValue(name);
    if (unitTestModulePtr)
    {
        return *unitTestModulePtr;
    }

    ISlangSharedLibrary* sharedLibrary = loadSharedLibrary(name);
    if (!sharedLibrary)
    {
        return nullptr;
    }

    // get the unit test export name
    UnitTestGetModuleFunc getModuleFunc = (UnitTestGetModuleFunc)sharedLibrary->findFuncByName("slangUnitTestGetModule");
    if (!getModuleFunc)
        return nullptr;

    IUnitTestModule* testModule = getModuleFunc();
    if (!testModule)
        return nullptr;

    m_unitTestModules.Add(name, testModule);
    return testModule;
}

TestServer::InnerMainFunc TestServer::getToolFunction(const String& name)
{
    StringBuilder sharedLibToolBuilder;
    sharedLibToolBuilder.append(name);
    sharedLibToolBuilder.append("-tool");

    ISlangSharedLibrary* sharedLibrary = loadSharedLibrary(sharedLibToolBuilder);
    if (!sharedLibrary)
    {
        return nullptr;
    }

    return (InnerMainFunc)sharedLibrary->findFuncByName("innerMain");
}

SlangResult TestServer::_executeSingle()
{
    // Block waiting for content (or error/closed)
    SLANG_RETURN_ON_FAIL(m_connection->waitForResult());

    // If we don't have content, we can quit for now
    if (!m_connection->hasContent())
    {
        return SLANG_OK;
    }

    auto content = m_connection->getContent();

    UnownedStringSlice slice((const char*)content.begin(), content.getCount());

    // Reset for parse
    m_sourceManager.reset();
    m_diagnosticSink.reset();

    JSONContainer container(&m_sourceManager);

    // Parse as RPC JSON
    JSONValue root;

    {
        SlangResult res = JSONRPCUtil::parseJSON(slice, &container, &m_diagnosticSink, root);
        // Consume that content/packet
        m_connection->consumeContent();
        SLANG_RETURN_ON_FAIL(res);
    }

    String method;

    // Do different things
    if (method == "quit")
    {
        m_quit = true;
        return SLANG_OK;
    }
    else if (method == "unitTest")
    {
        SLANG_RETURN_ON_FAIL(_executeUnitTest(&container, root));
        return SLANG_OK;
    }
    else if (method == "tool")
    {
        SLANG_RETURN_ON_FAIL(_executeTool(&container, root));
        return SLANG_OK;
    }

    return SLANG_FAIL;
}

static Index _findTestIndex(IUnitTestModule* testModule, const String& name)
{
    const auto testCount = testModule->getTestCount();
    for (SlangInt i = 0; i < testCount; ++i)
    {
        auto testName = testModule->getTestName(i);

        if (name == testName)
        {
            return Index(i);
        }
    }
    return -1;
}

SlangResult TestServer::_executeUnitTest(JSONContainer* container, const JSONValue& root)
{
    String moduleName;
    String testName;
    Int enabledApis = 0;

    IUnitTestModule* testModule = getUnitTestModule(moduleName);
    if (!testModule)
    {
        return SLANG_FAIL;
    }

    Index testIndex = _findTestIndex(testModule, moduleName);
    if (testIndex < 0)
    {
        return SLANG_FAIL;
    }

    TestReporter testReporter;

    testModule->setTestReporter(&testReporter);
    
    // Assume we will used the shared session
    slang::IGlobalSession* session = getGlobalSession();
    if (!session)
    {
        return SLANG_FAIL;
    }

    UnitTestContext unitTestContext;
    unitTestContext.slangGlobalSession = session;
    unitTestContext.workDirectory = "";
    unitTestContext.enabledApis = RenderApiFlags(enabledApis);
    unitTestContext.executableDirectory = m_exePath.getBuffer();

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

SlangResult TestServer::_executeTool(JSONContainer* container, const JSONValue& root)
{
    String toolName;

    auto func = getToolFunction(toolName);
    if (!func)
    {
        // Write out to diagnostics
        return SLANG_FAIL;
    }

    // Get the args list

    // Work out the args sent to the shared library
    List<const char*> args;

    
    RefPtr<StdWriters> stdWriters = StdWriters::createDefault();

    const SlangResult res = func(stdWriters, session, int(args.getCount()), args.begin());
    if (SLANG_FAILED(res))
    {
        return res;
    }
    return res;
}


SlangResult TestServer::execute()
{
    DiagnosticSink diagnosticSink;

    while (m_connection->isActive() && !m_quit)
    {
        SlangResult res = _executeSingle();
        if (m_quit)
        {
            break;
        }

        if (SLANG_FAILED(res))
        {
            // Return a result 
        }
    }

    return SLANG_OK;
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! TestReporter !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

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


SlangResult _execute(int argc, const char* const* argv)
{
    TestServer server;
    SLANG_RETURN_ON_FAIL(server.init(argc, argv));
    SLANG_RETURN_ON_FAIL(server.execute());

    return SLANG_OK;
}

} // namespace TestServer

int main(int argc, const char* const* argv)
{
    return (int)Slang::TestToolUtil::getReturnCode(TestServer:: _execute(argc, argv));
}
