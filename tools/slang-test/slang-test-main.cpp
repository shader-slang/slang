// slang-test-main.cpp

#include "../../source/core/slang-io.h"
#include "../../source/core/slang-token-reader.h"
#include "../../source/core/slang-std-writers.h"
#include "../../source/core/slang-hex-dump-util.h"

#include "../../slang-com-helper.h"

#include "../../source/core/slang-string-util.h"
#include "../../source/core/slang-byte-encode-util.h"

using namespace Slang;

#include "os.h"
#include "../../source/core/slang-render-api-util.h"
#include "test-context.h"
#include "test-reporter.h"
#include "options.h"
#include "slangc-tool.h"

#include "../../source/core/slang-downstream-compiler.h"

#include "../../source/core/slang-nvrtc-compiler.h"

#include "../../source/core/slang-process-util.h"

#define STB_IMAGE_IMPLEMENTATION
#include "external/stb/stb_image.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#define SLANG_PRELUDE_NAMESPACE CPPPrelude
#include "../../prelude/slang-cpp-types.h"

// Options for a particular test
struct TestOptions
{
    enum Type
    {
        Normal,             ///< A regular test
        Diagnostic,         ///< Diagnostic tests will always run (as form of failure is being tested)  
    };

    Type type = Type::Normal;

    String command;
    List<String> args;

    // The categories that this test was assigned to
    List<TestCategory*> categories;

    bool isSynthesized = false;
};

struct TestDetails
{
    TestDetails() {}
    explicit TestDetails(const TestOptions& inOptions):
        options(inOptions)
    {}

    TestOptions options;                    ///< The options for the test
    TestRequirements requirements;          ///< The requirements for the test to work
};

// Information on tests to run for a particular file
struct FileTestList
{
    List<TestDetails> tests;
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

    // Determines how the test will be spawned
    SpawnType spawnType;
};

typedef TestResult(*TestCallback)(TestContext* context, TestInput& input);

// Globals

// Pre declare
static void _addRenderTestOptions(const Options& options, CommandLine& cmdLine);

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


static TestResult _gatherTestOptions(
    TestCategorySet*    categorySet, 
    char const**    ioCursor,
    TestOptions&    outOptions)
{
    char const* cursor = *ioCursor;

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
                    

                    outOptions.categories.add(category);

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
    if(outOptions.categories.getCount() == 0)
    {
        outOptions.categories.add(categorySet->defaultCategory);
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

    outOptions.command = getString(commandStart, commandEnd);

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

        outOptions.args.add(getString(argBegin, argEnd));
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
        fileContents = Slang::File::readAllText(filePath);
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
            TestDetails testDetails;

            if(_gatherTestOptions(categorySet, &cursor, testDetails.options) != TestResult::Pass)
                return TestResult::Fail;

            testList->tests.add(testDetails);
        }
        else if (match(&cursor, "//DIAGNOSTIC_TEST"))
        {
            TestDetails testDetails;
            
            if (_gatherTestOptions(categorySet, &cursor, testDetails.options) != TestResult::Pass)
                return TestResult::Fail;

            // Mark that it is a diagnostic test
            testDetails.options.type = TestOptions::Type::Diagnostic;
            testList->tests.add(testDetails);
        }
        else
        {
            skipToEndOfLine(&cursor);
        }
    }

    return TestResult::Pass;
}

Result spawnAndWaitExe(TestContext* context, const String& testPath, const CommandLine& cmdLine, ExecuteResult& outRes)
{
    const auto& options = context->options;

    if (options.shouldBeVerbose)
    {
        String commandLine = ProcessUtil::getCommandLineString(cmdLine);
        context->reporter->messageFormat(TestMessageType::Info, "%s\n", commandLine.begin());
    }

    Result res = ProcessUtil::execute(cmdLine, outRes);
    if (SLANG_FAILED(res))
    {
        //        fprintf(stderr, "failed to run test '%S'\n", testPath.ToWString());
        context->reporter->messageFormat(TestMessageType::RunError, "failed to run test '%S'", testPath.toWString().begin());
    }
    return res;
}

Result spawnAndWaitSharedLibrary(TestContext* context, const String& testPath, const CommandLine& cmdLine, ExecuteResult& outRes)
{
    const auto& options = context->options;
    String exeName = Path::getFileNameWithoutExt(cmdLine.m_executable);

    if (options.shouldBeVerbose)
    {
        CommandLine testCmdLine;
        testCmdLine.setExecutableFilename("slang-test");

        if (options.binDir.getLength())
        {
            testCmdLine.addArg("-bindir");
            testCmdLine.addArg(options.binDir);
        }

        testCmdLine.addArg(exeName);
        testCmdLine.m_args.addRange(cmdLine.m_args);

        String testCmdLineString = ProcessUtil::getCommandLineString(testCmdLine);
        context->reporter->messageFormat(TestMessageType::Info, "%s\n", testCmdLineString.getBuffer());
    }

    auto func = context->getInnerMainFunc(context->options.binDir, exeName);
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
        args.add(exeName.getBuffer());
        for (Index i = 0; i < cmdLine.m_args.getCount(); ++i)
        {
            args.add(cmdLine.m_args[i].value.getBuffer());
        }

        SlangResult res = func(&stdWriters, context->getSession(), int(args.getCount()), args.begin());

        StdWriters::setSingleton(prevStdWriters);

        outRes.standardError = stdErrorString;
        outRes.standardOutput = stdOutString;

        outRes.resultCode = (int)TestToolUtil::getReturnCode(res);

        return SLANG_OK;
    }

    return SLANG_FAIL;
}


static SlangResult _extractArg(const CommandLine& cmdLine, const String& argName, String& outValue)
{
    SLANG_ASSERT(argName.getLength() > 0 && argName[0] == '-');
    Index index = cmdLine.findArgIndex(argName.getUnownedSlice());

    if (index >= 0 && index < cmdLine.getArgCount() - 1)
    {
        outValue = cmdLine.m_args[index + 1].value;
        return SLANG_OK;
    }
    return SLANG_FAIL;
}

static bool _hasOption(const List<String>& args, const String& argName)
{
    return args.indexOf(argName) != Index(-1);
}

static PassThroughFlags _getPassThroughFlagsForTarget(SlangCompileTarget target)
{
    switch (target)
    {
        case SLANG_TARGET_UNKNOWN:

        case SLANG_HLSL:
        case SLANG_GLSL:
        case SLANG_C_SOURCE:
        case SLANG_CPP_SOURCE:
        case SLANG_CUDA_SOURCE:
        {
            return 0;
        }
        case SLANG_DXBC:
        case SLANG_DXBC_ASM:
        {
            return PassThroughFlag::Fxc;
        }
        case SLANG_SPIRV:
        case SLANG_SPIRV_ASM:
        {
            return PassThroughFlag::Glslang;
        }
        case SLANG_DXIL:
        case SLANG_DXIL_ASM:
        {
            return PassThroughFlag::Dxc;
        }

        case SLANG_HOST_CALLABLE:
        case SLANG_EXECUTABLE:
        case SLANG_SHARED_LIBRARY:
        {
            return PassThroughFlag::Generic_C_CPP;
        }
        case SLANG_PTX:
        {
            return PassThroughFlag::NVRTC;
        }

        default:
        {
            SLANG_ASSERT(!"Unknown type");
            return 0;
        }
    }
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
        CASE("c", C_SOURCE)
        CASE("cpp", CPP_SOURCE)
        CASE("exe", EXECUTABLE)
        CASE("sharedlib", SHARED_LIBRARY)
        CASE("dll", SHARED_LIBRARY)
        CASE("callable", HOST_CALLABLE)
        CASE("host-callable", HOST_CALLABLE)
        CASE("ptx", PTX)
        CASE("cuda", CUDA_SOURCE)
#undef CASE

        return SLANG_TARGET_UNKNOWN;
}

static SlangResult _extractRenderTestRequirements(const CommandLine& cmdLine, TestRequirements* ioRequirements)
{
    const auto& args = cmdLine.m_args;
    
    // TODO(JS): 
    // This is rather convoluted in that it has to work out from the command line parameters passed
    // to render-test what renderer will be used. 
    // That a similar logic has to be kept inside the implementation of render-test and both this
    // and render-test will have to be kept in sync.

    bool useDxil = cmdLine.findArgIndex(UnownedStringSlice::fromLiteral("-use-dxil")) >= 0;

    bool usePassthru = false;

    // Work out what kind of render will be used
    RenderApiType renderApiType;
    {
        RenderApiType foundRenderApiType = RenderApiType::Unknown;
        RenderApiType foundLanguageRenderType = RenderApiType::Unknown;

        for (const auto& arg: args)
        {
            Slang::UnownedStringSlice argSlice = arg.value.getUnownedSlice();
            if (argSlice.size() && argSlice[0] == '-')
            {
                // Look up the rendering API if set
                UnownedStringSlice argName = UnownedStringSlice(argSlice.begin() + 1, argSlice.end());
                RenderApiType renderApiType = RenderApiUtil::findApiTypeByName(argName);

                if (renderApiType != RenderApiType::Unknown)
                {
                    foundRenderApiType = renderApiType;

                    // There should be only one explicit api
                    SLANG_ASSERT(ioRequirements->explicitRenderApi == RenderApiType::Unknown || ioRequirements->explicitRenderApi == renderApiType);

                    // Set the explicitly set render api
                    ioRequirements->explicitRenderApi = renderApiType;
                    continue;
                }

                // Lookup the target language type
                RenderApiType languageRenderType = RenderApiUtil::findImplicitLanguageRenderApiType(argName);
                if (languageRenderType != RenderApiType::Unknown)
                {
                    foundLanguageRenderType = languageRenderType;

                    // Use the pass thru compiler if these are the sources
                    usePassthru |= (argName == "hlsl" || argName == "glsl");

                    continue;
                }
            }
        }

        // If a render option isn't set use defaultRenderType 
        renderApiType = (foundRenderApiType == RenderApiType::Unknown) ? foundLanguageRenderType : foundRenderApiType;
    }

    // The native language for the API
    SlangSourceLanguage nativeLanguage = SLANG_SOURCE_LANGUAGE_UNKNOWN;
    SlangCompileTarget target = SLANG_TARGET_NONE;
    SlangPassThrough passThru = SLANG_PASS_THROUGH_NONE;

    switch (renderApiType)
    {
        case RenderApiType::D3D11:
            target = SLANG_DXBC;
            nativeLanguage = SLANG_SOURCE_LANGUAGE_HLSL;
            passThru = SLANG_PASS_THROUGH_FXC;
            break;
        case RenderApiType::D3D12:
            target = SLANG_DXBC;
            nativeLanguage = SLANG_SOURCE_LANGUAGE_HLSL;
            passThru = SLANG_PASS_THROUGH_FXC;
            if (useDxil)
            {
                target = SLANG_DXIL;
                passThru = SLANG_PASS_THROUGH_DXC;
            }
            break;

        case RenderApiType::OpenGl:
            target = SLANG_GLSL;
            nativeLanguage = SLANG_SOURCE_LANGUAGE_GLSL;
            passThru = SLANG_PASS_THROUGH_GLSLANG;
            break;
        case RenderApiType::Vulkan:
            target = SLANG_SPIRV;
            nativeLanguage = SLANG_SOURCE_LANGUAGE_GLSL;
            passThru = SLANG_PASS_THROUGH_GLSLANG;
            break;
        case RenderApiType::CPU:
            target = SLANG_HOST_CALLABLE;
            nativeLanguage = SLANG_SOURCE_LANGUAGE_CPP;
            passThru = SLANG_PASS_THROUGH_GENERIC_C_CPP;
            break;
        case RenderApiType::CUDA:
            target = SLANG_PTX;
            nativeLanguage = SLANG_SOURCE_LANGUAGE_CUDA;
            passThru = SLANG_PASS_THROUGH_NVRTC;
            break;
    }

    SlangSourceLanguage sourceLanguage = nativeLanguage;
    if (!usePassthru)
    {
        sourceLanguage = SLANG_SOURCE_LANGUAGE_SLANG;
        passThru = SLANG_PASS_THROUGH_NONE;
    }

    if (passThru == SLANG_PASS_THROUGH_NONE)
    {
        // Work out backends needed based on the target
        ioRequirements->addUsedBackends(_getPassThroughFlagsForTarget(target));
    }
    else
    {
        ioRequirements->addUsedBackEnd(passThru);
    }

    // Add the render api used
    ioRequirements->addUsedRenderApi(renderApiType);

    return SLANG_OK;
}

static SlangResult _extractSlangCTestRequirements(const CommandLine& cmdLine, TestRequirements* ioRequirements)
{
    // This determines what the requirements are for a slangc like command line
    // First check pass through
    {
        String passThrough;
        if (SLANG_SUCCEEDED(_extractArg(cmdLine, "-pass-through", passThrough)))
        {
            ioRequirements->addUsedBackEnd(DownstreamCompiler::getPassThroughFromName(passThrough.getUnownedSlice()));
        }
    }

    // The target if set will also imply a backend
    {
        String targetName;
        if (SLANG_SUCCEEDED(_extractArg(cmdLine, "-target", targetName)))
        {
            const SlangCompileTarget target = _getCompileTarget(targetName.getUnownedSlice());
            ioRequirements->addUsedBackends(_getPassThroughFlagsForTarget(target));
        }
    }
    return SLANG_OK;

}

static SlangResult _extractReflectionTestRequirements(const CommandLine& cmdLine, TestRequirements* ioRequirements)
{
    // There are no specialized constraints for a reflection test
    return SLANG_OK;
}

static SlangResult _extractTestRequirements(const CommandLine& cmdLine, TestRequirements* ioInfo)
{
    String exeName = Path::getFileNameWithoutExt(cmdLine.m_executable);

    if (exeName == "render-test")
    {
        return _extractRenderTestRequirements(cmdLine, ioInfo);
    }
    else if (exeName == "slangc")
    {
        return _extractSlangCTestRequirements(cmdLine, ioInfo);
    }
    else if (exeName == "slang-reflection-test")
    {
        return _extractReflectionTestRequirements(cmdLine, ioInfo);
    }

    SLANG_ASSERT(!"Unknown tool type");
    return SLANG_FAIL;
}

static RenderApiFlags _getAvailableRenderApiFlags(TestContext* context)
{
    // Only evaluate if it hasn't already been evaluated (the actual evaluation is slow...)
    if (!context->isAvailableRenderApiFlagsValid)
    {
        // Call the render-test tool asking it only to startup a specified render api
        // (taking into account adapter options)

        RenderApiFlags availableRenderApiFlags = 0;
        for (int i = 0; i < int(RenderApiType::CountOf); ++i)
        {
            const RenderApiType apiType = RenderApiType(i);

            if (apiType == RenderApiType::CPU)
            {
                if ((context->availableBackendFlags & PassThroughFlag::Generic_C_CPP) == 0)
                {
                    continue;
                }

                // Check that the session has the generic C/CPP compiler availability - which is all we should need for CPU target
                if (SLANG_SUCCEEDED(spSessionCheckPassThroughSupport(context->getSession(), SLANG_PASS_THROUGH_GENERIC_C_CPP)))
                {
                    availableRenderApiFlags |= RenderApiFlags(1) << int(apiType);
                }
                continue;
            }

            // See if it's possible the api is available
            if (RenderApiUtil::calcHasApi(apiType))
            {
                // Try starting up the device
                CommandLine cmdLine;
                cmdLine.setExecutablePath(Path::combine(context->options.binDir,  String("render-test") + ProcessUtil::getExecutableSuffix()));
                _addRenderTestOptions(context->options, cmdLine);
                // We just want to see if the device can be started up
                cmdLine.addArg("-only-startup");

                // Select what api to use
                StringBuilder builder;
                builder << "-" << RenderApiUtil::getApiName(apiType);
                cmdLine.addArg(builder);

                // Run the render-test tool and see if the device could startup
                ExecuteResult exeRes;
                if (SLANG_SUCCEEDED(spawnAndWaitSharedLibrary(context, "device-startup", cmdLine, exeRes))
                    && TestToolUtil::getReturnCodeFromInt(exeRes.resultCode) == ToolReturnCode::Success)
                {
                    availableRenderApiFlags |= RenderApiFlags(1) << int(apiType);   
                }
            }
        }

        context->availableRenderApiFlags = availableRenderApiFlags;
        context->isAvailableRenderApiFlagsValid = true;
    }

    return context->availableRenderApiFlags;
}

ToolReturnCode getReturnCode(const ExecuteResult& exeRes)
{
    return TestToolUtil::getReturnCodeFromInt(exeRes.resultCode);
}

ToolReturnCode spawnAndWait(TestContext* context, const String& testPath, SpawnType spawnType, const CommandLine& cmdLine, ExecuteResult& outExeRes)
{
    if (context->isCollectingRequirements())
    {
        // If we just want info... don't bother running anything
        const SlangResult res = _extractTestRequirements(cmdLine, context->testRequirements);
        // Keep compiler happy on release
        SLANG_UNUSED(res);
        SLANG_ASSERT(SLANG_SUCCEEDED(res));

        return ToolReturnCode::Success;
    }

    const auto& options = context->options;

    SlangResult spawnResult = SLANG_FAIL;
    switch (spawnType)
    {
        case SpawnType::UseExe:
        {
            spawnResult = spawnAndWaitExe(context, testPath, cmdLine, outExeRes);
            break;
        }
        case SpawnType::UseSharedLibrary:
        {
            spawnResult = spawnAndWaitSharedLibrary(context, testPath, cmdLine, outExeRes);
            break;
        }
        default: break;
    }

    if (SLANG_FAILED(spawnResult))
    {
        return ToolReturnCode::FailedToRun;
    }

    return getReturnCode(outExeRes);
}

String getOutput(const ExecuteResult& exeRes)
{
    ExecuteResult::ResultCode resultCode = exeRes.resultCode;
    
    String standardOuptut = exeRes.standardOutput;
    String standardError = exeRes.standardError;
    
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
    if (File::exists(specializedBuf))
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

    if (File::exists(defaultBuf))
    {
        return defaultBuf;
    }

    // Couldn't find either 
    printf("referenceOutput '%s' or '%s' not found.\n", defaultBuf.getBuffer(), specializedBuf.getBuffer());

    return "";
}

static void _initSlangCompiler(TestContext* context, CommandLine& ioCmdLine)
{
    ioCmdLine.setExecutablePath(Path::combine(context->options.binDir, String("slangc") + ProcessUtil::getExecutableSuffix()));

    if (context->options.verbosePaths)
    {
        ioCmdLine.addArg("-verbose-paths");
    }
}

TestResult asTestResult(ToolReturnCode code)
{
    switch (code)
    {
        case ToolReturnCode::Success:               return TestResult::Pass;
        case ToolReturnCode::Ignored:               return TestResult::Ignored;
        default:                                    return TestResult::Fail;
    }
}

#define TEST_RETURN_ON_DONE(x) \
    { \
        const ToolReturnCode toolRet_ = x; \
        if (TestToolUtil::isDone(toolRet_)) \
        { \
            return asTestResult(toolRet_); \
        } \
    }

static SlangResult _executeBinary(const UnownedStringSlice& hexDump, ExecuteResult& outExeRes)
{
    // We need to extract the binary
    List<uint8_t> data;
    SLANG_RETURN_ON_FAIL(HexDumpUtil::parseWithMarkers(hexDump, data));

    // Need to write this off to a temporary file
    String fileName;
    SLANG_RETURN_ON_FAIL(File::generateTemporary(UnownedStringSlice("slang-test"), fileName));

    fileName.append(ProcessUtil::getExecutableSuffix());

    TemporaryFileSet temporaryFileSet;
    temporaryFileSet.add(fileName);

    {
        ComPtr<ISlangWriter> writer;
        SLANG_RETURN_ON_FAIL(FileWriter::createBinary(fileName.getBuffer(), 0, writer));

        SLANG_RETURN_ON_FAIL(writer->write((const char*)data.getBuffer(), data.getCount()));
    }

    // Make executable... (for linux/unix like targets)
    SLANG_RETURN_ON_FAIL(File::makeExecutable(fileName));

    // Execute it
    CommandLine cmdLine;
    cmdLine.m_executable = fileName;
    cmdLine.m_executableType = CommandLine::ExecutableType::Path;
    return ProcessUtil::execute(cmdLine, outExeRes);
}


TestResult runSimpleTest(TestContext* context, TestInput& input)
{
    // need to execute the stand-alone Slang compiler on the file, and compare its output to what we expect
    auto outputStem = input.outputStem;

    CommandLine cmdLine;
    _initSlangCompiler(context, cmdLine);

    if (input.testOptions->command != "SIMPLE_EX")
    {
        cmdLine.addArg(input.filePath);
    }

    for( auto arg : input.testOptions->args )
    {
        cmdLine.addArg(arg);
    }

    ExecuteResult exeRes;
    TEST_RETURN_ON_DONE(spawnAndWait(context, outputStem, input.spawnType, cmdLine, exeRes));

    if (context->isCollectingRequirements())
    {
        return TestResult::Pass;
    }

    // See what kind of target it is
    SlangCompileTarget target = SLANG_TARGET_UNKNOWN;
    {
        const auto& args = input.testOptions->args;
        const Index targetIndex = args.indexOf("-target");
        if (targetIndex != Index(-1) && targetIndex + 1 < args.getCount())
        {
            target = _getCompileTarget(args[targetIndex + 1].getUnownedSlice());
        }
    }

    // If it's executable we run it and use it's output
    if (target == SLANG_EXECUTABLE)
    {
        ExecuteResult runExeRes;
        if (SLANG_FAILED(_executeBinary(exeRes.standardOutput.getUnownedSlice(), runExeRes)))
        {
            return TestResult::Fail;
        }
        exeRes = runExeRes;
    }

    String actualOutput = getOutput(exeRes);

    String expectedOutputPath = outputStem + ".expected";
    String expectedOutput;
    try
    {
        expectedOutput = Slang::File::readAllText(expectedOutputPath);
    }
    catch (Slang::IOException)
    {
    }

    // If no expected output file was found, then we
    // expect everything to be empty
    if (expectedOutput.getLength() == 0)
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
        Slang::File::writeAllText(actualOutputPath, actualOutput);

        context->reporter->dumpOutputDifference(expectedOutput, actualOutput);
    }

    return result;
}

TestResult runCompile(TestContext* context, TestInput& input)
{
    auto outputStem = input.outputStem;

    CommandLine cmdLine;
    _initSlangCompiler(context, cmdLine);

    for (auto arg : input.testOptions->args)
    {
        cmdLine.addArg(arg);
    }

    ExecuteResult exeRes;
    TEST_RETURN_ON_DONE(spawnAndWait(context, outputStem, input.spawnType, cmdLine, exeRes));
    if (context->isCollectingRequirements())
    {
        return TestResult::Pass;
    }

    if (exeRes.resultCode != 0)
    {
        auto reporter = context->reporter;
        if (reporter)
        {
            auto output = getOutput(exeRes);
            reporter->message(TestMessageType::TestFailure, output);
        }

        return TestResult::Fail;
    }

    return TestResult::Pass;
}


static SlangResult _loadAsSharedLibrary(const UnownedStringSlice& hexDump, TemporaryFileSet& inOutTemporaryFileSet, SharedLibrary::Handle& outSharedLibrary)
{
    // We need to extract the binary
    List<uint8_t> data;
    SLANG_RETURN_ON_FAIL(HexDumpUtil::parseWithMarkers(hexDump, data));

    // Need to write this off to a temporary file
    String fileName;
    SLANG_RETURN_ON_FAIL(File::generateTemporary(UnownedStringSlice("slang-test"), fileName));

    // Need to work out the dll name
    String sharedLibraryName = SharedLibrary::calcPlatformPath(fileName.getUnownedSlice());
    inOutTemporaryFileSet.add(sharedLibraryName);

    {
        ComPtr<ISlangWriter> writer;
        SLANG_RETURN_ON_FAIL(FileWriter::createBinary(sharedLibraryName.getBuffer(), 0, writer));
        SLANG_RETURN_ON_FAIL(writer->write((const char*)data.getBuffer(), data.getCount()));
    }

    // Make executable... (for linux/unix like targets)
    //SLANG_RETURN_ON_FAIL(File::makeExecutable(fileName));

    return SharedLibrary::loadWithPlatformPath(sharedLibraryName.getBuffer(), outSharedLibrary);
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

    bool isCPUTest = input.testOptions->command.startsWith("CPU_");

    CommandLine cmdLine;
    
    cmdLine.setExecutablePath(Path::combine(options.binDir, String("slang-reflection-test") + ProcessUtil::getExecutableSuffix()));
    cmdLine.addArg(filePath);

    for( auto arg : input.testOptions->args )
    {
        cmdLine.addArg(arg);
    }

    ExecuteResult exeRes;
    TEST_RETURN_ON_DONE(spawnAndWait(context, outputStem, input.spawnType, cmdLine, exeRes));

    if (context->isCollectingRequirements())
    {
        return TestResult::Pass;
    }

    String actualOutput = getOutput(exeRes);

    if (isCPUTest)
    {
#if SLANG_PTR_IS_32
        outputStem.append(".32");
#else
        outputStem.append(".64");
#endif
    }

    String expectedOutputPath = outputStem + ".expected";
    String expectedOutput;
    try
    {
        expectedOutput = Slang::File::readAllText(expectedOutputPath);
    }
    catch (Slang::IOException)
    {
    }

    // If no expected output file was found, then we
    // expect everything to be empty
    if (expectedOutput.getLength() == 0)
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
        Slang::File::writeAllText(actualOutputPath, actualOutput);

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
        expectedOutput = Slang::File::readAllText(expectedOutputPath);
    }
    catch (Slang::IOException)
    {
    }

    // If no expected output file was found, then we
    // expect everything to be empty
    if (expectedOutput.getLength() == 0)
    {
        expectedOutput = "result code = 0\nstandard error = {\n}\nstandard output = {\n}\n";
    }

    return expectedOutput;
}

static String _calcSummary(const DownstreamDiagnostics& inOutput)
{
    DownstreamDiagnostics output(inOutput);

    // We only want to analyse errors for now
    output.removeByType(DownstreamDiagnostics::Diagnostic::Type::Info);
    output.removeByType(DownstreamDiagnostics::Diagnostic::Type::Warning);

    StringBuilder builder;

    output.appendSimplifiedSummary(builder);
    return builder;
}

static String _calcModulePath(const TestInput& input)
{
    // Make the module name the same as the source file
    auto filePath = input.filePath;
    String directory = Path::getParentDirectory(input.outputStem);
    String moduleName = Path::getFileNameWithoutExt(filePath);
    return Path::combine(directory, moduleName);
}

static TestResult runCPPCompilerCompile(TestContext* context, TestInput& input)
{
    DownstreamCompiler* compiler = context->getDefaultCompiler(SLANG_SOURCE_LANGUAGE_CPP);
    if (!compiler)
    {
        return TestResult::Ignored;
    }

    // need to execute the stand-alone Slang compiler on the file, and compare its output to what we expect

    auto outputStem = input.outputStem;

    CommandLine cmdLine;
    _initSlangCompiler(context, cmdLine);

    cmdLine.addArg(input.filePath);
    for (auto arg : input.testOptions->args)
    {
        cmdLine.addArg(arg);
    }

    ExecuteResult exeRes;
    TEST_RETURN_ON_DONE(spawnAndWait(context, outputStem, input.spawnType, cmdLine, exeRes));
    if (context->isCollectingRequirements())
    {
        return TestResult::Pass;
    }

    // Dump out what happened
    {
        String actualOutputPath = outputStem + ".actual";
        Slang::File::writeAllText(actualOutputPath, getOutput(exeRes));
    }

    if (exeRes.resultCode != 0)
    {
        return TestResult::Fail;
    }

    return TestResult::Pass;
}

static TestResult runCPPCompilerSharedLibrary(TestContext* context, TestInput& input)
{
    DownstreamCompiler* compiler = context->getDefaultCompiler(SLANG_SOURCE_LANGUAGE_CPP);
    if (!compiler)
    {
        return TestResult::Ignored;
    }

    // If we are just collecting requirements, say it passed
    if (context->isCollectingRequirements())
    {
        context->testRequirements->addUsedBackEnd(SLANG_PASS_THROUGH_GENERIC_C_CPP);
        return TestResult::Pass;
    }

    auto outputStem = input.outputStem;
    auto filePath = input.filePath;

    String actualOutputPath = outputStem + ".actual";
    File::remove(actualOutputPath);

    // Make the module name the same as the source file
    String modulePath = _calcModulePath(input);
    String ext = Path::getFileExt(filePath);

    // Remove the binary..
    String sharedLibraryPath = SharedLibrary::calcPlatformPath(modulePath.getUnownedSlice());
    File::remove(sharedLibraryPath);

    // Set up the compilation options
    DownstreamCompiler::CompileOptions options;

    options.sourceLanguage = (ext == "c") ? SLANG_SOURCE_LANGUAGE_C : SLANG_SOURCE_LANGUAGE_CPP;

    // Build a shared library
    options.targetType = DownstreamCompiler::TargetType::SharedLibrary;

    // Compile this source
    options.sourceFiles.add(filePath);
    options.modulePath = modulePath;

    options.includePaths.add(".");

    RefPtr<DownstreamCompileResult> compileResult;
    if (SLANG_FAILED(compiler->compile(options, compileResult)))
    {
        return TestResult::Fail;
    }

    const auto& diagnostics = compileResult->getDiagnostics();

    if (SLANG_FAILED(diagnostics.result))
    {
        // Compilation failed
        String actualOutput = _calcSummary(diagnostics);

        // Write the output
        Slang::File::writeAllText(actualOutputPath, actualOutput);

        // Check that they are the same
        {
            // Read the expected
            String expectedOutput;
            try
            {
                String expectedOutputPath = outputStem + ".expected";
                expectedOutput = Slang::File::readAllText(expectedOutputPath);
            }
            catch (Slang::IOException)
            {
            }

            // Compare if they are the same 
            if (!StringUtil::areLinesEqual(actualOutput.getUnownedSlice(), expectedOutput.getUnownedSlice()))
            {
                context->reporter->dumpOutputDifference(expectedOutput, actualOutput);
                return TestResult::Fail;
            }
        }
    }
    else
    {
        SharedLibrary::Handle handle;
        if (SLANG_FAILED(SharedLibrary::loadWithPlatformPath(sharedLibraryPath.getBuffer(), handle)))
        {
            return TestResult::Fail;
        }

        const int inValue = 10;
        const char inBuffer[] = "Hello World!";

        char buffer[128] = "";
        int value = 0;

        typedef int(*TestFunc)(int intValue, const char* textValue, char* outTextValue);

        // We could capture output if we passed in a ISlangWriter - but for that to work we'd need a 
        TestFunc testFunc = (TestFunc)SharedLibrary::findFuncByName(handle, "test");
        if (testFunc)
        {
            value = testFunc(inValue, inBuffer, buffer);
        }
        else
        {
            printf("Unable to access 'test' function\n");
        }

        SharedLibrary::unload(handle);

        if (!(inValue == value && strcmp(inBuffer, buffer) == 0))
        {
            return TestResult::Fail;
        }
    }

    return TestResult::Pass;
}

static TestResult runCPPCompilerExecute(TestContext* context, TestInput& input)
{
    DownstreamCompiler* compiler = context->getDefaultCompiler(SLANG_SOURCE_LANGUAGE_CPP);
    if (!compiler)
    {
        return TestResult::Ignored;
    }

    // If we are just collecting requirements, say it passed
    if (context->isCollectingRequirements())
    {
        context->testRequirements->addUsedBackEnd(SLANG_PASS_THROUGH_GENERIC_C_CPP);
        return TestResult::Pass;
    }

    auto filePath = input.filePath;
    auto outputStem = input.outputStem;

    String actualOutputPath = outputStem + ".actual";
    File::remove(actualOutputPath);

    // Make the module name the same as the source file
    String ext = Path::getFileExt(filePath);
    String modulePath = _calcModulePath(input);
    
    // Remove the binary..
    {
        StringBuilder moduleExePath;
        moduleExePath << modulePath;
        moduleExePath << ProcessUtil::getExecutableSuffix();
        File::remove(moduleExePath);
    }

    // Set up the compilation options
    DownstreamCompiler::CompileOptions options;

    options.sourceLanguage = (ext == "c") ? SLANG_SOURCE_LANGUAGE_C : SLANG_SOURCE_LANGUAGE_CPP;

    // Compile this source
    options.sourceFiles.add(filePath);
    options.modulePath = modulePath;

    RefPtr<DownstreamCompileResult> compileResult;
    if (SLANG_FAILED(compiler->compile(options, compileResult)))
    {
        return TestResult::Fail;
    }

    String actualOutput;

    const auto& diagnostics = compileResult->getDiagnostics();

    // If the actual compilation failed, then the output will be
    if (SLANG_FAILED(diagnostics.result))
    {
        actualOutput = _calcSummary(diagnostics);
    }
    else
    {
       // Execute the binary and see what we get

        CommandLine cmdLine;

        StringBuilder exePath;
        exePath << modulePath << ProcessUtil::getExecutableSuffix();

        cmdLine.setExecutablePath(exePath);

        ExecuteResult exeRes;
        if (SLANG_FAILED(ProcessUtil::execute(cmdLine, exeRes)))
        {
            return TestResult::Fail;
        }

        // Write the output, and compare to expected
        actualOutput = getOutput(exeRes);
    }

    // Write the output
    Slang::File::writeAllText(actualOutputPath, actualOutput);

    // Check that they are the same
    {
        // Read the expected
        String expectedOutput;
        try
        {
            String expectedOutputPath = outputStem + ".expected";
            expectedOutput = Slang::File::readAllText(expectedOutputPath);
        }
        catch (Slang::IOException)
        {
        }

        // Compare if they are the same 
        if (!StringUtil::areLinesEqual(actualOutput.getUnownedSlice(), expectedOutput.getUnownedSlice()))
        {
            context->reporter->dumpOutputDifference(expectedOutput, actualOutput);
            return TestResult::Fail;
        }
    }
    
    return TestResult::Pass;
}

TestResult runCrossCompilerTest(TestContext* context, TestInput& input)
{
    // need to execute the stand-alone Slang compiler on the file
    // then on the same file + `.glsl` and compare output

    auto filePath = input.filePath;
    auto outputStem = input.outputStem;

    CommandLine actualCmdLine;
    CommandLine expectedCmdLine;

    _initSlangCompiler(context, actualCmdLine);
    _initSlangCompiler(context, expectedCmdLine);
    
    actualCmdLine.addArg(filePath);

    // TODO(JS): This should no longer be needed with TestInfo accumulated for a test

    const auto& args = input.testOptions->args;

    const Index targetIndex = args.indexOf("-target");
    if (targetIndex != Index(-1) && targetIndex + 1 < args.getCount())
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
                expectedCmdLine.addArg(filePath + ".hlsl");
                expectedCmdLine.addArg("-pass-through");
                expectedCmdLine.addArg("dxc");
                break;
            }
            case SLANG_DXBC_ASM:
            {
                expectedCmdLine.addArg(filePath + ".hlsl");
                expectedCmdLine.addArg("-pass-through");
                expectedCmdLine.addArg("fxc");
                break;
            }
            default:
            {
                expectedCmdLine.addArg(filePath + ".glsl");
                expectedCmdLine.addArg("-pass-through");
                expectedCmdLine.addArg("glslang");
                break;
            }
        }
    }
   
    for( auto arg : input.testOptions->args )
    {
        actualCmdLine.addArg(arg);
        expectedCmdLine.addArg(arg);
    }

    ExecuteResult expectedExeRes;
    TEST_RETURN_ON_DONE(spawnAndWait(context, outputStem, input.spawnType, expectedCmdLine, expectedExeRes));

    String expectedOutput;
    if (context->isExecuting())
    {
        expectedOutput = getOutput(expectedExeRes);
        String expectedOutputPath = outputStem + ".expected";
        try
        {
            Slang::File::writeAllText(expectedOutputPath, expectedOutput);
        }
        catch (Slang::IOException)
        {
            return TestResult::Fail;
        }
    }

    ExecuteResult actualExeRes;
    TEST_RETURN_ON_DONE(spawnAndWait(context, outputStem, input.spawnType, actualCmdLine, actualExeRes));

    if (context->isCollectingRequirements())
    {
        return TestResult::Pass;
    }

    String actualOutput = getOutput(actualExeRes);

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
    if(actualExeRes.resultCode != 0 )
    {
        result = TestResult::Fail;
    }

    // If the test failed, then we write the actual output to a file
    // so that we can easily diff it from the command line and
    // diagnose the problem.
    if (result == TestResult::Fail)
    {
        String actualOutputPath = outputStem + ".actual";
        Slang::File::writeAllText(actualOutputPath, actualOutput);

        context->reporter->dumpOutputDifference(expectedOutput, actualOutput);
    }

    return result;
}

TestResult generateHLSLBaseline(
    TestContext* context,
    TestInput& input,
    char const* targetFormat,
    char const* passThroughName)
{
    auto filePath999 = input.filePath;
    auto outputStem = input.outputStem;

    CommandLine cmdLine;
    _initSlangCompiler(context, cmdLine);

    cmdLine.addArg(filePath999);

    for( auto arg : input.testOptions->args )
    {
        cmdLine.addArg(arg);
    }

    cmdLine.addArg("-target");
    cmdLine.addArg(targetFormat);
    cmdLine.addArg("-pass-through");
    cmdLine.addArg(passThroughName);

    ExecuteResult exeRes;
    TEST_RETURN_ON_DONE(spawnAndWait(context, outputStem, input.spawnType, cmdLine, exeRes));

    if (context->isCollectingRequirements())
    {
        return TestResult::Pass;
    }

    String expectedOutput = getOutput(exeRes);
    String expectedOutputPath = outputStem + ".expected";
    try
    {
        Slang::File::writeAllText(expectedOutputPath, expectedOutput);
    }
    catch (Slang::IOException)
    {
        return TestResult::Fail;
    }
    return TestResult::Pass;
}

TestResult generateHLSLBaseline(
    TestContext* context,
    TestInput& input)
{
    return generateHLSLBaseline(context, input, "dxbc-assembly", "fxc");
}

static TestResult _runHLSLComparisonTest(
    TestContext* context,
    TestInput& input,
    char const* targetFormat,
    char const* passThroughName)
{
    auto filePath999 = input.filePath;
    auto outputStem = input.outputStem;

    // We will use the Microsoft compiler to generate out expected output here
    String expectedOutputPath = outputStem + ".expected";

    // Generate the expected output using standard HLSL compiler
    generateHLSLBaseline(context, input, targetFormat, passThroughName);

    // need to execute the stand-alone Slang compiler on the file, and compare its output to what we expect

    CommandLine cmdLine;
    _initSlangCompiler(context, cmdLine);

    cmdLine.addArg(filePath999);

    for( auto arg : input.testOptions->args )
    {
        cmdLine.addArg(arg);
    }

    // TODO: The compiler should probably define this automatically...
    cmdLine.addArg("-D");
    cmdLine.addArg("__SLANG__");

    cmdLine.addArg("-target");
    cmdLine.addArg(targetFormat);

    ExecuteResult exeRes;
    TEST_RETURN_ON_DONE(spawnAndWait(context, outputStem, input.spawnType, cmdLine, exeRes));

    if (context->isCollectingRequirements())
    {
        return TestResult::Pass;
    }

    // We ignore output to stdout, and only worry about what the compiler
    // wrote to stderr.

    ExecuteResult::ResultCode resultCode = exeRes.resultCode;
    
    String standardOutput = exeRes.standardOutput;
    String standardError = exeRes.standardError;
    
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
        expectedOutput = Slang::File::readAllText(expectedOutputPath);
    }
    catch (Slang::IOException)
    {
    }

    TestResult result = TestResult::Pass;

    // If no expected output file was found, then we
    // expect everything to be empty
    if (expectedOutput.getLength() == 0)
    {
        if (resultCode != 0)				result = TestResult::Fail;
        if (standardError.getLength() != 0)	result = TestResult::Fail;
        if (standardOutput.getLength() != 0)	result = TestResult::Fail;
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
        Slang::File::writeAllText(actualOutputPath, actualOutput);

        context->reporter->dumpOutputDifference(expectedOutput, actualOutput);
    }

    return result;
}

static TestResult runDXBCComparisonTest(
    TestContext* context,
    TestInput& input)
{
    return _runHLSLComparisonTest(context, input, "dxbc-assembly", "fxc");
}

static TestResult runDXILComparisonTest(
    TestContext* context,
    TestInput& input)
{
    return _runHLSLComparisonTest(context, input, "dxil-assembly", "dxc");
}

TestResult doGLSLComparisonTestRun(TestContext* context,
    TestInput& input,
    char const* langDefine,
    char const* passThrough,
    char const* outputKind,
    String* outOutput)
{
    auto filePath999 = input.filePath;
    auto outputStem = input.outputStem;

    CommandLine cmdLine;
    _initSlangCompiler(context, cmdLine);

    cmdLine.addArg(filePath999);

    if( langDefine )
    {
        cmdLine.addArg("-D");
        cmdLine.addArg(langDefine);
    }

    if( passThrough )
    {
        cmdLine.addArg("-pass-through");
        cmdLine.addArg(passThrough);
    }

    cmdLine.addArg("-target");
    cmdLine.addArg("spirv-assembly");

    for( auto arg : input.testOptions->args )
    {
        cmdLine.addArg(arg);
    }

    ExecuteResult exeRes;
    TEST_RETURN_ON_DONE(spawnAndWait(context, outputStem, input.spawnType, cmdLine, exeRes));

    if (context->isCollectingRequirements())
    {
        return TestResult::Pass;
    }

    ExecuteResult::ResultCode resultCode = exeRes.resultCode;

    String standardOuptut = exeRes.standardOutput;
    String standardError = exeRes.standardError;

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

    if (context->isCollectingRequirements())
    {
        return TestResult::Pass;
    }

    // If either is ignored, the whole test is
    if (hlslResult == TestResult::Ignored ||
        slangResult == TestResult::Ignored)
    {
        return TestResult::Ignored;
    }

    Slang::File::writeAllText(outputStem + ".expected", expectedOutput);
    Slang::File::writeAllText(outputStem + ".actual",   actualOutput);

    if( hlslResult  == TestResult::Fail )   return TestResult::Fail;
    if( slangResult == TestResult::Fail )   return TestResult::Fail;

    if (actualOutput != expectedOutput)
    {
        context->reporter->dumpOutputDifference(expectedOutput, actualOutput);

        return TestResult::Fail;
    }

    return TestResult::Pass;
}

static void _addRenderTestOptions(const Options& options, CommandLine& ioCmdLine)
{
    if (options.adapter.getLength())
    {
        ioCmdLine.addArg("-adapter");
        ioCmdLine.addArg(options.adapter);
    }
}

static SlangResult _extractProfileTime(const UnownedStringSlice& text, double& timeOut)
{
    // Need to find the profile figure..
    LineParser parser(text);

    const auto lineStart = UnownedStringSlice::fromLiteral("profile-time=");
    for (auto line : parser)
    {
        if (line.startsWith(lineStart))
        {
            UnownedStringSlice remaining(line.begin() + lineStart.size(), line.end());
            remaining.trim();

            timeOut = StringToDouble(String(remaining));
            return SLANG_OK;
        }
    }

    return SLANG_FAIL;
}

TestResult runPerformanceProfile(TestContext* context, TestInput& input)
{
    auto outputStem = input.outputStem;

    CommandLine cmdLine;

    cmdLine.setExecutablePath(Path::combine(context->options.binDir, String("render-test") + ProcessUtil::getExecutableSuffix()));
    
    cmdLine.addArg(input.filePath);
    cmdLine.addArg("-performance-profile");

    _addRenderTestOptions(context->options, cmdLine);

    for (auto arg : input.testOptions->args)
    {
        cmdLine.addArg(arg);
    }

    ExecuteResult exeRes;
    TEST_RETURN_ON_DONE(spawnAndWait(context, outputStem, input.spawnType, cmdLine, exeRes));
    if (context->isCollectingRequirements())
    {
        return TestResult::Pass;
    }

    auto actualOutput = getOutput(exeRes);

    double time;
    if (SLANG_FAILED(_extractProfileTime(actualOutput.getUnownedSlice(), time)))
    {
        return TestResult::Fail;
    }

    context->reporter->addExecutionTime(time);

    return TestResult::Pass;
}

TestResult runComputeComparisonImpl(TestContext* context, TestInput& input, const char *const* langOpts, size_t numLangOpts)
{
	// TODO: delete any existing files at the output path(s) to avoid stale outputs leading to a false pass
	auto filePath999 = input.filePath;
	auto outputStem = input.outputStem;

	CommandLine cmdLine;

    cmdLine.setExecutablePath(Path::combine(context->options.binDir, String("render-test") + ProcessUtil::getExecutableSuffix()));
    cmdLine.addArg(filePath999);

    _addRenderTestOptions(context->options, cmdLine);

	for (auto arg : input.testOptions->args)
	{
        cmdLine.addArg(arg);
	}

    for (int i = 0; i < int(numLangOpts); ++i)
    {
        cmdLine.addArg(langOpts[i]);
    }
    cmdLine.addArg("-o");
    auto actualOutputFile = outputStem + ".actual.txt";
    cmdLine.addArg(actualOutputFile);

    if (context->isExecuting())
    {
        // clear the stale actual output file first. This will allow us to detect error if render-test fails and outputs nothing.
        File::writeAllText(actualOutputFile, "");
    }

    ExecuteResult exeRes;
	TEST_RETURN_ON_DONE(spawnAndWait(context, outputStem, input.spawnType, cmdLine, exeRes));

    if (context->isCollectingRequirements())
    {
        return TestResult::Pass;
    }

    const String referenceOutput = findExpectedPath(input, ".expected.txt");
    if (referenceOutput.getLength() <= 0)
    {
        return TestResult::Fail;
    }

    auto actualOutput = getOutput(exeRes);
    auto expectedOutput = getExpectedOutput(outputStem);
    if (actualOutput != expectedOutput)
    {
        context->reporter->dumpOutputDifference(expectedOutput, actualOutput);

        String actualOutputPath = outputStem + ".actual";
        Slang::File::writeAllText(actualOutputPath, actualOutput);

        return TestResult::Fail;
    }

	// check against reference output
    if (!File::exists(actualOutputFile))
    {
        printf("render-test not producing expected outputs.\n");
        printf("render-test output:\n%s\n", actualOutput.getBuffer());
		return TestResult::Fail;
    }
    if (!File::exists(referenceOutput))
    {
        printf("referenceOutput %s not found.\n", referenceOutput.getBuffer());
		return TestResult::Fail;
    }
    auto actualOutputContent = File::readAllText(actualOutputFile);
	auto actualProgramOutput = Split(actualOutputContent, '\n');
	auto referenceProgramOutput = Split(File::readAllText(referenceOutput), '\n');
    auto printOutput = [&]()
    {
        context->reporter->messageFormat(TestMessageType::TestFailure, "output mismatch! actual output: {\n%s\n}, \n%s\n", actualOutputContent.getBuffer(), actualOutput.getBuffer());
    };
    if (actualProgramOutput.getCount() < referenceProgramOutput.getCount())
    {
        printOutput();
		return TestResult::Fail;
    }
	for (Index i = 0; i < referenceProgramOutput.getCount(); i++)
	{
		auto reference = String(referenceProgramOutput[i].trim());
		auto actual = String(actualProgramOutput[i].trim());
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

    CommandLine cmdLine;

    cmdLine.setExecutablePath(Path::combine(context->options.binDir, String("render-test") + ProcessUtil::getExecutableSuffix()));
    cmdLine.addArg(filePath);

    _addRenderTestOptions(context->options, cmdLine);

    for( auto arg : input.testOptions->args )
    {
        cmdLine.addArg(arg);
    }

    cmdLine.addArg(langOption);
    cmdLine.addArg("-o");
    cmdLine.addArg(outputStem + outputKind + ".png");

    ExecuteResult exeRes;
    TEST_RETURN_ON_DONE(spawnAndWait(context, outputStem, input.spawnType, cmdLine, exeRes));

    if (context->isCollectingRequirements())
    {
        return TestResult::Pass;
    }

    ExecuteResult::ResultCode resultCode = exeRes.resultCode;

    String standardOutput = exeRes.standardOutput;
    String standardError = exeRes.standardError;
    
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
    if (SLANG_FAILED(expectedImage.read(expectedPath.getBuffer())))
    {
        reporter->messageFormat(TestMessageType::RunError, "Unable to load image ;%s'", expectedPath.getBuffer());
        return TestResult::Fail;
    }

    STBImage actualImage;
    if (SLANG_FAILED(actualImage.read(actualPath.getBuffer())))
    {
        reporter->messageFormat(TestMessageType::RunError, "Unable to load image ;%s'", actualPath.getBuffer());
        return TestResult::Fail;
    }

    if (!expectedImage.isComparable(actualImage))
    {
        reporter->messageFormat(TestMessageType::TestFailure, "Images are different sizes '%s' '%s'", actualPath.getBuffer(), expectedPath.getBuffer());
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
    if (hlslResult != TestResult::Pass)
    {
        return hlslResult;
    }
    TestResult slangResult  =  doRenderComparisonTestRun(context, input, actualArg, ".actual", &actualOutput);
    if (slangResult != TestResult::Pass)
    {
        return slangResult;
    }

    if (context->isCollectingRequirements())
    {
        return TestResult::Pass;
    }

    Slang::File::writeAllText(outputStem + ".expected", expectedOutput);
    Slang::File::writeAllText(outputStem + ".actual",   actualOutput);

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

// based on command name, dispatch to an appropriate callback
struct TestCommandInfo
{
    char const*     name;
    TestCallback    callback;
};

static const TestCommandInfo s_testCommandInfos[] =
{
    { "SIMPLE",                                 &runSimpleTest},
    { "SIMPLE_EX",                              &runSimpleTest},
    { "REFLECTION",                             &runReflectionTest},
    { "CPU_REFLECTION",                         &runReflectionTest},
    { "COMMAND_LINE_SIMPLE",                    &runSimpleCompareCommandLineTest},
    { "COMPARE_HLSL",                           &runDXBCComparisonTest},
    { "COMPARE_DXIL",                           &runDXILComparisonTest},
    { "COMPARE_HLSL_RENDER",                    &runHLSLRenderComparisonTest},
    { "COMPARE_HLSL_CROSS_COMPILE_RENDER",      &runHLSLCrossCompileRenderComparisonTest},
    { "COMPARE_HLSL_GLSL_RENDER",               &runHLSLAndGLSLRenderComparisonTest},
    { "COMPARE_COMPUTE",                        &runSlangComputeComparisonTest},
    { "COMPARE_COMPUTE_EX",                     &runSlangComputeComparisonTestEx},
    { "HLSL_COMPUTE",                           &runHLSLComputeTest},
    { "COMPARE_RENDER_COMPUTE",                 &runSlangRenderComputeComparisonTest},
    { "COMPARE_GLSL",                           &runGLSLComparisonTest},
    { "CROSS_COMPILE",                          &runCrossCompilerTest},
    { "CPP_COMPILER_EXECUTE",                   &runCPPCompilerExecute},
    { "CPP_COMPILER_SHARED_LIBRARY",            &runCPPCompilerSharedLibrary},
    { "CPP_COMPILER_COMPILE",                   &runCPPCompilerCompile},
    { "PERFORMANCE_PROFILE",                    &runPerformanceProfile},
    { "COMPILE",                                &runCompile},
};

TestResult runTest(
    TestContext*        context, 
    String const&       filePath,
    String const&       outputStem,
    String const&       testName,
    TestOptions const&  testOptions)
{
    // If we are collecting requirements and it's diagnostic test, we always run
    // (ie no requirements need to be captured - effectively it has 'no requirements')
    if (context->isCollectingRequirements() && testOptions.type == TestOptions::Diagnostic)
    {
        return TestResult::Pass;
    }

    const SpawnType defaultSpawnType = context->options.useExes ? SpawnType::UseExe : SpawnType::UseSharedLibrary;

    for( const auto& command : s_testCommandInfos)
    {
        if(testOptions.command != command.name)
            continue;

        TestInput testInput;
        testInput.filePath = filePath;
        testInput.outputStem = outputStem;
        testInput.testOptions = &testOptions;
        testInput.spawnType = defaultSpawnType;

        return command.callback(context, testInput);
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

    // Otherwise include any test the user asked for
    for( auto testCategory : test.categories )
    {
        if(testCategoryMatches(testCategory, context->options.includeCategories))
            return true;
    }

    // skip by default
    return false;
}

static void _calcSynthesizedTests(TestContext* context, RenderApiType synthRenderApiType, const List<TestDetails>& srcTests, List<TestDetails>& ioSynthTests)
{
    // Add the explicit parameter
    for (const auto& srcTest: srcTests)
    {
        const auto& requirements = srcTest.requirements;

        // Render tests use renderApis...
        // If it's an explicit test, we don't synth from it now

        // In the case of CUDA, we can only synth from a CPU source
        if (synthRenderApiType == RenderApiType::CUDA)
        {
            if (requirements.explicitRenderApi != RenderApiType::CPU)
            {
                continue;
            }

            // If the source language is defined, and it's

            const Index index = srcTest.options.args.indexOf("-source-language");
            if (index >= 0)
            {
                //
                const auto& language = srcTest.options.args[index + 1];
                SlangSourceLanguage sourceLanguage = DownstreamCompiler::getSourceLanguageFromName(language.getUnownedSlice());

                bool isCrossCompile = true;

                switch (sourceLanguage)
                {
                    case SLANG_SOURCE_LANGUAGE_GLSL:
                    case SLANG_SOURCE_LANGUAGE_C:
                    case SLANG_SOURCE_LANGUAGE_CPP:
                    {
                        isCrossCompile = false;
                    }
                    default: break;
                }

                if (!isCrossCompile)
                {
                    continue;
                }
            }
        }
        else
        {
            // TODO(JS): Arguably we should synthesize from explicit tests. In principal we can remove the explicit api apply another
            // although that may not always work.
            if (requirements.usedRenderApiFlags == 0 ||
                requirements.explicitRenderApi != RenderApiType::Unknown)
            {
                continue;
            }
        }

        TestDetails synthTestDetails(srcTest.options);
        TestOptions& synthOptions = synthTestDetails.options;

        // Mark as synthesized
        synthOptions.isSynthesized = true;

        StringBuilder builder;
        builder << "-";
        builder << RenderApiUtil::getApiName(synthRenderApiType);

        synthOptions.args.add(builder);

        // If the target is vulkan remove the -hlsl option
        if (synthRenderApiType == RenderApiType::Vulkan)
        {
            const Index index = synthOptions.args.indexOf("-hlsl");
            if (index >= 0)
            {
                synthOptions.args.removeAt(index);
            }
        }
        else if (synthRenderApiType == RenderApiType::CUDA)
        {
            const Index index = synthOptions.args.indexOf("-cpu");
            if (index >= 0)
            {
                synthOptions.args.removeAt(index);
            }
        }

        // Work out the info about this tests
        context->testRequirements = &synthTestDetails.requirements;
        runTest(context, "", "", "", synthOptions);
        context->testRequirements = nullptr;

        // It does set the explicit render target
        SLANG_ASSERT(synthTestDetails.requirements.explicitRenderApi == synthRenderApiType);
        // Add to the tests
        ioSynthTests.add(synthTestDetails);
    }
}

static bool _canIgnore(TestContext* context,
    const TestRequirements& requirements)
{
    // Work out what render api flags are available
    const RenderApiFlags availableRenderApiFlags = requirements.usedRenderApiFlags ? _getAvailableRenderApiFlags(context) : 0;

    // Are all the required backends available?
    if (((requirements.usedBackendFlags & context->availableBackendFlags) != requirements.usedBackendFlags))
    {
        return true;
    }

    // Are all the required rendering apis available?
    if ((requirements.usedRenderApiFlags & availableRenderApiFlags) != requirements.usedRenderApiFlags)
    {
        return true;
    }

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
    if( testList.tests.getCount() == 0 )
    {
        context->reporter->addTest(filePath, TestResult::Ignored);
        return;
    }

    RenderApiFlags apiUsedFlags = 0;
    RenderApiFlags explictUsedApiFlags = 0;

    {
        // We can get the test info for each of them
        for (auto& testDetails : testList.tests)
        {
            auto& requirements = testDetails.requirements;

            // Collect what the test needs (by setting restRequirements the test isn't actually run)
            context->testRequirements = &requirements;
            runTest(context, filePath, filePath, filePath, testDetails.options);

            // 
            apiUsedFlags |= requirements.usedRenderApiFlags;
            explictUsedApiFlags |= (requirements.explicitRenderApi != RenderApiType::Unknown) ? (RenderApiFlags(1) << int(requirements.explicitRenderApi)) : 0;
        }
        context->testRequirements = nullptr;
    }

    SLANG_ASSERT((apiUsedFlags & explictUsedApiFlags) == explictUsedApiFlags);

    const RenderApiFlags availableRenderApiFlags = apiUsedFlags ? _getAvailableRenderApiFlags(context) : 0;

    // If synthesized tests are wanted look into adding them
    if (context->options.synthesizedTestApis && availableRenderApiFlags)
    {
        List<TestDetails> synthesizedTests;

        // What render options do we want to synthesize
        RenderApiFlags missingApis = (~apiUsedFlags) & (context->options.synthesizedTestApis & availableRenderApiFlags);

        //const Index numInitialTests = testList.tests.getCount();

        while (missingApis)
        {
            const int index = ByteEncodeUtil::calcMsb8(missingApis);
            SLANG_ASSERT(index >= 0 && index <= int(RenderApiType::CountOf));

            const RenderApiType synthRenderApiType = RenderApiType(index);

            _calcSynthesizedTests(context, synthRenderApiType, testList.tests, synthesizedTests);
            
            // Disable the bit
            missingApis &= ~(RenderApiFlags(1) << index);
        }

        // Add all the synthesized tests
        testList.tests.addRange(synthesizedTests);
    }

    // We have found a test to run!
    int subTestCount = 0;
    for( auto& testDetails : testList.tests )
    {
        int subTestIndex = subTestCount++;

        // Check that the test passes our current category mask
        if(!testPassesCategoryMask(context, testDetails.options))
        {
            continue;
        }

        // Work out the test stem

        StringBuilder outputStem;
        outputStem << filePath;
        if (subTestIndex != 0)
        {
            outputStem << "." << subTestIndex;
        }

        // Work out the test name - taking into account render api / if synthesized
        StringBuilder testName(outputStem);

        if (testDetails.options.isSynthesized)
        {
            testName << " syn";
        }

        const auto& requirements = testDetails.requirements;

        // Display list of used apis on render test
        if (requirements.usedRenderApiFlags)
        {
            RenderApiFlags usedFlags = requirements.usedRenderApiFlags;
            testName << " (";
            bool isPrev = false;
            while (usedFlags)
            {
                const int index = ByteEncodeUtil::calcMsb8(usedFlags);
                const RenderApiType renderApiType = RenderApiType(index);
                if (isPrev)
                {
                    testName << ",";
                }
                testName << RenderApiUtil::getApiName(renderApiType);

                // Disable bit
                usedFlags &= ~(RenderApiFlags(1) << index);
                isPrev = true;
            }
            testName << ")";
        }

        // Report the test and run/ignore
        {
            auto reporter = context->reporter;
            TestReporter::TestScope scope(reporter, testName);

            TestResult testResult = TestResult::Fail;

            // If this test can be ignored
            if (_canIgnore(context, testDetails.requirements))
            {
                testResult = TestResult::Ignored;
            }
            else
            {
                testResult = runTest(context, filePath, outputStem, testName, testDetails.options);
            }

            reporter->addResult(testResult);

            // Could determine if to continue or not here... based on result
        }        
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
        ".c",
        ".cpp",
        ".cu",
        };

    for( auto allowedExtension : allowedExtensions)
    {
        if(filePath.endsWith(allowedExtension))
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
//            fprintf(stderr, "slang-test: found '%s'\n", file.getBuffer());
            runTestsOnFile(context, file);
        }
    }
    for (auto subdir : osFindChildDirectories(directoryPath))
    {
        runTestsInDirectory(context, subdir);
    }
}

static void _disableCPPBackends(TestContext* context)
{
    const SlangPassThrough cppPassThrus[] =
    {
        SLANG_PASS_THROUGH_GENERIC_C_CPP,
        SLANG_PASS_THROUGH_VISUAL_STUDIO,
        SLANG_PASS_THROUGH_CLANG,
        SLANG_PASS_THROUGH_GCC,
    };

    for (auto passThru : cppPassThrus)
    {
        context->availableBackendFlags &= ~(PassThroughFlags(1) << int(passThru));
    }
}


SlangResult innerMain(int argc, char** argv)
{
    auto stdWriters = StdWriters::initDefaultSingleton();

    // The context holds useful things used during testing
    TestContext context;
    SLANG_RETURN_ON_FAIL(SLANG_FAILED(context.init()))

    TestToolUtil::setSessionDefaultPrelude(argv[0], context.getSession());

    auto& categorySet = context.categorySet;

    // Set up our test categories here
    auto fullTestCategory = categorySet.add("full", nullptr);
    auto quickTestCategory = categorySet.add("quick", fullTestCategory);
    /*auto smokeTestCategory = */categorySet.add("smoke", quickTestCategory);
    auto renderTestCategory = categorySet.add("render", fullTestCategory);
    /*auto computeTestCategory = */categorySet.add("compute", fullTestCategory);
    auto vulkanTestCategory = categorySet.add("vulkan", fullTestCategory);
    auto unitTestCatagory = categorySet.add("unit-test", fullTestCategory);
    auto cudaTestCategory = categorySet.add("cuda", fullTestCategory);

    auto compatibilityIssueCategory = categorySet.add("compatibility-issue", fullTestCategory);
        
#if SLANG_WINDOWS_FAMILY
    auto windowsCategory = categorySet.add("windows", fullTestCategory);
#endif

#if SLANG_UNIX_FAMILY
    auto unixCatagory = categorySet.add("unix", fullTestCategory);
#endif

    // An un-categorized test will always belong to the `full` category
    categorySet.defaultCategory = fullTestCategory;

    TestCategory* fxcCategory = nullptr;
    TestCategory* dxcCategory = nullptr;
    TestCategory* glslangCategory = nullptr;
    TestCategory* nvrtcCategory = nullptr; 

    // Work out what backends/pass-thrus are available
    {
        SlangSession* session = context.getSession();

        for (int i = 0; i < SLANG_PASS_THROUGH_COUNT_OF; ++i)
        {
            SlangPassThrough passThru = SlangPassThrough(i);

            if (SLANG_SUCCEEDED(spSessionCheckPassThroughSupport(session, passThru)))
            {
                context.availableBackendFlags |= PassThroughFlags(1) << int(i);
            }
        }

        if (context.availableBackendFlags & PassThroughFlag::Fxc)
        {
            fxcCategory = categorySet.add("fxc", fullTestCategory);
        }
        if (context.availableBackendFlags & PassThroughFlag::Glslang)
        {
            glslangCategory = categorySet.add("glslang", fullTestCategory);
        }
        if (context.availableBackendFlags & PassThroughFlag::Dxc)
        {
            dxcCategory = categorySet.add("dxc", fullTestCategory);
        }
        if (context.availableBackendFlags & PassThroughFlag::NVRTC)
        {
            nvrtcCategory = categorySet.add("nvrtc", fullTestCategory);
        }
    }

    // Working out what renderApis is worked on on demand through
    // _getAvailableRenderApiFlags()

    {
        // We can set the slangc command line tool, to just use the function defined here
        context.setInnerMainFunc("slangc", &SlangCTool::innerMain);
    }

    SLANG_RETURN_ON_FAIL(Options::parse(argc, argv, &categorySet, StdWriters::getError(), &context.options));
    
    Options& options = context.options;

    if (options.outputMode == TestOutputMode::TeamCity)
    {
        // On TeamCity CI there is an issue with unix/linux targets where test system may be different from the build system
        // That we rely on having compilation tools present such that on x64 systems we can build x86 binaries, and that appears to
        // not be the case.
#if SLANG_UNIX_FAMILY && SLANG_PROCESSOR_X86
        _disableCPPBackends(&context);
#endif
    }

    if (options.subCommand.getLength())
    {
        // Get the function from the tool
        auto func = context.getInnerMainFunc(options.binDir, options.subCommand);
        if (!func)
        {
            StdWriters::getError().print("error: Unable to launch tool '%s'\n", options.subCommand.getBuffer());
            return SLANG_FAIL;
        }

        // Copy args to a char* list
        const auto& srcArgs = options.subCommandArgs;
        List<const char*> args;
        args.setCount(srcArgs.getCount());
        for (Index i = 0; i < srcArgs.getCount(); ++i)
        {
            args[i] = srcArgs[i].getBuffer();
        }

        return func(StdWriters::getSingleton(), context.getSession(), int(args.getCount()), args.getBuffer());
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
                testOptions.categories.add(unitTestCatagory);
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
