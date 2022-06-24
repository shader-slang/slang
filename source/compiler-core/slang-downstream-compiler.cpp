// slang-downstream-compiler.cpp
#include "slang-downstream-compiler.h"

#include "../core/slang-common.h"
#include "../../slang-com-helper.h"
#include "../core/slang-string-util.h"

#include "../core/slang-type-text-util.h"

#include "../core/slang-io.h"
#include "../core/slang-shared-library.h"
#include "../core/slang-blob.h"
#include "../core/slang-char-util.h"

#ifdef SLANG_VC
#   include "windows/slang-win-visual-studio-util.h"
#endif

#include "slang-visual-studio-compiler-util.h"
#include "slang-gcc-compiler-util.h"
#include "slang-nvrtc-compiler.h"
#include "slang-fxc-compiler.h"
#include "slang-dxc-compiler.h"
#include "slang-glslang-compiler.h"
#include "slang-llvm-compiler.h"

namespace Slang
{

static DownstreamCompiler::Infos _calcInfos()
{
    typedef DownstreamCompiler::Info Info;
    typedef DownstreamCompiler::SourceLanguageFlag SourceLanguageFlag;
    typedef DownstreamCompiler::SourceLanguageFlags SourceLanguageFlags;

    DownstreamCompiler::Infos infos;

    infos.infos[int(SLANG_PASS_THROUGH_CLANG)] = Info(SourceLanguageFlag::CPP | SourceLanguageFlag::C);
    infos.infos[int(SLANG_PASS_THROUGH_VISUAL_STUDIO)] = Info(SourceLanguageFlag::CPP | SourceLanguageFlag::C);
    infos.infos[int(SLANG_PASS_THROUGH_GCC)] = Info(SourceLanguageFlag::CPP | SourceLanguageFlag::C);
    infos.infos[int(SLANG_PASS_THROUGH_LLVM)] = Info(SourceLanguageFlag::CPP | SourceLanguageFlag::C);

    infos.infos[int(SLANG_PASS_THROUGH_NVRTC)] = Info(SourceLanguageFlag::CUDA);

    infos.infos[int(SLANG_PASS_THROUGH_DXC)] = Info(SourceLanguageFlag::HLSL);
    infos.infos[int(SLANG_PASS_THROUGH_FXC)] = Info(SourceLanguageFlag::HLSL);
    infos.infos[int(SLANG_PASS_THROUGH_GLSLANG)] = Info(SourceLanguageFlag::GLSL);

    return infos;
}

/* static */DownstreamCompiler::Infos DownstreamCompiler::s_infos = _calcInfos();

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! DownstreamCompiler::Desc !!!!!!!!!!!!!!!!!!!!!!*/

void DownstreamCompiler::Desc::appendAsText(StringBuilder& out) const
{
    out << TypeTextUtil::getPassThroughAsHumanText(type);

    // Append the version if there is a version
    if (majorVersion || minorVersion)
    {
        out << " ";
        out << majorVersion;
        out << ".";
        out << minorVersion;
    }
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! DownstreamDiagnostic !!!!!!!!!!!!!!!!!!!!!!!!*/

/* static */UnownedStringSlice DownstreamDiagnostic::getSeverityText(Severity severity)
{
    switch (severity)
    {
        default:                return UnownedStringSlice::fromLiteral("Unknown");
        case Severity::Info:    return UnownedStringSlice::fromLiteral("Info");
        case Severity::Warning: return UnownedStringSlice::fromLiteral("Warning");
        case Severity::Error:   return UnownedStringSlice::fromLiteral("Error");
    }
}

/* static */SlangResult DownstreamDiagnostic::splitPathLocation(const UnownedStringSlice& pathLocation, DownstreamDiagnostic& outDiagnostic)
{
    const Index lineStartIndex = pathLocation.lastIndexOf('(');
    if (lineStartIndex >= 0)
    {
        outDiagnostic.filePath = UnownedStringSlice(pathLocation.head(lineStartIndex).trim());

        const UnownedStringSlice tail = pathLocation.tail(lineStartIndex + 1);
        const Index lineEndIndex = tail.indexOf(')');

        if (lineEndIndex >= 0)
        {
            // Extract the location info
            UnownedStringSlice locationSlice(tail.begin(), tail.begin() + lineEndIndex);

            UnownedStringSlice slices[2];
            const Index numSlices = StringUtil::split(locationSlice, ',', 2, slices);

            // NOTE! FXC actually outputs a range of columns in the form of START-END in the column position
            // We don't need to parse here, because we only care about the line number

            Int lineNumber = 0;
            if (numSlices > 0)
            {
                SLANG_RETURN_ON_FAIL(StringUtil::parseInt(slices[0], lineNumber));
            }

            // Store the line
            outDiagnostic.fileLine = lineNumber;
        }
    }
    else
    {
        outDiagnostic.filePath = pathLocation;
    }
    return SLANG_OK;
}

/* static */SlangResult DownstreamDiagnostic::splitColonDelimitedLine(const UnownedStringSlice& line, Int pathIndex, List<UnownedStringSlice>& outSlices)
{
    StringUtil::split(line, ':', outSlices);

    // Now we want to fix up a path as might have drive letter, and therefore :
    // If this is the situation then we need to have a slice after the one at the index
    if (outSlices.getCount() > pathIndex + 1)
    {
        const UnownedStringSlice pathStart = outSlices[pathIndex].trim();
        if (pathStart.getLength() == 1 && CharUtil::isAlpha(pathStart[0]))
        {
            // Splice back together
            outSlices[pathIndex] = UnownedStringSlice(outSlices[pathIndex].begin(), outSlices[pathIndex + 1].end());
            outSlices.removeAt(pathIndex + 1);
        }
    }

    return SLANG_OK;
}

/* static */SlangResult DownstreamDiagnostic::parseColonDelimitedDiagnostics(const UnownedStringSlice& inText, Int pathIndex, LineParser lineParser, List<DownstreamDiagnostic>& outDiagnostics)
{
    List<UnownedStringSlice> splitLine;

    UnownedStringSlice text(inText), line;
    while (StringUtil::extractLine(text, line))
    {
        SLANG_RETURN_ON_FAIL(splitColonDelimitedLine(line, pathIndex, splitLine));

        DownstreamDiagnostic diagnostic;
        diagnostic.severity = DownstreamDiagnostic::Severity::Error;
        diagnostic.stage = DownstreamDiagnostic::Stage::Compile;
        diagnostic.fileLine = 0;

        if (SLANG_SUCCEEDED(lineParser(line, splitLine, diagnostic)))
        {
            outDiagnostics.add(diagnostic);
        }
        else
        {
            // If couldn't parse, just add as a note
            DownstreamDiagnostics::addNote(line, outDiagnostics);
        }
    }

    return SLANG_OK;
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! DownstreamCompiler !!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

SlangResult DownstreamCompiler::disassemble(SlangCompileTarget sourceBlobTarget, const void* blob, size_t blobSize, ISlangBlob** out)
{
    SLANG_UNUSED(sourceBlobTarget);
    SLANG_UNUSED(blob);
    SLANG_UNUSED(blobSize);
    SLANG_UNUSED(out);

    return SLANG_E_NOT_AVAILABLE;
}


/* static */bool DownstreamCompiler::canCompile(SlangPassThrough compiler, SlangSourceLanguage sourceLanguage)
{
    const auto& info = getInfo(compiler);
    return (info.sourceLanguageFlags & (SourceLanguageFlags(1) << int(sourceLanguage))) != 0;
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! DownstreamDiagnostics !!!!!!!!!!!!!!!!!!!!!!*/

Index DownstreamDiagnostics::getCountAtLeastSeverity(Diagnostic::Severity severity) const
{
    Index count = 0;
    for (const auto& msg : diagnostics)
    {
        count += Index(Index(msg.severity) >= Index(severity));
    }
    return count;
}

Index DownstreamDiagnostics::getCountBySeverity(Diagnostic::Severity severity) const
{
    Index count = 0;
    for (const auto& msg : diagnostics)
    {
        count += Index(msg.severity == severity);
    }
    return count;
}

void DownstreamDiagnostics::requireErrorDiagnostic()
{
    // If we find an error, we don't need to add a generic diagnostic
    for (const auto& msg : diagnostics)
    {
        if (Index(msg.severity) >= Index(DownstreamDiagnostic::Severity::Error))
        {
            return;
        }
    }

    DownstreamDiagnostic diagnostic;
    diagnostic.reset();
    diagnostic.severity = DownstreamDiagnostic::Severity::Error;
    diagnostic.text = rawDiagnostics;

    // Add the diagnostic
    diagnostics.add(diagnostic);
}

Int DownstreamDiagnostics::countByStage(Diagnostic::Stage stage, Index counts[Int(Diagnostic::Severity::CountOf)]) const
{
    Int count = 0;
    ::memset(counts, 0, sizeof(Index) * Int(Diagnostic::Severity::CountOf));
    for (const auto& diagnostic : diagnostics)
    {
        if (diagnostic.stage == stage)
        {
            count++;
            counts[Index(diagnostic.severity)]++;
        }
    }
    return count++;
}

static void _appendCounts(const Index counts[Int(DownstreamDiagnostic::Severity::CountOf)], StringBuilder& out)
{
    typedef DownstreamDiagnostic::Severity Severity;

    for (Index i = 0; i < Int(Severity::CountOf); i++)
    {
        if (counts[i] > 0)
        {
            out << DownstreamDiagnostic::getSeverityText(Severity(i)) << "(" << counts[i] << ") ";
        }
    }
}

static void _appendSimplified(const Index counts[Int(DownstreamDiagnostic::Severity::CountOf)], StringBuilder& out)
{
    typedef DownstreamDiagnostic::Severity Severity;
    for (Index i = 0; i < Int(Severity::CountOf); i++)
    {
        if (counts[i] > 0)
        {
            out << DownstreamDiagnostic::getSeverityText(Severity(i)) << " ";
        }
    }
}

void DownstreamDiagnostics::appendSummary(StringBuilder& out) const
{
    Index counts[Int(Diagnostic::Severity::CountOf)];
    if (countByStage(Diagnostic::Stage::Compile, counts) > 0)
    {
        out << "Compile: ";
        _appendCounts(counts, out);
        out << "\n";
    }
    if (countByStage(Diagnostic::Stage::Link, counts) > 0)
    {
        out << "Link: ";
        _appendCounts(counts, out);
        out << "\n";
    }
}

void DownstreamDiagnostics::appendSimplifiedSummary(StringBuilder& out) const
{
    Index counts[Int(Diagnostic::Severity::CountOf)];
    if (countByStage(Diagnostic::Stage::Compile, counts) > 0)
    {
        out << "Compile: ";
        _appendSimplified(counts, out);
        out << "\n";
    }
    if (countByStage(Diagnostic::Stage::Link, counts) > 0)
    {
        out << "Link: ";
        _appendSimplified(counts, out);
        out << "\n";
    }
}

void DownstreamDiagnostics::removeBySeverity(Diagnostic::Severity severity)
{
    Index count = diagnostics.getCount();
    for (Index i = 0; i < count; ++i)
    {
        if (diagnostics[i].severity == severity)
        {
            diagnostics.removeAt(i);
            i--;
            count--;
        }
    }
}

/* static */void DownstreamDiagnostics::addNote(const UnownedStringSlice& in, List<DownstreamDiagnostic>& ioDiagnostics)
{
    // Don't bother adding an empty line
    if (in.trim().getLength() == 0)
    {
        return;
    }

    // If there's nothing previous, we'll ignore too, as note should be in addition to
    // a pre-existing error/warning
    if (ioDiagnostics.getCount() == 0)
    {
        return;
    }

    // Make it a note on the output
    DownstreamDiagnostic diagnostic;
    diagnostic.reset();
    diagnostic.severity = DownstreamDiagnostic::Severity::Info;
    diagnostic.text = in;
    ioDiagnostics.add(diagnostic);
}

void DownstreamDiagnostics::addNote(const UnownedStringSlice& in)
{
    addNote(in, diagnostics);
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! CommandLineDownstreamCompileResult !!!!!!!!!!!!!!!!!!!!!!*/

SlangResult CommandLineDownstreamCompileResult::getHostCallableSharedLibrary(ComPtr<ISlangSharedLibrary>& outLibrary)
{
    if (m_hostCallableSharedLibrary)
    {
        outLibrary = m_hostCallableSharedLibrary;
        return SLANG_OK;
    }

    // Okay we want to load
    // Try loading the shared library
    SharedLibrary::Handle handle;
    if (SLANG_FAILED(SharedLibrary::loadWithPlatformPath(m_moduleFilePath.getBuffer(), handle)))
    {
        return SLANG_FAIL;
    }
    // The shared library needs to keep temp files in scope
    RefPtr<TemporarySharedLibrary> sharedLib(new TemporarySharedLibrary(handle, m_moduleFilePath));
    sharedLib->m_temporaryFileSet = m_temporaryFiles;

    m_hostCallableSharedLibrary = sharedLib;
    outLibrary = m_hostCallableSharedLibrary;
    return SLANG_OK;
}

SlangResult CommandLineDownstreamCompileResult::getBinary(ComPtr<ISlangBlob>& outBlob)
{
    if (m_binaryBlob)
    {
        outBlob = m_binaryBlob;
        return SLANG_OK;
    }

    List<uint8_t> contents;
    // Read the binary
        // Read the contents of the binary
    SLANG_RETURN_ON_FAIL(File::readAllBytes(m_moduleFilePath, contents));

    m_binaryBlob = new ScopeRefObjectBlob(ListBlob::moveCreate(contents), m_temporaryFiles);
    outBlob = m_binaryBlob;
    return SLANG_OK;
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! CommandLineDownstreamCompiler !!!!!!!!!!!!!!!!!!!!!!*/

static bool _isContentsInFile(const DownstreamCompiler::CompileOptions& options)
{
    if (options.sourceContentsPath.getLength() <= 0)
    {
        return false;
    }

    // We can see if we can load it
    if (File::exists(options.sourceContentsPath))
    {
        // Here we look for the file on the regular file system (as opposed to using the 
        // ISlangFileSystem. This is unfortunate but necessary - because when we call out
        // to the compiler all it is able to (currently) see are files on the file system.
        //
        // Note that it could be coincidence that the filesystem has a file that's identical in
        // contents/name. That being the case though, any includes wouldn't work for a generated
        // file either from some specialized ISlangFileSystem, so this is probably as good as it gets
        // until we can integrate directly to a C/C++ compiler through say a shared library where we can control
        // file system access.
        String readContents;

        if (SLANG_SUCCEEDED(File::readAllText(options.sourceContentsPath, readContents)))
        {
            return options.sourceContents == readContents.getUnownedSlice();
        }
    }
    return false;
}

SlangResult CommandLineDownstreamCompiler::compile(const CompileOptions& inOptions, RefPtr<DownstreamCompileResult>& out)
{
    // Copy the command line options
    CommandLine cmdLine(m_cmdLine);

    CompileOptions options(inOptions);

    // Find all the files that will be produced
    RefPtr<TemporaryFileSet> productFileSet(new TemporaryFileSet);
    
    
    if (options.modulePath.getLength() == 0 || options.sourceContents.getLength() != 0)
    {
        String modulePath = options.modulePath;

        // If there is no module path, generate one.
        if (modulePath.getLength() == 0)
        {
            // Holds the temporary lock path, if a temporary path is used
            String temporaryLockPath;

            // Generate a unique module path name
            SLANG_RETURN_ON_FAIL(File::generateTemporary(UnownedStringSlice::fromLiteral("slang-generated"), temporaryLockPath));
            productFileSet->add(temporaryLockPath);

            modulePath = temporaryLockPath;

            options.modulePath = modulePath;
        }

        if (_isContentsInFile(options))
        {
            options.sourceFiles.add(options.sourceContentsPath);
        }
        else
        {
            String compileSourcePath = modulePath;

            // NOTE: Strictly speaking producing filenames by modifying the generateTemporary path that may introduce a temp filename clash, but in practice is extraordinary unlikely
            compileSourcePath.append("-src");

            // Make the temporary filename have the appropriate extension.
            if (options.sourceLanguage == SLANG_SOURCE_LANGUAGE_C)
            {
                compileSourcePath.append(".c");
            }
            else
            {
                compileSourcePath.append(".cpp");
            }

            // Write it out
            productFileSet->add(compileSourcePath);
            SLANG_RETURN_ON_FAIL(File::writeAllText(compileSourcePath, options.sourceContents));
            
            // Add it as a source file
            options.sourceFiles.add(compileSourcePath);
        }

        // There is no source contents
        options.sourceContents = String();
        options.sourceContentsPath = String();
    }

    // Append command line args to the end of cmdLine using the target specific function for the specified options
    SLANG_RETURN_ON_FAIL(calcArgs(options, cmdLine));

    String moduleFilePath;

    {
        StringBuilder builder;
        SLANG_RETURN_ON_FAIL(calcModuleFilePath(options, builder));
        moduleFilePath = builder.ProduceString();
    }

    {
        List<String> paths;
        SLANG_RETURN_ON_FAIL(calcCompileProducts(options, DownstreamCompiler::ProductFlag::All, paths));
        productFileSet->add(paths);
    }

    ExecuteResult exeRes;

#if 0
    // Test
    {
        String line = ProcessUtil::getCommandLineString(cmdLine);
        printf("%s", line.getBuffer());
    }
#endif

    SLANG_RETURN_ON_FAIL(ProcessUtil::execute(cmdLine, exeRes));

#if 0
    {
        printf("stdout=\"%s\"\nstderr=\"%s\"\nret=%d\n", exeRes.standardOutput.getBuffer(), exeRes.standardError.getBuffer(), int(exeRes.resultCode));
    }
#endif

    DownstreamDiagnostics diagnostics;
    SLANG_RETURN_ON_FAIL(parseOutput(exeRes, diagnostics));

    out = new CommandLineDownstreamCompileResult(diagnostics, moduleFilePath, productFileSet);
    
    return SLANG_OK;
}

/* !!!!!!!!!!!!!!!!!!!!!!!!! DownstreamCompiler::Desc !!!!!!!!!!!!!!!!!!!!!!*/

static DownstreamCompilerMatchVersion _calcCompiledVersion()
{
    DownstreamCompilerMatchVersion matchVersion;

#if SLANG_VC
    matchVersion = WinVisualStudioUtil::getCompiledVersion();
#elif SLANG_CLANG
    matchVersion.type = SLANG_PASS_THROUGH_CLANG;
    matchVersion.matchVersion.set(Index(__clang_major__), Index(__clang_minor__));
#elif SLANG_GCC
    matchVersion.type = SLANG_PASS_THROUGH_GCC;
    matchVersion.matchVersion.set(Index(__GNUC__), Index(__GNUC_MINOR__));
#else
    // TODO(JS): Hmmm None is not quite the same as unknown. It works for now, but we might want to have a distinct enum for unknown.
    matchVersion.type = SLANG_PASS_THROUGH_NONE;
#endif

    return matchVersion;
}

/* !!!!!!!!!!!!!!!!!!!!!!!!! DownstreamCompilerUtil !!!!!!!!!!!!!!!!!!!!!!*/

DownstreamCompilerMatchVersion DownstreamCompilerUtil::getCompiledVersion()
{
    static DownstreamCompilerMatchVersion s_version = _calcCompiledVersion();
    return s_version;
}

/* static */DownstreamCompiler* DownstreamCompilerUtil::findCompiler(const DownstreamCompilerSet* set, MatchType matchType, const DownstreamCompiler::Desc& desc)
{
    List<DownstreamCompiler*> compilers;
    set->getCompilers(compilers);
    return findCompiler(compilers, matchType, desc);
}

/* static */DownstreamCompiler* DownstreamCompilerUtil::findCompiler(const List<DownstreamCompiler*>& compilers, MatchType matchType, const DownstreamCompiler::Desc& desc)
{
    if (compilers.getCount() <= 0)
    {
        return nullptr;
    }

    Int bestIndex = -1;

    const SlangPassThrough compilerType = desc.type;

    Int maxVersionValue = 0;
    Int minVersionDiff = 0x7fffffff;

    Int descVersionValue = desc.getVersionValue();

    // If we don't have version set, then anything 0 or above is good enough, and just take newest
    if (descVersionValue == 0)
    {
        maxVersionValue = -1;
        matchType = MatchType::Newest;
    }

    for (Index i = 0; i < compilers.getCount(); ++i)
    {
        DownstreamCompiler* compiler = compilers[i];
        auto compilerDesc = compiler->getDesc();

        if (compilerType == compilerDesc.type)
        {
            const Int versionValue = compilerDesc.getVersionValue();
            switch (matchType)
            {
                case MatchType::MinGreaterEqual:
                {
                    auto diff = descVersionValue - versionValue;
                    if (diff >= 0 && diff < minVersionDiff)
                    {
                        bestIndex = i;
                        minVersionDiff = diff;
                    }
                    break;
                }
                case MatchType::MinAbsolute:
                {
                    auto diff = descVersionValue - versionValue;
                    diff = (diff >= 0) ? diff : -diff;
                    if (diff < minVersionDiff)
                    {
                        bestIndex = i;
                        minVersionDiff = diff;
                    }
                    break;
                }
                case MatchType::Newest:
                {
                    if (versionValue > maxVersionValue)
                    {
                        maxVersionValue = versionValue;
                        bestIndex = i;
                    }
                    break;
                }
            }
        }
    }

    return (bestIndex >= 0) ? compilers[bestIndex] : nullptr;
}

/* static */DownstreamCompiler* DownstreamCompilerUtil::findCompiler(const List<DownstreamCompiler*>& compilers, const DownstreamCompiler::Desc& desc)
{
    for (auto compiler : compilers)
    {
        if (compiler->getDesc() == desc)
        {
            return compiler;
        }
    }
    return nullptr;
}

/* static */DownstreamCompiler* DownstreamCompilerUtil::findCompiler(const List<DownstreamCompiler*>& compilers, SlangPassThrough type, const SemanticVersion& version)
{
    DownstreamCompiler::Desc desc;
    desc.type = type;
    desc.majorVersion = version.m_major;
    desc.minorVersion = version.m_minor;
    return findCompiler(compilers, desc);
}

/* static */void DownstreamCompilerUtil::findVersions(const List<DownstreamCompiler*>& compilers, SlangPassThrough type, List<SemanticVersion>& outVersions)
{
    for (auto compiler : compilers)
    {
        auto desc = compiler->getDesc();

        if (desc.type == type)
        {
            outVersions.add(SemanticVersion(int(desc.majorVersion), int(desc.minorVersion), 0));
        }
    }
}

/* static */DownstreamCompiler* DownstreamCompilerUtil::findClosestCompiler(const List<DownstreamCompiler*>& compilers, const DownstreamCompilerMatchVersion& matchVersion)
{
    List<SemanticVersion> versions;

    findVersions(compilers, matchVersion.type, versions);

    if (versions.getCount() > 0)
    {
        if (versions.getCount() == 1)
        {
            // Must be that one
            return findCompiler(compilers, matchVersion.type, versions[0]);
        }

        // Okay lets find the best one
        auto bestVersion = MatchSemanticVersion::findAnyBest(versions.getBuffer(), versions.getCount(), matchVersion.matchVersion);

        // If one is found use it
        if (bestVersion.isSet())
        {
            return findCompiler(compilers, matchVersion.type, bestVersion);
        }
    }

    {
        // TODO(JS):
        // NOTE! This may not really be appropriate, because LLVM is *not* interchangable with 
        // a 'normal' C++ compiler as cannot access standard libraries/headers.
        // So `slang-llvm` can't be used for 'host' code.

        // These compilers should be usable interchangably. The order is important, as the first one that matches will
        // be used, so LLVM is used before CLANG or GCC if appropriate
        const SlangPassThrough compatiblePassThroughs[] =
        {
            SLANG_PASS_THROUGH_LLVM,
            SLANG_PASS_THROUGH_CLANG,
            SLANG_PASS_THROUGH_GCC,
        };

        // Check the version is one of the compatible types
        if (makeConstArrayView(compatiblePassThroughs).indexOf(matchVersion.type) >= 0)
        {
            // Try each compatible type in turn
            for (auto passThrough : compatiblePassThroughs)
            {
                versions.clear();
                findVersions(compilers, passThrough, versions);

                if (versions.getCount() > 0)
                {
                    // Get the latest version (as we have no way to really compare)
                    auto latestVersion = SemanticVersion::getLatest(versions.getBuffer(), versions.getCount());
                    return findCompiler(compilers, matchVersion.type, latestVersion);
                }
            }
        }
    }

    return nullptr;
}

/* static */DownstreamCompiler* DownstreamCompilerUtil::findClosestCompiler(const DownstreamCompilerSet* set, const DownstreamCompilerMatchVersion& matchVersion)
{
    List<DownstreamCompiler*> compilers;
    set->getCompilers(compilers);
    return findClosestCompiler(compilers, matchVersion);
}

/* static */void DownstreamCompilerUtil::updateDefault(DownstreamCompilerSet* set, SlangSourceLanguage sourceLanguage)
{
    DownstreamCompiler* compiler = nullptr;

    switch (sourceLanguage)
    {
        case SLANG_SOURCE_LANGUAGE_CPP:
        case SLANG_SOURCE_LANGUAGE_C:
        {
            // Find the compiler closest to the compiler this was compiled with
            if (!compiler)
            {
                compiler = findClosestCompiler(set, getCompiledVersion());
            }
            break;
        }
        case SLANG_SOURCE_LANGUAGE_CUDA:
        {
            DownstreamCompiler::Desc desc;
            desc.type = SLANG_PASS_THROUGH_NVRTC;
            compiler = findCompiler(set, MatchType::Newest, desc);
            break;
        }
        default: break;
    }

    set->setDefaultCompiler(sourceLanguage, compiler);
}

/* static */void DownstreamCompilerUtil::updateDefaults(DownstreamCompilerSet* set)
{
    for (Index i = 0; i < Index(SLANG_SOURCE_LANGUAGE_COUNT_OF); ++i)
    {
        updateDefault(set, SlangSourceLanguage(i));
    }
}

/* static */void DownstreamCompilerUtil::setDefaultLocators(DownstreamCompilerLocatorFunc outFuncs[int(SLANG_PASS_THROUGH_COUNT_OF)])
{
    outFuncs[int(SLANG_PASS_THROUGH_VISUAL_STUDIO)] = &VisualStudioCompilerUtil::locateCompilers;
    outFuncs[int(SLANG_PASS_THROUGH_CLANG)] = &GCCDownstreamCompilerUtil::locateClangCompilers;
    outFuncs[int(SLANG_PASS_THROUGH_GCC)] = &GCCDownstreamCompilerUtil::locateGCCCompilers;
    outFuncs[int(SLANG_PASS_THROUGH_NVRTC)] = &NVRTCDownstreamCompilerUtil::locateCompilers;
    outFuncs[int(SLANG_PASS_THROUGH_DXC)] = &DXCDownstreamCompilerUtil::locateCompilers;
    outFuncs[int(SLANG_PASS_THROUGH_FXC)] = &FXCDownstreamCompilerUtil::locateCompilers;
    outFuncs[int(SLANG_PASS_THROUGH_GLSLANG)] = &GlslangDownstreamCompilerUtil::locateCompilers;
    outFuncs[int(SLANG_PASS_THROUGH_LLVM)] = &LLVMDownstreamCompilerUtil::locateCompilers;
}

static String _getParentPath(const String& path)
{
    // If we can get the canonical path, we'll do that before getting the parent
    String canonicalPath;
    if (SLANG_SUCCEEDED(Path::getCanonical(path, canonicalPath)))
    {
        return Path::getParentDirectory(canonicalPath);
    }
    else
    {
        return Path::getParentDirectory(path);
    }
}

static SlangResult _findPaths(const String& path, const char* libraryName, String& outParentPath, String& outLibraryPath)
{
    // Try to determine what the path is by looking up the path type
    SlangPathType pathType;
    if (SLANG_SUCCEEDED(Path::getPathType(path, &pathType)))
    {
        if (pathType == SLANG_PATH_TYPE_DIRECTORY)
        {
            outParentPath = path;
            outLibraryPath = Path::combine(outParentPath, libraryName);
        }
        else
        {
            SLANG_ASSERT(pathType == SLANG_PATH_TYPE_FILE);

            outParentPath = _getParentPath(path);
            outLibraryPath = path;
        }

        return SLANG_OK;
    }

    // If this failed the path could be to a shared library, but we may need to convert to the shared library filename first
    const String sharedLibraryFilePath = SharedLibrary::calcPlatformPath(path.getUnownedSlice());
    if (SLANG_SUCCEEDED(Path::getPathType(sharedLibraryFilePath, &pathType)) && pathType == SLANG_PATH_TYPE_FILE)
    {
        // We pass in the shared library path, as canonical paths can sometimes only apply to pre-existing objects. 
        outParentPath = _getParentPath(sharedLibraryFilePath);
        // The original path should work as is for the SharedLibrary load. Notably we don't use the sharedLibraryFilePath
        // as this is the wrong name to do a SharedLibrary load with.
        outLibraryPath = path;

        return SLANG_OK;
    }

    return SLANG_FAIL;
}

/* static */SlangResult DownstreamCompilerUtil::loadSharedLibrary(const String& path, ISlangSharedLibraryLoader* loader, const char*const* dependentNames, const char* inLibraryName, ComPtr<ISlangSharedLibrary>& outSharedLib)
{
    String parentPath;
    String libraryPath;

    // If a path is passed in lets, try and determine what kind of path it is.
    if (path.getLength())
    {
        if (SLANG_FAILED(_findPaths(path, inLibraryName, parentPath, libraryPath)))
        {
            // We have a few scenarios here.
            // 1) The path could be the shared library/dll filename, that will be found through some operating system mechanism
            // 2) That the shared library is *NOT* on the filesystem directly (the loader does something different)
            // 3) Permissions or some other mechanism stops the lookup from working

            // We should probably assume that the path means something, else why set it.
            // It's probably less likely that it is a directory that we can't detect - as if it's a directory as part of an app
            // it's permissions should allow detection, or be made to allow it.

            // All this being the case we should probably assume that it is the shared library name.
            libraryPath = path;

            // Attempt to get a parent. If there isn't one this will be empty, which will mean it will be ignored, which is probably
            // what we want if path is just a shared library name
            parentPath = Path::getParentDirectory(libraryPath);
        }
    }

    // Keep all dependent libs in scope, before we load the library we want
    List<ComPtr<ISlangSharedLibrary>> dependentLibs;

    // Try to load any dependent libs from the parent path
    if (dependentNames)
    {
        for (const char*const* cur = dependentNames; *cur; ++cur)
        {
            const char* dependentName = *cur;
            ComPtr<ISlangSharedLibrary> lib;
            if (parentPath.getLength())
            {
                String dependentPath = Path::combine(parentPath, dependentName);
                loader->loadSharedLibrary(dependentPath.getBuffer(), lib.writeRef());
            }
            else
            {
                loader->loadSharedLibrary(dependentName, lib.writeRef());
            }

            if (lib)
            {
                dependentLibs.add(lib);
            }
        }
    }

    if (libraryPath.getLength())
    {
        // If we hare a library path use that
        return loader->loadSharedLibrary(libraryPath.getBuffer(), outSharedLib.writeRef());
    }
    else
    {
        // Else just use the name that was passed in.
        return loader->loadSharedLibrary(inLibraryName, outSharedLib.writeRef());
    }
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! DownstreamCompilerSet !!!!!!!!!!!!!!!!!!!!!!*/

void DownstreamCompilerSet::getCompilerDescs(List<DownstreamCompiler::Desc>& outCompilerDescs) const
{
    outCompilerDescs.clear();
    for (DownstreamCompiler* compiler : m_compilers)
    {
        outCompilerDescs.add(compiler->getDesc());
    }
}

Index DownstreamCompilerSet::_findIndex(const DownstreamCompiler::Desc& desc) const
{
    const Index count = m_compilers.getCount();
    for (Index i = 0; i < count; ++i)
    { 
        if (m_compilers[i]->getDesc() == desc)
        {
            return i;
        }
    }
    return -1;
}

DownstreamCompiler* DownstreamCompilerSet::getCompiler(const DownstreamCompiler::Desc& compilerDesc) const
{
    const Index index = _findIndex(compilerDesc);
    return index >= 0 ? m_compilers[index] : nullptr;
}

void DownstreamCompilerSet::getCompilers(List<DownstreamCompiler*>& outCompilers) const
{
    outCompilers.clear();
    outCompilers.addRange((DownstreamCompiler*const*)m_compilers.begin(), m_compilers.getCount());
}

bool DownstreamCompilerSet::hasSharedLibrary(ISlangSharedLibrary* lib)
{
    const Index foundIndex = m_sharedLibraries.findFirstIndex([lib](ISlangSharedLibrary* inLib) -> bool { return lib == inLib;  });
    return(foundIndex >= 0);
}

void DownstreamCompilerSet::addSharedLibrary(ISlangSharedLibrary* lib)
{
    SLANG_ASSERT(lib);
    if (!hasSharedLibrary(lib))
    {
        m_sharedLibraries.add(ComPtr<ISlangSharedLibrary>(lib));
    }
}

bool DownstreamCompilerSet::hasCompiler(SlangPassThrough compilerType) const
{
    for (DownstreamCompiler* compiler : m_compilers)
    {
        const auto& desc = compiler->getDesc();
        if (desc.type == compilerType)
        {
            return true;
        }
    }
    return false;
}

void DownstreamCompilerSet::remove(SlangPassThrough compilerType)
{
    for (Index i = 0; i < m_compilers.getCount(); ++i)
    {
        DownstreamCompiler* compiler = m_compilers[i];
        if (compiler->getDesc().type == compilerType)
        {
            m_compilers.fastRemoveAt(i);
            i--;
        }
    }
}

void DownstreamCompilerSet::addCompiler(DownstreamCompiler* compiler)
{
    const Index index = _findIndex(compiler->getDesc());
    if (index >= 0)
    {
        m_compilers[index] = compiler;
    }
    else
    {
        m_compilers.add(compiler);
    }
}

}
