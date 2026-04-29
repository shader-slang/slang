// test-server.cpp

#include "../../source/compiler-core/slang-json-rpc-connection.h"
#include "../../source/compiler-core/slang-test-server-protocol.h"
#include "../../source/core/slang-io.h"
#include "../../source/core/slang-process-util.h"
#include "../../source/core/slang-secure-crt.h"
#include "../../source/core/slang-shared-library.h"
#include "../../source/core/slang-string-util.h"
#include "../../source/core/slang-string.h"
#include "../../source/core/slang-test-diagnostics.h"
#include "../../source/core/slang-test-tool-util.h"
#include "../../source/core/slang-writer.h"
#include "../render-test/slang-support.h"
#include "gfx-unit-test/gfx-test-util.h"
#include "slang-com-helper.h"
#include "slang-rhi.h"
#include "test-server-diagnostics.h"
#include "unit-test/slang-unit-test.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if SLANG_UNIX_FAMILY
#include <signal.h>
#endif

#if defined(_WIN32)
#include <slang-rhi/agility-sdk.h>
SLANG_RHI_EXPORT_AGILITY_SDK
#endif

namespace TestServer
{
using namespace Slang;

static double _getElapsedTimeInMs(uint64_t startTick)
{
    const uint64_t elapsedTicks = Process::getClockTick() - startTick;
    return (double(elapsedTicks) * 1000.0) / double(Process::getClockFrequency());
}

static bool _isRPCDiagnosticsEnabled()
{
    return isTestDiagnosticEnabled("rpc");
}

static bool _isTimingDiagnosticsEnabled()
{
    return isTestDiagnosticEnabled("timing") || isTestDiagnosticEnabled("timing-phases");
}

static bool _isTimingPhaseDiagnosticsEnabled()
{
    return isTestDiagnosticEnabled("timing-phases");
}

static void _vwriteDiagnostic(const char* prefix, const char* format, va_list args)
{
    fprintf(stderr, "%s ", prefix);
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");
    fflush(stderr);
}

static void _writeRPCDiagnostic(const char* format, ...)
{
    if (!_isRPCDiagnosticsEnabled())
        return;

    va_list args;
    va_start(args, format);
    _vwriteDiagnostic("[TEST-SERVER-RPC]", format, args);
    va_end(args);
}

static void _writeTimingDiagnostic(const char* format, ...)
{
    if (!_isTimingDiagnosticsEnabled())
        return;

    va_list args;
    va_start(args, format);
    _vwriteDiagnostic("[TEST-SERVER-TIMING]", format, args);
    va_end(args);
}

static void _writeTimingPhaseDiagnostic(const char* format, ...)
{
    if (!_isTimingPhaseDiagnosticsEnabled())
        return;

    va_list args;
    va_start(args, format);
    _vwriteDiagnostic("[TEST-SERVER-TIMING]", format, args);
    va_end(args);
}

class TestReporter : public ITestReporter
{
public:
    // ITestReporter
    virtual SLANG_NO_THROW void SLANG_MCALL startTest(const char* testName) SLANG_OVERRIDE {}
    virtual SLANG_NO_THROW void SLANG_MCALL addResult(TestResult result) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW void SLANG_MCALL
    addResultWithLocation(TestResult result, const char* testText, const char* file, int line)
        SLANG_OVERRIDE;
    virtual SLANG_NO_THROW void SLANG_MCALL
    addResultWithLocation(bool testSucceeded, const char* testText, const char* file, int line)
        SLANG_OVERRIDE;
    virtual SLANG_NO_THROW void SLANG_MCALL addExecutionTime(double time) SLANG_OVERRIDE {}
    virtual SLANG_NO_THROW void SLANG_MCALL message(TestMessageType type, const char* message)
        SLANG_OVERRIDE;
    virtual SLANG_NO_THROW void SLANG_MCALL endTest() SLANG_OVERRIDE {}

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
    slang::IGlobalSession* getOrCreateGlobalSession();

    /// Can return nullptr if cannot load the tool
    ISlangSharedLibrary* loadSharedLibrary(const String& name, DiagnosticSink* sink = nullptr);

    /// Get a unit test module. Returns nullptr if not found.
    IUnitTestModule* getUnitTestModule(const String& name, DiagnosticSink* sink = nullptr);

    /// Given a tool name return it's function pointer. Or nullptr on failure.
    InnerMainFunc getToolFunction(const String& name, DiagnosticSink* sink = nullptr);

    /// Execute the server
    SlangResult execute();

    /// Dtor
    ~TestServer();

protected:
    SlangResult _executeSingle();
    SlangResult _executeUnitTest(const JSONRPCCall& call);
    SlangResult _executeTool(const JSONRPCCall& root);

    bool m_quit = false;

    ComPtr<slang::IGlobalSession> m_session; /// The slang session. Is created on demand

    Dictionary<String, ComPtr<ISlangSharedLibrary>>
        m_sharedLibraryMap;                                 ///< Maps tool names to the dll
    Dictionary<String, IUnitTestModule*> m_unitTestModules; ///< All the unit test modules.

    String m_exePath;      ///< Path to executable (including exe name)
    String m_exeDirectory; ///< The directory that holds the exe

    RefPtr<JSONRPCConnection> m_connection; ///< RPC connection, recieves calls to execute and
                                            ///< returns results via JSON-RPC
};

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!! TestServer !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

namespace SlangCTool
{

static void _diagnosticCallback(char const* message, void* userData)
{
    ISlangWriter* writer = (ISlangWriter*)userData;
    writer->write(message, strlen(message));
}

SlangResult innerMain(
    StdWriters* stdWriters,
    slang::IGlobalSession* sharedSession,
    int argc,
    const char* const* argv)
{
    // Assume we will used the shared session
    ComPtr<slang::IGlobalSession> session(sharedSession);

    // The sharedSession always has a pre-loaded core module.
    // This differed test checks if the command line has an option to setup the core module.
    // If so we *don't* use the sharedSession, and create a new session without the core module just
    // for this compilation.
    if (TestToolUtil::hasDeferredCoreModule(Index(argc - 1), argv + 1))
    {
        SLANG_RETURN_ON_FAIL(
            slang_createGlobalSessionWithoutCoreModule(SLANG_API_VERSION, session.writeRef()));
    }

    ComPtr<slang::ICompileRequest> compileRequest;
    SLANG_ALLOW_DEPRECATED_BEGIN
    SLANG_RETURN_ON_FAIL(session->createCompileRequest(compileRequest.writeRef()));
    SLANG_ALLOW_DEPRECATED_END

    // Do any app specific configuration
    for (int i = 0; i < int{SLANG_WRITER_CHANNEL_COUNT_OF}; ++i)
    {
        const auto channel = SlangWriterChannel(i);
        compileRequest->setWriter(channel, stdWriters->getWriter(channel));
    }

    compileRequest->setDiagnosticCallback(
        &_diagnosticCallback,
        stdWriters->getWriter(SLANG_WRITER_CHANNEL_STD_ERROR));
    compileRequest->setCommandLineCompilerMode();

    {
        const SlangResult res = compileRequest->processCommandLineArguments(&argv[1], argc - 1);
        if (SLANG_FAILED(res))
        {
            // TODO: print usage message
            return res;
        }
    }

    SlangResult compileRes = SLANG_OK;

#ifndef _DEBUG
    try
#endif
    {
        // Run the compiler (this will produce any diagnostics through
        // SLANG_WRITER_TARGET_TYPE_DIAGNOSTIC).
        compileRes = compileRequest->compile();

        // If the compilation failed, then get out of here...
        // Turn into an internal Result -> such that return code can be used to vary result to match
        // previous behavior
        compileRes = SLANG_FAILED(compileRes) ? SLANG_E_INTERNAL_FAIL : compileRes;
    }
#ifndef _DEBUG
    catch (const Exception& e)
    {
        WriterHelper writerHelper(stdWriters->getWriter(SLANG_WRITER_CHANNEL_STD_OUTPUT));
        writerHelper.print("internal compiler error: %S\n", e.Message.toWString().begin());
        compileRes = SLANG_FAIL;
    }
#endif

    return compileRes;
}

} // namespace SlangCTool

// SlangITool
#include "../slang-test/slangi-tool-impl.h"

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!! TestServer !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

SlangResult TestServer::init(int argc, const char* const* argv)
{
    m_exePath = argv[0];

#if SLANG_IGNORE_ABORT_MSG && defined(_MSC_VER)
    // Suppress the modal abort() dialog in unattended/LLM-driven builds.
    _set_abort_behavior(0, _WRITE_ABORT_MSG);
#endif

    String canonicalPath;
    if (SLANG_SUCCEEDED(Path::getCanonical(m_exePath, canonicalPath)))
    {
        m_exeDirectory = Path::getParentDirectory(canonicalPath);
    }
    else
    {
        m_exeDirectory = Path::getParentDirectory(m_exePath);
    }

    m_connection = new JSONRPCConnection;
    SLANG_RETURN_ON_FAIL(m_connection->initWithStdStreams());
    return SLANG_OK;
}

TestServer::~TestServer()
{
    for (auto& [_, value] : m_unitTestModules)
        value->destroy();
}

slang::IGlobalSession* TestServer::getOrCreateGlobalSession()
{
    if (!m_session)
    {
        // Just create the global session in the regular way if there isn't one set
        SlangGlobalSessionDesc desc = {};
        desc.enableGLSL = true;
        if (SLANG_FAILED(slang_createGlobalSession2(&desc, m_session.writeRef())))
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
    if (m_sharedLibraryMap.tryGetValue(name, lib))
    {
        return lib;
    }

    auto loader = DefaultSharedLibraryLoader::getSingleton();

    ComPtr<ISlangSharedLibrary> sharedLibrary;
    if (SLANG_FAILED(loader->loadSharedLibrary(name.getBuffer(), sharedLibrary.writeRef())))
    {
        if (sink)
        {
            sink->diagnose(SourceLoc(), ServerDiagnostics::unableToLoadSharedLibrary, name);
        }

        return nullptr;
    }

    m_sharedLibraryMap.add(name, sharedLibrary);
    return sharedLibrary;
}

IUnitTestModule* TestServer::getUnitTestModule(const String& name, DiagnosticSink* sink)
{
    auto unitTestModulePtr = m_unitTestModules.tryGetValue(name);
    if (unitTestModulePtr)
    {
        return *unitTestModulePtr;
    }

    ISlangSharedLibrary* sharedLibrary = loadSharedLibrary(name, sink);
    if (!sharedLibrary)
    {
        return nullptr;
    }

    const char funcName[] = "slangUnitTestGetModule";

    // get the unit test export name
    UnitTestGetModuleFunc getModuleFunc =
        (UnitTestGetModuleFunc)sharedLibrary->findFuncByName(funcName);
    if (!getModuleFunc)
    {
        if (sink)
        {
            sink->diagnose(
                SourceLoc(),
                ServerDiagnostics::unableToFindFunctionInSharedLibrary,
                funcName);
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

    m_unitTestModules.add(name, testModule);
    return testModule;
}

TestServer::InnerMainFunc TestServer::getToolFunction(const String& name, DiagnosticSink* sink)
{
    if (name == "slangc")
    {
        return &SlangCTool::innerMain;
    }
    else if (name == "slangi")
    {
        return &SlangITool::innerMain;
    }

    StringBuilder sharedLibToolBuilder;
    sharedLibToolBuilder.append(name);
    sharedLibToolBuilder.append("-tool");

    ISlangSharedLibrary* sharedLibrary = loadSharedLibrary(sharedLibToolBuilder, sink);
    if (!sharedLibrary)
    {
        return nullptr;
    }

    const char funcName[] = "innerMain";

    auto func = (InnerMainFunc)sharedLibrary->findFuncByName(funcName);
    if (!func && sink)
    {
        sink->diagnose(
            SourceLoc(),
            ServerDiagnostics::unableToFindFunctionInSharedLibrary,
            funcName);
    }

    return func;
}

SlangResult TestServer::_executeSingle()
{
    const uint64_t waitStartTick = Process::getClockTick();
    _writeRPCDiagnostic("wait begin pid=%u", Process::getId());

    // Block waiting for content (or error/closed)
    SlangResult waitResult = m_connection->waitForResult();
    if (SLANG_FAILED(waitResult))
    {
        _writeRPCDiagnostic(
            "wait failed pid=%u elapsedMs=%.3f result=0x%08x",
            Process::getId(),
            _getElapsedTimeInMs(waitStartTick),
            uint32_t(waitResult));
        return waitResult;
    }

    _writeRPCDiagnostic(
        "wait complete pid=%u elapsedMs=%.3f hasMessage=%s",
        Process::getId(),
        _getElapsedTimeInMs(waitStartTick),
        m_connection->hasMessage() ? "true" : "false");

    // If we don't have a message, we can quit for now
    if (!m_connection->hasMessage())
    {
        return SLANG_OK;
    }

    const JSONRPCMessageType msgType = m_connection->getMessageType();

    switch (msgType)
    {
    case JSONRPCMessageType::Call:
        {
            JSONRPCCall call;
            SLANG_RETURN_ON_FAIL(m_connection->getRPCOrSendError(&call));
            String methodName(call.method);
            _writeRPCDiagnostic(
                "call received pid=%u method=%s",
                Process::getId(),
                methodName.getBuffer());

            // Do different things
            if (call.method == TestServerProtocol::QuitArgs::g_methodName)
            {
                _writeRPCDiagnostic("quit received pid=%u", Process::getId());
                m_quit = true;
                return SLANG_OK;
            }
            else if (call.method == TestServerProtocol::ExecuteUnitTestArgs::g_methodName)
            {
                const uint64_t dispatchStartTick = Process::getClockTick();
                _writeTimingPhaseDiagnostic(
                    "dispatch begin pid=%u method=%s",
                    Process::getId(),
                    methodName.getBuffer());

                SlangResult result = _executeUnitTest(call);
                _writeTimingDiagnostic(
                    "dispatch complete pid=%u method=%s elapsedMs=%.3f result=0x%08x",
                    Process::getId(),
                    methodName.getBuffer(),
                    _getElapsedTimeInMs(dispatchStartTick),
                    uint32_t(result));
                SLANG_RETURN_ON_FAIL(result);
                return SLANG_OK;
            }
            else if (call.method == TestServerProtocol::ExecuteToolTestArgs::g_methodName)
            {
                const uint64_t dispatchStartTick = Process::getClockTick();
                _writeTimingPhaseDiagnostic(
                    "dispatch begin pid=%u method=%s",
                    Process::getId(),
                    methodName.getBuffer());

                SlangResult result = _executeTool(call);
                _writeTimingDiagnostic(
                    "dispatch complete pid=%u method=%s elapsedMs=%.3f result=0x%08x",
                    Process::getId(),
                    methodName.getBuffer(),
                    _getElapsedTimeInMs(dispatchStartTick),
                    uint32_t(result));
                SLANG_RETURN_ON_FAIL(result);
                break;
            }
            else
            {
                return m_connection->sendError(JSONRPC::ErrorCode::MethodNotFound, call.id);
            }
        }
    default:
        {
            return m_connection->sendError(
                JSONRPC::ErrorCode::InvalidRequest,
                m_connection->getCurrentMessageId());
        }
    }

    return SLANG_OK;
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
    const uint64_t totalStartTick = Process::getClockTick();
    auto id = m_connection->getPersistentValue(call.id);

    TestServerProtocol::ExecuteUnitTestArgs args;
    SLANG_RETURN_ON_FAIL(m_connection->toNativeArgsOrSendError(call.params, &args, call.id));

    _writeRPCDiagnostic(
        "unit-test begin pid=%u module=%s test=%s",
        Process::getId(),
        args.moduleName.getBuffer(),
        args.testName.getBuffer());

    auto sink = m_connection->getSink();

    IUnitTestModule* testModule = getUnitTestModule(args.moduleName, m_connection->getSink());
    if (!testModule)
    {
        sink->diagnose(SourceLoc(), ServerDiagnostics::unableToFindUnitTestModule, args.moduleName);
        return m_connection->sendError(JSONRPC::ErrorCode::InvalidParams, id);
    }

    const Index testIndex = _findTestIndex(testModule, args.testName);
    if (testIndex < 0)
    {
        sink->diagnose(SourceLoc(), ServerDiagnostics::unableToFindTest, args.testName);
        return m_connection->sendError(JSONRPC::ErrorCode::InvalidParams, id);
    }

    TestReporter testReporter;
    renderer_test::CoreDebugCallback coreDebugCallback;
    renderer_test::CoreToRHIDebugBridge rhiDebugCallback;
    rhiDebugCallback.setCoreCallback(&coreDebugCallback);

    testModule->setTestReporter(&testReporter);

    // Assume we will used the shared session
    slang::IGlobalSession* session = getOrCreateGlobalSession();
    if (!session)
    {
        return SLANG_FAIL;
    }

    UnitTestContext unitTestContext;
    unitTestContext.slangGlobalSession = session;
    unitTestContext.workDirectory = "";
    unitTestContext.enabledApis = RenderApiFlags(args.enabledApis);
    unitTestContext.executableDirectory = m_exeDirectory.getBuffer();
    unitTestContext.enableDebugLayers = args.enableDebugLayers;
    unitTestContext.debugCallback = &rhiDebugCallback;

    auto testCount = testModule->getTestCount();
    SLANG_ASSERT(testIndex >= 0 && testIndex < testCount);

    UnitTestFunc testFunc = testModule->getTestFunc(testIndex);

    const uint64_t bodyStartTick = Process::getClockTick();
    _writeTimingPhaseDiagnostic(
        "unit-test body begin pid=%u module=%s test=%s",
        Process::getId(),
        args.moduleName.getBuffer(),
        args.testName.getBuffer());
    try
    {
        testFunc(&unitTestContext);
    }
    catch (...)
    {
        testReporter.m_failCount++;
    }
    _writeTimingDiagnostic(
        "unit-test body complete pid=%u module=%s test=%s elapsedMs=%.3f failures=%lld tests=%lld",
        Process::getId(),
        args.moduleName.getBuffer(),
        args.testName.getBuffer(),
        _getElapsedTimeInMs(bodyStartTick),
        (long long)testReporter.m_failCount,
        (long long)testReporter.m_testCount);

    TestServerProtocol::ExecutionResult result;
    result.result = SLANG_OK;
    result.debugLayer = coreDebugCallback.getString();

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

    const uint64_t sendStartTick = Process::getClockTick();
    _writeRPCDiagnostic(
        "unit-test send-result begin pid=%u module=%s test=%s returnCode=%d",
        Process::getId(),
        args.moduleName.getBuffer(),
        args.testName.getBuffer(),
        result.returnCode);
    SlangResult sendResult = m_connection->sendResult(&result, id);
    _writeTimingDiagnostic(
        "unit-test send-result complete pid=%u module=%s test=%s elapsedMs=%.3f result=0x%08x",
        Process::getId(),
        args.moduleName.getBuffer(),
        args.testName.getBuffer(),
        _getElapsedTimeInMs(sendStartTick),
        uint32_t(sendResult));
    _writeRPCDiagnostic(
        "unit-test complete pid=%u module=%s test=%s elapsedMs=%.3f result=0x%08x",
        Process::getId(),
        args.moduleName.getBuffer(),
        args.testName.getBuffer(),
        _getElapsedTimeInMs(totalStartTick),
        uint32_t(sendResult));
    return sendResult;
}

SlangResult TestServer::_executeTool(const JSONRPCCall& call)
{
    const uint64_t totalStartTick = Process::getClockTick();
    auto id = m_connection->getPersistentValue(call.id);

    TestServerProtocol::ExecuteToolTestArgs args;

    SLANG_RETURN_ON_FAIL(m_connection->toNativeArgsOrSendError(call.params, &args, id));

    _writeRPCDiagnostic(
        "tool begin pid=%u tool=%s argc=%lld",
        Process::getId(),
        args.toolName.getBuffer(),
        (long long)args.args.getCount());

    auto sink = m_connection->getSink();

    auto func = getToolFunction(args.toolName, sink);
    if (!func)
    {
        return m_connection->sendError(JSONRPC::ErrorCode::InvalidParams, id);
    }

    // Assume we will used the shared session
    slang::IGlobalSession* session = getOrCreateGlobalSession();
    if (!session)
    {
        return SLANG_FAIL;
    }

    // Work out the args sent to the shared library
    List<const char*> toolArgs;

    // Add the 'exe' name
    toolArgs.add(args.toolName.getBuffer());

    // Add the args
    for (const auto& arg : args.args)
    {
        toolArgs.add(arg.getBuffer());
    }

    StdWriters stdWriters;
    StringBuilder stdOut;
    StringBuilder stdError;
    renderer_test::CoreDebugCallback debugCallback;

    RefPtr<StringWriter> stdErrorWriter(new StringWriter(&stdError));
    // Use IsConsole on stdout because we have tests which output spirv
    // which we want to have disassembled
    RefPtr<StringWriter> stdOutWriter(new StringWriter(&stdOut, WriterFlag::IsConsole));

    stdWriters.setWriter(SLANG_WRITER_CHANNEL_STD_ERROR, stdErrorWriter);
    stdWriters.setWriter(SLANG_WRITER_CHANNEL_STD_OUTPUT, stdOutWriter);
    stdWriters.setDebugCallback(&debugCallback);

    // HACK, to make behavior the same as previously
    if (args.toolName == "slangc")
    {
        stdWriters.setWriter(SLANG_WRITER_CHANNEL_DIAGNOSTIC, stdErrorWriter);
    }

    const uint64_t bodyStartTick = Process::getClockTick();
    _writeTimingPhaseDiagnostic(
        "tool body begin pid=%u tool=%s",
        Process::getId(),
        args.toolName.getBuffer());
    const SlangResult funcRes =
        func(&stdWriters, session, int(toolArgs.getCount()), toolArgs.begin());
    _writeTimingDiagnostic(
        "tool body complete pid=%u tool=%s elapsedMs=%.3f result=0x%08x",
        Process::getId(),
        args.toolName.getBuffer(),
        _getElapsedTimeInMs(bodyStartTick),
        uint32_t(funcRes));

    TestServerProtocol::ExecutionResult result;
    result.result = funcRes;
    result.stdError = stdError;
    result.stdOut = stdOut;
    result.debugLayer = debugCallback.getString();

    result.returnCode = int32_t(TestToolUtil::getReturnCode(result.result));

    const uint64_t sendStartTick = Process::getClockTick();
    _writeRPCDiagnostic(
        "tool send-result begin pid=%u tool=%s returnCode=%d",
        Process::getId(),
        args.toolName.getBuffer(),
        result.returnCode);
    SlangResult sendResult = m_connection->sendResult(&result, id);
    _writeTimingDiagnostic(
        "tool send-result complete pid=%u tool=%s elapsedMs=%.3f result=0x%08x",
        Process::getId(),
        args.toolName.getBuffer(),
        _getElapsedTimeInMs(sendStartTick),
        uint32_t(sendResult));
    _writeRPCDiagnostic(
        "tool complete pid=%u tool=%s elapsedMs=%.3f result=0x%08x",
        Process::getId(),
        args.toolName.getBuffer(),
        _getElapsedTimeInMs(totalStartTick),
        uint32_t(sendResult));
    return sendResult;
}

SlangResult TestServer::execute()
{
    while (m_connection->isActive() && !m_quit)
    {
        // Failure doesn't make the execution terminate
        [[maybe_unused]] const SlangResult res = _executeSingle();
    }

    return SLANG_OK;
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! TestReporter !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

void TestReporter::message(TestMessageType type, const char* message)
{
    if (type == TestMessageType::RunError || type == TestMessageType::TestFailure)
    {
        m_failCount++;
    }

    m_buf << message << "\n";
}

void TestReporter::addResultWithLocation(
    TestResult result,
    const char* testText,
    const char* file,
    int line)
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

void TestReporter::addResultWithLocation(
    bool testSucceeded,
    const char* testText,
    const char* file,
    int line)
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

    // Clean up cached GPU devices before shutdown. The DeviceCache is a static
    // singleton in render-test-tool that holds Vulkan/CUDA devices. If not cleaned
    // up explicitly, its destructor runs during process exit (__run_exit_handlers)
    // after the GPU driver's own static destructors, causing segfaults from
    // corrupted vtables.
    typedef void (*CleanDeviceCacheFunc)();
    ISlangSharedLibrary* renderTestLib = server.loadSharedLibrary("render-test-tool");
    if (renderTestLib)
    {
        auto cleanFunc = (CleanDeviceCacheFunc)renderTestLib->findFuncByName("cleanDeviceCache");
        if (cleanFunc)
            cleanFunc();
    }

    slang::shutdown();
    return SLANG_OK;
}

} // namespace TestServer

int main(int argc, const char* const* argv)
{
#if SLANG_UNIX_FAMILY
    // Ignore SIGPIPE so that writing to a broken pipe returns EPIPE
    // instead of killing this process.
    signal(SIGPIPE, SIG_IGN);
#endif

    return (int)Slang::TestToolUtil::getReturnCode(TestServer::_execute(argc, argv));
}
