// slang-gcc-compiler-util.cpp
#include "slang-gcc-compiler-util.h"

#include "slang-common.h"
#include "../../slang-com-helper.h"
#include "slang-string-util.h"

#include "slang-io.h"
#include "slang-shared-library.h"

namespace Slang
{

/* static */SlangResult GCCCompilerUtil::parseVersion(const UnownedStringSlice& text, const UnownedStringSlice& prefix, CPPCompiler::Desc& outDesc)
{
    List<UnownedStringSlice> lines;
    StringUtil::calcLines(text, lines);

    for (auto line : lines)
    {
        if (line.startsWith(prefix))
        {
            const UnownedStringSlice remainingSlice = UnownedStringSlice(line.begin() + prefix.size(), line.end()).trim();
            const Index versionEndIndex = remainingSlice.indexOf(' ');
            if (versionEndIndex < 0)
            {
                return SLANG_FAIL;
            }

            const UnownedStringSlice versionSlice(remainingSlice.begin(), remainingSlice.begin() + versionEndIndex);

            // Version is in format 0.0.0 
            List<UnownedStringSlice> split;
            StringUtil::split(versionSlice, '.', split);
            List<Int> digits;

            for (auto v : split)
            {
                Int version;
                SLANG_RETURN_ON_FAIL(StringUtil::parseInt(v, version));
                digits.add(version);
            }

            if (digits.getCount() < 2)
            {
                return SLANG_FAIL;
            }

            outDesc.majorVersion = digits[0];
            outDesc.minorVersion = digits[1];
            return SLANG_OK;
        }
    }

    return SLANG_FAIL;
}

SlangResult GCCCompilerUtil::calcVersion(const String& exeName, CPPCompiler::Desc& outDesc)
{
    CommandLine cmdLine;
    cmdLine.setExecutableFilename(exeName);
    cmdLine.addArg("-v");

    ExecuteResult exeRes;
    SLANG_RETURN_ON_FAIL(ProcessUtil::execute(cmdLine, exeRes));

    const UnownedStringSlice prefixes[] =
    {
        UnownedStringSlice::fromLiteral("clang version"),
        UnownedStringSlice::fromLiteral("gcc version"),
        UnownedStringSlice::fromLiteral("Apple LLVM version"),
    };
    const CPPCompiler::CompilerType types[] =
    {
        CPPCompiler::CompilerType::Clang,
        CPPCompiler::CompilerType::GCC,
        CPPCompiler::CompilerType::Clang,
    };

    SLANG_COMPILE_TIME_ASSERT(SLANG_COUNT_OF(prefixes) == SLANG_COUNT_OF(types));

    for (Index i = 0; i < SLANG_COUNT_OF(prefixes); ++i)
    {
        // Set the type
        outDesc.type = types[i];
        // Extract the version
        if (SLANG_SUCCEEDED(parseVersion(exeRes.standardError.getUnownedSlice(), prefixes[i], outDesc)))
        {
            return SLANG_OK;
        }
    }
    return SLANG_FAIL;
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

namespace { // anonymous

enum class LineParseResult
{
    Start,              ///< Line was the start of a message
    Continuation,       ///< Not totally clear, add to previous line if nothing else hit
    Ignore,             ///< Ignore the line
};
    
} // anonymous
    
static SlangResult _parseGCCFamilyLine(const UnownedStringSlice& line, LineParseResult& outLineParseResult, CPPCompiler::OutputMessage& outMsg)
{
    typedef CPPCompiler::OutputMessage OutputMessage;
    typedef OutputMessage::Type Type;
    
    // Set to default case
    outLineParseResult = LineParseResult::Ignore;
    
    /* example error output from different scenarios */
    
    /*
        tests/cpp-compiler/c-compile-error.c: In function ‘int main(int, char**)’:
        tests/cpp-compiler/c-compile-error.c:8:13: error: ‘b’ was not declared in this scope
        int a = b + c;
        ^
        tests/cpp-compiler/c-compile-error.c:8:17: error: ‘c’ was not declared in this scope
        int a = b + c;
        ^
    */

    /* /tmp/ccS0JCWe.o:c-compile-link-error.c:(.rdata$.refptr.thing[.refptr.thing]+0x0): undefined reference to `thing'
       collect2: error: ld returned 1 exit status*/

    /*
     clang: warning: treating 'c' input as 'c++' when in C++ mode, this behavior is deprecated [-Wdeprecated]
     Undefined symbols for architecture x86_64:
     "_thing", referenced from:
     _main in c-compile-link-error-a83ace.o
     ld: symbol(s) not found for architecture x86_64
     clang: error: linker command failed with exit code 1 (use -v to see invocation) */
    
    outMsg.stage = OutputMessage::Stage::Compile;

    List<UnownedStringSlice> split;
    StringUtil::split(line, ':', split);

    if (split.getCount() == 2)
    {
        const auto split0 = split[0].trim();
        if (split0 == UnownedStringSlice::fromLiteral("ld"))
        {
            // We'll ignore for now
            outMsg.stage = OutputMessage::Stage::Link;
            outMsg.type = Type::Info;
            outMsg.text = split[1].trim();
            outLineParseResult = LineParseResult::Start;
            return SLANG_OK;
        }
        outLineParseResult = LineParseResult::Ignore;
        return SLANG_OK;
    }
    else if (split.getCount() == 3)
    {
        const auto split0 = split[0].trim();
        const auto split2 = split[2].trim();
        // Check for special handling for clang
        if (split0 == UnownedStringSlice::fromLiteral("clang"))
        {
            SLANG_RETURN_ON_FAIL(_parseErrorType(split[1].trim(), outMsg.type));
            
            auto text = split[2].trim();
            if (text.startsWith("linker command failed"))
            {
                outMsg.stage = OutputMessage::Stage::Link;
            }
            
            outMsg.text = text;
            outLineParseResult = LineParseResult::Start;
            return SLANG_OK;
        }
        else if (split2.startsWith("ld returned"))
        {
            outMsg.stage = CPPCompiler::OutputMessage::Stage::Link;
            SLANG_RETURN_ON_FAIL(_parseErrorType(split[1].trim(), outMsg.type));
            outMsg.text = line;
            outLineParseResult = LineParseResult::Start;
            return SLANG_OK;
        }
        else if (split2 == "")
        {
            // This is probably a prelude line, we'll just ignore it
            outLineParseResult =LineParseResult::Ignore;
            return SLANG_OK;
        }
    }
    else if (split.getCount() == 4)
    {
        // Probably a link error, give the source line
        String ext = Path::getFileExt(split[0]);

        // Maybe a bit fragile -> but probably okay for now
        if (ext != "o" && ext != "obj")
        {
            outLineParseResult = LineParseResult::Ignore;
            return SLANG_OK;
        }
        else
        {
            outMsg.filePath = split[1];
            outMsg.fileLine = 0;
            outMsg.type = OutputMessage::Type::Error;
            outMsg.stage = OutputMessage::Stage::Link;
            outMsg.text = split[3];
            
            outLineParseResult = LineParseResult::Start;
            return SLANG_OK;
        }
    }
    else if (split.getCount() == 5)
    {
        // Probably a regular error line
        SLANG_RETURN_ON_FAIL(_parseErrorType(split[3].trim(), outMsg.type));

        outMsg.filePath = split[0];
        SLANG_RETURN_ON_FAIL(StringUtil::parseInt(split[1], outMsg.fileLine));
        outMsg.text = split[4];

        outLineParseResult = LineParseResult::Start;
        return SLANG_OK;
    }
    
    // Assume it's a continuation
    outLineParseResult = LineParseResult::Continuation;
    return SLANG_OK;
}

/* static */SlangResult GCCCompilerUtil::parseOutput(const ExecuteResult& exeRes, CPPCompiler::Output& outOutput)
{
    LineParseResult prevLineResult = LineParseResult::Ignore;
    
    outOutput.reset();

    for (auto line : LineParser(exeRes.standardError.getUnownedSlice()))
    {
        CPPCompiler::OutputMessage msg;
        LineParseResult lineRes;
        
        SLANG_RETURN_ON_FAIL(_parseGCCFamilyLine(line, lineRes, msg));
        
        switch (lineRes)
        {
            case LineParseResult::Start:
            {
                // It's start of a new message
                outOutput.messages.add(msg);
                break;
            }
            case LineParseResult::Continuation:
            {
                if (prevLineResult == LineParseResult::Start || prevLineResult == LineParseResult::Continuation)
                {
                    // We are now in a continuation, add to the last
                    auto& text = outOutput.messages.getLast().text;
                    text.append("\n");
                    text.append(line);
                }
                break;
            }
            case LineParseResult::Ignore: break;
            default: return SLANG_FAIL;
        }
        
        // Remember last lines result
        prevLineResult = lineRes;
    }

    if (outOutput.has(CPPCompiler::OutputMessage::Type::Error) || exeRes.resultCode != 0)
    {
        outOutput.result = SLANG_FAIL;
    }

    return SLANG_OK;
}

/* static */void GCCCompilerUtil::calcArgs(const CompileOptions& options, CommandLine& cmdLine)
{
    cmdLine.addArg("-fvisibility=hidden");
    // Use shared libraries
    //cmdLine.addArg("-shared");

    switch (options.optimizationLevel)
    {
        case OptimizationLevel::Debug:
        {
            // No optimization
            cmdLine.addArg("-O0");
            break;
        }
        case OptimizationLevel::Normal:
        {
            cmdLine.addArg("-Os");
            break;
        }
        default: break;
    }

    if (options.debugInfoType != DebugInfoType::None)
    {
        cmdLine.addArg("-g");
    }

    switch (options.targetType)
    {
        case TargetType::SharedLibrary:
        {
            // Shared library
            cmdLine.addArg("-shared");
            // Position independent
            cmdLine.addArg("-fPIC");

            String sharedLibraryPath = SharedLibrary::calcPlatformPath(options.modulePath.getUnownedSlice());

            cmdLine.addArg("-o");
            cmdLine.addArg(sharedLibraryPath);
            break;
        }
        case TargetType::Executable:
        {
            cmdLine.addArg("-o");

            StringBuilder builder;
            builder << options.modulePath;
            builder << ProcessUtil::getExecutableSuffix();

            cmdLine.addArg(options.modulePath);
            break;
        }
        case TargetType::Object:
        {
            // Don't link, just produce object file
            cmdLine.addArg("-c");
            break;
        }
        default: break;
    }

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
        cmdLine.addArg("-I");
        cmdLine.addArg(include);
    }

    // Link options
    if (0)
    {
        StringBuilder linkOptions;
        linkOptions << "Wl,";
        cmdLine.addArg(linkOptions);
    }

    // Files to compile
    for (const auto& sourceFile : options.sourceFiles)
    {
        cmdLine.addArg(sourceFile);
    }

    for (const auto& libPath : options.libraryPaths)
    {
        // Note that any escaping of the path is handled in the ProcessUtil::
        cmdLine.addArg("-L");
        cmdLine.addArg(libPath);
        cmdLine.addArg("-F");
        cmdLine.addArg(libPath);
    }

    if (options.sourceType == SourceType::CPP)
    {
        // Make STD libs available
        cmdLine.addArg("-lstdc++");
    }
}

}
