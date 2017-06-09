// main.cpp

#include "../../source/core/slang-io.h"

using namespace CoreLib::Basic;
using namespace CoreLib::IO;

#include "os.h"

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
};
Options options;

void parseOptions(int* argc, char** argv)
{
    int argCount = *argc;
    char const* const* argCursor = argv;
    char const* const* argEnd = argCursor + argCount;

    char const** writeCursor = (char const**) argv;

    // first argument is the application name
    if( argCursor != argEnd )
    {
        options.appName = *argCursor++;
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
                char const* arg = *argCursor++;
                *writeCursor++ = arg;
            }
            break;
        }

        if( strcmp(arg, "--bindir") == 0 )
        {
            if( argCursor == argEnd )
            {
                fprintf(stderr, "error: expected operand for '%s'\n", arg);
                exit(1);
            }
            options.binDir = *argCursor++;
        }
        else if( strcmp(arg, "-v") == 0 )
        {
            options.shouldBeVerbose = true;
        }
        else if( strcmp(arg, "-generate-hlsl-baselines") == 0 )
        {
            options.generateHLSLBaselines = true;
        }
        else if( strcmp(arg, "-release") == 0 )
        {
            // Assumed to be handle by .bat file that called us
        }
        else if( strcmp(arg, "-debug") == 0 )
        {
            // Assumed to be handle by .bat file that called us
        }
        else
        {
            fprintf(stderr, "unknown option '%s'\n", arg);
            exit(1);
        }
    }
    
    // any arguments left over were positional arguments
    argCount = (int)(writeCursor - argv);
    argCursor = argv;
    argEnd = argCursor + argCount;

    // first positional argument is a "filter" to apply
    if( argCursor != argEnd )
    {
        options.testPrefix = *argCursor++;
    }

    // any remaining arguments represent an error
    if(argCursor != argEnd)
    {
        fprintf(stderr, "unexpected arguments\n");
        exit(1);
    }

    *argc = 0;
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

enum TestResult
{
    kTestResult_Fail,
    kTestResult_Pass,
    kTestResult_Ignored,
};

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
            // fall through to:
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

// Optiosn for a particular test
struct TestOptions
{
    String command;
    List<String> args;
};

// Information on tests to run for a particular file
struct FileTestList
{
    List<TestOptions> tests;
};

TestResult gatherTestOptions(
    char const**    ioCursor,
    FileTestList*   testList)
{
    char const* cursor = *ioCursor;

    // Start by scanning for the sub-command name:
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
            return kTestResult_Fail;
        }

        break;
    }
    char const* commandEnd = cursor;
    if(*cursor == ':')
        cursor++;

    TestOptions testOptions;
    testOptions.command = getString(commandStart, commandEnd);

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
            return kTestResult_Pass;

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
        fileContents = CoreLib::IO::File::ReadAllText(filePath);
    }
    catch (CoreLib::IO::IOException)
    {
        return kTestResult_Fail;
    }


    // Walk through the lines of the file, looking for test commands
    char const* cursor = fileContents.begin();

    while(*cursor)
    {
        // We are at the start of a line of input.

        skipHorizontalSpace(&cursor);

        // Look for a pattern that matches what we want
        if(match(&cursor, "//TEST:"))
        {
            if(gatherTestOptions(&cursor, testList) != kTestResult_Pass)
                return kTestResult_Fail;
        }
        else if(match(&cursor, "//TEST_IGNORE_FILE"))
        {
            return kTestResult_Ignored;
        }
        else
        {
            skipToEndOfLine(&cursor);
        }
    }

    return kTestResult_Pass;
}

OSError spawnAndWait(String	testPath, OSProcessSpawner& spawner)
{
    if( options.shouldBeVerbose )
    {
        fprintf(stderr, "%s\n", spawner.commandLine_.Buffer());
    }

    OSError err = spawner.spawnAndWaitForCompletion();
    if (err != kOSError_None)
    {
        error("failed to run test '%S'", testPath.ToWString());
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

struct TestInput
{
    String              filePath;
    TestOptions const*  testOptions;
    FileTestList const* testList;
};

typedef TestResult (*TestCallback)(TestInput& input);

TestResult runSimpleTest(TestInput& input)
{
    // need to execute the stand-alone Slang compiler on the file, and compare its output to what we expect

    auto filePath = input.filePath;

    OSProcessSpawner spawner;

    spawner.pushExecutableName(String(options.binDir) + "slangc.exe");
    spawner.pushArgument(filePath);

    for( auto arg : input.testOptions->args )
    {
        spawner.pushArgument(arg);
    }

    if (spawnAndWait(filePath, spawner) != kOSError_None)
    {
        return kTestResult_Fail;
    }

    String actualOutput = getOutput(spawner);

    String expectedOutputPath = filePath + ".expected";
    String expectedOutput;
    try
    {
        expectedOutput = CoreLib::IO::File::ReadAllText(expectedOutputPath);
    }
    catch (CoreLib::IO::IOException)
    {
    }

    // If no expected output file was found, then we
    // expect everything to be empty
    if (expectedOutput.Length() == 0)
    {
        expectedOutput = "result code = 0\nstandard error = {\n}\nstandard output = {\n}\n";
    }

    TestResult result = kTestResult_Pass;

    // Otherwise we compare to the expected output
    if (actualOutput != expectedOutput)
    {
        result = kTestResult_Fail;
    }

    // If the test failed, then we write the actual output to a file
    // so that we can easily diff it from the command line and
    // diagnose the problem.
    if (result == kTestResult_Fail)
    {
        String actualOutputPath = filePath + ".actual";
        CoreLib::IO::File::WriteAllText(actualOutputPath, actualOutput);
    }

    return result;
}

#ifdef SLANG_TEST_SUPPORT_HLSL
TestResult generateHLSLBaseline(TestInput& input)
{
    auto filePath = input.filePath;

    OSProcessSpawner spawner;
    spawner.pushExecutableName(String(options.binDir) + "slangc.exe");
    spawner.pushArgument(filePath);

    for( auto arg : input.testOptions->args )
    {
        spawner.pushArgument(arg);
    }

    spawner.pushArgument("-target");
    spawner.pushArgument("dxbc-assembly");
    spawner.pushArgument("-pass-through");
    spawner.pushArgument("fxc");

    if (spawnAndWait(filePath, spawner) != kOSError_None)
    {
        return kTestResult_Fail;
    }

    String expectedOutput = getOutput(spawner);
    String expectedOutputPath = filePath + ".expected";
    try
    {
        CoreLib::IO::File::WriteAllText(expectedOutputPath, expectedOutput);
    }
    catch (CoreLib::IO::IOException)
    {
        return kTestResult_Fail;
    }
    return kTestResult_Pass;
}

TestResult runHLSLComparisonTest(TestInput& input)
{
    auto filePath = input.filePath;

    // We will use the Microsoft compiler to generate out expected output here
    String expectedOutputPath = filePath + ".expected";

    // Generate the expected output using standard HLSL compiler
    generateHLSLBaseline(input);

    // need to execute the stand-alone Slang compiler on the file, and compare its output to what we expect

    OSProcessSpawner spawner;

    spawner.pushExecutableName(String(options.binDir) + "slangc.exe");
    spawner.pushArgument(filePath);

    for( auto arg : input.testOptions->args )
    {
        spawner.pushArgument(arg);
    }

    // TODO: The compiler should probably define this automatically...
    spawner.pushArgument("-D");
    spawner.pushArgument("__SLANG__");

    spawner.pushArgument("-target");
    spawner.pushArgument("dxbc-assembly");

    if (spawnAndWait(filePath, spawner) != kOSError_None)
    {
        return kTestResult_Fail;
    }

    // We ignore output to stdout, and only worry about what the compiler
    // wrote to stderr.

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

    String actualOutput = actualOutputBuilder.ProduceString();

    String expectedOutput;
    try
    {
        expectedOutput = CoreLib::IO::File::ReadAllText(expectedOutputPath);
    }
    catch (CoreLib::IO::IOException)
    {
    }

    TestResult result = kTestResult_Pass;

    // If no expected output file was found, then we
    // expect everything to be empty
    if (expectedOutput.Length() == 0)
    {
        if (resultCode != 0)				result = kTestResult_Fail;
        if (standardError.Length() != 0)	result = kTestResult_Fail;
        if (standardOuptut.Length() != 0)	result = kTestResult_Fail;
    }
    // Otherwise we compare to the expected output
    else if (actualOutput != expectedOutput)
    {
        result = kTestResult_Fail;
    }

    // If the test failed, then we write the actual output to a file
    // so that we can easily diff it from the command line and
    // diagnose the problem.
    if (result == kTestResult_Fail)
    {
        String actualOutputPath = filePath + ".actual";
        CoreLib::IO::File::WriteAllText(actualOutputPath, actualOutput);
    }

    return result;
}
#endif

TestResult doGLSLComparisonTestRun(
    TestInput& input,
    char const* langDefine,
    char const* passThrough,
    char const* outputKind,
    String* outOutput)
{
    auto filePath = input.filePath;

    OSProcessSpawner spawner;

    spawner.pushExecutableName(String(options.binDir) + "slangc.exe");
    spawner.pushArgument(filePath);

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

    spawner.pushArgument("-no-checking");

    spawner.pushArgument("-target");
    spawner.pushArgument("spirv-assembly");

    for( auto arg : input.testOptions->args )
    {
        spawner.pushArgument(arg);
    }

    if (spawnAndWait(filePath, spawner) != kOSError_None)
    {
        return kTestResult_Fail;
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

    String outputPath = filePath + outputKind;
    String output = outputBuilder.ProduceString();

    *outOutput = output;

    return kTestResult_Pass;
}

TestResult runGLSLComparisonTest(TestInput& input)
{
    auto filePath = input.filePath;

    String expectedOutput;
    String actualOutput;

    TestResult hlslResult   =  doGLSLComparisonTestRun(input, "__GLSL__",  "glslang", ".expected",    &expectedOutput);
    TestResult slangResult  =  doGLSLComparisonTestRun(input, "__SLANG__", nullptr,   ".actual",      &actualOutput);

    CoreLib::IO::File::WriteAllText(filePath + ".expected", expectedOutput);
    CoreLib::IO::File::WriteAllText(filePath + ".actual",   actualOutput);

    if( hlslResult  == kTestResult_Fail )   return kTestResult_Fail;
    if( slangResult == kTestResult_Fail )   return kTestResult_Fail;

    if (actualOutput != expectedOutput)
    {
        return kTestResult_Fail;
    }

    return kTestResult_Pass;
}



TestResult doRenderComparisonTestRun(TestInput& input, char const* langOption, char const* outputKind, String* outOutput)
{
    // TODO: delete any existing files at the output path(s) to avoid stale outputs leading to a false pass

    auto filePath = input.filePath;

    OSProcessSpawner spawner;

    spawner.pushExecutableName(String(options.binDir) + "render-test.exe");
    spawner.pushArgument(filePath);

    for( auto arg : input.testOptions->args )
    {
        spawner.pushArgument(arg);
    }

    spawner.pushArgument(langOption);
    spawner.pushArgument("-o");
    spawner.pushArgument(filePath + outputKind + ".png");

    if (spawnAndWait(filePath, spawner) != kOSError_None)
    {
        return kTestResult_Fail;
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

    String outputPath = filePath + outputKind;
    String output = outputBuilder.ProduceString();

    *outOutput = output;

    return kTestResult_Pass;
}

TestResult doImageComparison(String const& filePath)
{
    // Allow a difference in the low bits of the 8-bit result, just to play it safe
    static const int kAbsoluteDiffCutoff = 2;

    // Allow a relatie 1% difference
    static const float kRelativeDiffCutoff = 0.01f;

    String expectedPath = filePath + ".expected.png";
    String actualPath = filePath + ".actual.png";

    int expectedX,  expectedY,  expectedN;
    int actualX,    actualY,    actualN;


    unsigned char* expectedData = stbi_load(expectedPath.begin(), &expectedX, &expectedY, &expectedN, 0);
    unsigned char* actualData = stbi_load(actualPath.begin(), &actualX, &actualY, &actualN, 0);

    if(!expectedData)   return kTestResult_Fail;
    if(!actualData)     return kTestResult_Fail;

    if(expectedX != actualX)    return kTestResult_Fail;
    if(expectedY != actualY)    return kTestResult_Fail;
    if(expectedN != actualN)    return kTestResult_Fail;

    unsigned char* expectedCursor = expectedData;
    unsigned char* actualCursor = actualData;

    for( int y = 0; y < actualY; ++y )
    for( int x = 0; x < actualX; ++x )
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

        if( expectedVal != 0 )
        {
            float relativeDiff = fabsf(float(actualVal) - float(expectedVal)) / float(expectedVal);

            if( relativeDiff < kRelativeDiffCutoff )
            {
                // relative difference was small enough
                continue;
            }
        }

        // TODO: may need to do some local search sorts of things, to deal with
        // cases where vertex shader results lead to rendering that is off
        // by one pixel...

        // There was a difference we couldn't excuse!
        return kTestResult_Fail;
    }

    return kTestResult_Pass;
}

TestResult runHLSLRenderComparisonTestImpl(
    TestInput& input,
    char const* expectedArg,
    char const* actualArg)
{
    auto filePath = input.filePath;

    String expectedOutput;
    String actualOutput;

    TestResult hlslResult   =  doRenderComparisonTestRun(input, expectedArg,  ".expected",    &expectedOutput);
    TestResult slangResult  =  doRenderComparisonTestRun(input, actualArg, ".actual",      &actualOutput);

    CoreLib::IO::File::WriteAllText(filePath + ".expected", expectedOutput);
    CoreLib::IO::File::WriteAllText(filePath + ".actual",   actualOutput);

    if( hlslResult  == kTestResult_Fail )   return kTestResult_Fail;
    if( slangResult == kTestResult_Fail )   return kTestResult_Fail;

    if (actualOutput != expectedOutput)
    {
        return kTestResult_Fail;
    }

    // Next do an image comparison on the expected output images!

    TestResult imageCompareResult = doImageComparison(filePath);
    if(imageCompareResult != kTestResult_Pass)
        return imageCompareResult;

    return kTestResult_Pass;
}

TestResult runHLSLRenderComparisonTest(TestInput& input)
{
    return runHLSLRenderComparisonTestImpl(input, "-hlsl", "-slang");
}

TestResult runHLSLCrossCompileRenderComparisonTest(TestInput& input)
{
    return runHLSLRenderComparisonTestImpl(input, "-slang", "-glsl-cross");
}

TestResult runTest(
    String const&       filePath,
    TestOptions const&  testOptions,
    FileTestList const& testList)
{
    // based on command name, dispatch to an appropriate callback
    static const struct TestCommands
    {
        char const*     name;
        TestCallback    callback;
    } kTestCommands[] = {
        { "SIMPLE", &runSimpleTest },
        { "COMPARE_HLSL", &runHLSLComparisonTest },
        { "COMPARE_HLSL_RENDER", &runHLSLRenderComparisonTest },
        { "COMPARE_HLSL_CROSS_COMPILE_RENDER", &runHLSLCrossCompileRenderComparisonTest},
        { "COMPARE_GLSL", &runGLSLComparisonTest },
        { nullptr, nullptr },
    };

    for( auto ii = kTestCommands; ii->name; ++ii )
    {
        if(testOptions.command != ii->name)
            continue;

        TestInput testInput;
        testInput.filePath = filePath;
        testInput.testOptions = &testOptions;
        testInput.testList = &testList;

        return ii->callback(testInput);
    }

    // No actual test runner found!

    return kTestResult_Fail;
}


struct TestContext
{
    int totalTestCount;
    int passedTestCount;
    int failedTestCount;
};

void runTestsOnFile(
    TestContext*    context,
    String          filePath)
{
    // Gather a list of tests to run
    FileTestList testList;

    if( gatherTestsForFile(filePath, &testList) == kTestResult_Ignored )
    {
        // Test was explicitly ignored
        return;
    }

    // Note cases where a test file exists, but we found nothing to run
    if( testList.tests.Count() == 0 )
    {
        context->totalTestCount++;
        context->failedTestCount++;

        printf("FAILED test: '%S' (no test commands found)\n", filePath.ToWString());
        return;
    }

    // We have found a test to run!
    int subTestCount = 0;
    for( auto& tt : testList.tests )
    {
        context->totalTestCount++;

        int subTestIndex = subTestCount++;

        TestResult result = runTest(filePath, tt, testList);
        if(result == kTestResult_Ignored)
            return;

        if (result == kTestResult_Pass)
        {
            printf("passed");
            context->passedTestCount++;
        }
        else
        {
            printf("FAILED");
            context->failedTestCount++;
        }

        printf(" test: '%S'", filePath.ToWString());
        if( subTestIndex )
        {
            printf(" subtest:%d", subTestIndex);
        }
        printf("\n");
    }
}


static bool endsWithAllowedExtension(
    TestContext*    context,
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

    if( options.testPrefix )
    {
        if( strncmp(options.testPrefix, filePath.begin(), strlen(options.testPrefix)) != 0 )
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
    parseOptions(&argc, argv);

    TestContext context = { 0 };

    // Enumerate test files according to policy
    // TODO: add more directories to this list
    // TODO: allow for a command-line argument to select a particular directory
    runTestsInDirectory(&context, "tests/");

    if (!context.totalTestCount)
    {
        printf("no tests run\n");
        return 0;
    }

    printf("\n===\n%d%% of tests passed (%d/%d)\n===\n\n", (context.passedTestCount*100) / context.totalTestCount, context.passedTestCount, context.totalTestCount);
    return context.passedTestCount == context.totalTestCount ? 0 : 1;
}
