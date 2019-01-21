// slang-test-main.cpp

#include "../../source/core/slang-io.h"
#include "../../source/core/token-reader.h"
#include "../../source/core/slang-std-writers.h"

#include "../../slang-com-helper.h"

#include "../../source/core/slang-string-util.h"

using namespace Slang;

#include "os.h"
#include "render-api-util.h"
#include "test-context.h"
#include "test-reporter.h"
#include "options.h"
#include "slangc-tool.h"

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

enum class SpawnType
{
    UseExe,
    UseSharedLibrary,
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

    // Determines how the test will be spawned
    SpawnType spawnType;
};

typedef TestResult(*TestCallback)(TestContext* context, TestInput& input);

// Globals

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!! Functions !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

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



TestResult gatherTestOptions(
    TestCategorySet*    categorySet, 
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
                    TestCategory* category = categorySet->find(categoryName);

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
        testOptions.categories.Add(categorySet->defaultCategory);
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
    TestCategorySet*    categorySet,
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
            if(gatherTestOptions(categorySet, &cursor, testList) != TestResult::Pass)
                return TestResult::Fail;
        }
        else
        {
            skipToEndOfLine(&cursor);
        }
    }

    return TestResult::Pass;
}

OSError spawnAndWaitExe(TestContext* context, const String& testPath, OSProcessSpawner& spawner)
{
    const auto& options = context->options;

    if (options.shouldBeVerbose)
    {
        String commandLine = spawner.getCommandLine();
        context->reporter->messageFormat(TestMessageType::Info, "%s\n", commandLine.begin());
    }

    OSError err = spawner.spawnAndWaitForCompletion();
    if (err != kOSError_None)
    {
        //        fprintf(stderr, "failed to run test '%S'\n", testPath.ToWString());
        context->reporter->messageFormat(TestMessageType::RunError, "failed to run test '%S'", testPath.ToWString().begin());
    }
    return err;
}

OSError spawnAndWaitSharedLibrary(TestContext* context, const String& testPath, OSProcessSpawner& spawner)
{
    const auto& options = context->options;
    String exeName = Path::GetFileNameWithoutEXT(spawner.executableName_);

    if (options.shouldBeVerbose)
    {
        StringBuilder builder;

        builder << "slang-test";

        if (options.binDir)
        {
            builder << " -bindir " << options.binDir;
        }

        builder << " " << exeName;

        // TODO(js): Potentially this should handle escaping parameters for the command line if need be
        const auto& argList = spawner.argumentList_;
        for (UInt i = 0; i < argList.Count(); ++i)
        {
            builder << " " << argList[i];
        }

        context->reporter->messageFormat(TestMessageType::Info, "%s\n", builder.begin());
    }

    auto func = context->getInnerMainFunc(String(context->options.binDir), exeName);
    if (func)
    {
        StringBuilder stdErrorString;
        StringBuilder stdOutString;

        // Say static so not released
        StringWriter stdError(&stdErrorString, WriterFlag::IsConsole | WriterFlag::IsStatic);
        StringWriter stdOut(&stdOutString, WriterFlag::IsConsole | WriterFlag::IsStatic);

        StdWriters* prevStdWriters = StdWriters::getSingleton();

        StdWriters stdWriters;
        stdWriters.setWriter(SLANG_WRITER_CHANNEL_STD_ERROR, &stdError);
        stdWriters.setWriter(SLANG_WRITER_CHANNEL_STD_OUTPUT, &stdOut);

        if (exeName == "slangc")
        {
            stdWriters.setWriter(SLANG_WRITER_CHANNEL_DIAGNOSTIC, &stdError);
        }

        List<const char*> args;
        args.Add(exeName.Buffer());
        for (int i = 0; i < int(spawner.argumentList_.Count()); ++i)
        {
            args.Add(spawner.argumentList_[i].Buffer());
        }

        SlangResult res = func(&stdWriters, context->getSession(), int(args.Count()), args.begin());

        StdWriters::setSingleton(prevStdWriters);

        spawner.standardError_ = stdErrorString;
        spawner.standardOutput_ = stdOutString;

        spawner.resultCode_ = TestToolUtil::getReturnCode(res);

        return kOSError_None;
    }

    return kOSError_OperationFailed;
}


OSError spawnAndWait(TestContext* context, const String& testPath, SpawnType spawnType, OSProcessSpawner& spawner)
{
    switch (spawnType)
    {
        case SpawnType::UseExe:
        {
            return spawnAndWaitExe(context, testPath, spawner);
        }
        case SpawnType::UseSharedLibrary:
        {
            return spawnAndWaitSharedLibrary(context, testPath, spawner);
        }
    }
    return kOSError_OperationFailed;
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

static void _initSlangCompiler(TestContext* context, OSProcessSpawner& spawnerOut)
{
    spawnerOut.pushExecutablePath(String(context->options.binDir) + "slangc" + osGetExecutableSuffix());

    if (context->options.verbosePaths)
    {
        spawnerOut.pushArgument("-verbose-paths");
    }
}

TestResult runSimpleTest(TestContext* context, TestInput& input)
{
    // need to execute the stand-alone Slang compiler on the file, and compare its output to what we expect

    auto filePath999 = input.filePath;
    auto outputStem = input.outputStem;

    OSProcessSpawner spawner;
    _initSlangCompiler(context, spawner);

    spawner.pushArgument(filePath999);

    for( auto arg : input.testOptions->args )
    {
        spawner.pushArgument(arg);
    }

    if (spawnAndWait(context, outputStem, input.spawnType, spawner) != kOSError_None)
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
        context->reporter->dumpOutputDifference(expectedOutput, actualOutput);
        result = TestResult::Fail;
    }

    // If the test failed, then we write the actual output to a file
    // so that we can easily diff it from the command line and
    // diagnose the problem.
    if (result == TestResult::Fail)
    {
        String actualOutputPath = outputStem + ".actual";
        Slang::File::WriteAllText(actualOutputPath, actualOutput);

        context->reporter->dumpOutputDifference(expectedOutput, actualOutput);
    }

    return result;
}

TestResult runSimpleCompareCommandLineTest(TestContext* context, TestInput& input)
{
    TestInput workInput(input);
    // Use the original files input to compare with
    workInput.outputStem = input.filePath;
    // Force to using exes
    workInput.spawnType = SpawnType::UseExe;

    return runSimpleTest(context, workInput);
}

TestResult runReflectionTest(TestContext* context, TestInput& input)
{
    const auto& options = context->options;
    auto filePath = input.filePath;
    auto outputStem = input.outputStem;

    OSProcessSpawner spawner;

    spawner.pushExecutablePath(String(options.binDir) + "slang-reflection-test" + osGetExecutableSuffix());
    spawner.pushArgument(filePath);

    for( auto arg : input.testOptions->args )
    {
        spawner.pushArgument(arg);
    }

    if (spawnAndWait(context, outputStem, input.spawnType, spawner) != kOSError_None)
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

        context->reporter->dumpOutputDifference(expectedOutput, actualOutput);
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

static SlangCompileTarget _getCompileTarget(const UnownedStringSlice& name)
{
#define CASE(NAME, TARGET)  if(name == NAME) return SLANG_##TARGET;

    CASE("hlsl", HLSL)
    CASE("glsl", GLSL)
    CASE("dxbc", DXBC)
    CASE("dxbc-assembly", DXBC_ASM)
    CASE("dxbc-asm", DXBC_ASM)
    CASE("spirv", SPIRV)
    CASE("spirv-assembly", SPIRV_ASM)
    CASE("spirv-asm", SPIRV_ASM)
    CASE("dxil", DXIL)
    CASE("dxil-assembly", DXIL_ASM)
    CASE("dxil-asm", DXIL_ASM)
#undef CASE

    return SLANG_TARGET_UNKNOWN;
}



TestResult runCrossCompilerTest(TestContext* context, TestInput& input)
{
    // need to execute the stand-alone Slang compiler on the file
    // then on the same file + `.glsl` and compare output

    auto filePath = input.filePath;
    auto outputStem = input.outputStem;

    OSProcessSpawner actualSpawner;
    OSProcessSpawner expectedSpawner;

    _initSlangCompiler(context, actualSpawner);
    _initSlangCompiler(context, expectedSpawner);
    
    actualSpawner.pushArgument(filePath);
    
    const auto& args = input.testOptions->args;

    const UInt targetIndex = args.IndexOf("-target");
    if (targetIndex != UInt(-1) && targetIndex + 1 < args.Count())
    {
        SlangCompileTarget target = _getCompileTarget(args[targetIndex + 1].getUnownedSlice());

        // Check the session supports it. If not we ignore it
        if (SLANG_FAILED(spSessionCheckCompileTargetSupport(context->getSession(), target)))
        {
            return TestResult::Ignored;
        }

        switch (target)
        {
            case SLANG_DXIL_ASM:
            {
                expectedSpawner.pushArgument(filePath + ".hlsl");
                expectedSpawner.pushArgument("-pass-through");
                expectedSpawner.pushArgument("dxc");
                break;
            }
            case SLANG_DXBC_ASM:
            {
                expectedSpawner.pushArgument(filePath + ".hlsl");
                expectedSpawner.pushArgument("-pass-through");
                expectedSpawner.pushArgument("fxc");
                break;
            }
            default:
            {
                expectedSpawner.pushArgument(filePath + ".glsl");
                expectedSpawner.pushArgument("-pass-through");
                expectedSpawner.pushArgument("glslang");
                break;
            }
        }
    }
   
    for( auto arg : input.testOptions->args )
    {
        actualSpawner.pushArgument(arg);
        expectedSpawner.pushArgument(arg);
    }

    if (spawnAndWait(context, outputStem, input.spawnType, expectedSpawner) != kOSError_None)
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

    if (spawnAndWait(context, outputStem, input.spawnType, actualSpawner) != kOSError_None)
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

        context->reporter->dumpOutputDifference(expectedOutput, actualOutput);
    }

    return result;
}


#ifdef SLANG_TEST_SUPPORT_HLSL
TestResult generateHLSLBaseline(TestContext* context, TestInput& input)
{
    auto filePath999 = input.filePath;
    auto outputStem = input.outputStem;

    OSProcessSpawner spawner;
    _initSlangCompiler(context, spawner);

    spawner.pushArgument(filePath999);

    for( auto arg : input.testOptions->args )
    {
        spawner.pushArgument(arg);
    }

    spawner.pushArgument("-target");
    spawner.pushArgument("dxbc-assembly");
    spawner.pushArgument("-pass-through");
    spawner.pushArgument("fxc");

    if (spawnAndWait(context, outputStem, input.spawnType, spawner) != kOSError_None)
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
    _initSlangCompiler(context, spawner);

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

    if (spawnAndWait(context, outputStem, input.spawnType, spawner) != kOSError_None)
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

        context->reporter->dumpOutputDifference(expectedOutput, actualOutput);
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
    _initSlangCompiler(context, spawner);

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

    if (spawnAndWait(context, outputStem, input.spawnType, spawner) != kOSError_None)
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
        context->reporter->dumpOutputDifference(expectedOutput, actualOutput);

        return TestResult::Fail;
    }

    return TestResult::Pass;
}


TestResult runComputeComparisonImpl(TestContext* context, TestInput& input, const char *const* langOpts, size_t numLangOpts)
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

	spawner.pushExecutablePath(String(context->options.binDir) + "render-test" + osGetExecutableSuffix());
	spawner.pushArgument(filePath999);

	for (auto arg : input.testOptions->args)
	{
		spawner.pushArgument(arg);
	}

    for (int i = 0; i < int(numLangOpts); ++i)
    {
        spawner.pushArgument(langOpts[i]);
    }
	spawner.pushArgument("-o");
    auto actualOutputFile = outputStem + ".actual.txt";
	spawner.pushArgument(actualOutputFile);

    // clear the stale actual output file first. This will allow us to detect error if render-test fails and outputs nothing.
    File::WriteAllText(actualOutputFile, "");

	if (spawnAndWait(context, outputStem, input.spawnType, spawner) != kOSError_None)
	{
        printf("error spawning render-test\n");
		return TestResult::Fail;
	}

    auto actualOutput = getOutput(spawner);
    auto expectedOutput = getExpectedOutput(outputStem);
    if (actualOutput != expectedOutput)
    {
        context->reporter->dumpOutputDifference(expectedOutput, actualOutput);

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
        context->reporter->messageFormat(TestMessageType::TestFailure, "output mismatch! actual output: {\n%s\n}, \n%s\n", actualOutputContent.Buffer(), actualOutput.Buffer());
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
            printOutput();
            return TestResult::Fail;
        }
	}
	return TestResult::Pass;
}

TestResult runSlangComputeComparisonTest(TestContext* context, TestInput& input)
{
    const char* langOpts[] = { "-slang", "-compute" };
	return runComputeComparisonImpl(context, input, langOpts, SLANG_COUNT_OF(langOpts));
}

TestResult runSlangComputeComparisonTestEx(TestContext* context, TestInput& input)
{
	return runComputeComparisonImpl(context, input, nullptr, 0);
}

TestResult runHLSLComputeTest(TestContext* context, TestInput& input)
{
    const char* langOpts[] = { "--hlsl-rewrite", "-compute" };
    return runComputeComparisonImpl(context, input, langOpts, SLANG_COUNT_OF(langOpts));
}

TestResult runSlangRenderComputeComparisonTest(TestContext* context, TestInput& input)
{
    const char* langOpts[] = { "-slang", "-gcompute" };
    return runComputeComparisonImpl(context, input, langOpts, SLANG_COUNT_OF(langOpts));
}

TestResult doRenderComparisonTestRun(TestContext* context, TestInput& input, char const* langOption, char const* outputKind, String* outOutput)
{
    // TODO: delete any existing files at the output path(s) to avoid stale outputs leading to a false pass

    auto filePath = input.filePath;
    auto outputStem = input.outputStem;

    OSProcessSpawner spawner;

    spawner.pushExecutablePath(String(context->options.binDir) + "render-test" + osGetExecutableSuffix());
    spawner.pushArgument(filePath);

    for( auto arg : input.testOptions->args )
    {
        spawner.pushArgument(arg);
    }

    spawner.pushArgument(langOption);
    spawner.pushArgument("-o");
    spawner.pushArgument(outputStem + outputKind + ".png");

    if (spawnAndWait(context, outputStem, input.spawnType, spawner) != kOSError_None)
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

class STBImage
{
public:
    typedef STBImage ThisType;

        /// Reset back to default initialized state (frees any image set)
    void reset();
        /// True if rhs has same size and amount of channels
    bool isComparable(const ThisType& rhs) const;

        /// The width in pixels
    int getWidth() const { return m_width; }
        /// The height in pixels
    int getHeight() const { return m_height; }
        /// The number of channels (typically held as bytes in order)
    int getNumChannels() const { return m_numChannels; }

        /// Get the contained pixels, nullptr if nothing loaded
    const unsigned char* getPixels() const { return m_pixels; }
    unsigned char* getPixels() { return m_pixels; }

        /// Read an image with filename. SLANG_OK on success
    SlangResult read(const char* filename);

    ~STBImage() { reset(); }

    int m_width = 0;
    int m_height = 0;
    int m_numChannels = 0;
    unsigned char* m_pixels = nullptr;
};

void STBImage::reset()
{
    if (m_pixels)
    {
        stbi_image_free(m_pixels);
        m_pixels = nullptr;
    }
    m_width = 0;
    m_height = 0;
    m_numChannels = 0;
}

SlangResult STBImage::read(const char* filename)
{
    reset();

    m_pixels = stbi_load(filename, &m_width, &m_height, &m_numChannels, 0);
    if (!m_pixels)
    {
        return SLANG_FAIL;
    }
    return SLANG_OK;
}

bool STBImage::isComparable(const ThisType& rhs) const
{
    return (this == &rhs) ||
        (m_width == rhs.m_width && m_height == rhs.m_height && m_numChannels == rhs.m_numChannels);
}


TestResult doImageComparison(TestContext* context, String const& filePath)
{
    auto reporter = context->reporter;

    // Allow a difference in the low bits of the 8-bit result, just to play it safe
    static const int kAbsoluteDiffCutoff = 2;

    // Allow a relative 1% difference
    static const float kRelativeDiffCutoff = 0.01f;

    String expectedPath = filePath + ".expected.png";
    String actualPath = filePath + ".actual.png";

    STBImage expectedImage;
    if (SLANG_FAILED(expectedImage.read(expectedPath.Buffer())))
    {
        reporter->messageFormat(TestMessageType::RunError, "Unable to load image ;%s'", expectedPath.Buffer());
        return TestResult::Fail;
    }

    STBImage actualImage;
    if (SLANG_FAILED(actualImage.read(actualPath.Buffer())))
    {
        reporter->messageFormat(TestMessageType::RunError, "Unable to load image ;%s'", actualPath.Buffer());
        return TestResult::Fail;
    }

    if (!expectedImage.isComparable(actualImage))
    {
        reporter->messageFormat(TestMessageType::TestFailure, "Images are different sizes '%s' '%s'", actualPath.Buffer(), expectedPath.Buffer());
        return TestResult::Fail;
    }

    {
        const unsigned char* expectedPixels = expectedImage.getPixels();
        const unsigned char* actualPixels = actualImage.getPixels();

        const int height = actualImage.getHeight();
        const int width = actualImage.getWidth();
        const int numChannels = actualImage.getNumChannels();
        const int rowSize = width * numChannels;

        for (int y = 0; y < height; ++y)
        {
            for (int i = 0; i < rowSize; ++i)
            {
                int expectedVal = expectedPixels[i];
                int actualVal = actualPixels[i];

                int absoluteDiff = actualVal - expectedVal;
                if (absoluteDiff < 0) absoluteDiff = -absoluteDiff;

                if (absoluteDiff < kAbsoluteDiffCutoff)
                {
                    // There might be a difference, but we'll consider it to be inside tolerance
                    continue;
                }

                float relativeDiff = 0.0f;
                if (expectedVal != 0)
                {
                    relativeDiff = fabsf(float(actualVal) - float(expectedVal)) / float(expectedVal);

                    if (relativeDiff < kRelativeDiffCutoff)
                    {
                        // relative difference was small enough
                        continue;
                    }
                }

                // TODO: may need to do some local search sorts of things, to deal with
                // cases where vertex shader results lead to rendering that is off
                // by one pixel...

                const int x = i / numChannels;
                const int channelIndex = i % numChannels;

                reporter->messageFormat(TestMessageType::TestFailure, "image compare failure at (%d,%d) channel %d. expected %d got %d (absolute error: %d, relative error: %f)\n",
                    x, y, channelIndex,
                    expectedVal,
                    actualVal,
                    absoluteDiff,
                    relativeDiff);

                // There was a difference we couldn't excuse!
                return TestResult::Fail;
            }

            expectedPixels += rowSize;
            actualPixels += rowSize; 
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
        context->reporter->dumpOutputDifference(expectedOutput, actualOutput);

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

static bool canIgnoreTestWithDisabledRenderer(const TestOptions& testOptions, const Options& appOptions)
{
    for (int i = 0; i < int(RenderApiType::CountOf); ++i)
    {
        RenderApiType apiType = RenderApiType(i);
        RenderApiFlag::Enum apiFlag = RenderApiFlag::Enum(1 << i);
        
        if (hasRenderOption(apiType, testOptions) && (appOptions.enabledApis & apiFlag) == 0)
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
    if (canIgnoreTestWithDisabledRenderer(testOptions, context->options))
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
        { "COMMAND_LINE_SIMPLE", &runSimpleCompareCommandLineTest}, 
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
        { nullptr, nullptr },
    };

    const SpawnType defaultSpawnType = context->options.useExes ? SpawnType::UseExe : SpawnType::UseSharedLibrary;

    for( auto ii = kTestCommands; ii->name; ++ii )
    {
        if(testOptions.command != ii->name)
            continue;

        TestInput testInput;
        testInput.filePath = filePath;
        testInput.outputStem = outputStem;
        testInput.testOptions = &testOptions;
        testInput.testList = &testList;
        testInput.spawnType = defaultSpawnType;

        {
            TestReporter* reporter = context->reporter;
            TestReporter::TestScope scope(reporter, outputStem);

            TestResult testResult = ii->callback(context, testInput);
            reporter->addResult(testResult);

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
    TestContext*        context,
    TestOptions const&  test)
{
    // Don't include a test we should filter out
    for( auto testCategory : test.categories )
    {
        if(testCategoryMatches(testCategory, context->options.excludeCategories))
            return false;
    }

    // Otherwise inclue any test the user asked for
    for( auto testCategory : test.categories )
    {
        if(testCategoryMatches(testCategory, context->options.includeCategories))
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

    if( gatherTestsForFile(&context->categorySet, filePath, &testList) == TestResult::Ignored )
    {
        // Test was explicitly ignored
        return;
    }

    // Note cases where a test file exists, but we found nothing to run
    if( testList.tests.Count() == 0 )
    {
        context->reporter->addTest(filePath, TestResult::Ignored);
        return;
    }

    List<TestOptions> synthesizedTests;

    // If dx12 is available synthesize Dx12 test
    if ((context->options.synthesizedTestApis & RenderApiFlag::D3D12) != 0)
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
    if ((context->options.synthesizedTestApis & RenderApiFlag::Vulkan) != 0)
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

    if( context->options.testPrefix )
    {
        if( strncmp(context->options.testPrefix, filePath.begin(), strlen(context->options.testPrefix)) != 0 )
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

SlangResult innerMain(int argc, char** argv)
{
    auto stdWriters = StdWriters::initDefaultSingleton();

    // The context holds useful things used during testing
    TestContext context;
    SLANG_RETURN_ON_FAIL(SLANG_FAILED(context.init()))

    auto& categorySet = context.categorySet;

    // Set up our test categories here
    auto fullTestCategory = categorySet.add("full", nullptr);
    auto quickTestCategory = categorySet.add("quick", fullTestCategory);
    /*auto smokeTestCategory = */categorySet.add("smoke", quickTestCategory);
    auto renderTestCategory = categorySet.add("render", fullTestCategory);
    /*auto computeTestCategory = */categorySet.add("compute", fullTestCategory);
    auto vulkanTestCategory = categorySet.add("vulkan", fullTestCategory);
    auto unitTestCatagory = categorySet.add("unit-test", fullTestCategory);
    auto compatibilityIssueCatagory = categorySet.add("compatibility-issue", fullTestCategory);
    
#if SLANG_WINDOWS_FAMILY
    auto windowsCatagory = categorySet.add("windows", fullTestCategory);
#endif

#if SLANG_UNIX_FAMILY
    auto unixCatagory = categorySet.add("unix", fullTestCategory);
#endif

    TestCategory* fxcCategory = nullptr;
    TestCategory* dxcCategory = nullptr;
    TestCategory* glslangCategory = nullptr;

    // Might be better if we had an API on slang so we could get what 'pass-through's are available
    // This works whilst these targets imply the pass-through/backends
    {
        SlangSession* session = context.getSession();
        if (SLANG_SUCCEEDED(spSessionCheckPassThroughSupport(session, SLANG_PASS_THROUGH_FXC)))
        {
            fxcCategory = categorySet.add("fxc", fullTestCategory);
        }
        if (SLANG_SUCCEEDED(spSessionCheckPassThroughSupport(session, SLANG_PASS_THROUGH_GLSLANG)))
        {
            glslangCategory = categorySet.add("glslang", fullTestCategory);
        }
        if (SLANG_SUCCEEDED(spSessionCheckPassThroughSupport(session, SLANG_PASS_THROUGH_DXC)))
        {
            dxcCategory = categorySet.add("dxc", fullTestCategory);
        }
    }

    // An un-categorized test will always belong to the `full` category
    categorySet.defaultCategory = fullTestCategory;

    {
        // We can set the slangc command line tool, to just use the function defined here
        context.setInnerMainFunc("slangc", &SlangCTool::innerMain);
    }

    SLANG_RETURN_ON_FAIL(Options::parse(argc, argv, &categorySet, StdWriters::getError(), &context.options));
    
    Options& options = context.options;

    if (options.subCommand.Length())
    {
        // Get the function from the tool
        auto func = context.getInnerMainFunc(options.binDir, options.subCommand);
        if (!func)
        {
            StdWriters::getError().print("error: Unable to launch tool '%s'\n", options.subCommand.Buffer());
            return SLANG_FAIL;
        }

        // Copy args to a char* list
        const auto& srcArgs = options.subCommandArgs;
        List<const char*> args;
        args.SetSize(srcArgs.Count());
        for (UInt i = 0; i < srcArgs.Count(); ++i)
        {
            args[i] = srcArgs[i].Buffer();
        }

        return func(StdWriters::getSingleton(), context.getSession(), int(args.Count()), args.Buffer());
    }

    if( options.includeCategories.Count() == 0 )
    {
        options.includeCategories.Add(fullTestCategory, fullTestCategory);
    }

    // Exclude rendering tests when building under AppVeyor.
    //
    // TODO: this is very ad hoc, and we should do something cleaner.
    if( options.outputMode == TestOutputMode::AppVeyor )
    {
        options.excludeCategories.Add(renderTestCategory, renderTestCategory);
        options.excludeCategories.Add(vulkanTestCategory, vulkanTestCategory);
    }

    {
        // Setup the reporter
        TestReporter reporter;
        SLANG_RETURN_ON_FAIL(reporter.init(options.outputMode));

        context.reporter = &reporter;

        reporter.m_dumpOutputOnFailure = options.dumpOutputOnFailure;
        reporter.m_isVerbose = options.shouldBeVerbose;

        {
            TestReporter::SuiteScope suiteScope(&reporter, "tests");
            // Enumerate test files according to policy
            // TODO: add more directories to this list
            // TODO: allow for a command-line argument to select a particular directory
            runTestsInDirectory(&context, "tests/");
        }

        // Run the unit tests (these are internal C++ tests - not specified via files in a directory) 
        // They are registered with SLANG_UNIT_TEST macro
        {
            TestReporter::SuiteScope suiteScope(&reporter, "unit tests");
            TestReporter::set(&reporter);

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
                        reporter.startTest(testOptions.command);
                        // Run the test function
                        cur->m_func();
                        reporter.endTest();
                    }
                    else
                    {
                        reporter.addTest(testOptions.command, TestResult::Ignored);
                    }
                }

                // Next
                cur = cur->m_next;
            }

            TestReporter::set(nullptr);
        }

        reporter.outputSummary();
        return reporter.didAllSucceed() ? SLANG_OK : SLANG_FAIL;
    }
}

int main(int argc, char** argv)
{
    const SlangResult res = innerMain(argc, argv);
#ifdef _MSC_VER
    _CrtDumpMemoryLeaks();
#endif
    return SLANG_SUCCEEDED(res) ? 0 : 1;
}
