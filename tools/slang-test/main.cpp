// main.cpp

#include "../../source/core/slang-io.h"
#include "../../source/core/token-reader.h"

#include "../../slang-com-helper.h"

#include "../../source/core/slang-string-util.h"

using namespace Slang;

#include "os.h"
#include "render-api-util.h"
#include "test-context.h"

#define STB_IMAGE_IMPLEMENTATION
#include "external/stb/stb_image.h"


#ifdef _WIN32
#define SLANG_TEST_SUPPORT_HLSL 1
#include <d3dcompiler.h>
#endif

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

// A category that a test can be tagged with
struct TestCategory
{
    // The name of the category, from the user perspective
    String name;

    // The logical "super-category" of this category
    TestCategory* parent;
};

// Options for a particular test
struct TestOptions
{
    String command;
    List<String> args;

    // The categories that this test was assigned to
    List<TestCategory*> categories;
};

// Information on tests to run for a particular file
struct FileTestList
{
    List<TestOptions> tests;
};

struct TestInput
{
    // Path to the input file for the test
    String              filePath;

    // Prefix for the path that test output should write to
    // (usually the same as `filePath`, but will differ when
    // we run multiple tests out of the same file)
    String              outputStem;

    // Arguments for the test (usually to be interpreted
    // as command line args)
    TestOptions const*  testOptions;

    // The list of tests that will be run on this file
    FileTestList const* testList;
};

typedef TestResult(*TestCallback)(TestContext* context, TestInput& input);

struct Options
{
    char const* appName = "slang-test";

    // Directory to use when looking for binaries to run
    char const* binDir = "";

    // only run test cases with names that have this prefix
    char const* testPrefix = nullptr;

    // generate extra output (notably: command lines we run)
    bool shouldBeVerbose = false;

    // force generation of baselines for HLSL tests
    bool generateHLSLBaselines = false;

    // Dump expected/actual output on failures, for debugging.
    // This is especially intended for use in continuous
    // integration builds.
    bool dumpOutputOnFailure = false;

    // kind of output to generate
    TestOutputMode outputMode = TestOutputMode::Default;

    // Only run tests that match one of the given categories
    Dictionary<TestCategory*, TestCategory*> includeCategories;

    // Exclude test that match one these categories
    Dictionary<TestCategory*, TestCategory*> excludeCategories;

    // By default we can test against all apis
    RenderApiFlags enabledApis = RenderApiFlag::AllOf;

    // By default we potentially synthesize test for all 
    // TODO: Vulkan is disabled by default for now as the majority as vulkan synthesized tests fail  
    RenderApiFlags synthesizedTestApis = RenderApiFlag::AllOf & ~RenderApiFlag::Vulkan;
};

// Globals

Options g_options;
Dictionary<String, TestCategory*> g_testCategories;
TestCategory* g_defaultTestCategory;

// pre declare

TestCategory* findTestCategory(String const& name);

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!! Functions !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */


Result parseOptions(int* argc, char** argv)
{
    int argCount = *argc;
    char const* const* argCursor = argv;
    char const* const* argEnd = argCursor + argCount;

    char const** writeCursor = (char const**) argv;

    // first argument is the application name
    if( argCursor != argEnd )
    {
        g_options.appName = *argCursor++;
    }

    // now iterate over arguments to collect options
    while(argCursor != argEnd)
    {
        char const* arg = *argCursor++;
        if( arg[0] != '-' )
        {
            *writeCursor++ = arg;
            continue;
        }

        if( strcmp(arg, "--") == 0 )
        {
            while(argCursor != argEnd)
            {
                char const* nxtArg = *argCursor++;
                *writeCursor++ = nxtArg;
            }
            break;
        }

        if( strcmp(arg, "-bindir") == 0 )
        {
            if( argCursor == argEnd )
            {
                fprintf(stderr, "error: expected operand for '%s'\n", arg);
                return SLANG_FAIL;
            }
            g_options.binDir = *argCursor++;
        }
        else if( strcmp(arg, "-v") == 0 )
        {
            g_options.shouldBeVerbose = true;
        }
        else if( strcmp(arg, "-generate-hlsl-baselines") == 0 )
        {
            g_options.generateHLSLBaselines = true;
        }
        else if( strcmp(arg, "-release") == 0 )
        {
            // Assumed to be handle by .bat file that called us
        }
        else if( strcmp(arg, "-debug") == 0 )
        {
            // Assumed to be handle by .bat file that called us
        }
        else if( strcmp(arg, "-configuration") == 0 )
        {
            if( argCursor == argEnd )
            {
                fprintf(stderr, "error: expected operand for '%s'\n", arg);
				return SLANG_FAIL;
            }
            argCursor++;
            // Assumed to be handle by .bat file that called us
        }
        else if( strcmp(arg, "-platform") == 0 )
        {
            if( argCursor == argEnd )
            {
                fprintf(stderr, "error: expected operand for '%s'\n", arg);
				return SLANG_FAIL;
            }
            argCursor++;
            // Assumed to be handle by .bat file that called us
        }
        else if( strcmp(arg, "-appveyor") == 0 )
        {
            g_options.outputMode = TestOutputMode::AppVeyor;
            g_options.dumpOutputOnFailure = true;
        }
        else if( strcmp(arg, "-travis") == 0 )
        {
            g_options.outputMode = TestOutputMode::Travis;
            g_options.dumpOutputOnFailure = true;
        }
        else if (strcmp(arg, "-xunit") == 0)
        {
            g_options.outputMode = TestOutputMode::XUnit;
        }
        else if (strcmp(arg, "-xunit2") == 0)
        {
            g_options.outputMode = TestOutputMode::XUnit2;
        }
        else if( strcmp(arg, "-category") == 0 )
        {
            if( argCursor == argEnd )
            {
                fprintf(stderr, "error: expected operand for '%s'\n", arg);
                return SLANG_FAIL;
            }
            auto category = findTestCategory(*argCursor++);
            if(category)
            {
                g_options.includeCategories.Add(category, category);
            }
        }
        else if( strcmp(arg, "-exclude") == 0 )
        {
            if( argCursor == argEnd )
            {
                fprintf(stderr, "error: expected operand for '%s'\n", arg);
                return SLANG_FAIL;
            }
            auto category = findTestCategory(*argCursor++);
            if(category)
            {
                g_options.excludeCategories.Add(category, category);
            }
        }
        else if (strcmp(arg, "-api") == 0)
        {
            if (argCursor == argEnd)
            {
                fprintf(stderr, "error: expecting an api expression (eg 'vk+dx12' or '+dx11') '%s'\n", arg);
                return SLANG_FAIL;
            }
            const char* apiList = *argCursor++;

            SlangResult res = RenderApiUtil::parseApiFlags(UnownedStringSlice(apiList), g_options.enabledApis, &g_options.enabledApis);
            if (SLANG_FAILED(res))
            {
                fprintf(stderr, "error: unable to parse api expression '%s'\n", apiList);
                return res;
            }
        }
        else if (strcmp(arg, "-synthesizedTestApi") == 0)
        {
            if (argCursor == argEnd)
            {
                fprintf(stderr, "error: expected an api expression (eg 'vk+dx12' or '+dx11') '%s'\n", arg);
                return SLANG_FAIL;
            }
            const char* apiList = *argCursor++;

            SlangResult res = RenderApiUtil::parseApiFlags(UnownedStringSlice(apiList), g_options.synthesizedTestApis, &g_options.synthesizedTestApis);
            if (SLANG_FAILED(res))
            {
                fprintf(stderr, "error: unable to parse api expression '%s'\n", apiList);
                return res;
            }
        }
        else
        {
            fprintf(stderr, "unknown option '%s'\n", arg);
            return SLANG_FAIL;
        }
    }
    
    {
        // Find out what apis are available
        const int availableApis = RenderApiUtil::getAvailableApis();
        // Only allow apis we know are available
        g_options.enabledApis &= availableApis;

        // Can only synth for apis that are available
        g_options.synthesizedTestApis &= g_options.enabledApis;
    }

    // any arguments left over were positional arguments
    argCount = (int)((char**)writeCursor - argv);
    argCursor = argv;
    argEnd = argCursor + argCount;

    // first positional argument is a "filter" to apply
    if( argCursor != argEnd )
    {
        g_options.testPrefix = *argCursor++;
    }

    // any remaining arguments represent an error
    if(argCursor != argEnd)
    {
        fprintf(stderr, "unexpected arguments\n");
        return SLANG_FAIL;
    }

    *argc = 0;
	return SLANG_OK;
}

// Called for an error in the test-runner (not for an error involving
// a test itself).
void error(char const* message, ...)
{
    fprintf(stderr, "error: ");

    va_list args;
    va_start(args, message);
    vfprintf(stderr, message, args);
    va_end(args);

    fprintf(stderr, "\n");
}

bool match(char const** ioCursor, char const* expected)
{
    char const* cursor = *ioCursor;
    while(*expected && *cursor == *expected)
    {
        cursor++;
        expected++;
    }
    if(*expected != 0) return false;

    *ioCursor = cursor;
    return true;
}

void skipHorizontalSpace(char const** ioCursor)
{
    char const* cursor = *ioCursor;
    for( ;;)
    {
        switch( *cursor )
        {
        case ' ':
        case '\t':
            cursor++;
            continue;

        default:
            break;
        }

        break;
    }
    *ioCursor = cursor;
}

void skipToEndOfLine(char const** ioCursor)
{
    char const* cursor = *ioCursor;
    for( ;;)
    {
        int c = *cursor;
        switch( c )
        {
        default:
            cursor++;
            continue;

        case '\r': case '\n':
            {
                cursor++;
                int d = *cursor;
                if( (c ^ d) == ('\r' ^ '\n') )
                {
                    cursor++;
                }
            }
            ; // fall through to:
        case 0:
            *ioCursor = cursor;
            return;
        }
    }
}

String getString(char const* textBegin, char  const* textEnd)
{
    StringBuilder sb;
    sb.Append(textBegin, textEnd - textBegin);
    return sb.ProduceString();
}

String collectRestOfLine(char const** ioCursor)
{
    char const* cursor = *ioCursor;

    char const* textBegin = cursor;
    skipToEndOfLine(&cursor);
    char const* textEnd = cursor;

    *ioCursor = cursor;
    return getString(textBegin, textEnd);
}

TestCategory* addTestCategory(String const& name, TestCategory* parent)
{
    TestCategory* category = new TestCategory();
    category->name = name;

    category->parent = parent;

    g_testCategories.Add(name, category);

    return category;
}

TestCategory* findTestCategory(String const& name)
{
    TestCategory* category = nullptr;
    if( !g_testCategories.TryGetValue(name, category) )
    {
        error("unknown test category name '%s'\n", name.Buffer());
        return nullptr;
    }
    return category;
}

TestResult gatherTestOptions(
    char const**    ioCursor,
    FileTestList*   testList)
{
    char const* cursor = *ioCursor;

    TestOptions testOptions;

    // Right after the `TEST` keyword, the user may specify
    // one or more categories for the test.
    if(*cursor == '(')
    {
        cursor++;
        // optional test category
        skipHorizontalSpace(&cursor);
        char const* categoryStart = cursor;
        for(;;)
        {
            switch( *cursor )
            {
            default:
                cursor++;
                continue;

            case ',':
            case ')':
                {
                    char const* categoryEnd = cursor;
                    cursor++;

                    auto categoryName = getString(categoryStart, categoryEnd);
                    TestCategory* category = findTestCategory(categoryName);

                    if(!category)
                    {
                        return TestResult::Fail;
                    }

                    testOptions.categories.Add(category);

                    if( *categoryEnd == ',' )
                    {
                        skipHorizontalSpace(&cursor);
                        categoryStart = cursor;
                        continue;
                    }
                }
                break;
        
            case 0: case '\r': case '\n':
                return TestResult::Fail;
            }

            break;
        }
    }

    // If no categories were specified, then add the default category
    if(testOptions.categories.Count() == 0)
    {
        testOptions.categories.Add(g_defaultTestCategory);
    }

    if(*cursor == ':')
        cursor++;
    else
    {
        return TestResult::Fail;
    }

    // Next scan for a sub-command name
    char const* commandStart = cursor;
    for(;;)
    {
        switch(*cursor)
        {
        default:
            cursor++;
            continue;

        case ':':
            break;
        
        case 0: case '\r': case '\n':
            return TestResult::Fail;
        }

        break;
    }
    char const* commandEnd = cursor;

    testOptions.command = getString(commandStart, commandEnd);

    if(*cursor == ':')
        cursor++;
    else
    {
        return TestResult::Fail;
    }

    // Now scan for arguments. For now we just assume that
    // any whitespace separation indicates a new argument
    // (we don't support quoting)
    for(;;)
    {
        skipHorizontalSpace(&cursor);

        // End of line? then no more options.
        switch( *cursor )
        {
        case 0: case '\r': case '\n':
            skipToEndOfLine(&cursor);
            testList->tests.Add(testOptions);
            return TestResult::Pass;

        default:
            break;
        }

        // Let's try to read one option
        char const* argBegin = cursor;
        for(;;)
        {
            switch( *cursor )
            {
            default:
                cursor++;
                continue;

            case 0: case '\r': case '\n': case ' ': case '\t':
                break;
            }

            break;
        }
        char const* argEnd = cursor;
        assert(argBegin != argEnd);

        testOptions.args.Add(getString(argBegin, argEnd));
    }
}

// Try to read command-line options from the test file itself
TestResult gatherTestsForFile(
    String				filePath,
    FileTestList*       testList)
{
    String fileContents;
    try
    {
        fileContents = Slang::File::ReadAllText(filePath);
    }
    catch (Slang::IOException)
    {
        return TestResult::Fail;
    }


    // Walk through the lines of the file, looking for test commands
    char const* cursor = fileContents.begin();

    while(*cursor)
    {
        // We are at the start of a line of input.

        skipHorizontalSpace(&cursor);

        // Look for a pattern that matches what we want
        if(match(&cursor, "//TEST_IGNORE_FILE"))
        {
            return TestResult::Ignored;
        }
        else if(match(&cursor, "//TEST"))
        {
            if(gatherTestOptions(&cursor, testList) != TestResult::Pass)
                return TestResult::Fail;
        }
        else
        {
            skipToEndOfLine(&cursor);
        }
    }

    return TestResult::Pass;
}

OSError spawnAndWait(TestContext* context, const String& testPath, OSProcessSpawner& spawner)
{
    SLANG_UNUSED(context);

    if(context->m_isVerbose)
    {
        String commandLine = spawner.getCommandLine();
        context->messageFormat(TestMessageType::Info, "%s\n", commandLine.begin());
    }
    
    OSError err = spawner.spawnAndWaitForCompletion();
    if (err != kOSError_None)
    {
//        fprintf(stderr, "failed to run test '%S'\n", testPath.ToWString());
        context->messageFormat(TestMessageType::RunError, "failed to run test '%S'", testPath.ToWString().begin());
    }
    return err;
}

String getOutput(OSProcessSpawner& spawner)
{
    OSProcessSpawner::ResultCode resultCode = spawner.getResultCode();

    String standardOuptut = spawner.getStandardOutput();
    String standardError = spawner.getStandardError();

    // We construct a single output string that captures the results
    StringBuilder actualOutputBuilder;
    actualOutputBuilder.Append("result code = ");
    actualOutputBuilder.Append(resultCode);
    actualOutputBuilder.Append("\nstandard error = {\n");
    actualOutputBuilder.Append(standardError);
    actualOutputBuilder.Append("}\nstandard output = {\n");
    actualOutputBuilder.Append(standardOuptut);
    actualOutputBuilder.Append("}\n");

    return actualOutputBuilder.ProduceString();
}

// Finds the specialized or default path for expected data for a test. 
// If neither are found, will return an empty string
String findExpectedPath(const TestInput& input, const char* postFix)
{
    StringBuilder specializedBuf;

    // Try the specialized name first
    specializedBuf << input.outputStem;
    if (postFix)
    {
        specializedBuf << postFix;
    }
    if (File::Exists(specializedBuf))
    {
        return specializedBuf;
    }


    // Try the default name
    StringBuilder defaultBuf;
    defaultBuf.Clear();
    defaultBuf << input.filePath;
    if (postFix)
    {
        defaultBuf << postFix;
    }

    if (File::Exists(defaultBuf))
    {
        return defaultBuf;
    }

    // Couldn't find either 
    printf("referenceOutput '%s' or '%s' not found.\n", defaultBuf.Buffer(), specializedBuf.Buffer());

    return "";
}

TestResult runSimpleTest(TestContext* context, TestInput& input)
{
    // need to execute the stand-alone Slang compiler on the file, and compare its output to what we expect

    auto filePath999 = input.filePath;
    auto outputStem = input.outputStem;

    OSProcessSpawner spawner;

    spawner.pushExecutablePath(String(g_options.binDir) + "slangc" + osGetExecutableSuffix());
    spawner.pushArgument(filePath999);

    for( auto arg : input.testOptions->args )
    {
        spawner.pushArgument(arg);
    }

    if (spawnAndWait(context, outputStem, spawner) != kOSError_None)
    {
        return TestResult::Fail;
    }

    String actualOutput = getOutput(spawner);

    String expectedOutputPath = outputStem + ".expected";
    String expectedOutput;
    try
    {
        expectedOutput = Slang::File::ReadAllText(expectedOutputPath);
    }
    catch (Slang::IOException)
    {
    }

    // If no expected output file was found, then we
    // expect everything to be empty
    if (expectedOutput.Length() == 0)
    {
        expectedOutput = "result code = 0\nstandard error = {\n}\nstandard output = {\n}\n";
    }

    TestResult result = TestResult::Pass;

    // Otherwise we compare to the expected output
    if (actualOutput != expectedOutput)
    {
        context->dumpOutputDifference(expectedOutput, actualOutput);
        result = TestResult::Fail;
    }

    // If the test failed, then we write the actual output to a file
    // so that we can easily diff it from the command line and
    // diagnose the problem.
    if (result == TestResult::Fail)
    {
        String actualOutputPath = outputStem + ".actual";
        Slang::File::WriteAllText(actualOutputPath, actualOutput);

        context->dumpOutputDifference(expectedOutput, actualOutput);
    }

    return result;
}

TestResult runReflectionTest(TestContext* context, TestInput& input)
{
    auto filePath = input.filePath;
    auto outputStem = input.outputStem;

    OSProcessSpawner spawner;

    spawner.pushExecutablePath(String(g_options.binDir) + "slang-reflection-test" + osGetExecutableSuffix());
    spawner.pushArgument(filePath);

    for( auto arg : input.testOptions->args )
    {
        spawner.pushArgument(arg);
    }

    if (spawnAndWait(context, outputStem, spawner) != kOSError_None)
    {
        return TestResult::Fail;
    }

    String actualOutput = getOutput(spawner);

    String expectedOutputPath = outputStem + ".expected";
    String expectedOutput;
    try
    {
        expectedOutput = Slang::File::ReadAllText(expectedOutputPath);
    }
    catch (Slang::IOException)
    {
    }

    // If no expected output file was found, then we
    // expect everything to be empty
    if (expectedOutput.Length() == 0)
    {
        expectedOutput = "result code = 0\nstandard error = {\n}\nstandard output = {\n}\n";
    }

    TestResult result = TestResult::Pass;

    // Otherwise we compare to the expected output
    if (actualOutput != expectedOutput)
    {
        result = TestResult::Fail;
    }

    // If the test failed, then we write the actual output to a file
    // so that we can easily diff it from the command line and
    // diagnose the problem.
    if (result == TestResult::Fail)
    {
        String actualOutputPath = outputStem + ".actual";
        Slang::File::WriteAllText(actualOutputPath, actualOutput);

        context->dumpOutputDifference(expectedOutput, actualOutput);
    }

    return result;
}

String getExpectedOutput(String const& outputStem)
{
    String expectedOutputPath = outputStem + ".expected";
    String expectedOutput;
    try
    {
        expectedOutput = Slang::File::ReadAllText(expectedOutputPath);
    }
    catch (Slang::IOException)
    {
    }

    // If no expected output file was found, then we
    // expect everything to be empty
    if (expectedOutput.Length() == 0)
    {
        expectedOutput = "result code = 0\nstandard error = {\n}\nstandard output = {\n}\n";
    }

    return expectedOutput;
}

TestResult runEvalTest(TestContext* context, TestInput& input)
{
    // We are going to load and evaluate the code

    auto filePath = input.filePath;
    auto outputStem = input.outputStem;

    OSProcessSpawner spawner;

    spawner.pushExecutablePath(String(g_options.binDir) + "slang-eval-test" + osGetExecutableSuffix());
    spawner.pushArgument(filePath);

    for( auto arg : input.testOptions->args )
    {
        spawner.pushArgument(arg);
    }

    if (spawnAndWait(context, outputStem, spawner) != kOSError_None)
    {
        return TestResult::Fail;
    }

    String actualOutput = getOutput(spawner);
    String expectedOutput = getExpectedOutput(outputStem);

    TestResult result = TestResult::Pass;

    // Otherwise we compare to the expected output
    if (actualOutput != expectedOutput)
    {
        result = TestResult::Fail;
    }

    // If the test failed, then we write the actual output to a file
    // so that we can easily diff it from the command line and
    // diagnose the problem.
    if (result == TestResult::Fail)
    {
        String actualOutputPath = outputStem + ".actual";
        Slang::File::WriteAllText(actualOutputPath, actualOutput);

        context->dumpOutputDifference(expectedOutput, actualOutput);
    }

    return result;
}


TestResult runCrossCompilerTest(TestContext* context, TestInput& input)
{
    // need to execute the stand-alone Slang compiler on the file
    // then on the same file + `.glsl` and compare output

    auto filePath = input.filePath;
    auto outputStem = input.outputStem;

    OSProcessSpawner actualSpawner;
    OSProcessSpawner expectedSpawner;

    actualSpawner.pushExecutablePath(String(g_options.binDir) + "slangc" + osGetExecutableSuffix());
    expectedSpawner.pushExecutablePath(String(g_options.binDir) + "slangc" + osGetExecutableSuffix());

    actualSpawner.pushArgument(filePath);
    expectedSpawner.pushArgument(filePath + ".glsl");
    expectedSpawner.pushArgument("-pass-through");
    expectedSpawner.pushArgument("glslang");

    for( auto arg : input.testOptions->args )
    {
        actualSpawner.pushArgument(arg);
        expectedSpawner.pushArgument(arg);
    }

    if (spawnAndWait(context, outputStem, expectedSpawner) != kOSError_None)
    {
        return TestResult::Fail;
    }

    String expectedOutput = getOutput(expectedSpawner);
    String expectedOutputPath = outputStem + ".expected";
    try
    {
        Slang::File::WriteAllText(expectedOutputPath, expectedOutput);
    }
    catch (Slang::IOException)
    {
        return TestResult::Fail;
    }

    if (spawnAndWait(context, outputStem, actualSpawner) != kOSError_None)
    {
        return TestResult::Fail;
    }
    String actualOutput = getOutput(actualSpawner);

    TestResult result = TestResult::Pass;

    // Otherwise we compare to the expected output
    if (actualOutput != expectedOutput)
    {
        result = TestResult::Fail;
    }

    // Always fail if the compilation produced a failure, just
    // to catch situations where, e.g., command-line options parsing
    // caused the same error in both the Slang and glslang cases.
    //
    if( actualSpawner.getResultCode() != 0 )
    {
        result = TestResult::Fail;
    }

    // If the test failed, then we write the actual output to a file
    // so that we can easily diff it from the command line and
    // diagnose the problem.
    if (result == TestResult::Fail)
    {
        String actualOutputPath = outputStem + ".actual";
        Slang::File::WriteAllText(actualOutputPath, actualOutput);

        context->dumpOutputDifference(expectedOutput, actualOutput);
    }

    return result;
}


#ifdef SLANG_TEST_SUPPORT_HLSL
TestResult generateHLSLBaseline(TestContext* context, TestInput& input)
{
    auto filePath999 = input.filePath;
    auto outputStem = input.outputStem;

    OSProcessSpawner spawner;
    spawner.pushExecutablePath(String(g_options.binDir) + "slangc" + osGetExecutableSuffix());
    spawner.pushArgument(filePath999);

    for( auto arg : input.testOptions->args )
    {
        spawner.pushArgument(arg);
    }

    spawner.pushArgument("-target");
    spawner.pushArgument("dxbc-assembly");
    spawner.pushArgument("-pass-through");
    spawner.pushArgument("fxc");

    if (spawnAndWait(context, outputStem, spawner) != kOSError_None)
    {
        return TestResult::Fail;
    }

    String expectedOutput = getOutput(spawner);
    String expectedOutputPath = outputStem + ".expected";
    try
    {
        Slang::File::WriteAllText(expectedOutputPath, expectedOutput);
    }
    catch (Slang::IOException)
    {
        return TestResult::Fail;
    }
    return TestResult::Pass;
}

TestResult runHLSLComparisonTest(TestContext* context, TestInput& input)
{
    auto filePath999 = input.filePath;
    auto outputStem = input.outputStem;

    // We will use the Microsoft compiler to generate out expected output here
    String expectedOutputPath = outputStem + ".expected";

    // Generate the expected output using standard HLSL compiler
    generateHLSLBaseline(context, input);

    // need to execute the stand-alone Slang compiler on the file, and compare its output to what we expect

    OSProcessSpawner spawner;

    spawner.pushExecutablePath(String(g_options.binDir) + "slangc" + osGetExecutableSuffix());
    spawner.pushArgument(filePath999);

    for( auto arg : input.testOptions->args )
    {
        spawner.pushArgument(arg);
    }

    // TODO: The compiler should probably define this automatically...
    spawner.pushArgument("-D");
    spawner.pushArgument("__SLANG__");

    spawner.pushArgument("-target");
    spawner.pushArgument("dxbc-assembly");

    if (spawnAndWait(context, outputStem, spawner) != kOSError_None)
    {
        return TestResult::Fail;
    }

    // We ignore output to stdout, and only worry about what the compiler
    // wrote to stderr.

    OSProcessSpawner::ResultCode resultCode = spawner.getResultCode();

    String standardOutput = spawner.getStandardOutput();
    String standardError = spawner.getStandardError();

    // We construct a single output string that captures the results
    StringBuilder actualOutputBuilder;
    actualOutputBuilder.Append("result code = ");
    actualOutputBuilder.Append(resultCode);
    actualOutputBuilder.Append("\nstandard error = {\n");
    actualOutputBuilder.Append(standardError);
    actualOutputBuilder.Append("}\nstandard output = {\n");
    actualOutputBuilder.Append(standardOutput);
    actualOutputBuilder.Append("}\n");

    String actualOutput = actualOutputBuilder.ProduceString();

    String expectedOutput;
    try
    {
        expectedOutput = Slang::File::ReadAllText(expectedOutputPath);
    }
    catch (Slang::IOException)
    {
    }

    TestResult result = TestResult::Pass;

    // If no expected output file was found, then we
    // expect everything to be empty
    if (expectedOutput.Length() == 0)
    {
        if (resultCode != 0)				result = TestResult::Fail;
        if (standardError.Length() != 0)	result = TestResult::Fail;
        if (standardOutput.Length() != 0)	result = TestResult::Fail;
    }
    // Otherwise we compare to the expected output
    else if (actualOutput != expectedOutput)
    {
        result = TestResult::Fail;
    }

    // Always fail if the compilation produced a failure, just
    // to catch situations where, e.g., command-line options parsing
    // caused the same error in both the Slang and fxc cases.
    //
    if( resultCode != 0 )
    {
        result = TestResult::Fail;
    }

    // If the test failed, then we write the actual output to a file
    // so that we can easily diff it from the command line and
    // diagnose the problem.
    if (result == TestResult::Fail)
    {
        String actualOutputPath = outputStem + ".actual";
        Slang::File::WriteAllText(actualOutputPath, actualOutput);

        context->dumpOutputDifference(expectedOutput, actualOutput);
    }

    return result;
}
#endif

TestResult doGLSLComparisonTestRun(TestContext* context,
    TestInput& input,
    char const* langDefine,
    char const* passThrough,
    char const* outputKind,
    String* outOutput)
{
    auto filePath999 = input.filePath;
    auto outputStem = input.outputStem;

    OSProcessSpawner spawner;

    spawner.pushExecutablePath(String(g_options.binDir) + "slangc" + osGetExecutableSuffix());
    spawner.pushArgument(filePath999);

    if( langDefine )
    {
        spawner.pushArgument("-D");
        spawner.pushArgument(langDefine);
    }

    if( passThrough )
    {
        spawner.pushArgument("-pass-through");
        spawner.pushArgument(passThrough);
    }

    spawner.pushArgument("-target");
    spawner.pushArgument("spirv-assembly");

    for( auto arg : input.testOptions->args )
    {
        spawner.pushArgument(arg);
    }

    if (spawnAndWait(context, outputStem, spawner) != kOSError_None)
    {
        return TestResult::Fail;
    }

    OSProcessSpawner::ResultCode resultCode = spawner.getResultCode();

    String standardOuptut = spawner.getStandardOutput();
    String standardError = spawner.getStandardError();

    // We construct a single output string that captures the results
    StringBuilder outputBuilder;
    outputBuilder.Append("result code = ");
    outputBuilder.Append(resultCode);
    outputBuilder.Append("\nstandard error = {\n");
    outputBuilder.Append(standardError);
    outputBuilder.Append("}\nstandard output = {\n");
    outputBuilder.Append(standardOuptut);
    outputBuilder.Append("}\n");

    String outputPath = outputStem + outputKind;
    String output = outputBuilder.ProduceString();

    *outOutput = output;

    return TestResult::Pass;
}

TestResult runGLSLComparisonTest(TestContext* context, TestInput& input)
{
    auto filePath999 = input.filePath;
    auto outputStem = input.outputStem;

    String expectedOutput;
    String actualOutput;

    TestResult hlslResult   =  doGLSLComparisonTestRun(context, input, "__GLSL__",  "glslang", ".expected",    &expectedOutput);
    TestResult slangResult  =  doGLSLComparisonTestRun(context, input, "__SLANG__", nullptr,   ".actual",      &actualOutput);

    Slang::File::WriteAllText(outputStem + ".expected", expectedOutput);
    Slang::File::WriteAllText(outputStem + ".actual",   actualOutput);

    if( hlslResult  == TestResult::Fail )   return TestResult::Fail;
    if( slangResult == TestResult::Fail )   return TestResult::Fail;

    if (actualOutput != expectedOutput)
    {
        context->dumpOutputDifference(expectedOutput, actualOutput);

        return TestResult::Fail;
    }

    return TestResult::Pass;
}


TestResult runComputeComparisonImpl(TestContext* context, TestInput& input, const char * langOption)
{
	// TODO: delete any existing files at the output path(s) to avoid stale outputs leading to a false pass
	auto filePath999 = input.filePath;
	auto outputStem = input.outputStem;

    const String referenceOutput = findExpectedPath(input, ".expected.txt");
    if (referenceOutput.Length() <= 0)
    {
        return TestResult::Fail;
    }

	OSProcessSpawner spawner;

	spawner.pushExecutablePath(String(g_options.binDir) + "render-test" + osGetExecutableSuffix());
	spawner.pushArgument(filePath999);

	for (auto arg : input.testOptions->args)
	{
		spawner.pushArgument(arg);
	}

	spawner.pushArgument(langOption);
	spawner.pushArgument("-o");
    auto actualOutputFile = outputStem + ".actual.txt";
	spawner.pushArgument(actualOutputFile);

    // clear the stale actual output file first. This will allow us to detect error if render-test fails and outputs nothing.
    File::WriteAllText(actualOutputFile, "");

	if (spawnAndWait(context, outputStem, spawner) != kOSError_None)
	{
        printf("error spawning render-test\n");
		return TestResult::Fail;
	}

    auto actualOutput = getOutput(spawner);
    auto expectedOutput = getExpectedOutput(outputStem);
    if (actualOutput != expectedOutput)
    {
        context->dumpOutputDifference(expectedOutput, actualOutput);

        String actualOutputPath = outputStem + ".actual";
        Slang::File::WriteAllText(actualOutputPath, actualOutput);

        return TestResult::Fail;
    }

	// check against reference output
    if (!File::Exists(actualOutputFile))
    {
        printf("render-test not producing expected outputs.\n");
        printf("render-test output:\n%s\n", actualOutput.Buffer());
		return TestResult::Fail;
    }
    if (!File::Exists(referenceOutput))
    {
        printf("referenceOutput %s not found.\n", referenceOutput.Buffer());
		return TestResult::Fail;
    }
    auto actualOutputContent = File::ReadAllText(actualOutputFile);
	auto actualProgramOutput = Split(actualOutputContent, '\n');
	auto referenceProgramOutput = Split(File::ReadAllText(referenceOutput), '\n');
    auto printOutput = [&]()
    {
        context->messageFormat(TestMessageType::TestFailure, "output mismatch! actual output: {\n%s\n}, \n%s\n", actualOutputContent.Buffer(), actualOutput.Buffer());
    };
    if (actualProgramOutput.Count() < referenceProgramOutput.Count())
    {
        printOutput();
		return TestResult::Fail;
    }
	for (int i = 0; i < (int)referenceProgramOutput.Count(); i++)
	{
		auto reference = String(referenceProgramOutput[i].Trim());
		auto actual = String(actualProgramOutput[i].Trim());
        if (actual != reference)
        {
            // try to parse reference as float, and compare again
            auto val = StringToFloat(reference);
            auto uval = String((unsigned int)FloatAsInt(val), 16).ToUpper();
            if (actual != uval)
            {
                printOutput();
			    return TestResult::Fail;
            }
            else
                return TestResult::Pass;
        }
	}
	return TestResult::Pass;
}

TestResult runSlangComputeComparisonTest(TestContext* context, TestInput& input)
{
	return runComputeComparisonImpl(context, input, "-slang -compute");
}

TestResult runSlangComputeComparisonTestEx(TestContext* context, TestInput& input)
{
	return runComputeComparisonImpl(context, input, "");
}

TestResult runHLSLComputeTest(TestContext* context, TestInput& input)
{
    return runComputeComparisonImpl(context, input, "-hlsl-rewrite -compute");
}

TestResult runSlangRenderComputeComparisonTest(TestContext* context, TestInput& input)
{
    return runComputeComparisonImpl(context, input, "-slang -gcompute");
}

TestResult doRenderComparisonTestRun(TestContext* context, TestInput& input, char const* langOption, char const* outputKind, String* outOutput)
{
    // TODO: delete any existing files at the output path(s) to avoid stale outputs leading to a false pass

    auto filePath = input.filePath;
    auto outputStem = input.outputStem;

    OSProcessSpawner spawner;

    spawner.pushExecutablePath(String(g_options.binDir) + "render-test" + osGetExecutableSuffix());
    spawner.pushArgument(filePath);

    for( auto arg : input.testOptions->args )
    {
        spawner.pushArgument(arg);
    }

    spawner.pushArgument(langOption);
    spawner.pushArgument("-o");
    spawner.pushArgument(outputStem + outputKind + ".png");

    if (spawnAndWait(context, outputStem, spawner) != kOSError_None)
    {
        return TestResult::Fail;
    }

    OSProcessSpawner::ResultCode resultCode = spawner.getResultCode();

    String standardOutput = spawner.getStandardOutput();
    String standardError = spawner.getStandardError();

    // We construct a single output string that captures the results
    StringBuilder outputBuilder;
    outputBuilder.Append("result code = ");
    outputBuilder.Append(resultCode);
    outputBuilder.Append("\nstandard error = {\n");
    outputBuilder.Append(standardError);
    outputBuilder.Append("}\nstandard output = {\n");
    outputBuilder.Append(standardOutput);
    outputBuilder.Append("}\n");

    String outputPath = outputStem + outputKind;
    String output = outputBuilder.ProduceString();

    *outOutput = output;

    return TestResult::Pass;
}

TestResult doImageComparison(TestContext* context, String const& filePath)
{
    // Allow a difference in the low bits of the 8-bit result, just to play it safe
    static const int kAbsoluteDiffCutoff = 2;

    // Allow a relative 1% difference
    static const float kRelativeDiffCutoff = 0.01f;

    String expectedPath = filePath + ".expected.png";
    String actualPath = filePath + ".actual.png";

    int expectedX,  expectedY,  expectedN;
    int actualX,    actualY,    actualN;

    unsigned char* expectedData = stbi_load(expectedPath.begin(), &expectedX, &expectedY, &expectedN, 0);
    unsigned char* actualData = stbi_load(actualPath.begin(), &actualX, &actualY, &actualN, 0);

    if(!expectedData)   
    {
        context->messageFormat(TestMessageType::RunError, "Unable to load image ;%s'", expectedPath.Buffer());
        return TestResult::Fail;
    }
    if(!actualData)
    {
        context->messageFormat(TestMessageType::RunError, "Unable to load image '%s'", actualPath.Buffer());
        return TestResult::Fail;
    }

    if(expectedX != actualX || expectedY != actualY || expectedN != actualN)    
    {
        context->messageFormat(TestMessageType::TestFailure, "Images are different sizes '%s' '%s'", actualPath.Buffer(), expectedPath.Buffer());
        return TestResult::Fail;
    }

    unsigned char* expectedCursor = expectedData;
    unsigned char* actualCursor = actualData;

    for( int y = 0; y < actualY; ++y )
	{
		for( int x = 0; x < actualX; ++x )
		{
			for( int n = 0; n < actualN; ++n )
			{
				int expectedVal = *expectedCursor++;
				int actualVal = *actualCursor++;

				int absoluteDiff = actualVal - expectedVal;
				if(absoluteDiff < 0) absoluteDiff = -absoluteDiff;

				if( absoluteDiff < kAbsoluteDiffCutoff )
				{
					// There might be a difference, but we'll consider it to be inside tolerance
					continue;
				}

				float relativeDiff = 0.0f;
				if( expectedVal != 0 )
				{
					relativeDiff = fabsf(float(actualVal) - float(expectedVal)) / float(expectedVal);

					if( relativeDiff < kRelativeDiffCutoff )
					{
						// relative difference was small enough
						continue;
					}
				}

				// TODO: may need to do some local search sorts of things, to deal with
				// cases where vertex shader results lead to rendering that is off
				// by one pixel...

                context->messageFormat(TestMessageType::TestFailure, "image compare failure at (%d,%d) channel %d. expected %d got %d (absolute error: %d, relative error: %f)\n",
                    x, y, n,
                    expectedVal,
                    actualVal,
                    absoluteDiff,
                    relativeDiff);

			    // There was a difference we couldn't excuse!
			    return TestResult::Fail;
		    }
		}
	}

    return TestResult::Pass;
}

TestResult runHLSLRenderComparisonTestImpl(
    TestContext* context,
    TestInput& input,
    char const* expectedArg,
    char const* actualArg)
{
    auto filePath = input.filePath;
    auto outputStem = input.outputStem;

    String expectedOutput;
    String actualOutput;

    TestResult hlslResult   =  doRenderComparisonTestRun(context, input, expectedArg, ".expected", &expectedOutput);
    TestResult slangResult  =  doRenderComparisonTestRun(context, input, actualArg, ".actual", &actualOutput);

    Slang::File::WriteAllText(outputStem + ".expected", expectedOutput);
    Slang::File::WriteAllText(outputStem + ".actual",   actualOutput);

    if( hlslResult  == TestResult::Fail )   return TestResult::Fail;
    if( slangResult == TestResult::Fail )   return TestResult::Fail;

    if (actualOutput != expectedOutput)
    {
        context->dumpOutputDifference(expectedOutput, actualOutput);

        return TestResult::Fail;
    }

    // Next do an image comparison on the expected output images!

    TestResult imageCompareResult = doImageComparison(context, outputStem);
    if(imageCompareResult != TestResult::Pass)
        return imageCompareResult;

    return TestResult::Pass;
}

TestResult runHLSLRenderComparisonTest(TestContext* context, TestInput& input)
{
    return runHLSLRenderComparisonTestImpl(context, input, "-hlsl", "-slang");
}

TestResult runHLSLCrossCompileRenderComparisonTest(TestContext* context, TestInput& input)
{
    return runHLSLRenderComparisonTestImpl(context, input, "-slang", "-glsl-cross");
}

TestResult runHLSLAndGLSLRenderComparisonTest(TestContext* context, TestInput& input)
{
    return runHLSLRenderComparisonTestImpl(context, input, "-hlsl-rewrite", "-glsl-rewrite");
}

TestResult skipTest(TestContext* /* context */, TestInput& /*input*/)
{
    return TestResult::Ignored;
}


static bool hasRenderOption(RenderApiType apiType, const List<String>& options)
{
    const RenderApiUtil::Info& info = RenderApiUtil::getInfo(apiType);

    List<UnownedStringSlice> namesList;

    for (UInt i = 0; i < options.Count(); ++i)
    {
        const String& option = options[i];

        if (option.StartsWith("-"))
        {
            const UnownedStringSlice parameter(option.Buffer() + 1, option.Buffer() + option.Length());
            // See if we have a match
            for (int j = 0; j < SLANG_COUNT_OF(RenderApiUtil::s_infos); j++)
            {
                const auto& apiInfo = RenderApiUtil::s_infos[j];
                const UnownedStringSlice names(info.names);

                if (names.indexOf(',') >= 0)
                {
                    StringUtil::split(names, ',', namesList);

                    if (namesList.IndexOf(parameter) != UInt(-1))
                    {
                        return true;
                    }
                }
                else if (names == parameter)
                {
                    return true;
                }
            }
        }
    }

    return false;
}

bool hasRenderOption(RenderApiType apiType, const TestOptions& options)
{
    return hasRenderOption(apiType, options.args);
}

bool hasRenderOption(RenderApiType apiType, const FileTestList& testList)
{
    const int numTests = int(testList.tests.Count());
    for (int i = 0; i < numTests; i++)
    {
        if (hasRenderOption(apiType, testList.tests[i].args))
        {
            return true;
        }
    }
    return false;
}

bool isHLSLTest(const String& command)
{
    return command == "COMPARE_HLSL" || 
        command == "COMPARE_HLSL_RENDER" || 
        command == "COMPARE_HLSL_CROSS_COMPILE_RENDER" || 
        command == "COMPARE_HLSL_GLSL_RENDER";
}

bool isRenderTest(const String& command)
{
    return command == "COMPARE_COMPUTE" ||
        command == "COMPARE_COMPUTE_EX" ||
        command == "HLSL_COMPUTE" ||
        command == "COMPARE_RENDER_COMPUTE" ||
        command == "COMPARE_HLSL_RENDER" ||
        command == "COMPARE_HLSL_CROSS_COMPILE_RENDER" ||
        command == "COMPARE_HLSL_GLSL_RENDER";
}

static bool canIgnoreTestWithDisabledRenderer(const TestOptions& testOptions)
{
    for (int i = 0; i < int(RenderApiType::CountOf); ++i)
    {
        RenderApiType apiType = RenderApiType(i);
        RenderApiFlag::Enum apiFlag = RenderApiFlag::Enum(1 << i);
        
        if (hasRenderOption(apiType, testOptions) && (g_options.enabledApis & apiFlag) == 0)
        {
            return true;
        }
    }

    return false;
}

TestResult runTest(
    TestContext*        context, 
    String const&       filePath,
    String const&       outputStem,
    TestOptions const&  testOptions,
    FileTestList const& testList)
{
    // If this test can be ignored
    if (canIgnoreTestWithDisabledRenderer(testOptions))
    {
        return TestResult::Ignored;
    }
    
    // based on command name, dispatch to an appropriate callback
    struct TestCommands
    {
        char const*     name;
        TestCallback    callback;
    };
	
	static const TestCommands kTestCommands[] = 
	{
        { "SIMPLE", &runSimpleTest},
        { "REFLECTION", &runReflectionTest},
#if SLANG_TEST_SUPPORT_HLSL
        { "COMPARE_HLSL", &runHLSLComparisonTest},
        { "COMPARE_HLSL_RENDER", &runHLSLRenderComparisonTest},
        { "COMPARE_HLSL_CROSS_COMPILE_RENDER", &runHLSLCrossCompileRenderComparisonTest},
        { "COMPARE_HLSL_GLSL_RENDER", &runHLSLAndGLSLRenderComparisonTest },
        { "COMPARE_COMPUTE", runSlangComputeComparisonTest},
        { "COMPARE_COMPUTE_EX", runSlangComputeComparisonTestEx},
        { "HLSL_COMPUTE", runHLSLComputeTest},
        { "COMPARE_RENDER_COMPUTE", &runSlangRenderComputeComparisonTest },

#else
        { "COMPARE_HLSL",                       &skipTest },
        { "COMPARE_HLSL_RENDER",                &skipTest },
        { "COMPARE_HLSL_CROSS_COMPILE_RENDER",  &skipTest},
        { "COMPARE_HLSL_GLSL_RENDER",           &skipTest },
        { "COMPARE_COMPUTE",                    &skipTest},
        { "COMPARE_COMPUTE_EX",                 &skipTest},
        { "HLSL_COMPUTE",                       &skipTest},
        { "COMPARE_RENDER_COMPUTE",             &skipTest },
#endif
        { "COMPARE_GLSL", &runGLSLComparisonTest },
        { "CROSS_COMPILE", &runCrossCompilerTest },
        { "EVAL", &runEvalTest },
        { nullptr, nullptr },
    };

    for( auto ii = kTestCommands; ii->name; ++ii )
    {
        if(testOptions.command != ii->name)
            continue;

        TestInput testInput;
        testInput.filePath = filePath;
        testInput.outputStem = outputStem;
        testInput.testOptions = &testOptions;
        testInput.testList = &testList;

        {
            TestContext::Scope scope(context, outputStem);

            TestResult testResult = ii->callback(context, testInput);
            context->addResult(testResult); 

            return testResult;
       }
    }

    // No actual test runner found!

    return TestResult::Fail;
}

bool testCategoryMatches(
    TestCategory*   sub,
    TestCategory*   sup)
{
    auto ss = sub;
    while(ss)
    {
        if(ss == sup)
            return true;

        ss = ss->parent;
    }
    return false;
}

bool testCategoryMatches(
    TestCategory*                           categoryToMatch,
    Dictionary<TestCategory*, TestCategory*> categorySet)
{
    for( auto item : categorySet )
    {
        if(testCategoryMatches(categoryToMatch, item.Value))
            return true;
    }
    return false;
}

bool testPassesCategoryMask(
    TestContext*        /*context*/,
    TestOptions const&  test)
{
    // Don't include a test we should filter out
    for( auto testCategory : test.categories )
    {
        if(testCategoryMatches(testCategory, g_options.excludeCategories))
            return false;
    }

    // Otherwise inclue any test the user asked for
    for( auto testCategory : test.categories )
    {
        if(testCategoryMatches(testCategory, g_options.includeCategories))
            return true;
    }

    // skip by default
    return false;
}

void runTestsOnFile(
    TestContext*    context,
    String          filePath)
{
    // Gather a list of tests to run
    FileTestList testList;

    if( gatherTestsForFile(filePath, &testList) == TestResult::Ignored )
    {
        // Test was explicitly ignored
        return;
    }

    // Note cases where a test file exists, but we found nothing to run
    if( testList.tests.Count() == 0 )
    {
        context->addTest(filePath, TestResult::Ignored);
        return;
    }

    List<TestOptions> synthesizedTests;

    // If dx12 is available synthesize Dx12 test
    if ((g_options.synthesizedTestApis & RenderApiFlag::D3D12) != 0)
    {
        // If doesn't have option generate dx12 options from dx11
        if (!hasRenderOption(RenderApiType::D3D12, testList))
        {
            const int numTests = int(testList.tests.Count());
            for (int i = 0; i < numTests; i++)
            {
                const TestOptions& testOptions = testList.tests[i];
                // If it's a render test, and there is on d3d option, add one
                if (isRenderTest(testOptions.command) && !hasRenderOption(RenderApiType::D3D12, testOptions))
                {
                    // Add with -dx12 option
                    TestOptions testOptionsCopy(testOptions);
                    testOptionsCopy.args.Add("-dx12");

                    synthesizedTests.Add(testOptionsCopy);
                }
            }
        }
    }

    // If Vulkan is available synthesize Vulkan test
    if ((g_options.synthesizedTestApis & RenderApiFlag::Vulkan) != 0)
    {
        // If doesn't have option generate dx12 options from dx11
        if (!hasRenderOption(RenderApiType::Vulkan, testList))
        {
            const int numTests = int(testList.tests.Count());
            for (int i = 0; i < numTests; i++)
            {
                const TestOptions& testOptions = testList.tests[i];
                // If it's a render test, and there is on d3d option, add one
                if (isRenderTest(testOptions.command) && !isHLSLTest(testOptions.command) && !hasRenderOption(RenderApiType::Vulkan, testOptions))
                {
                    // Add with -vk option
                    TestOptions testOptionsCopy(testOptions);
                    testOptionsCopy.args.Add("-vk");

                    UInt index = testOptionsCopy.args.IndexOf("-hlsl");
                    if (index != UInt(-1))
                    {
                        testOptionsCopy.args.RemoveAt(index);
                    }

                    synthesizedTests.Add(testOptionsCopy);
                }
            }
        }
    }

    // Add any tests that were synthesized
    for (UInt i = 0; i < synthesizedTests.Count(); ++i)
    {
        testList.tests.Add(synthesizedTests[i]);
    }

    // We have found a test to run!
    int subTestCount = 0;
    for( auto& tt : testList.tests )
    {
        int subTestIndex = subTestCount++;

        // Check that the test passes our current category mask
        if(!testPassesCategoryMask(context, tt))
        {
            continue;
        }

        String outputStem = filePath;
        if(subTestIndex != 0)
        {
            outputStem = outputStem + "." + String(subTestIndex);
        }

        /* TestResult result = */ runTest(context, filePath, outputStem, tt, testList);

        // Could determine if to continue or not here... based on result
    }
}


static bool endsWithAllowedExtension(
    TestContext*    /*context*/,
    String          filePath)
{
    char const* allowedExtensions[] = {
        ".slang",
        ".hlsl",
        ".fx",
        ".glsl",
        ".vert",
        ".frag",
        ".geom",
        ".tesc",
        ".tese",
        ".comp",
        ".internal",
        ".ahit",
        ".chit",
        ".miss",
        ".rgen",
        nullptr };

    for( auto ii = allowedExtensions; *ii; ++ii )
    {
        if(filePath.EndsWith(*ii))
            return true;
    }

    return false;
}

static bool shouldRunTest(
    TestContext*    context,
    String          filePath)
{
    if(!endsWithAllowedExtension(context, filePath))
        return false;

    if( g_options.testPrefix )
    {
        if( strncmp(g_options.testPrefix, filePath.begin(), strlen(g_options.testPrefix)) != 0 )
        {
            return false;
        }
    }

    return true;
}

void runTestsInDirectory(
    TestContext*		context,
    String				directoryPath)
{
    for (auto file : osFindFilesInDirectory(directoryPath))
    {
        if( shouldRunTest(context, file) )
        {
//            fprintf(stderr, "slang-test: found '%s'\n", file.Buffer());
            runTestsOnFile(context, file);
        }
    }
    for (auto subdir : osFindChildDirectories(directoryPath))
    {
        runTestsInDirectory(context, subdir);
    }
}


//

int main(
    int		argc,
    char**	argv)
{
    // Set up our test categories here

    auto fullTestCategory = addTestCategory("full", nullptr);

    auto quickTestCategory = addTestCategory("quick", fullTestCategory);

    /*auto smokeTestCategory = */addTestCategory("smoke", quickTestCategory);

    auto renderTestCategory = addTestCategory("render", fullTestCategory);

	/*auto computeTestCategory = */addTestCategory("compute", fullTestCategory);

    auto vulkanTestCategory = addTestCategory("vulkan", fullTestCategory);

    auto unitTestCatagory = addTestCategory("unit-test", fullTestCategory);

    // An un-categorized test will always belong to the `full` category
    g_defaultTestCategory = fullTestCategory;

    //

    if (SLANG_FAILED(parseOptions(&argc, argv)))
    {
    	// Return exit code with error
    	return 1;
    }

    if( g_options.includeCategories.Count() == 0 )
    {
        g_options.includeCategories.Add(fullTestCategory, fullTestCategory);
    }

    // Exclude rendering tests when building under AppVeyor.
    //
    // TODO: this is very ad hoc, and we should do something cleaner.
    if( g_options.outputMode == TestOutputMode::AppVeyor )
    {
        g_options.excludeCategories.Add(renderTestCategory, renderTestCategory);
        g_options.excludeCategories.Add(vulkanTestCategory, vulkanTestCategory);
    }

    // Setup the context 
    TestContext context(g_options.outputMode);
   
    context.m_dumpOutputOnFailure = g_options.dumpOutputOnFailure;
    context.m_isVerbose = g_options.shouldBeVerbose;
 
    // Enumerate test files according to policy
    // TODO: add more directories to this list
    // TODO: allow for a command-line argument to select a particular directory
    runTestsInDirectory(&context, "tests/");
    
    // Run the unit tests (these are internal C++ tests - not specified via files in a directory) 
    // They are registered with SLANG_UNIT_TEST macro
    {
        TestContext::set(&context);

        // Run the unit tests
        TestRegister* cur = TestRegister::s_first;
        while (cur)
        {
            StringBuilder filePath;
            filePath << "unit-tests/" << cur->m_name << ".internal";

            TestOptions testOptions;
            testOptions.categories.Add(unitTestCatagory);
            testOptions.command = filePath;

            if (shouldRunTest(&context, testOptions.command))
            {
                if (testPassesCategoryMask(&context, testOptions))
                {
                    context.startTest(testOptions.command);
                    // Run the test function
                    cur->m_func();
                    context.endTest();
                }
                else
                {
                    context.addTest(testOptions.command, TestResult::Ignored);
                }
            }
                
            // Next
            cur = cur->m_next;
        }

        TestContext::set(nullptr);
    }
        

    context.outputSummary();

    return context.didAllSucceed() ? 0 : 1; 
}
