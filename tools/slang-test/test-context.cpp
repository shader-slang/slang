// test-context.cpp
#include "test-context.h"

#include "os.h"
#include "../../source/core/slang-string-util.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

using namespace Slang;

TestContext::TestContext(TestOutputMode outputMode) :
    m_outputMode(outputMode)
{
    m_totalTestCount = 0;
    m_passedTestCount = 0;
    m_failedTestCount = 0;
    m_ignoredTestCount = 0;

    m_inTest = false;
    m_dumpOutputOnFailure = false;
    m_isVerbose = false;
}

bool TestContext::canWriteStdError() const
{
    switch (m_outputMode)
    {
        case TestOutputMode::eXUnit:
        case TestOutputMode::eXUnit2:
        {
            return false;
        }
        default: return true;
    }
}

void TestContext::startTest(const String& testName)
{
    assert(!m_inTest);
    m_inTest = true;

    m_currentInfo = TestInfo();
    m_currentInfo.name = testName;
    m_currentMessage.Clear();
}

TestResult TestContext::endTest(TestResult result)
{
    assert(m_inTest);

    m_currentInfo.testResult = result;
    m_currentInfo.message = m_currentMessage;

    _addResult(m_currentInfo);

    m_inTest = false;

    return result;
}

void TestContext::dumpOutputDifference(const String& expectedOutput, const String& actualOutput)
{
    StringBuilder builder;

    StringUtil::appendFormat(builder,
        "ERROR:\n"
        "EXPECTED{{{\n%s}}}\n"
        "ACTUAL{{{\n%s}}}\n",
        expectedOutput.Buffer(),
        actualOutput.Buffer());


    if (m_dumpOutputOnFailure && canWriteStdError())
    {
        fprintf(stderr, "%s", builder.Buffer());
        fflush(stderr);
    }

    // Add to the m_currentInfo
    message(TestMessageType::eTestFailure, builder);
}

void TestContext::_addResult(const TestInfo& info)
{
    m_totalTestCount++;

    switch (info.testResult)
    {
        case TestResult::eFail:
            m_failedTestCount++;
            break;

        case TestResult::ePass:
            m_passedTestCount++;
            break;

        case TestResult::eIgnored:
            m_ignoredTestCount++;
            break;

        default:
            assert(!"unexpected");
            break;
    }

    m_testInfos.Add(info);

    //    printf("OUTPUT_MODE: %d\n", options.outputMode);
    switch (m_outputMode)
    {
        default:
        {
            char const* resultString = "UNEXPECTED";
            switch (info.testResult)
            {
                case TestResult::eFail:      resultString = "FAILED";  break;
                case TestResult::ePass:      resultString = "passed";  break;
                case TestResult::eIgnored:   resultString = "ignored"; break;
                default:
                    assert(!"unexpected");
                    break;
            }
            printf("%s test: '%S'\n", resultString, info.name.ToWString().begin());
            break;
        }
        case TestOutputMode::eXUnit2:
        case TestOutputMode::eXUnit:
        {
            // Don't output anything -> we'll output all in one go at the end
            break;
        }
        case TestOutputMode::eAppVeyor:
        {
            char const* resultString = "None";
            switch (info.testResult)
            {
                case TestResult::eFail:      resultString = "Failed";  break;
                case TestResult::ePass:      resultString = "Passed";  break;
                case TestResult::eIgnored:   resultString = "Ignored"; break;
                default:
                    assert(!"unexpected");
                    break;
            }

            OSProcessSpawner spawner;
            spawner.pushExecutableName("appveyor");
            spawner.pushArgument("AddTest");
            spawner.pushArgument(info.name);
            spawner.pushArgument("-FileName");
            // TODO: this isn't actually a file name in all cases
            spawner.pushArgument(info.name);
            spawner.pushArgument("-Framework");
            spawner.pushArgument("slang-test");
            spawner.pushArgument("-Outcome");
            spawner.pushArgument(resultString);

            auto err = spawner.spawnAndWaitForCompletion();

            if (err != kOSError_None)
            {
                messageFormat(TestMessageType::eInfo, "failed to add appveyor test results for '%S'\n", info.name.ToWString().begin());

#if 0
                fprintf(stderr, "[%d] TEST RESULT: %s {%d} {%s} {%s}\n", err, spawner.commandLine_.Buffer(),
                    spawner.getResultCode(),
                    spawner.getStandardOutput().begin(),
                    spawner.getStandardError().begin());
#endif
            }

            break;
        }
    }
}

void TestContext::addResult(const String& testName, TestResult testResult)
{
    // Can't add this way if in test
    assert(!m_inTest);

    TestInfo info;
    info.name = testName;
    info.testResult = testResult;
    _addResult(info);
}

void TestContext::message(TestMessageType type, const String& message)
{
    if (type == TestMessageType::eInfo)
    {
        if (m_isVerbose && canWriteStdError())
        {
            fputs(message.Buffer(), stderr);
        }

        // Just dump out if can dump out
        return;
    }

    if (canWriteStdError())
    {
        if (type == TestMessageType::eRunError || type == TestMessageType::eTestFailure)
        {
            fprintf(stderr, "error: ");
            fputs(message.Buffer(), stderr);
            fprintf(stderr, "\n");
        }
        else
        {
            fputs(message.Buffer(), stderr);
        }
    }

    if (m_currentMessage.Length() > 0)
    {
        m_currentMessage << "\n";
    }
    m_currentMessage.Append(message);
}

void TestContext::messageFormat(TestMessageType type, char const* format, ...)
{
    StringBuilder builder;

    va_list args;
    va_start(args, format);
    StringUtil::append(format, args, builder);
    va_end(args);

    message(type, builder);
}
