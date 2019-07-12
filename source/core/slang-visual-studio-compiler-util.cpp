// slang-visual-studio-compiler-util.cpp
#include "slang-visual-studio-compiler-util.h"

#include "slang-common.h"
#include "../../slang-com-helper.h"
#include "slang-string-util.h"

#include "slang-io.h"

namespace Slang
{

/* static */ SlangResult VisualStudioCompilerUtil::calcModuleFilePath(const CompileOptions& options, StringBuilder& outPath)
{
    outPath.Clear();

    switch (options.targetType)
    {
        case TargetType::SharedLibrary:
        {
            outPath << options.modulePath << ".dll";
            return SLANG_OK;
        }
        case TargetType::Executable:
        {
            outPath << options.modulePath << ".exe";
            return SLANG_OK;
        }
        case TargetType::Object:
        {
            outPath << options.modulePath << ".obj";
            return SLANG_OK;
        }
        default: break;
    }

    return SLANG_FAIL;
}

/* static */void VisualStudioCompilerUtil::calcArgs(const CompileOptions& options, CommandLine& cmdLine)
{
    // https://docs.microsoft.com/en-us/cpp/build/reference/compiler-options-listed-alphabetically?view=vs-2019

    cmdLine.addArg("/nologo");
    // Generate complete debugging information
    cmdLine.addArg("/Zi");
    // Display full path of source files in diagnostics
    cmdLine.addArg("/FC");

    if (options.flags & CompileOptions::Flag::EnableExceptionHandling)
    {
        if (options.sourceType == SourceType::CPP)
        {
            // https://docs.microsoft.com/en-us/cpp/build/reference/eh-exception-handling-model?view=vs-2019
            // Assumes c functions cannot throw
            cmdLine.addArg("/EHsc");
        }
    }

    switch (options.debugInfoType)
    {
        default:
        {
            // Multithreaded statically linked runtime library
            cmdLine.addArg("/MD");
            break;
        }
        case DebugInfoType::Maximal:
        {
            // Multithreaded statically linked *debug* runtime library
            cmdLine.addArg("/MDd");
            break;
        }
    }

    // /Fd - followed by name of the pdb file
    if (options.debugInfoType != DebugInfoType::None)
    {
        cmdLine.addPrefixPathArg("/Fd", options.modulePath, ".pdb");
    }

    switch (options.optimizationLevel)
    {
        case OptimizationLevel::None:
        {
            // No optimization
            cmdLine.addArg("/Od");   
            break;
        }
        case OptimizationLevel::Default:
        {
            break;
        }
        case OptimizationLevel::High:
        {
            cmdLine.addArg("/O2");
            break;
        }
        case OptimizationLevel::Maximal:
        {
            cmdLine.addArg("/Ox");
            break;
        }
        default: break;
    }

    switch (options.floatingPointMode)
    {
        case FloatingPointMode::Default: break;
        case FloatingPointMode::Precise:
        {
            // precise is default behavior, VS also has 'strict'
            //
            // ```/fp:strict has behavior similar to /fp:precise, that is, the compiler preserves the source ordering and rounding properties of floating-point code when
            // it generates and optimizes object code for the target machine, and observes the standard when handling special values. In addition, the program may safely
            // access or modify the floating-point environment at runtime.```

            cmdLine.addArg("/fp:precise");
            break;
        }
        case FloatingPointMode::Fast:
        {
            cmdLine.addArg("/fp:fast");
            break;
        }
    }

    switch (options.targetType)
    {
        case TargetType::SharedLibrary:
        {
            // Create dynamic link library
            if (options.debugInfoType == DebugInfoType::None)
            {
                cmdLine.addArg("/LDd");
            }
            else
            {
                cmdLine.addArg("/LD");
            }

            cmdLine.addPrefixPathArg("/Fe", options.modulePath, ".dll");
            break;
        }
        case TargetType::Executable:
        {
            cmdLine.addPrefixPathArg("/Fe", options.modulePath, ".exe");
            break;
        }
        default: break;
    }

    // Object file specify it's location - needed if we are out
    cmdLine.addPrefixPathArg("/Fo", options.modulePath, ".obj");

    // Add defines
    for (const auto& define : options.defines)
    {
        StringBuilder builder;
        builder << define.nameWithSig;
        if (define.value.getLength())
        {
            builder << "=" << define.value;
        }

        cmdLine.addArg(builder);
    }

    // Add includes
    for (const auto& include : options.includePaths)
    {
        cmdLine.addArg("/I");
        cmdLine.addArg(include);
    }

    // https://docs.microsoft.com/en-us/cpp/build/reference/eh-exception-handling-model?view=vs-2019
    // /Eha - Specifies the model of exception handling. (a, s, c, r are options)

    // Files to compile
    for (const auto& sourceFile : options.sourceFiles)
    {
        cmdLine.addArg(sourceFile);
    }

    // Link options (parameters past /link go to linker)
    cmdLine.addArg("/link");

    for (const auto& libPath : options.libraryPaths)
    {
        // Note that any escaping of the path is handled in the ProcessUtil::
        cmdLine.addPrefixPathArg("/LIBPATH:", libPath);
    }
}

static SlangResult _parseErrorType(const UnownedStringSlice& in, CPPCompiler::OutputMessage::Type& outType)
{
    typedef CPPCompiler::OutputMessage::Type Type;

    if (in == "error" || in == "fatal error")
    {
        outType = Type::Error;
    }
    else if (in == "warning")
    {
        outType = Type::Warning;
    }
    else if (in == "info")
    {
        outType = Type::Info;
    }
    else
    {
        return SLANG_FAIL;
    }
    return SLANG_OK;
}

static SlangResult _parseVisualStudioLine(const UnownedStringSlice& line, CPPCompiler::OutputMessage& outMsg)
{
    typedef CPPCompiler::OutputMessage OutputMessage;

    UnownedStringSlice linkPrefix = UnownedStringSlice::fromLiteral("LINK :");
    if (line.startsWith(linkPrefix))
    {
        outMsg.stage = OutputMessage::Stage::Link;
        outMsg.type = OutputMessage::Type::Info;

        outMsg.text = UnownedStringSlice(line.begin() + linkPrefix.size(), line.end());

        return SLANG_OK;
    }

    outMsg.stage = OutputMessage::Stage::Compile;

    const char*const start = line.begin();
    const char*const end = line.end();

    UnownedStringSlice postPath;
    // Handle the path and line no
    {
        const char* cur = start;

        // We have to assume it is a path up to the first : that isn't part of a drive specification

        if ((end - cur > 2) && Path::isDriveSpecification(UnownedStringSlice(start, start + 2)))
        {
            // Skip drive spec
            cur += 2;
        }

        // Find the first colon after this
        Index colonIndex = UnownedStringSlice(cur, end).indexOf(':');
        if (colonIndex < 0)
        {
            return SLANG_FAIL;
        }

        // Looks like we have a line number
        if (cur[colonIndex - 1] == ')')
        {
            const char* lineNoEnd = cur + colonIndex - 1;
            const char* lineNoStart = lineNoEnd;
            while (lineNoStart > start && *lineNoStart != '(')
            {
                lineNoStart--;
            }
            // Check this appears plausible
            if (*lineNoStart != '(' || *lineNoEnd != ')')
            {
                return SLANG_FAIL;
            }
            Int numDigits = 0;
            Int lineNo = 0;
            for (const char* digitCur = lineNoStart + 1; digitCur < lineNoEnd; ++digitCur)
            {
                char c = *digitCur;
                if (c >= '0' && c <= '9')
                {
                    lineNo = lineNo * 10 + (c - '0');
                    numDigits++;
                }
                else
                {
                    return SLANG_FAIL;
                }
            }
            if (numDigits == 0)
            {
                return SLANG_FAIL;
            }

            outMsg.filePath = UnownedStringSlice(start, lineNoStart);
            outMsg.fileLine = lineNo;
        }
        else
        {
            outMsg.filePath = UnownedStringSlice(start, cur + colonIndex);
            outMsg.fileLine = 0;
        }

        // Save the remaining text in 'postPath'
        postPath = UnownedStringSlice(cur + colonIndex + 1, end);
    }

    // Split up the error section
    UnownedStringSlice postError;
    {
        // tests/cpp-compiler/c-compile-link-error.exe : fatal error LNK1120: 1 unresolved externals

        const Index errorColonIndex = postPath.indexOf(':');
        if (errorColonIndex < 0)
        {
            return SLANG_FAIL;
        }

        const UnownedStringSlice errorSection = UnownedStringSlice(postPath.begin(), postPath.begin() + errorColonIndex);
        Index errorCodeIndex = errorSection.lastIndexOf(' ');
        if (errorCodeIndex < 0)
        {
            return SLANG_FAIL;
        }

        // Extract the code
        outMsg.code = UnownedStringSlice(errorSection.begin() + errorCodeIndex + 1, errorSection.end());
        if (outMsg.code.startsWith(UnownedStringSlice::fromLiteral("LNK")))
        {
            outMsg.stage = OutputMessage::Stage::Link;
        }

        // Extract the bit before the code
        SLANG_RETURN_ON_FAIL(_parseErrorType(UnownedStringSlice(errorSection.begin(), errorSection.begin() + errorCodeIndex).trim(), outMsg.type));

        // Link codes start with LNK prefix
        postError = UnownedStringSlice(postPath.begin() + errorColonIndex + 1, end); 
    }

    outMsg.text = postError;

    return SLANG_OK;
}

/* static */SlangResult VisualStudioCompilerUtil::parseOutput(const ExecuteResult& exeRes, CPPCompiler::Output& outOutput)
{
    outOutput.reset();

    for (auto line : LineParser(exeRes.standardOutput.getUnownedSlice()))
    {
#if 0
        fwrite(line.begin(), 1, line.size(), stdout);
        fprintf(stdout, "\n");
#endif

        OutputMessage msg;
        if (SLANG_SUCCEEDED(_parseVisualStudioLine(line, msg)))
        {
            outOutput.messages.add(msg);
        }
    }

    // if it has a compilation error.. set on output
    if (outOutput.has(OutputMessage::Type::Error))
    {
        outOutput.result = SLANG_FAIL;
    }

    return SLANG_OK;
}

}
