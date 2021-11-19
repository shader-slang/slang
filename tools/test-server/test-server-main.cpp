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
#include "../../source/compiler-core/slang-json-native.h"
#include "../../source/compiler-core/slang-json-rpc.h"

#include "../../source/compiler-core/slang-test-server-protocol.h"

#include "test-server-diagnostics.h"

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
    ISlangSharedLibrary* loadSharedLibrary(const String& name, DiagnosticSink* sink = nullptr);

        /// Get a unit test module. Returns nullptr if not found.
    IUnitTestModule* getUnitTestModule(const String& name, DiagnosticSink* sink = nullptr);

        /// Given a tool name return it's function pointer. Or nullptr on failure.
    InnerMainFunc getToolFunction(const String& name, DiagnosticSink* sink = nullptr);

        /// Execute the server
    SlangResult execute();

        /// 
    TestServer();
        /// Dtor
    ~TestServer();

protected:
    SlangResult _executeSingle();
    SlangResult _executeUnitTest(const JSONRPCCall& call);
    SlangResult _executeTool(const JSONRPCCall& root);

    SlangResult _writeResponse(const RttiInfo* info, const void* data);

    template <typename T>
    SlangResult _writeResponse(const T* data)
    {
        return _writeResponse(GetRttiInfo<T>::get(), (const void*)data);
    }

        /// Will write response on fail
    SlangResult _toNativeOrRespond(const JSONValue& value, const RttiInfo* info, void* dst);
    template <typename T>
    SlangResult _toNativeOrRespond(const JSONValue& value, T* data)
    {
        return _toNativeOrRespond(value, GetRttiInfo<T>::get(), data);
    }
    template <typename T>
    SlangResult _toValidNativeOrRespond(const JSONValue& value, T* data)
    {
        const RttiInfo* rttiInfo = GetRttiInfo<T>::get();

        SLANG_RETURN_ON_FAIL(_toNativeOrRespond(value, rttiInfo, (void*)data));
        if (!data->isValid())
        {
            // If it has a name add validation info
            if (rttiInfo->isNamed())
            {
                const NamedRttiInfo* namedRttiInfo = static_cast<const NamedRttiInfo*>(rttiInfo);
                m_diagnosticSink.diagnose(SourceLoc(), ServerDiagnostics::argsAreInvalid, namedRttiInfo->m_name);
            }

            return _respondWithError(JSONRPC::ErrorCode::InvalidRequest);
        }
        return SLANG_OK;
    }

    SlangResult _respondWithError(JSONRPC::ErrorCode code);
    SlangResult _respondWithErrorMessage(JSONRPC::ErrorCode errorCode, const UnownedStringSlice& msg);

    bool m_quit = false;

    ComPtr<slang::IGlobalSession> m_session;

    Dictionary<String, ComPtr<ISlangSharedLibrary>> m_sharedLibraryMap;      ///< Maps tool names to the dll
    Dictionary<String, IUnitTestModule*> m_unitTestModules;

    String m_exePath;                                               ///< Path to executable

    DiagnosticSink m_diagnosticSink;
    SourceManager m_sourceManager;
    JSONContainer m_container;

    JSONValue m_jsonRoot;

    RefPtr<HTTPPacketConnection> m_connection;
};

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!! TestServer !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

TestServer::TestServer() :
    m_container(nullptr)
{
}

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
    m_container.setSourceManager(&m_sourceManager);

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

ISlangSharedLibrary* TestServer::loadSharedLibrary(const String& name, DiagnosticSink* sink)
{
    ComPtr<ISlangSharedLibrary> lib;
    if (m_sharedLibraryMap.TryGetValue(name, lib))
    {
        return lib;
    }

    auto loader = DefaultSharedLibraryLoader::getSingleton();

    auto toolPath = Path::combine(m_exePath, name);

    ComPtr<ISlangSharedLibrary> sharedLibrary;
    if (SLANG_FAILED(loader->loadSharedLibrary(toolPath.getBuffer(), sharedLibrary.writeRef())))
    {
        if (sink)
        {
            sink->diagnose(SourceLoc(), ServerDiagnostics::unableToLoadSharedLibrary, name);
        }

        return nullptr;
    }

    m_sharedLibraryMap.Add(name, sharedLibrary);
    return sharedLibrary;
}

IUnitTestModule* TestServer::getUnitTestModule(const String& name, DiagnosticSink* sink)
{
    auto unitTestModulePtr = m_unitTestModules.TryGetValue(name);
    if (unitTestModulePtr)
    {
        return *unitTestModulePtr;
    }

    ISlangSharedLibrary* sharedLibrary = loadSharedLibrary(name, sink);
    if (!sharedLibrary)
    {
        return nullptr;
    }

    UnownedStringSlice funcName = UnownedStringSlice::fromLiteral("slangUnitTestGetModule");

    // get the unit test export name
    UnitTestGetModuleFunc getModuleFunc = (UnitTestGetModuleFunc)sharedLibrary->findFuncByName(funcName.begin());
    if (!getModuleFunc)
    {
        if (sink)
        {
            sink->diagnose(SourceLoc(), ServerDiagnostics::unableToFindFunctionInSharedLibrary, funcName);
        }
        return nullptr;
    }

    IUnitTestModule* testModule = getModuleFunc();
    if (!testModule)
    {
        if (sink)
        {
            sink->diagnose(SourceLoc(), ServerDiagnostics::unableToGetUnitTestModule);
        }
        return nullptr;
    }

    m_unitTestModules.Add(name, testModule);
    return testModule;
}

TestServer::InnerMainFunc TestServer::getToolFunction(const String& name, DiagnosticSink* sink)
{
    StringBuilder sharedLibToolBuilder;
    sharedLibToolBuilder.append(name);
    sharedLibToolBuilder.append("-tool");

    ISlangSharedLibrary* sharedLibrary = loadSharedLibrary(sharedLibToolBuilder, sink);
    if (!sharedLibrary)
    {
        return nullptr;
    }

    UnownedStringSlice funcName = UnownedStringSlice::fromLiteral("innerMain");

    auto func = (InnerMainFunc)sharedLibrary->findFuncByName(funcName.begin());
    if (!func && sink)
    {
        sink->diagnose(SourceLoc(), ServerDiagnostics::unableToFindFunctionInSharedLibrary, funcName);
    }
    
    return func;
}

SlangResult TestServer::_writeResponse(const RttiInfo* rttiInfo, const void* data)
{
    // Convert to JSON
    NativeToJSONConverter converter(&m_container, &m_diagnosticSink);
    JSONValue value;
    SLANG_RETURN_ON_FAIL(converter.convert(rttiInfo, data, value));

    // Convert to text
    JSONWriter writer(JSONWriter::IndentationStyle::Allman);
    m_container.traverseRecursively(value, &writer);
    const StringBuilder& builder = writer.getBuilder();
    return m_connection->write(builder.getBuffer(), builder.getLength());
}

SlangResult TestServer::_respondWithError(JSONRPC::ErrorCode code)
{
    JSONRPCErrorResponse errorResponse;
    errorResponse.error.code = Int(JSONRPC::ErrorCode::InvalidRequest);
    errorResponse.error.message = m_diagnosticSink.outputBuffer.getUnownedSlice();
    errorResponse.id = JSONRPCUtil::getId(&m_container, m_jsonRoot);
    return _writeResponse(&errorResponse);
}

SlangResult TestServer::_respondWithErrorMessage(JSONRPC::ErrorCode errorCode, const UnownedStringSlice& msg)
{
    JSONRPCErrorResponse errorResponse;
    errorResponse.error.code = Int(JSONRPC::ErrorCode::InvalidRequest);
    errorResponse.error.message = msg;
    errorResponse.id = JSONRPCUtil::getId(&m_container, m_jsonRoot);
    return _writeResponse(&errorResponse);
}

SlangResult TestServer::_toNativeOrRespond(const JSONValue& value, const RttiInfo* info, void* dst)
{
    m_diagnosticSink.outputBuffer.Clear();
    if (SLANG_FAILED(JSONRPCUtil::convertToNative(&m_container, value, &m_diagnosticSink, info, dst)))
    {
        return _respondWithError(JSONRPC::ErrorCode::InvalidRequest);
    }
    return SLANG_OK;
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
    m_container.reset();
    m_jsonRoot.reset();

    {
        const SlangResult res = JSONRPCUtil::parseJSON(slice, &m_container, &m_diagnosticSink, m_jsonRoot);

        // Consume that content/packet
        m_connection->consumeContent();

        if (SLANG_FAILED(res))
        {
            return _respondWithError(JSONRPC::ErrorCode::ParseError);
        }
    }

    // Get the call
    JSONRPCCall call;
    SLANG_RETURN_ON_FAIL(_toValidNativeOrRespond(m_jsonRoot, &call));

    // Do different things
    if (call.method == "quit")
    {
        m_quit = true;
        return SLANG_OK;
    }
    else if (call.method == "unitTest")
    {
        SLANG_RETURN_ON_FAIL(_executeUnitTest(call));
        return SLANG_OK;
    }
    else if (call.method == "tool")
    {
        SLANG_RETURN_ON_FAIL(_executeTool(call));
        return SLANG_OK;
    }

    return _respondWithError(JSONRPC::ErrorCode::MethodNotFound);
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

SlangResult TestServer::_executeUnitTest(const JSONRPCCall& call)
{
    TestServerProtocol::ExecuteUnitTestArgs args;
    SLANG_RETURN_ON_FAIL(_toNativeOrRespond(call.params, &args));

    IUnitTestModule* testModule = getUnitTestModule(args.moduleName, &m_diagnosticSink);
    if (!testModule)
    {
        m_diagnosticSink.diagnose(SourceLoc(), ServerDiagnostics::unableToFindUnitTestModule, args.moduleName);
        return _respondWithError(JSONRPC::ErrorCode::InvalidParams);
    }

    const Index testIndex = _findTestIndex(testModule, args.testName);
    if (testIndex < 0)
    {
        m_diagnosticSink.diagnose(SourceLoc(), ServerDiagnostics::unableToFindTest, args.testName);
        return _respondWithError(JSONRPC::ErrorCode::InvalidParams);
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
    unitTestContext.enabledApis = RenderApiFlags(args.enabledApis);
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

    TestServerProtocol::ExecutionResult result;
    result.result = SLANG_OK;

    if (testReporter.m_failCount > 0)
    {
        result.result = SLANG_FAIL;
        result.stdError = testReporter.m_buf.getUnownedSlice();
    }
    else if (testReporter.m_testCount == 0)
    {
        result.result = SLANG_E_NOT_AVAILABLE;
    }

    result.returnCode = int32_t(TestToolUtil::getReturnCode(result.result));
    return _writeResponse(&result);
}

SlangResult TestServer::_executeTool(const JSONRPCCall& call)
{
    TestServerProtocol::ExecuteToolTestArgs args;
    SLANG_RETURN_ON_FAIL(_toNativeOrRespond(call.params, &args));

    auto func = getToolFunction(args.toolName, &m_diagnosticSink);
    if (!func)
    {
        return _respondWithError(JSONRPC::ErrorCode::InvalidParams);
    }

    // Assume we will used the shared session
    slang::IGlobalSession* session = getGlobalSession();
    if (!session)
    {
        return SLANG_FAIL;
    }

    // Work out the args sent to the shared library
    List<const char*> toolArgs;
    for (const auto& arg : args.args)
    {
        toolArgs.add(arg.getBuffer());
    }

    StdWriters stdWriters;
    StringBuilder stdOut;
    StringBuilder stdError;

    RefPtr<StringWriter> stdOutWriter(new StringWriter(&stdOut, 0));
    RefPtr<StringWriter> stdErrorWriter(new StringWriter(&stdError, 0));

    stdWriters.setWriter(SLANG_WRITER_CHANNEL_STD_ERROR, stdErrorWriter);
    stdWriters.setWriter(SLANG_WRITER_CHANNEL_STD_OUTPUT, stdOutWriter);

    // HACK, to make behavior the same as previously
    if (args.toolName== "slangc")
    {
        stdWriters.setWriter(SLANG_WRITER_CHANNEL_DIAGNOSTIC, stdErrorWriter);
    }

    const SlangResult funcRes = func(&stdWriters, session, int(toolArgs.getCount()), toolArgs.begin());

    TestServerProtocol::ExecutionResult result;
    result.result = funcRes;
    result.stdError = stdError;
    result.stdOut = stdOut;

    result.returnCode = int32_t(TestToolUtil::getReturnCode(result.result));
    return _writeResponse(&result);
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
