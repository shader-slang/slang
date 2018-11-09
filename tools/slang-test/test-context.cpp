// test-context.cpp
#include "test-context.h"

#include "os.h"
#include "../../source/core/slang-string-util.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

using namespace Slang;

/* static */TestContext* TestContext::s_context = nullptr;
/* static */TestRegister* TestRegister::s_first;

static void appendXmlEncode(char c, StringBuilder& out)
{
    switch (c)
    {
        case '&':   out << "&amp;"; break;
        case '<':   out << "&lt;"; break;
        case '>':   out << "&gt;"; break;
        case '\'':  out << "&apos;"; break;
        case '"':   out << "&quot;"; break;
        default:    out.Append(c);
    }
}

static bool isXmlEncodeChar(char c)
{
    switch (c)
    {
        case '&':
        case '<':
        case '>':
        {
            return true;
        }
    }
    return false;
}

static void appendXmlEncode(const String& in, StringBuilder& out)
{
    const char* cur = in.Buffer();
    const char* end = cur + in.Length();

    while (cur < end)
    {
        const char* start = cur;
        // Look for a run of non encoded
        while (cur < end && !isXmlEncodeChar(*cur))
        {
            cur++;
        }
        // Write it
        if (cur > start)
        {
            out.Append(start, UInt(end - start));
        }

        // if not at the end, we must be on an xml encoded character, so just output it xml encoded.
        if (cur < end)
        {
            const char encodeChar = *cur++;
            assert(isXmlEncodeChar(encodeChar));
            appendXmlEncode(encodeChar, out);
        }
    }
}

TestContext::TestContext(TestOutputMode outputMode) :
    m_outputMode(outputMode)
{
    m_totalTestCount = 0;
    m_passedTestCount = 0;
    m_failedTestCount = 0;
    m_ignoredTestCount = 0;

    m_maxFailTestResults = 10;

    m_inTest = false;
    m_dumpOutputOnFailure = false;
    m_isVerbose = false;
}

bool TestContext::canWriteStdError() const
{
    switch (m_outputMode)
    {
        case TestOutputMode::XUnit:
        case TestOutputMode::XUnit2:
        {
            return false;
        }
        default: return true;
    }
}

void TestContext::startTest(const String& testName)
{
    // Must be in a suite
    assert(m_suiteStack.Count());
    assert(!m_inTest);

    m_inTest = true;

    m_numCurrentResults = 0;
    m_numFailResults = 0;

    m_currentInfo = TestInfo();
    m_currentInfo.name = testName;
    m_currentMessage.Clear();
}

void TestContext::endTest()
{
    assert(m_suiteStack.Count());
    assert(m_inTest);

    m_currentInfo.message = m_currentMessage;

    _addResult(m_currentInfo);

    m_inTest = false;
}

void TestContext::addResult(TestResult result)
{
    assert(m_inTest);

    m_currentInfo.testResult = combine(m_currentInfo.testResult, result);
    m_numCurrentResults++;
}

void TestContext::addResultWithLocation(TestResult result, const char* testText, const char* file, int line)
{
    assert(m_inTest);
    m_numCurrentResults++;

    m_currentInfo.testResult = combine(m_currentInfo.testResult, result);
    if (result != TestResult::Fail)
    {
        // We don't need to output the result if it 
        return;
    }

    m_numFailResults++;

    if (m_maxFailTestResults > 0)
    {
        if (m_numFailResults > m_maxFailTestResults)
        {
            if (m_numFailResults == m_maxFailTestResults + 1)
            {
                // It's a failure, but to show that there are more than are going to be shown, just show '...'
                message(TestMessageType::TestFailure, "...");
            }
            return;
        }
    } 

    StringBuilder buf;
    buf <<  testText << " - " << file << " (" << line << ")";

    message(TestMessageType::TestFailure, buf);
}

void TestContext::addResultWithLocation(bool testSucceeded, const char* testText, const char* file, int line)
{
    addResultWithLocation(testSucceeded ? TestResult::Pass : TestResult::Fail, testText, file, line);
}

TestResult TestContext::addTest(const String& testName, bool isPass)
{
    const TestResult res = isPass ? TestResult::Pass : TestResult::Fail;
    addTest(testName, res);
    return res;
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
    message(TestMessageType::TestFailure, builder);
}

static char _getTeamCityEscapeChar(char c)
{
    switch (c)
    {
        case '|':   return '|';
        case '\'':  return '\''; 
        case '\n':  return 'n';
        case '\r':  return 'r'; 
        case '[':   return '[';
        case ']':   return ']';
        default:    return 0;
    }
}

static void _appendEncodedTeamCityString(const UnownedStringSlice& in, StringBuilder& builder)
{
    const char* start = in.begin();
    const char* cur = start;
    const char* end = in.end();
    
    for (const char* cur = start; cur < end; cur++)
    {
        const char c = *cur;
        const char escapeChar = _getTeamCityEscapeChar(c);
        if (escapeChar)
        {
            // Flush
            if (cur > start)
            {
                builder.Append(start, UInt(cur - start));
            }

            builder.Append('|');
            builder.Append(escapeChar);
            start = cur + 1;
        }
    }    

    // Flush the end
    if (end > start)
    {
        builder.Append(start, UInt(end - start));
    }
}

void TestContext::_addResult(const TestInfo& info)
{
    m_totalTestCount++;

    switch (info.testResult)
    {
        case TestResult::Fail:
            m_failedTestCount++;
            break;

        case TestResult::Pass:
            m_passedTestCount++;
            break;

        case TestResult::Ignored:
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
                case TestResult::Fail:      resultString = "FAILED";  break;
                case TestResult::Pass:      resultString = "passed";  break;
                case TestResult::Ignored:   resultString = "ignored"; break;
                default:
                    assert(!"unexpected");
                    break;
            }
            printf("%s test: '%S'\n", resultString, info.name.ToWString().begin());
            break;
        }
        case TestOutputMode::TeamCity:
        {
            StringBuilder escapedTestName;
            _appendEncodedTeamCityString(info.name.getUnownedSlice(), escapedTestName);

            printf("##teamcity[testStarted name='%s']\n", escapedTestName.begin());
             
            switch (info.testResult)
            {
                case TestResult::Fail:      
                {
                    if (info.message.Length())
                    {
                        StringBuilder escapedMessage;
                        _appendEncodedTeamCityString(info.message.getUnownedSlice(), escapedMessage);            
                        printf("##teamcity[testFailed name='%s' message='%s']\n", escapedTestName.begin(), escapedMessage.begin());
                    }
                    else
                    {
                        printf("##teamcity[testFailed name='%s']\n", escapedTestName.begin());
                    }
                    break;
                }
                case TestResult::Pass:     
                {
                    if (info.message.Length())
                    {
                        StringBuilder escapedMessage;
                        _appendEncodedTeamCityString(info.message.getUnownedSlice(), escapedMessage);
                        printf("##teamcity[testStdOut name='%s' out='%s']\n", escapedTestName.begin(), escapedMessage.begin());
                    }
                    break;
                }
                case TestResult::Ignored:  
                {
                    if (info.message.Length())
                    {
                        StringBuilder escapedMessage;
                        _appendEncodedTeamCityString(info.message.getUnownedSlice(), escapedMessage);

                        printf("##teamcity[testIgnored name='%s' message='%s']\n", escapedTestName.begin(), escapedMessage.begin());
                    }
                    else
                    {
                        printf("##teamcity[testIgnored name='%s']\n", escapedTestName.begin());
                    }
                    break;
                }
                default:
                    assert(!"unexpected");
                    break;
            }

            printf("##teamcity[testFinished name='%s']\n", escapedTestName.begin());
            fflush(stdout);
            break;
        }
        case TestOutputMode::XUnit2:
        case TestOutputMode::XUnit:
        {
            // Don't output anything -> we'll output all in one go at the end
            break;
        }
        case TestOutputMode::AppVeyor:
        {
            char const* resultString = "None";
            switch (info.testResult)
            {
                case TestResult::Fail:      resultString = "Failed";  break;
                case TestResult::Pass:      resultString = "Passed";  break;
                case TestResult::Ignored:   resultString = "Ignored"; break;
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
                messageFormat(TestMessageType::Info, "failed to add appveyor test results for '%S'\n", info.name.ToWString().begin());

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

void TestContext::addTest(const String& testName, TestResult testResult)
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
    if (type == TestMessageType::Info)
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
        if (type == TestMessageType::RunError || type == TestMessageType::TestFailure)
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

bool TestContext::didAllSucceed() const
{
    return m_passedTestCount == (m_totalTestCount - m_ignoredTestCount);
}

void TestContext::outputSummary()
{
    auto passCount = m_passedTestCount;
    auto rawTotal = m_totalTestCount;
    auto ignoredCount = m_ignoredTestCount;

    auto runTotal = rawTotal - ignoredCount;

    switch (m_outputMode)
    {
        default:
        {
            if (!m_totalTestCount)
            {
                printf("no tests run\n");
                return;
            }

            int percentPassed = 0;
            if (runTotal > 0)
            {
                percentPassed = (passCount * 100) / runTotal;
            }

            printf("\n===\n%d%% of tests passed (%d/%d)", percentPassed, passCount, runTotal);
            if (ignoredCount)
            {
                printf(", %d tests ignored", ignoredCount);
            }
            printf("\n===\n\n");

            if (m_failedTestCount)
            {
                printf("failing tests:\n");
                printf("---\n");
                for (const auto& testInfo : m_testInfos)
                {
                    if (testInfo.testResult == TestResult::Fail)
                    {
                        printf("%s\n", testInfo.name.Buffer());
                    }
                }
                printf("---\n");
            }
            break;
        }
        
        case TestOutputMode::XUnit:
        {
            // xUnit 1.0 format  

            printf("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
            printf("<testsuites tests=\"%d\" failures=\"%d\" disabled=\"%d\" errors=\"0\" name=\"AllTests\">\n", m_totalTestCount, m_failedTestCount, m_ignoredTestCount);
            printf("  <testsuite name=\"all\" tests=\"%d\" failures=\"%d\" disabled=\"%d\" errors=\"0\" time=\"0\">\n", m_totalTestCount, m_failedTestCount, m_ignoredTestCount);

            for (const auto& testInfo : m_testInfos)
            {
                const int numFailed = (testInfo.testResult == TestResult::Fail);
                const int numIgnored = (testInfo.testResult == TestResult::Ignored);
                //int numPassed = (testInfo.testResult == TestResult::ePass);

                if (testInfo.testResult == TestResult::Pass)
                {
                    printf("    <testcase name=\"%s\" status=\"run\"/>\n", testInfo.name.Buffer());
                }
                else
                {
                    printf("    <testcase name=\"%s\" status=\"run\">\n", testInfo.name.Buffer());
                    switch (testInfo.testResult)
                    {
                        case TestResult::Fail:
                        {
                            StringBuilder buf;
                            appendXmlEncode(testInfo.message, buf);

                            printf("      <error>\n");
                            printf("%s", buf.Buffer());
                            printf("      </error>\n");
                            break;
                        }
                        case TestResult::Ignored:
                        {
                            printf("      <skip>Ignored</skip>\n");
                            break;
                        }
                        default: break;
                    }
                    printf("    </testcase>\n");
                }
            }

            printf("  </testsuite>\n");
            printf("</testSuites>\n");
            break;
        }
        case TestOutputMode::XUnit2:
        {
            // https://xunit.github.io/docs/format-xml-v2
            assert("Not currently supported");
            break;
        }
        case TestOutputMode::TeamCity:
        {
            // Don't output a summary
            break;
        }
    }
}

void TestContext::startSuite(const String& name)
{
    m_suiteStack.Add(name);

    switch (m_outputMode)
    {
        case TestOutputMode::TeamCity:
        {
            StringBuilder escapedSuiteName;
            _appendEncodedTeamCityString(name.getUnownedSlice(), escapedSuiteName);
            printf("##teamcity[testSuiteStarted name='%s']\n", escapedSuiteName.begin());
            break;
        }
        default: break;
    }
}

void TestContext::endSuite()
{
    assert(m_suiteStack.Count());

    switch (m_outputMode)
    {
        case TestOutputMode::TeamCity:
        {
            const String& name = m_suiteStack.Last();
            StringBuilder escapedSuiteName;
            _appendEncodedTeamCityString(name.getUnownedSlice(), escapedSuiteName);
            printf("##teamcity[testSuiteFinished name='%s']\n", escapedSuiteName.begin());
            break;
        }
        default: break;
    }
    
    m_suiteStack.RemoveLast();
}