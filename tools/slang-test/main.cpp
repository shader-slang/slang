// main.cpp

#include "../../source/core/slang-io.h"

using namespace Slang;

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

enum OutputMode
{
    // Default mode is to write test results to the console
    kOutputMode_Default = 0,

    // When running under AppVeyor contiuous integration, we
    // need to output test results in a way that the AppVeyor
    // environment can pick up and display.
    kOutputMode_AppVeyor,
};

struct TestCategory;
TestCategory* findTestCategory(String const& name);

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
    OutputMode outputMode = kOutputMode_Default;

    // Only run tests that match one of the given categories
    Dictionary<TestCategory*, TestCategory*> includeCategories;

    // Exclude test taht match one these categories
    Dictionary<TestCategory*, TestCategory*> excludeCategories;
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

        if( strcmp(arg, "-bindir") == 0 )
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
        else if( strcmp(arg, "-configuration") == 0 )
        {
            if( argCursor == argEnd )
            {
                fprintf(stderr, "error: expected operand for '%s'\n", arg);
                exit(1);
            }
            argCursor++;
            // Assumed to be handle by .bat file that called us
        }
        else if( strcmp(arg, "-platform") == 0 )
        {
            if( argCursor == argEnd )
            {
                fprintf(stderr, "error: expected operand for '%s'\n", arg);
                exit(1);
            }
            argCursor++;
            // Assumed to be handle by .bat file that called us
        }
        else if( strcmp(arg, "-appveyor") == 0 )
        {
            options.outputMode = kOutputMode_AppVeyor;
            options.dumpOutputOnFailure = true;
        }
        else if( strcmp(arg, "-travis") == 0 )
        {
            options.dumpOutputOnFailure = true;
        }
        else if( strcmp(arg, "-category") == 0 )
        {
            if( argCursor == argEnd )
            {
                fprintf(stderr, "error: expected operand for '%s'\n", arg);
                exit(1);
            }
            auto category = findTestCategory(*argCursor++);
            if(category)
            {
                options.includeCategories.Add(category, category);
            }
        }
        else if( strcmp(arg, "-exclude") == 0 )
        {
            if( argCursor == argEnd )
            {
                fprintf(stderr, "error: expected operand for '%s'\n", arg);
                exit(1);
            }
            auto category = findTestCategory(*argCursor++);
            if(category)
            {
                options.excludeCategories.Add(category, category);
            }
        }
        else
        {
            fprintf(stderr, "unknown option '%s'\n", arg);
            exit(1);
        }
    }
    
    // any arguments left over were positional arguments
    argCount = (int)((char**)writeCursor - argv);
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

// A category that a test can be tagged with
struct TestCategory
{
    // The name of the category, from the user perspective
    String name;

    // The logical "super-category" of this category
    TestCategory* parent;

    // A list of categories that we explcitly want to exclude
    List<TestCategory*> prohibitedCategories;
};

Dictionary<String, TestCategory*> testCategories;
TestCategory* defaultTestCategory;

TestCategory* addTestCategory(String const& name, TestCategory* parent)
{
    TestCategory* category = new TestCategory();
    category->name = name;

    category->parent = parent;

    testCategories.Add(name, category);

    return category;
}

TestCategory* findTestCategory(String const& name)
{
    TestCategory* category = nullptr;
    if( !testCategories.TryGetValue(name, category) )
    {
        error("unknown test category name '%s'\n", name.Buffer());
        return nullptr;
    }
    return category;
}

// Optiosn for a particular test
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
                        return kTestResult_Fail;
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
                return kTestResult_Fail;
            }

            break;
        }
    }

    // If no categories were specified, then add the default category
    if(testOptions.categories.Count() == 0)
    {
        testOptions.categories.Add(defaultTestCategory);
    }

    if(*cursor == ':')
        cursor++;
    else
    {
        return kTestResult_Fail;
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
            return kTestResult_Fail;
        }

        break;
    }
    char const* commandEnd = cursor;

    testOptions.command = getString(commandStart, commandEnd);

    if(*cursor == ':')
        cursor++;
    else
    {
        return kTestResult_Fail;
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
        fileContents = Slang::File::ReadAllText(filePath);
    }
    catch (Slang::IOException)
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
        if(match(&cursor, "//TEST_IGNORE_FILE"))
        {
            return kTestResult_Ignored;
        }
        else if(match(&cursor, "//TEST"))
        {
            if(gatherTestOptions(&cursor, testList) != kTestResult_Pass)
                return kTestResult_Fail;
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
        String commandLine = spawner.getCommandLine();
        fprintf(stderr, "%s\n", commandLine.begin());
    }

    OSError err = spawner.spawnAndWaitForCompletion();
    if (err != kOSError_None)
    {
//        fprintf(stderr, "failed to run test '%S'\n", testPath.ToWString());
        error("failed to run test '%S'", testPath.ToWString().begin());
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

List<String> gFailedTests;

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

typedef TestResult (*TestCallback)(TestInput& input);

void maybeDumpOutput(
    String const& expectedOutput,
    String const& actualOutput)
{
    if (!options.dumpOutputOnFailure)
        return;

    fprintf(stderr, "ERROR:\n"
        "EXPECTED{{{\n%s}}}\n"
        "ACTUAL{{{\n%s}}}\n",
        expectedOutput.Buffer(),
        actualOutput.Buffer());
    fflush(stderr);
}

TestResult runSimpleTest(TestInput& input)
{
    // need to execute the stand-alone Slang compiler on the file, and compare its output to what we expect

    auto filePath999 = input.filePath;
    auto outputStem = input.outputStem;

    OSProcessSpawner spawner;

    spawner.pushExecutablePath(String(options.binDir) + "slangc" + osGetExecutableSuffix());
    spawner.pushArgument(filePath999);

    for( auto arg : input.testOptions->args )
    {
        spawner.pushArgument(arg);
    }

    if (spawnAndWait(outputStem, spawner) != kOSError_None)
    {
        return kTestResult_Fail;
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
        String actualOutputPath = outputStem + ".actual";
        Slang::File::WriteAllText(actualOutputPath, actualOutput);

        maybeDumpOutput(expectedOutput, actualOutput);
    }

    return result;
}

TestResult runReflectionTest(TestInput& input)
{
    auto filePath = input.filePath;
    auto outputStem = input.outputStem;

    OSProcessSpawner spawner;

    spawner.pushExecutablePath(String(options.binDir) + "slang-reflection-test" + osGetExecutableSuffix());
    spawner.pushArgument(filePath);

    for( auto arg : input.testOptions->args )
    {
        spawner.pushArgument(arg);
    }

    if (spawnAndWait(outputStem, spawner) != kOSError_None)
    {
        return kTestResult_Fail;
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
        String actualOutputPath = outputStem + ".actual";
        Slang::File::WriteAllText(actualOutputPath, actualOutput);

        maybeDumpOutput(expectedOutput, actualOutput);
    }

    return result;
}

TestResult runEvalTest(TestInput& input)
{
    // We are going to load and evaluate the code

    auto filePath = input.filePath;
    auto outputStem = input.outputStem;

    OSProcessSpawner spawner;

    spawner.pushExecutablePath(String(options.binDir) + "slang-eval-test" + osGetExecutableSuffix());
    spawner.pushArgument(filePath);

    for( auto arg : input.testOptions->args )
    {
        spawner.pushArgument(arg);
    }

    if (spawnAndWait(outputStem, spawner) != kOSError_None)
    {
        return kTestResult_Fail;
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
        String actualOutputPath = outputStem + ".actual";
        Slang::File::WriteAllText(actualOutputPath, actualOutput);

        maybeDumpOutput(expectedOutput, actualOutput);
    }

    return result;
}


TestResult runCrossCompilerTest(TestInput& input)
{
    // need to execute the stand-alone Slang compiler on the file
    // then on the same file + `.glsl` and compare output

    auto filePath = input.filePath;
    auto outputStem = input.outputStem;

    OSProcessSpawner actualSpawner;
    OSProcessSpawner expectedSpawner;

    actualSpawner.pushExecutablePath(String(options.binDir) + "slangc" + osGetExecutableSuffix());
    expectedSpawner.pushExecutablePath(String(options.binDir) + "slangc" + osGetExecutableSuffix());

    actualSpawner.pushArgument(filePath);
    expectedSpawner.pushArgument(filePath + ".glsl");

    for( auto arg : input.testOptions->args )
    {
        actualSpawner.pushArgument(arg);
        expectedSpawner.pushArgument(arg);
    }
    expectedSpawner.pushArgument("-no-checking");

    if (spawnAndWait(outputStem, expectedSpawner) != kOSError_None)
    {
        return kTestResult_Fail;
    }

    String expectedOutput = getOutput(expectedSpawner);
    String expectedOutputPath = outputStem + ".expected";
    try
    {
        Slang::File::WriteAllText(expectedOutputPath, expectedOutput);
    }
    catch (Slang::IOException)
    {
        return kTestResult_Fail;
    }

    if (spawnAndWait(outputStem, actualSpawner) != kOSError_None)
    {
        return kTestResult_Fail;
    }
    String actualOutput = getOutput(actualSpawner);

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
        String actualOutputPath = outputStem + ".actual";
        Slang::File::WriteAllText(actualOutputPath, actualOutput);

        maybeDumpOutput(expectedOutput, actualOutput);
    }

    return result;
}


#ifdef SLANG_TEST_SUPPORT_HLSL
TestResult generateHLSLBaseline(TestInput& input)
{
    auto filePath999 = input.filePath;
    auto outputStem = input.outputStem;

    OSProcessSpawner spawner;
    spawner.pushExecutablePath(String(options.binDir) + "slangc" + osGetExecutableSuffix());
    spawner.pushArgument(filePath999);

    for( auto arg : input.testOptions->args )
    {
        spawner.pushArgument(arg);
    }

    spawner.pushArgument("-target");
    spawner.pushArgument("dxbc-assembly");
    spawner.pushArgument("-pass-through");
    spawner.pushArgument("fxc");

    if (spawnAndWait(outputStem, spawner) != kOSError_None)
    {
        return kTestResult_Fail;
    }

    String expectedOutput = getOutput(spawner);
    String expectedOutputPath = outputStem + ".expected";
    try
    {
        Slang::File::WriteAllText(expectedOutputPath, expectedOutput);
    }
    catch (Slang::IOException)
    {
        return kTestResult_Fail;
    }
    return kTestResult_Pass;
}

TestResult runHLSLComparisonTest(TestInput& input)
{
    auto filePath999 = input.filePath;
    auto outputStem = input.outputStem;

    // We will use the Microsoft compiler to generate out expected output here
    String expectedOutputPath = outputStem + ".expected";

    // Generate the expected output using standard HLSL compiler
    generateHLSLBaseline(input);

    // need to execute the stand-alone Slang compiler on the file, and compare its output to what we expect

    OSProcessSpawner spawner;

    spawner.pushExecutablePath(String(options.binDir) + "slangc" + osGetExecutableSuffix());
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

    if (spawnAndWait(outputStem, spawner) != kOSError_None)
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
        expectedOutput = Slang::File::ReadAllText(expectedOutputPath);
    }
    catch (Slang::IOException)
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
        String actualOutputPath = outputStem + ".actual";
        Slang::File::WriteAllText(actualOutputPath, actualOutput);

        maybeDumpOutput(expectedOutput, actualOutput);
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
    auto filePath999 = input.filePath;
    auto outputStem = input.outputStem;

    OSProcessSpawner spawner;

    spawner.pushExecutablePath(String(options.binDir) + "slangc" + osGetExecutableSuffix());
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

    spawner.pushArgument("-no-checking");

    spawner.pushArgument("-target");
    spawner.pushArgument("spirv-assembly");

    for( auto arg : input.testOptions->args )
    {
        spawner.pushArgument(arg);
    }

    if (spawnAndWait(outputStem, spawner) != kOSError_None)
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

    String outputPath = outputStem + outputKind;
    String output = outputBuilder.ProduceString();

    *outOutput = output;

    return kTestResult_Pass;
}

TestResult runGLSLComparisonTest(TestInput& input)
{
    auto filePath999 = input.filePath;
    auto outputStem = input.outputStem;

    String expectedOutput;
    String actualOutput;

    TestResult hlslResult   =  doGLSLComparisonTestRun(input, "__GLSL__",  "glslang", ".expected",    &expectedOutput);
    TestResult slangResult  =  doGLSLComparisonTestRun(input, "__SLANG__", nullptr,   ".actual",      &actualOutput);

    Slang::File::WriteAllText(outputStem + ".expected", expectedOutput);
    Slang::File::WriteAllText(outputStem + ".actual",   actualOutput);

    if( hlslResult  == kTestResult_Fail )   return kTestResult_Fail;
    if( slangResult == kTestResult_Fail )   return kTestResult_Fail;

    if (actualOutput != expectedOutput)
    {
        maybeDumpOutput(expectedOutput, actualOutput);

        return kTestResult_Fail;
    }

    return kTestResult_Pass;
}

TestResult doComputeComparisonTestRunImpl(TestInput& input, const char * langOption, String referenceOutput)
{
	// TODO: delete any existing files at the output path(s) to avoid stale outputs leading to a false pass
	auto filePath999 = input.filePath;
	auto outputStem = input.outputStem;

	OSProcessSpawner spawner;

	spawner.pushExecutablePath(String(options.binDir) + "render-test" + osGetExecutableSuffix());
	spawner.pushArgument(filePath999);

	for (auto arg : input.testOptions->args)
	{
		spawner.pushArgument(arg);
	}

	spawner.pushArgument(langOption);
	spawner.pushArgument("-o");
	spawner.pushArgument(outputStem + ".actual.txt");

	if (spawnAndWait(outputStem, spawner) != kOSError_None)
	{
		return kTestResult_Fail;
	}

	// check against reference output
	if (!File::Exists(outputStem + ".actual.txt"))
		return kTestResult_Fail;
	if (!File::Exists(referenceOutput))
		return kTestResult_Fail;

	auto actualProgramOutput = Split(File::ReadAllText(outputStem + ".actual.txt"), '\n');
	auto referenceProgramOutput = Split(File::ReadAllText(referenceOutput), '\n');
	if (actualProgramOutput.Count() < referenceProgramOutput.Count())
		return kTestResult_Fail;
	for (int i = 0; i < referenceProgramOutput.Count(); i++)
	{
		auto reference = StringToFloat(referenceProgramOutput[i]);
		auto actual = StringToFloat(actualProgramOutput[i]);
		if (abs(actual - reference) > 1e-7f)
			return kTestResult_Fail;
	}
	return kTestResult_Pass;
}

TestResult doSlangComputeComparisonTest(TestInput& input)
{
	return doComputeComparisonTestRunImpl(input, "-slang -compute", input.outputStem + ".expected.txt");
}

TestResult doRenderComparisonTestRun(TestInput& input, char const* langOption, char const* outputKind, String* outOutput)
{
    // TODO: delete any existing files at the output path(s) to avoid stale outputs leading to a false pass

    auto filePath999 = input.filePath;
    auto outputStem = input.outputStem;

    OSProcessSpawner spawner;

    spawner.pushExecutablePath(String(options.binDir) + "render-test" + osGetExecutableSuffix());
    spawner.pushArgument(filePath999);

    for( auto arg : input.testOptions->args )
    {
        spawner.pushArgument(arg);
    }

    spawner.pushArgument(langOption);
    spawner.pushArgument("-o");
    spawner.pushArgument(outputStem + outputKind + ".png");

    if (spawnAndWait(outputStem, spawner) != kOSError_None)
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

    String outputPath = outputStem + outputKind;
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

        fprintf(stderr, "image compare failure at (%d,%d) channel %d. expected %d got %d (absolute error: %d, relative error: %f)\n",
            x, y, n,
            expectedVal,
            actualVal,
            absoluteDiff,
            relativeDiff);

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
    auto filePath999 = input.filePath;
    auto outputStem = input.outputStem;

    String expectedOutput;
    String actualOutput;

    TestResult hlslResult   =  doRenderComparisonTestRun(input, expectedArg,  ".expected",    &expectedOutput);
    TestResult slangResult  =  doRenderComparisonTestRun(input, actualArg,    ".actual",      &actualOutput);

    Slang::File::WriteAllText(outputStem + ".expected", expectedOutput);
    Slang::File::WriteAllText(outputStem + ".actual",   actualOutput);

    if( hlslResult  == kTestResult_Fail )   return kTestResult_Fail;
    if( slangResult == kTestResult_Fail )   return kTestResult_Fail;

    if (actualOutput != expectedOutput)
    {
        maybeDumpOutput(expectedOutput, actualOutput);

        return kTestResult_Fail;
    }

    // Next do an image comparison on the expected output images!

    TestResult imageCompareResult = doImageComparison(outputStem);
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

TestResult runHLSLAndGLSLComparisonTest(TestInput& input)
{
    return runHLSLRenderComparisonTestImpl(input, "-hlsl-rewrite", "-glsl-rewrite");
}

TestResult skipTest(TestInput& input)
{
    return kTestResult_Ignored;
}

TestResult runTest(
    String const&       filePath,
    String const&       outputStem,
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
        { "REFLECTION", &runReflectionTest },
#if SLANG_TEST_SUPPORT_HLSL
        { "COMPARE_HLSL", &runHLSLComparisonTest },
        { "COMPARE_HLSL_RENDER", &runHLSLRenderComparisonTest },
        { "COMPARE_HLSL_CROSS_COMPILE_RENDER", &runHLSLCrossCompileRenderComparisonTest},
        { "COMPARE_HLSL_GLSL_RENDER", &runHLSLAndGLSLComparisonTest },
#else
        { "COMPARE_HLSL",                       &skipTest },
        { "COMPARE_HLSL_RENDER",                &skipTest },
        { "COMPARE_HLSL_CROSS_COMPILE_RENDER",  &skipTest},
        { "COMPARE_HLSL_GLSL_RENDER",           &skipTest },
#endif
        { "COMPARE_GLSL", &runGLSLComparisonTest },
		{ "COMPARE_COMPUTE", &doSlangComputeComparisonTest},

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
    int ignoredTestCount;
};

// deal with the fallout of a test having completed, whether
// passed or failed or who-knows-what.
void handleTestResult(
    TestContext*    context,
    String const&   testName,
    TestResult      testResult)
{
    switch( testResult )
    {
    case kTestResult_Fail:
        context->failedTestCount++;
        gFailedTests.Add(testName);
        break;

    case kTestResult_Pass:
        context->passedTestCount++;
        break;

    case kTestResult_Ignored:
        context->ignoredTestCount++;
        break;

    default:
        assert(!"unexpected");
        break;
    }

//    printf("OUTPUT_MODE: %d\n", options.outputMode);
    switch( options.outputMode )
    {
    case kOutputMode_Default:
        {
            char const* resultString = "UNEXPECTED";
            switch( testResult )
            {
            case kTestResult_Fail:      resultString = "FAILED";  break;
            case kTestResult_Pass:      resultString = "passed";  break;
            case kTestResult_Ignored:   resultString = "ignored"; break;
            default:
                assert(!"unexpected");
                break;
            }

            printf("%s test: '%S'\n", resultString, testName.ToWString().begin());
        }
        break;

    case kOutputMode_AppVeyor:
        {
            char const* resultString = "None";
            switch( testResult )
            {
            case kTestResult_Fail:      resultString = "Failed";  break;
            case kTestResult_Pass:      resultString = "Passed";  break;
            case kTestResult_Ignored:   resultString = "Ignored"; break;
            default:
                assert(!"unexpected");
                break;
            }


            OSProcessSpawner spawner;
            spawner.pushExecutableName("appveyor");
            spawner.pushArgument("AddTest");
            spawner.pushArgument(testName);
            spawner.pushArgument("-FileName");
            // TODO: this isn't actually a file name in all cases
            spawner.pushArgument(testName);
            spawner.pushArgument("-Framework");
            spawner.pushArgument("slang-test");
            spawner.pushArgument("-Outcome");
            spawner.pushArgument(resultString);

            auto err = spawner.spawnAndWaitForCompletion();

            if( err != kOSError_None )
            {
                error("failed to add appveyor test results for '%S'\n", testName.ToWString().begin());

#if 0
                fprintf(stderr, "[%d] TEST RESULT: %s {%d} {%s} {%s}\n", err, spawner.commandLine_.Buffer(),
                    spawner.getResultCode(),
                    spawner.getStandardOutput().begin(),
                    spawner.getStandardError().begin());
#endif
            }
        }
        break;

    default:
        assert(!"unexpected");
        break;
    }
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
    TestContext*        context,
    TestOptions const&  test)
{
    // Don't include a test we should filter out
    for( auto testCategory : test.categories )
    {
        if(testCategoryMatches(testCategory, options.excludeCategories))
            return false;
    }

    // Otherwise inclue any test the user asked for
    for( auto testCategory : test.categories )
    {
        if(testCategoryMatches(testCategory, options.includeCategories))
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

    if( gatherTestsForFile(filePath, &testList) == kTestResult_Ignored )
    {
        // Test was explicitly ignored
        return;
    }

    // Note cases where a test file exists, but we found nothing to run
    if( testList.tests.Count() == 0 )
    {
        handleTestResult(context, filePath, kTestResult_Ignored);
        return;
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

        context->totalTestCount++;


        String outputStem = filePath;
        if(subTestIndex != 0)
        {
            outputStem = outputStem + "." + String(subTestIndex);
        }

        TestResult result = runTest(filePath, outputStem, tt, testList);

        handleTestResult(context, outputStem, result);

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

    auto smokeTestCategory = addTestCategory("smoke", quickTestCategory);

    auto renderTestCategory = addTestCategory("render", fullTestCategory);

	auto computeTestCategory = addTestCategory("compute", fullTestCategory);


    // An un-categorized test will always belong to the `full` category
    defaultTestCategory = fullTestCategory;

    //


    parseOptions(&argc, argv);

    if( options.includeCategories.Count() == 0 )
    {
        options.includeCategories.Add(fullTestCategory, fullTestCategory);
    }

    // Exclude rendering tests when building under AppVeyor.
    //
    // TODO: this is very ad hoc, and we should do something cleaner.
    if( options.outputMode == kOutputMode_AppVeyor )
    {
        options.excludeCategories.Add(renderTestCategory, renderTestCategory);
    }

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


    auto passCount = context.passedTestCount;
    auto rawTotal = context.totalTestCount;
    auto ignoredCount = context.ignoredTestCount;

    auto runTotal = rawTotal - ignoredCount;

    printf("\n===\n%d%% of tests passed (%d/%d)", (passCount*100) / runTotal, passCount, runTotal);
    if(ignoredCount)
    {
        printf(", %d tests ingored", ignoredCount);
    }
    printf("\n===\n\n");

    if(context.failedTestCount)
    {
        printf("failing tests:\n");
        printf("---\n");
        for(auto name : gFailedTests)
        {
            printf("%s\n", name.Buffer());
        }
        printf("---\n");
    }

    return passCount == runTotal ? 0 : 1;
}
