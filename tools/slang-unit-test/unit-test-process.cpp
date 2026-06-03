// unit-test-process.cpp

#include "../../source/core/slang-http.h"
#include "../../source/core/slang-io.h"
#include "../../source/core/slang-platform.h"
#include "../../source/core/slang-process-util.h"
#include "../../source/core/slang-random-generator.h"
#include "../../source/core/slang-string-util.h"
#include "unit-test/slang-unit-test.h"

#if defined(_WIN32)
#include <wchar.h>
#include <windows.h>
// clang-format off
#include <tlhelp32.h>
// clang-format on
#endif

using namespace Slang;

static SlangResult _createProcess(
    UnitTestContext* context,
    const char* toolName,
    const List<String>* optArgs,
    RefPtr<Process>& outProcess)
{
    CommandLine cmdLine;
    cmdLine.setExecutableLocation(ExecutableLocation(context->executableDirectory, "test-process"));
    cmdLine.addArg(toolName);
    if (optArgs)
    {
        cmdLine.m_args.addRange(optArgs->getBuffer(), optArgs->getCount());
    }

    SLANG_RETURN_ON_FAIL(Process::create(cmdLine, Process::Flag::AttachDebugger, outProcess));

    return SLANG_OK;
}

static SlangResult _httpReflectTest(UnitTestContext* context)
{
    SlangResult finalRes = SLANG_OK;

    RefPtr<Process> process;
    SLANG_RETURN_ON_FAIL(_createProcess(context, "http-reflect", nullptr, process));

    Stream* writeStream = process->getStream(StdStreamType::In);
    RefPtr<BufferedReadStream> readStream(
        new BufferedReadStream(process->getStream(StdStreamType::Out)));
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
        SLANG_RETURN_ON_FAIL(connection->waitForResult());

        // If we don't have content then something has gone wrong
        if (!connection->hasContent())
        {
            finalRes = SLANG_FAIL;
            break;
        }

        // Check the content is the same
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

static SlangResult _httpCrashTest(UnitTestContext* context)
{
    RefPtr<Process> process;
    SLANG_RETURN_ON_FAIL(_createProcess(context, "http-crash", nullptr, process));

    Stream* writeStream = process->getStream(StdStreamType::In);
    RefPtr<BufferedReadStream> readStream(
        new BufferedReadStream(process->getStream(StdStreamType::Out)));
    RefPtr<HTTPPacketConnection> connection = new HTTPPacketConnection(readStream, writeStream);

    const char payload[] = "ping";
    SLANG_RETURN_ON_FAIL(connection->write(payload, SLANG_COUNT_OF(payload) - 1));

    // The server exits without replying; waitForResult should fail and no content should exist.
    if (SLANG_SUCCEEDED(connection->waitForResult(1000)))
    {
        return SLANG_FAIL;
    }
    if (connection->hasContent())
    {
        return SLANG_FAIL;
    }

    process->waitForTermination();
    return SLANG_OK;
}

#if defined(_WIN32)
struct ScopedWinHandle
{
    HANDLE handle = nullptr;

    ScopedWinHandle() {}
    explicit ScopedWinHandle(HANDLE inHandle)
        : handle(inHandle)
    {
    }
    ScopedWinHandle(const ScopedWinHandle&) = delete;
    ScopedWinHandle& operator=(const ScopedWinHandle&) = delete;
    bool isValid() const { return handle && handle != INVALID_HANDLE_VALUE; }
    ~ScopedWinHandle()
    {
        if (isValid())
        {
            CloseHandle(handle);
        }
    }
};

struct ScopedWinEnvironmentVariable
{
    const char* name = nullptr;
    String oldValue;
    bool hadOldValue = false;
    bool isSet = false;

    ScopedWinEnvironmentVariable(const char* inName, const String& value)
        : name(inName)
    {
        StringBuilder oldValueBuilder;
        hadOldValue = SLANG_SUCCEEDED(
            PlatformUtil::getEnvironmentVariable(UnownedStringSlice(name), oldValueBuilder));
        oldValue = oldValueBuilder.produceString();
        isSet = SetEnvironmentVariableA(name, value.getBuffer()) != 0;
    }

    ScopedWinEnvironmentVariable(const ScopedWinEnvironmentVariable&) = delete;
    ScopedWinEnvironmentVariable& operator=(const ScopedWinEnvironmentVariable&) = delete;
    ~ScopedWinEnvironmentVariable()
    {
        if (name && isSet)
        {
            SetEnvironmentVariableA(name, hadOldValue ? oldValue.getBuffer() : nullptr);
        }
    }
};

static void _terminateWindowsProcessAndWait(HANDLE process)
{
    if (process)
    {
        TerminateProcess(process, 0);
        WaitForSingleObject(process, INFINITE);
    }
}

static SlangResult _createWindowsProcess(const CommandLine& cmdLine, PROCESS_INFORMATION& out)
{
    STARTUPINFOW startupInfo;
    ZeroMemory(&startupInfo, sizeof(startupInfo));
    startupInfo.cb = sizeof(startupInfo);

    String cmdString = cmdLine.toString();
    OSString cmdStringBuffer = cmdString.toWString();
    List<wchar_t> mutableCmdString;
    for (const wchar_t* cursor = cmdStringBuffer.begin(); cursor != cmdStringBuffer.end(); ++cursor)
    {
        mutableCmdString.add(*cursor);
    }
    mutableCmdString.add(0);

    OSString pathBuffer;
    LPCWSTR path = nullptr;
    if (cmdLine.m_executableLocation.m_type == ExecutableLocation::Type::Path)
    {
        pathBuffer = cmdLine.m_executableLocation.m_pathOrName.toWString();
        path = pathBuffer.begin();
    }

    ZeroMemory(&out, sizeof(out));
    BOOL success = CreateProcessW(
        path,
        mutableCmdString.getBuffer(),
        nullptr,
        nullptr,
        FALSE,
        CREATE_NO_WINDOW,
        nullptr,
        nullptr,
        &startupInfo,
        &out);

    return success ? SLANG_OK : SLANG_FAIL;
}

static SlangResult _expectTestServerTerminates(
    UnitTestContext* context,
    const List<String>& args,
    int32_t expectedReturnCode)
{
    CommandLine serverCmdLine;
    serverCmdLine.setExecutableLocation(
        ExecutableLocation(context->executableDirectory, "test-server"));
    serverCmdLine.m_args.addRange(args.getBuffer(), args.getCount());

    RefPtr<Process> testServerProcess;
    SLANG_RETURN_ON_FAIL(Process::create(
        serverCmdLine,
        Process::Flag::AttachDebugger | Process::Flag::DisableStdErrRedirection,
        testServerProcess));

    if (!testServerProcess->waitForTermination(5 * 1000))
    {
        testServerProcess->kill(-1);
        return SLANG_FAIL;
    }
    if (testServerProcess->getReturnValue() != expectedReturnCode)
    {
        return SLANG_FAIL;
    }

    return SLANG_OK;
}

static SlangResult _parentMonitorFailureModeTests(UnitTestContext* context)
{
    {
        List<String> args;
        args.add("-parent-pid");
        args.add("abc");
        SLANG_RETURN_ON_FAIL(_expectTestServerTerminates(context, args, 1));
    }

    {
        List<String> args;
        args.add("-parent-pid");
        args.add("0");
        SLANG_RETURN_ON_FAIL(_expectTestServerTerminates(context, args, 1));
    }

    {
        List<String> args;
        args.add("-parent-pid");
        args.add("4294967296");
        SLANG_RETURN_ON_FAIL(_expectTestServerTerminates(context, args, 1));
    }

    {
        List<String> args;
        args.add("-parent-pid");
        args.add(String(uint32_t(MAXDWORD)));
        SLANG_RETURN_ON_FAIL(_expectTestServerTerminates(context, args, 1));
    }

    {
        List<String> args;
        args.add("-parent-pid");
        SLANG_RETURN_ON_FAIL(_expectTestServerTerminates(context, args, 1));
    }

    // Without -parent-pid, test-server should keep its normal RPC wait behavior.
    CommandLine serverCmdLine;
    serverCmdLine.setExecutableLocation(
        ExecutableLocation(context->executableDirectory, "test-server"));

    RefPtr<Process> testServerProcess;
    SLANG_RETURN_ON_FAIL(Process::create(
        serverCmdLine,
        Process::Flag::AttachDebugger | Process::Flag::DisableStdErrRedirection,
        testServerProcess));

    if (testServerProcess->waitForTermination(500))
    {
        return SLANG_FAIL;
    }
    testServerProcess->kill(0);

    return SLANG_OK;
}

static String _makeParentMonitorEventName(const char* label)
{
    StringBuilder builder;
    builder << "Local\\SlangParentMonitor-" << label << "-" << Process::getId() << "-"
            << Process::getClockTick();
    return builder.produceString();
}

static String _makeParentMonitorReadyEventName()
{
    return _makeParentMonitorEventName("ready");
}

static SlangResult _parentMonitorParentExitTest(UnitTestContext* context, bool terminateParent)
{
    String parentExitEventName = _makeParentMonitorEventName("parent-exit");
    OSString parentExitEventOSName = parentExitEventName.toWString();
    ScopedWinHandle parentExitEvent(
        CreateEventW(nullptr, TRUE, FALSE, parentExitEventOSName.begin()));
    if (!parentExitEvent.isValid())
        return SLANG_FAIL;

    CommandLine parentCmdLine;
    parentCmdLine.setExecutableLocation(
        ExecutableLocation(context->executableDirectory, "test-process"));
    parentCmdLine.addArg("wait-event");
    parentCmdLine.addArg(parentExitEventName);

    PROCESS_INFORMATION parentProcessInfo;
    SLANG_RETURN_ON_FAIL(_createWindowsProcess(parentCmdLine, parentProcessInfo));
    ScopedWinHandle parentProcess(parentProcessInfo.hProcess);
    ScopedWinHandle parentThread(parentProcessInfo.hThread);

    String readyEventName = _makeParentMonitorReadyEventName();
    OSString readyEventOSName = readyEventName.toWString();
    ScopedWinHandle readyEvent(CreateEventW(nullptr, TRUE, FALSE, readyEventOSName.begin()));
    if (!readyEvent.isValid())
    {
        _terminateWindowsProcessAndWait(parentProcess.handle);
        return SLANG_FAIL;
    }

    CommandLine serverCmdLine;
    serverCmdLine.setExecutableLocation(
        ExecutableLocation(context->executableDirectory, "test-server"));
    serverCmdLine.addArg("-parent-pid");
    serverCmdLine.addArg(String(uint32_t(parentProcessInfo.dwProcessId)));
    serverCmdLine.addArg("-parent-monitor-ready-event");
    serverCmdLine.addArg(readyEventName);

    RefPtr<Process> testServerProcess;
    SlangResult createServerResult = Process::create(
        serverCmdLine,
        Process::Flag::AttachDebugger | Process::Flag::DisableStdErrRedirection,
        testServerProcess);
    if (SLANG_FAILED(createServerResult))
    {
        _terminateWindowsProcessAndWait(parentProcess.handle);
        return createServerResult;
    }

    if (testServerProcess->isTerminated())
    {
        _terminateWindowsProcessAndWait(parentProcess.handle);
        return SLANG_FAIL;
    }

    if (WaitForSingleObject(readyEvent.handle, 5 * 1000) != WAIT_OBJECT_0 ||
        testServerProcess->isTerminated())
    {
        _terminateWindowsProcessAndWait(parentProcess.handle);
        return SLANG_FAIL;
    }

    if (terminateParent)
    {
        TerminateProcess(parentProcess.handle, 0);
    }
    else
    {
        if (!SetEvent(parentExitEvent.handle))
        {
            _terminateWindowsProcessAndWait(parentProcess.handle);
            return SLANG_FAIL;
        }
    }

    if (WaitForSingleObject(parentProcess.handle, 5 * 1000) != WAIT_OBJECT_0)
    {
        _terminateWindowsProcessAndWait(parentProcess.handle);
        return SLANG_FAIL;
    }

    if (!testServerProcess->waitForTermination(5 * 1000))
    {
        testServerProcess->kill(-1);
        return SLANG_FAIL;
    }
    if (testServerProcess->getReturnValue() != 0)
    {
        return SLANG_FAIL;
    }

    return SLANG_OK;
}

static SlangResult _parentMonitorTest(UnitTestContext* context)
{
    SLANG_RETURN_ON_FAIL(_parentMonitorParentExitTest(context, true));
    SLANG_RETURN_ON_FAIL(_parentMonitorParentExitTest(context, false));
    SLANG_RETURN_ON_FAIL(_parentMonitorFailureModeTests(context));
    return SLANG_OK;
}

static SlangResult _findChildTestServerProcessIds(DWORD parentProcessId, List<DWORD>& outProcessIds)
{
    ScopedWinHandle snapshot(CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0));
    if (!snapshot.isValid())
        return SLANG_FAIL;

    PROCESSENTRY32W entry;
    ZeroMemory(&entry, sizeof(entry));
    entry.dwSize = sizeof(entry);
    if (!Process32FirstW(snapshot.handle, &entry))
        return SLANG_FAIL;

    do
    {
        if (entry.th32ParentProcessID == parentProcessId &&
            _wcsicmp(entry.szExeFile, L"test-server.exe") == 0)
        {
            outProcessIds.add(entry.th32ProcessID);
        }
    } while (Process32NextW(snapshot.handle, &entry));

    return SLANG_OK;
}

static void _closeWindowsHandles(List<HANDLE>& handles)
{
    for (HANDLE handle : handles)
    {
        if (handle)
        {
            CloseHandle(handle);
        }
    }
    handles.clear();
}

static SlangResult _testServerParentMonitorIntegrationTest(UnitTestContext* context)
{
    String readyEventName = _makeParentMonitorReadyEventName();
    OSString readyEventOSName = readyEventName.toWString();
    ScopedWinHandle readyEvent(CreateEventW(nullptr, TRUE, FALSE, readyEventOSName.begin()));
    if (!readyEvent.isValid())
        return SLANG_FAIL;

    CommandLine slangTestCmdLine;
    slangTestCmdLine.setExecutableLocation(
        ExecutableLocation(context->executableDirectory, "slang-test"));
    slangTestCmdLine.addArg("-use-test-server");
    slangTestCmdLine.addArg("-server-count");
    slangTestCmdLine.addArg("1");
    slangTestCmdLine.addArg("slang-unit-test-tool/CommandLineProcess");

    PROCESS_INFORMATION slangTestProcessInfo;
    {
        ScopedWinEnvironmentVariable readyEventEnv(
            "SLANG_TEST_PARENT_MONITOR_READY_EVENT",
            readyEventName);
        if (!readyEventEnv.isSet)
            return SLANG_FAIL;

        SLANG_RETURN_ON_FAIL(_createWindowsProcess(slangTestCmdLine, slangTestProcessInfo));
    }

    ScopedWinHandle slangTestProcess(slangTestProcessInfo.hProcess);
    ScopedWinHandle slangTestThread(slangTestProcessInfo.hThread);
    List<HANDLE> testServerHandles;
    SlangResult result = SLANG_OK;

    if (WaitForSingleObject(readyEvent.handle, 30 * 1000) != WAIT_OBJECT_0)
    {
        result = SLANG_FAIL;
    }

    if (SLANG_SUCCEEDED(result))
    {
        List<DWORD> testServerProcessIds;
        result =
            _findChildTestServerProcessIds(slangTestProcessInfo.dwProcessId, testServerProcessIds);
        if (SLANG_SUCCEEDED(result) && testServerProcessIds.getCount() == 0)
        {
            result = SLANG_FAIL;
        }

        for (DWORD processId : testServerProcessIds)
        {
            HANDLE process = OpenProcess(SYNCHRONIZE | PROCESS_TERMINATE, FALSE, processId);
            if (!process)
            {
                result = SLANG_FAIL;
                break;
            }
            testServerHandles.add(process);
        }
    }

    TerminateProcess(slangTestProcess.handle, 0);
    WaitForSingleObject(slangTestProcess.handle, INFINITE);

    if (SLANG_SUCCEEDED(result))
    {
        for (HANDLE testServerHandle : testServerHandles)
        {
            if (WaitForSingleObject(testServerHandle, 5 * 1000) != WAIT_OBJECT_0)
            {
                TerminateProcess(testServerHandle, 0);
                result = SLANG_FAIL;
            }
        }
    }

    _closeWindowsHandles(testServerHandles);
    return result;
}
#endif

static SlangResult _countTest(UnitTestContext* context, Index size, Index crashIndex = -1)
{
    /* Here we are trying to test what happens if the server produces a large amount of data, and
    we just wait for termination. Do we receive all of the data irrespective of how much there is?
  */

    List<String> args;
    {
        StringBuilder buf;
        buf << size;

        args.add(buf);

        if (crashIndex >= 0)
        {
            buf.clear();
            buf << crashIndex;
            args.add(buf);
        }
    }

    RefPtr<Process> process;
    SLANG_RETURN_ON_FAIL(_createProcess(context, "count", &args, process));

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
    const Index sizes[] = {1, 10, 1000, 1000, 10000, 100000};
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
    SLANG_RETURN_ON_FAIL(_createProcess(context, "reflect", nullptr, process));

    // Write a bunch of stuff to the stream
    Stream* readStream = process->getStream(StdStreamType::Out);
    Stream* writeStream = process->getStream(StdStreamType::In);

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
    SLANG_CHECK(SLANG_SUCCEEDED(_httpCrashTest(unitTestContext)));
#if defined(_WIN32)
    SLANG_CHECK(SLANG_SUCCEEDED(_parentMonitorTest(unitTestContext)));
#endif
}

#if defined(_WIN32)
SLANG_UNIT_TEST(TestServerParentMonitorIntegration)
{
    SLANG_CHECK(SLANG_SUCCEEDED(_testServerParentMonitorIntegrationTest(unitTestContext)));
}
#endif
