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


namespace Slang
{

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! DownstreamCompilerDesc !!!!!!!!!!!!!!!!!!!!!!*/

void DownstreamCompilerDesc::appendAsText(StringBuilder& out) const
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

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! DownstreamCompilerBase !!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

SlangResult DownstreamCompilerBase::disassemble(SlangCompileTarget sourceBlobTarget, const void* blob, size_t blobSize, ISlangBlob** out)
{
    SLANG_UNUSED(sourceBlobTarget);
    SLANG_UNUSED(blob);
    SLANG_UNUSED(blobSize);
    SLANG_UNUSED(out);

    return SLANG_E_NOT_AVAILABLE;
}

void* DownstreamCompilerBase::castAs(const Guid& guid)
{
    if (auto ptr = getInterface(guid))
    {
        return ptr;
    }
    return getObject(guid);
}

void* DownstreamCompilerBase::getInterface(const Guid& guid)
{
    if (guid == ISlangUnknown::getTypeGuid() ||
        guid == ICastable::getTypeGuid() ||
        guid == IDownstreamCompiler::getTypeGuid())
    {
        return static_cast<IDownstreamCompiler*>(this);
    }

    return nullptr;
}

void* DownstreamCompilerBase::getObject(const Guid& guid)
{
    SLANG_UNUSED(guid);
    return nullptr;
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
    
    {
        // The shared library needs to keep temp files in scope
        auto temporarySharedLibrary = new TemporarySharedLibrary(handle, m_moduleFilePath);
        // Make sure it gets a ref count
        m_hostCallableSharedLibrary = temporarySharedLibrary;
        // Set any additional info on the non COM pointer
        temporarySharedLibrary->m_temporaryFileSet = m_temporaryFiles;
    }

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

    m_binaryBlob = ScopeRefObjectBlob::create(ListBlob::moveCreate(contents), m_temporaryFiles);
    outBlob = m_binaryBlob;
    return SLANG_OK;
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! CommandLineDownstreamCompiler !!!!!!!!!!!!!!!!!!!!!!*/

static bool _isContentsInFile(const DownstreamCompileOptions& options)
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
        SLANG_RETURN_ON_FAIL(calcCompileProducts(options, DownstreamProductFlag::All, paths));
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

}
