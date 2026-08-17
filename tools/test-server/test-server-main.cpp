// test-server.cpp

#include "compiler-core/slang-json-rpc-connection.h"
#include "compiler-core/slang-test-server-protocol.h"
#include "core/slang-io.h"
#include "core/slang-process-util.h"
#include "core/slang-secure-crt.h"
#include "core/slang-shared-library.h"
#include "core/slang-string-util.h"
#include "core/slang-string.h"
#include "core/slang-test-tool-util.h"
#include "core/slang-writer.h"
#include "gfx-unit-test/gfx-test-util.h"
#include "render-test/slang-support.h"
#include "slang-com-helper.h"
#include "slang-rhi.h"
#include "test-server-diagnostics.h"
#include "unit-test/slang-unit-test.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if SLANG_UNIX_FAMILY
#include <signal.h>
#endif

#if defined(_WIN32)
#include <slang-rhi/agility-sdk.h>
#include <windows.h>
SLANG_RHI_EXPORT_AGILITY_SDK
#endif

namespace TestServer
{
using namespace Slang;

#if defined(_WIN32)
static const UINT kParentMonitorFailedExitCode = 1;

// This monitor is Windows-only because issue #10109 is specifically about orphaned test-server
// processes holding DLLs open after slang-test crashes. Unix platforms do not prevent loaded
// shared libraries from being replaced in the same way.
static DWORD WINAPI _parentMonitorThreadProc(void* data)
{
    HANDLE parentProcess = (HANDLE)data;
    DWORD waitResult = WaitForSingleObject(parentProcess, INFINITE);
    CloseHandle(parentProcess);

    if (waitResult == WAIT_OBJECT_0)
    {
        // The RPC peer is gone, so graceful shutdown cannot be coordinated. Exit hard to release
        // DLL file handles promptly; Windows will reclaim the process resources.
        TerminateProcess(GetCurrentProcess(), 0);
    }

    return 0;
}

static void _signalParentMonitorReady(const char* readyEventName)
{
    if (!readyEventName || !readyEventName[0])
        return;

    OSString readyEventNameString = String(readyEventName).toWString();
    HANDLE readyEvent = OpenEventW(EVENT_MODIFY_STATE, FALSE, readyEventNameString.begin());
    if (readyEvent)
    {
        SetEvent(readyEvent);
        CloseHandle(readyEvent);
    }
}

static void _startParentMonitor(DWORD parentProcessId, const char* readyEventName)
{
    // Keep this scoped to test-server instead of changing shared process-launch plumbing. A
    // duplicated inheritable parent handle would remove PID reuse entirely, but Process::create
    // does not currently expose selective handle inheritance. The PID is captured immediately
    // before spawning this process and consumed during init, so the reuse window is tiny; if we
    // cannot open it at all, avoid leaving an unmonitored orphan.
    HANDLE parentProcess = OpenProcess(SYNCHRONIZE, FALSE, parentProcessId);
    if (!parentProcess)
    {
        TerminateProcess(GetCurrentProcess(), kParentMonitorFailedExitCode);
        return;
    }

    HANDLE thread = CreateThread(nullptr, 0, _parentMonitorThreadProc, parentProcess, 0, nullptr);
    if (!thread)
    {
        CloseHandle(parentProcess);
        TerminateProcess(GetCurrentProcess(), kParentMonitorFailedExitCode);
        return;
    }
    CloseHandle(thread);
    _signalParentMonitorReady(readyEventName);
}

static void _startParentMonitorFromArgs(int argc, const char* const* argv)
{
    bool hasParentProcessId = false;
    const char* parentProcessIdArg = nullptr;
    const char* readyEventName = nullptr;

    for (int i = 1; i < argc; ++i)
    {
        if (strcmp(argv[i], "-parent-pid") == 0)
        {
            hasParentProcessId = true;
            if (i + 1 >= argc)
                break;
            parentProcessIdArg = argv[++i];
            continue;
        }
        if (strcmp(argv[i], "-parent-monitor-ready-event") == 0)
        {
            if (i + 1 >= argc)
                break;
            readyEventName = argv[++i];
            continue;
        }
    }

    if (!hasParentProcessId)
        return;

    Int parentProcessId = 0;
    if (parentProcessIdArg &&
        SLANG_SUCCEEDED(
            StringUtil::parseInt(UnownedStringSlice(parentProcessIdArg), parentProcessId)) &&
        parentProcessId > 0 && parentProcessId <= Int(MAXDWORD))
    {
        _startParentMonitor(DWORD(parentProcessId), readyEventName);
        return;
    }

    TerminateProcess(GetCurrentProcess(), kParentMonitorFailedExitCode);
}
#endif

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
#include "slang-test/slangi-tool-impl.h"

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!! TestServer !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

SlangResult TestServer::init(int argc, const char* const* argv)
{
    m_exePath = argv[0];

#if defined(_WIN32)
    _startParentMonitorFromArgs(argc, argv);
#endif

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
    // Block waiting for content (or error/closed)
    SLANG_RETURN_ON_FAIL(m_connection->waitForResult());

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

            // Do different things
            if (call.method == TestServerProtocol::QuitArgs::g_methodName)
            {
                m_quit = true;
                return SLANG_OK;
            }
            else if (call.method == TestServerProtocol::ExecuteUnitTestArgs::g_methodName)
            {
                SLANG_RETURN_ON_FAIL(_executeUnitTest(call));
                return SLANG_OK;
            }
            else if (call.method == TestServerProtocol::ExecuteToolTestArgs::g_methodName)
            {
                SLANG_RETURN_ON_FAIL(_executeTool(call));
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
    auto id = m_connection->getPersistentValue(call.id);

    TestServerProtocol::ExecuteUnitTestArgs args;
    SLANG_RETURN_ON_FAIL(m_connection->toNativeArgsOrSendError(call.params, &args, call.id));

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
    auto rhiDebugCallback = renderer_test::createRetainedCoreToRHIDebugBridge();
    renderer_test::ScopedCoreDebugCallback scopedDebugCallback(
        *rhiDebugCallback,
        &coreDebugCallback);

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
    unitTestContext.debugCallback = rhiDebugCallback.Ptr();

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
    return m_connection->sendResult(&result, id);
}

SlangResult TestServer::_executeTool(const JSONRPCCall& call)
{
    auto id = m_connection->getPersistentValue(call.id);

    TestServerProtocol::ExecuteToolTestArgs args;

    SLANG_RETURN_ON_FAIL(m_connection->toNativeArgsOrSendError(call.params, &args, id));

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

    const SlangResult funcRes =
        func(&stdWriters, session, int(toolArgs.getCount()), toolArgs.begin());

    TestServerProtocol::ExecutionResult result;
    result.result = funcRes;
    result.stdError = stdError;
    result.stdOut = stdOut;
    result.debugLayer = debugCallback.getString();

    result.returnCode = int32_t(TestToolUtil::getReturnCode(result.result));
    return m_connection->sendResult(&result, id);
}

/// Hidden integration-test hook, in the same spirit as -parent-monitor-ready-event above.
///
/// SLANG_TEST_SERVER_DIE_ON_REQUEST=N makes the server vanish when the Nth request arrives,
/// leaving that request unanswered. That is the exact shape of the failure slang-test's
/// lost-server handling exists for -- a client blocked on a pipe that will never deliver --
/// and it is otherwise reproducible only by waiting for a server to die on its own, which is
/// the one thing nobody can schedule.
///
/// Returns 0 (disabled) for anything unset or unparseable, so a typo in the variable name
/// leaves the server behaving normally rather than dying on request one. Shared by the
/// exit-on-request and kill-on-request hooks.
static int _requestOrdinalFromEnv(const char* name)
{
    StringBuilder value;
    if (SLANG_FAILED(PlatformUtil::getEnvironmentVariable(UnownedStringSlice(name), value)))
    {
        return 0;
    }
    Int64 ordinal = 0;
    if (SLANG_FAILED(StringUtil::parseInt64(value.getUnownedSlice(), ordinal)) || ordinal <= 0)
    {
        return 0;
    }
    return int(ordinal);
}

static const char* _readStateName(HTTPPacketConnection::ReadState state)
{
    switch (state)
    {
    case HTTPPacketConnection::ReadState::Header:
        return "Header";
    case HTTPPacketConnection::ReadState::Content:
        return "Content";
    case HTTPPacketConnection::ReadState::Done:
        return "Done";
    case HTTPPacketConnection::ReadState::Closed:
        return "Closed";
    case HTTPPacketConnection::ReadState::Error:
        return "Error";
    default:
        return "?";
    }
}

/// Say why the server stopped serving, on the way out.
///
/// Leaving the serve loop means exiting 0 -- a clean, deliberate shutdown -- and from the
/// client's side that is indistinguishable from a crash: the pipe closes while it is waiting.
/// Seen in CI: a server exited with status 0 after answering 989 requests while slang-test
/// was still waiting on the 990th. An exit status alone cannot explain that, because there
/// is nothing wrong with the status.
///
/// The loop's own condition IS the explanation, so print it. `-quit` is an orderly shutdown
/// the client asked for; a Closed or Error read state is the server deciding unilaterally
/// that the session is over, which is the case worth catching -- the client is still there.
///
/// stderr, never stdout: stdout is the JSON-RPC channel, and a stray write to it would
/// corrupt the very protocol this is trying to explain.
static void _reportExitReason(JSONRPCConnection* connection, bool quitRequested, int servedCount)
{
    if (quitRequested)
    {
        // The client asked. Nothing to explain, and the common case -- stay quiet so the
        // interesting lines are not buried under one of these per server per run.
        return;
    }

    const char* stateName = "no-transport";
    if (auto* packetConnection = connection ? connection->getUnderlyingConnection() : nullptr)
    {
        stateName = _readStateName(packetConnection->getReadState());
    }

    fprintf(
        stderr,
        "test-server: leaving the serve loop after answering %d request(s) without being asked "
        "to quit; read state is %s. The client sees this as the server vanishing "
        "mid-request.\n",
        servedCount,
        stateName);
    fflush(stderr);
}

SlangResult TestServer::execute()
{
    const int dieOnRequest = _requestOrdinalFromEnv("SLANG_TEST_SERVER_DIE_ON_REQUEST");

    // Companion to the above that dies by SIGNAL rather than by exiting. The two are not
    // interchangeable: an exit is reported through getReturnValue(), a signal through
    // getTerminationSignal(), and those are separate paths on the client side -- the one
    // that turns "the server vanished" into "the OOM killer took it" is reachable ONLY this
    // way. Without it the headline diagnostic of this change has no test driving it, and a
    // regression in the WTERMSIG recording would pass CI in silence.
    const int killOnRequest = _requestOrdinalFromEnv("SLANG_TEST_SERVER_KILL_ON_REQUEST");

    // Third of the family and the only one where the server stays alive: it writes non-message
    // bytes into the channel before the Nth reply, so the client reads something it cannot
    // frame. That is the malformed-response shape (#12534) the protocol-error retry exists
    // for, and neither hook above can produce it.
    const int garbleOnRequest = _requestOrdinalFromEnv("SLANG_TEST_SERVER_GARBLE_ON_REQUEST");
    int servedCount = 0;

    while (m_connection->isActive() && !m_quit)
    {
        // Before _executeSingle, so the request is left unread and unanswered. The client has
        // already written it, so it sees EOF on a call it is waiting for -- a mid-request
        // loss, not a tidy shutdown.
        //
        // _Exit, not exit or return: running atexit handlers or unwinding would close the
        // connection in an orderly way, which is the case that is NOT interesting here.
        if (dieOnRequest && servedCount == dieOnRequest - 1)
        {
            _Exit(1);
        }

        // SIGKILL specifically: uncatchable, so no handler can soften it, and it is what the
        // OOM killer actually sends -- the case the client-side gloss names.
        //
        // Unix only, and that costs nothing: Windows has no signal deaths for the client to
        // report, since WinProcess reports every termination as an exit code. The variable is
        // simply inert there rather than conditionally compiled away at its read, so a
        // Windows run of the same test still exercises the ordinary exit path.
        if (killOnRequest && servedCount == killOnRequest - 1)
        {
#if SLANG_UNIX_FAMILY
            ::raise(SIGKILL);
#else
            _Exit(1);
#endif
        }

        // Straight to the fd, not through m_connection: the connection would frame it
        // correctly, which is the one thing this must not do. The real reply still follows.
        if (garbleOnRequest && servedCount == garbleOnRequest - 1)
        {
            const char garbage[] = "this-is-not-a-jsonrpc-header\r\n\r\n";
            fwrite(garbage, 1, sizeof(garbage) - 1, stdout);
            fflush(stdout);
        }

        // Failure doesn't make the execution terminate
        const SlangResult res = _executeSingle();
        if (SLANG_SUCCEEDED(res))
        {
            // Counted on success only, so it lines up with slang-test's own request ordinal
            // and means what it says. The iteration that discovers a dead connection has not
            // answered anything, and counting it would report "1 request" for a server that
            // served none -- which is exactly the confusion this number exists to resolve.
            servedCount++;
        }
    }

    _reportExitReason(m_connection, m_quit, servedCount);
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
