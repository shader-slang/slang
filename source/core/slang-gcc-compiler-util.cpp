// slang-gcc-compiler-util.cpp
#include "slang-gcc-compiler-util.h"

#include "slang-common.h"
#include "../../slang-com-helper.h"
#include "slang-string-util.h"

#include "slang-io.h"
#include "slang-shared-library.h"

namespace Slang
{

/* static */SlangResult GCCDownstreamCompilerUtil::parseVersion(const UnownedStringSlice& text, const UnownedStringSlice& prefix, DownstreamCompiler::Desc& outDesc)
{
    List<UnownedStringSlice> lines;
    StringUtil::calcLines(text, lines);

    for (auto line : lines)
    {
        if (line.startsWith(prefix))
        {
            const UnownedStringSlice remainingSlice = UnownedStringSlice(line.begin() + prefix.getLength(), line.end()).trim();
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

SlangResult GCCDownstreamCompilerUtil::calcVersion(const String& exeName, DownstreamCompiler::Desc& outDesc)
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
    const SlangPassThrough types[] =
    {
        SLANG_PASS_THROUGH_CLANG,
        SLANG_PASS_THROUGH_GCC,
        SLANG_PASS_THROUGH_CLANG,
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

static SlangResult _parseSeverity(const UnownedStringSlice& in, DownstreamDiagnostic::Severity& outSeverity)
{
    typedef DownstreamDiagnostic::Severity Severity;

    if (in == "error" || in == "fatal error")
    {
        outSeverity = Severity::Error;
    }
    else if (in == "warning")
    {
        outSeverity = Severity::Warning;
    }
    else if (in == "info" || in == "note")
    {
        outSeverity = Severity::Info;
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
    Single,             ///< It's a single line
    Start,              ///< Line was the start of a message
    Continuation,       ///< Not totally clear, add to previous line if nothing else hit
    Ignore,             ///< Ignore the line
};
    
} // anonymous
    
static SlangResult _parseGCCFamilyLine(const UnownedStringSlice& line, LineParseResult& outLineParseResult, DownstreamDiagnostic& outDiagnostic)
{
    typedef DownstreamDiagnostic Diagnostic;
    typedef Diagnostic::Severity Severity;
    
    // Set to default case
    outLineParseResult = LineParseResult::Ignore;

    /* example error output from different scenarios */
    
    /*
        tests/cpp-compiler/c-compile-error.c: In function 'int main(int, char**)':
        tests/cpp-compiler/c-compile-error.c:8:13: error: 'b' was not declared in this scope
        int a = b + c;
        ^
        tests/cpp-compiler/c-compile-error.c:8:17: error: 'c' was not declared in this scope
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

     /* /tmp/c-compile-link-error-ccf151.o: In function `main':
      c-compile-link-error.c:(.text+0x19): undefined reference to `thing'
     clang: error: linker command failed with exit code 1 (use -v to see invocation)
     */

     /* /tmp/c-compile-link-error-301c8c.o: In function `main':
        /home/travis/build/shader-slang/slang/tests/cpp-compiler/c-compile-link-error.c:10: undefined reference to `thing'
        clang-7: error: linker command failed with exit code 1 (use -v to see invocation)*/

    /*  /path/slang-cpp-prelude.h:4:10: fatal error: ../slang.h: No such file or directory
        #include "../slang.h"
        ^~~~~~~~~~~~
        compilation terminated.*/
    
    outDiagnostic.stage = Diagnostic::Stage::Compile;

    List<UnownedStringSlice> split;
    StringUtil::split(line, ':', split);

    // On windows we can have paths that are a: etc... if we detect this we can combine 0 - 1 to be 1.
    if (split.getCount() > 1 && split[0].getLength() == 1)
    {
        const char c = split[0][0];
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
        {
            // We'll assume it's a path
            UnownedStringSlice path(split[0].begin(), split[1].end());
            split.removeAt(0);
            split[0] = path;
        }
    }

    if (split.getCount() == 2)
    {
        const auto split0 = split[0].trim();
        if (split0 == UnownedStringSlice::fromLiteral("ld"))
        {
            // We'll ignore for now
            outDiagnostic.stage = Diagnostic::Stage::Link;
            outDiagnostic.severity = Severity::Info;
            outDiagnostic.text = split[1].trim();
            outLineParseResult = LineParseResult::Start;
            return SLANG_OK;
        }

        if (SLANG_SUCCEEDED(_parseSeverity(split0, outDiagnostic.severity)))
        {
            // Command line errors can be just contain 'error:' etc. Can be seen on apple/clang
            outDiagnostic.stage = Diagnostic::Stage::Compile;
            outDiagnostic.text = split[1].trim();
            outLineParseResult = LineParseResult::Single;
            return SLANG_OK;
        }

        outLineParseResult = LineParseResult::Ignore;
        return SLANG_OK;
    }
    else if (split.getCount() == 3)
    {
        const auto split0 = split[0].trim();
        const auto split1 = split[1].trim();
        const auto text = split[2].trim();

        // Check for special handling for clang (Can be Clang or clang apparently)
        if (split0.startsWith(UnownedStringSlice::fromLiteral("clang")) ||
            split0.startsWith(UnownedStringSlice::fromLiteral("Clang")) )
        {
            // Extract the type
            SLANG_RETURN_ON_FAIL(_parseSeverity(split[1].trim(), outDiagnostic.severity));

            if (text.startsWith("linker command failed"))
            {
                outDiagnostic.stage = Diagnostic::Stage::Link;
            }

            outDiagnostic.text = text;
            outLineParseResult = LineParseResult::Start;
            return SLANG_OK;
        }
        else if (split1.startsWith("(.text"))
        {
            // This is a little weak... but looks like it's a link error
            outDiagnostic.filePath = split[0];
            outDiagnostic.severity = Severity::Error;
            outDiagnostic.stage = Diagnostic::Stage::Link;
            outDiagnostic.text = text;
            outLineParseResult = LineParseResult::Single;
            return SLANG_OK;
        }
        else if (text.startsWith("ld returned"))
        {
            outDiagnostic.stage = DownstreamDiagnostic::Stage::Link;
            SLANG_RETURN_ON_FAIL(_parseSeverity(split[1].trim(), outDiagnostic.severity));
            outDiagnostic.text = line;
            outLineParseResult = LineParseResult::Single;
            return SLANG_OK;
        }
        else if (text == "")
        {
            // This is probably a prelude line, we'll just ignore it
            outLineParseResult = LineParseResult::Ignore;
            return SLANG_OK;
        }
    }
    else if (split.getCount() == 4)
    {
        // Probably a link error, give the source line
        String ext = Path::getPathExt(split[0]);

        // Maybe a bit fragile -> but probably okay for now
        if (ext != "o" && ext != "obj")
        {
            outLineParseResult = LineParseResult::Ignore;
            return SLANG_OK;
        }
        else
        {
            outDiagnostic.filePath = split[1];
            outDiagnostic.fileLine = 0;
            outDiagnostic.severity = Diagnostic::Severity::Error;
            outDiagnostic.stage = Diagnostic::Stage::Link;
            outDiagnostic.text = split[3];
            
            outLineParseResult = LineParseResult::Start;
            return SLANG_OK;
        }
    }
    else if (split.getCount() >= 5)
    {
        // Probably a regular error line
        SLANG_RETURN_ON_FAIL(_parseSeverity(split[3].trim(), outDiagnostic.severity));

        outDiagnostic.filePath = split[0];
        SLANG_RETURN_ON_FAIL(StringUtil::parseInt(split[1], outDiagnostic.fileLine));

        // Everything from 4 to the end is the error
        outDiagnostic.text = UnownedStringSlice(split[4].begin(), split.getLast().end());

        outLineParseResult = LineParseResult::Start;
        return SLANG_OK;
    }

    // Assume it's a continuation
    outLineParseResult = LineParseResult::Continuation;
    return SLANG_OK;
}

/* static */SlangResult GCCDownstreamCompilerUtil::parseOutput(const ExecuteResult& exeRes, DownstreamDiagnostics& outOutput)
{
    LineParseResult prevLineResult = LineParseResult::Ignore;
    
    outOutput.reset();
    outOutput.rawDiagnostics = exeRes.standardError;

    for (auto line : LineParser(exeRes.standardError.getUnownedSlice()))
    {
        Diagnostic diagnostic;
        diagnostic.reset();

        LineParseResult lineRes;
        
        SLANG_RETURN_ON_FAIL(_parseGCCFamilyLine(line, lineRes, diagnostic));
        
        switch (lineRes)
        {
            case LineParseResult::Start:
            {
                // It's start of a new message
                outOutput.diagnostics.add(diagnostic);
                prevLineResult = LineParseResult::Start;
                break;
            }
            case LineParseResult::Single:
            {
                // It's a single message, without anything following
                outOutput.diagnostics.add(diagnostic);
                prevLineResult = LineParseResult::Ignore;
                break;
            }
            case LineParseResult::Continuation:
            {
                if (prevLineResult == LineParseResult::Start || prevLineResult == LineParseResult::Continuation)
                {
                    if (outOutput.diagnostics.getCount() > 0)
                    {
                        // We are now in a continuation, add to the last
                        auto& text = outOutput.diagnostics.getLast().text;
                        text.append("\n");
                        text.append(line);
                    }
                    prevLineResult = LineParseResult::Continuation;
                }
                break;
            }
            case LineParseResult::Ignore:
            {
                prevLineResult = lineRes;
                break;
            }
            default: return SLANG_FAIL;
        }
    }

    if (outOutput.has(Diagnostic::Severity::Error) || exeRes.resultCode != 0)
    {
        outOutput.result = SLANG_FAIL;
    }

    return SLANG_OK;
}

/* static */ SlangResult GCCDownstreamCompilerUtil::calcModuleFilePath(const CompileOptions& options, StringBuilder& outPath)
{
    SLANG_ASSERT(options.modulePath.getLength());

    outPath.Clear();

    switch (options.targetType)
    {
        case TargetType::SharedLibrary:
        {
            outPath << SharedLibrary::calcPlatformPath(options.modulePath.getUnownedSlice());
            return SLANG_OK;
        }
        case TargetType::Executable:
        {
            outPath << options.modulePath;
            outPath << ProcessUtil::getExecutableSuffix();
            return SLANG_OK;
        }
        case TargetType::Object:
        {
#if __CYGWIN__
            outPath << options.modulePath << ".obj";
#else
            // Will be .o for typical gcc targets
            outPath << options.modulePath << ".o";
#endif
            return SLANG_OK;
        }
    }

    return SLANG_FAIL;
}

/* static */SlangResult GCCDownstreamCompilerUtil::calcCompileProducts(const CompileOptions& options, ProductFlags flags, List<String>& outPaths)
{
    SLANG_ASSERT(options.modulePath.getLength());

    outPaths.clear();

    if (flags & ProductFlag::Execution)
    {
        StringBuilder builder;
        SLANG_RETURN_ON_FAIL(calcModuleFilePath(options, builder));
        outPaths.add(builder);
    }

    return SLANG_OK;
}

/* static */SlangResult GCCDownstreamCompilerUtil::calcArgs(const CompileOptions& options, CommandLine& cmdLine)
{
    SLANG_ASSERT(options.sourceContents.getLength() == 0);
    SLANG_ASSERT(options.modulePath.getLength());

    PlatformKind platformKind = (options.platform == PlatformKind::Unknown) ? PlatformUtil::getPlatformKind() : options.platform;
        
    if (options.sourceLanguage == SLANG_SOURCE_LANGUAGE_CPP)
    {
        cmdLine.addArg("-fvisibility=hidden");

        // Need C++14 for partial specialization
        cmdLine.addArg("-std=c++14");
    }

    // TODO(JS): Here we always set -m32 on x86. It could be argued it is only necessary when creating a shared library
    // but if we create an object file, we don't know what to choose because we don't know what final usage is.
    // It could also be argued that the platformKind could define the actual desired target - but as it stands
    // we only have a target of 'Linux' (as opposed to Win32/64). Really it implies we need an arch enumeration too.
    //
    // For now we just make X86 binaries try and produce x86 compatible binaries as fixes the immediate problems.
#if SLANG_PROCESSOR_X86
    /* Used to specify the processor more broadly. For a x86 binary we need to make sure we build x86 builds
    even when on an x64 system.
    -m32
    -m64*/
    cmdLine.addArg("-m32");
#endif

    switch (options.optimizationLevel)
    {
        case OptimizationLevel::None:
        {
            // No optimization
            cmdLine.addArg("-O0");
            break;
        }
        case OptimizationLevel::Default:
        {
            cmdLine.addArg("-Os");
            break;
        }
        case OptimizationLevel::High:
        {
            cmdLine.addArg("-O2");
            break;
        }
        case OptimizationLevel::Maximal:
        {
            cmdLine.addArg("-O4");
            break;
        }
        default: break;
    }

    if (options.debugInfoType != DebugInfoType::None)
    {
        cmdLine.addArg("-g");
    }

    if (options.flags & CompileOptions::Flag::Verbose)
    {
        cmdLine.addArg("-v");
    }

    switch (options.floatingPointMode)
    {
        case FloatingPointMode::Default: break;
        case FloatingPointMode::Precise:
        {
            //cmdLine.addArg("-fno-unsafe-math-optimizations");
            break;
        }
        case FloatingPointMode::Fast:
        {
            // We could enable SSE with -mfpmath=sse
            // But that would only make sense on a x64/x86 type processor and only if that feature is present (it is on all x64)
            cmdLine.addArg("-ffast-math");
            break;
        }
    }

    StringBuilder moduleFilePath;
    calcModuleFilePath(options, moduleFilePath);

    cmdLine.addArg("-o");
    cmdLine.addArg(moduleFilePath);

    switch (options.targetType)
    {
        case TargetType::SharedLibrary:
        {
            // Shared library
            cmdLine.addArg("-shared");

            if (PlatformUtil::isFamily(PlatformFamily::Unix, platformKind))
            {
                // Position independent
                cmdLine.addArg("-fPIC");
            }
            break;
        }
        case TargetType::Executable:
        {
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

        builder << "-D";
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
    if (0) // && options.targetType != TargetType::Object)
    {
        //linkOptions << "-Wl,";
        //cmdLine.addArg(linkOptions);
    }

    if (options.targetType == TargetType::SharedLibrary)
    {
        if (!PlatformUtil::isFamily(PlatformFamily::Apple, platformKind))
        {
            // On MacOS, this linker option is not supported. That's ok though in
            // so far as on MacOS it does report any unfound symbols without the option.

            // Linker flag to report any undefined symbols as a link error
            cmdLine.addArg("-Wl,--no-undefined");
        }
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

    if (options.sourceLanguage == SLANG_SOURCE_LANGUAGE_CPP && !PlatformUtil::isFamily(PlatformFamily::Windows, platformKind))
    {
        // Make STD libs available
        cmdLine.addArg("-lstdc++");
	    // Make maths lib available
        cmdLine.addArg("-lm");
    }

    return SLANG_OK;
}

/* static */SlangResult GCCDownstreamCompilerUtil::createCompiler(const String& path, const String& inExeName, RefPtr<DownstreamCompiler>& outCompiler)
{
    String exeName(inExeName);
    if (path.getLength() > 0)
    {
        exeName = Path::combine(path, inExeName);
    }

    DownstreamCompiler::Desc desc;
    SLANG_RETURN_ON_FAIL(GCCDownstreamCompilerUtil::calcVersion(exeName, desc));

    RefPtr<CommandLineDownstreamCompiler> compiler(new GCCDownstreamCompiler(desc));
    compiler->m_cmdLine.setExecutableFilename(exeName);

    outCompiler = compiler;
    return SLANG_OK;
}

/* static */SlangResult GCCDownstreamCompilerUtil::locateGCCCompilers(const String& path, ISlangSharedLibraryLoader* loader, DownstreamCompilerSet* set)
{
    SLANG_UNUSED(loader);
    RefPtr<DownstreamCompiler> compiler;
    if (SLANG_SUCCEEDED(createCompiler(path, "g++", compiler)))
    {
        set->addCompiler(compiler);
    }
    return SLANG_OK;
}

/* static */SlangResult GCCDownstreamCompilerUtil::locateClangCompilers(const String& path, ISlangSharedLibraryLoader* loader, DownstreamCompilerSet* set)
{
    SLANG_UNUSED(loader);

    RefPtr<DownstreamCompiler> compiler;
    if (SLANG_SUCCEEDED(createCompiler(path, "clang", compiler)))
    {
        set->addCompiler(compiler);
    }
    return SLANG_OK;
}

}
