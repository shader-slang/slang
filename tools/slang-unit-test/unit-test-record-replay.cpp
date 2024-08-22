// unit-test-record-replay.cpp

#include "../../source/core/slang-string-util.h"
#include "../../source/core/slang-process-util.h"

#include "../../source/core/slang-io.h"
#include "../../source/core/slang-http.h"
#include "../../source/core/slang-random-generator.h"

#include "tools/unit-test/slang-unit-test.h"

#ifdef _WIN32
#include <windows.h>
#include <shellapi.h>
#endif

using namespace Slang;

static SlangResult createProcess(UnitTestContext* context, const char* processName, const List<String>* optArgs, RefPtr<Process>& outProcess)
{
    CommandLine cmdLine;
    cmdLine.setExecutableLocation(ExecutableLocation(context->executableDirectory, processName));
    if (optArgs)
    {
        cmdLine.m_args.addRange(optArgs->getBuffer(), optArgs->getCount());
    }

    SLANG_RETURN_ON_FAIL(Process::create(cmdLine, Process::Flag::AttachDebugger, outProcess));

    return SLANG_OK;
}

struct entryHashInfo
{
    int64_t targetIndex = -1;
    int64_t entryPointIndex = -1;
    String hash;
};

static SlangResult parseHashes(List<String> const& lines, List<entryHashInfo>& outHashes)
{
    SlangResult res = SLANG_OK;

    for (const auto& line : lines)
    {
        List<UnownedStringSlice> tokens;
        Index skipCharacters = line.indexOf(UnownedStringSlice("[slang-record-replay]:"));
        if (skipCharacters == -1)
        {
            skipCharacters = 0;
        }
        else
        {
            skipCharacters += strlen("[slang-record-replay]:");
        }
        StringUtil::split(UnownedStringSlice(line.getBuffer() + skipCharacters), ',', tokens);

        if (tokens.getCount() != 3)
        {
            return SLANG_FAIL;
        }

        entryHashInfo hashInfo;
        auto extractToken = [](const UnownedStringSlice& token, const char splitChar, UnownedStringSlice& outToken) -> SlangResult
        {
            List<UnownedStringSlice> subTokens;
            StringUtil::split(token, splitChar, subTokens);
            if (subTokens.getCount() != 2)
            {
                return SLANG_FAIL;
            }
            outToken = subTokens[1];
            return SLANG_OK;
        };

        {
            UnownedStringSlice subToken;
            SLANG_RETURN_ON_FAIL(extractToken(tokens[0], ':', subToken));
            int64_t outNumer = 0;
            StringUtil::parseInt64(subToken, outNumer);
            hashInfo.entryPointIndex = outNumer;
        }

        {
            UnownedStringSlice subToken;
            SLANG_RETURN_ON_FAIL(extractToken(tokens[1], ':', subToken));
            int64_t outNumer = 0;
            StringUtil::parseInt64(subToken, outNumer);
            hashInfo.targetIndex = outNumer;
        }

        {
            UnownedStringSlice subToken;
            SLANG_RETURN_ON_FAIL(extractToken(tokens[2], ':', subToken));
            hashInfo.hash = subToken.begin() + 1;
        }

        outHashes.add(hashInfo);
    }
    return res;
}

static int writeEnvironmentVariable(char* var)
{
#ifdef _WIN32
    return _putenv(var);
#else
    return putenv(var);
#endif
}

static bool enableRecordLayer()
{
    int retCode = writeEnvironmentVariable("SLANG_RECORD_LAYER=1");
    return retCode == 0;
}

static bool disableRecordLayer()
{
    int retCode = writeEnvironmentVariable("SLANG_RECORD_LAYER=0");
    return retCode == 0;
}

static bool enableLogInReplayer()
{
    int retCode = writeEnvironmentVariable("SLANG_RECORD_LOG_LEVEL=3");
    return retCode == 0;
}

static bool disableLogInReplayer()
{
    int retCode = writeEnvironmentVariable("SLANG_RECORD_LOG_LEVEL=0");
    return retCode == 0;
}

static void findRecordFileName(List<String>* fileNames)
{
    struct Visitor : Path::Visitor
    {
        void accept(Path::Type type, const UnownedStringSlice& filename) SLANG_OVERRIDE
        {
            if (type == Path::Type::File)
            {
                m_fileNames->add(filename);
            }
        }
        Visitor(List<String>* fileNames) : m_fileNames(fileNames) {}
        List<String>* m_fileNames;
    };

    Visitor visitor(fileNames);
    Path::find("slang-record", "*.cap", &visitor);
}

static SlangResult runExamples(UnitTestContext* context, const char* exampleName, List<entryHashInfo>& outHashes)
{
    SlangResult finalRes = SLANG_OK;

    RefPtr<Process> process;
    ExecuteResult exeRes;
    List<String> optArgs;
    optArgs.add("--test-mode");

    enableRecordLayer();
    SLANG_RETURN_ON_FAIL(createProcess(context, exampleName, &optArgs, process));
    SLANG_RETURN_ON_FAIL(ProcessUtil::readUntilTermination(process, exeRes));
    disableRecordLayer();

    List<String> hashLines;
    for (auto line : LineParser(exeRes.standardOutput.getUnownedSlice()))
    {
        if (line.getLength() == 0)
        {
            continue;
        }

        if (line.indexOf(UnownedStringSlice("hash:")) == -1)
        {
            continue;
        }

        hashLines.add(line);
    }

    SLANG_RETURN_ON_FAIL(parseHashes(hashLines, outHashes));

    return SLANG_OK;
}

static SlangResult replayExamples(UnitTestContext* context, List<entryHashInfo>& outHashes)
{
    List<String> fileNames;
    findRecordFileName(&fileNames);

    List<String> optArgs;
    String recordFileName = Path::combine("slang-record", fileNames[0]);
    optArgs.add(recordFileName.getBuffer());

    RefPtr<Process> process;
    ExecuteResult exeRes;

    enableLogInReplayer();
    SLANG_RETURN_ON_FAIL(createProcess(context, "slang-replay", &optArgs, process));
    SLANG_RETURN_ON_FAIL(ProcessUtil::readUntilTermination(process, exeRes));
    disableLogInReplayer();

    List<String> hashLines;
    for (auto line : LineParser(exeRes.standardOutput.getUnownedSlice()))
    {
        if (line.getLength() == 0)
        {
            continue;
        }

        if (line.indexOf(UnownedStringSlice("hash:")) == -1)
        {
            continue;
        }

        hashLines.add(line);
    }

    SLANG_RETURN_ON_FAIL(parseHashes(hashLines, outHashes));

    return SLANG_OK;
}

static SlangResult resultCompare(List<entryHashInfo> const& expectHashes, List<entryHashInfo> const& resultHashes)
{
    if (expectHashes.getCount() != resultHashes.getCount())
    {
        return SLANG_FAIL;
    }

    for (Index i = 0; i < expectHashes.getCount(); i++)
    {
        if (expectHashes[i].targetIndex != resultHashes[i].targetIndex)
        {
            return SLANG_FAIL;
        }
        if (expectHashes[i].entryPointIndex != resultHashes[i].entryPointIndex)
        {
            return SLANG_FAIL;
        }

        if (expectHashes[i].hash != resultHashes[i].hash)
        {
            return SLANG_FAIL;
        }
    }
    return SLANG_OK;
}

static void cleanupRecordFiles()
{
#ifdef _WIN32
    // Path::remove doesn't support remove a non-empty directory
    // https://learn.microsoft.com/en-us/windows/win32/api/shellapi/nf-shellapi-shfileoperationa
    SHFILEOPSTRUCTA file_op = {
        NULL,
        FO_DELETE,
        "slang-record",
        "",
        FOF_NOCONFIRMATION |
        FOF_NOERRORUI |
        FOF_SILENT,
        false,
        0,
        "" };
    SHFileOperationA(&file_op);
#else
    Path::remove("slang-record");
#endif
}

static SlangResult helloworldExample(UnitTestContext* context)
{
    List<entryHashInfo> expectHashes;
    List<entryHashInfo> resultHashes;
    SLANG_RETURN_ON_FAIL(runExamples(context, "hello-world", expectHashes));
    SLANG_RETURN_ON_FAIL(replayExamples(context, resultHashes));
    SLANG_RETURN_ON_FAIL(resultCompare(expectHashes, resultHashes));

    cleanupRecordFiles();
    return SLANG_OK;
}

static SlangResult triangleExample(UnitTestContext* context)
{
    List<entryHashInfo> expectHashes;
    List<entryHashInfo> resultHashes;
    SLANG_RETURN_ON_FAIL(runExamples(context, "triangle", expectHashes));
    SLANG_RETURN_ON_FAIL(replayExamples(context, resultHashes));
    SLANG_RETURN_ON_FAIL(resultCompare(expectHashes, resultHashes));

    cleanupRecordFiles();
    return SLANG_OK;
}

SLANG_UNIT_TEST(RecordReplay_HelloWorld)
{
    SLANG_CHECK(SLANG_SUCCEEDED(helloworldExample(unitTestContext)));
}

SLANG_UNIT_TEST(RecordReplay_Triangle)
{
    SLANG_CHECK(SLANG_SUCCEEDED(triangleExample(unitTestContext)));
}
