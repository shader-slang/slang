// slang-emit-dependency-file.cpp
#include "slang-emit-dependency-file.h"

#include "slang-compiler.h"

namespace Slang
{

static void _writeString(Stream& stream, const char* string)
{
    stream.write(string, strlen(string));
}

static void _escapeDependencyString(const char* string, StringBuilder& outBuilder)
{
    // make has unusual escaping rules, but we only care about characters that are acceptable in a
    // path
    for (const char* p = string; *p; ++p)
    {
        char c = *p;
        switch (c)
        {
        case ' ':
        case ':':
        case '#':
        case '[':
        case ']':
        case '\\':
            outBuilder.appendChar('\\');
            break;

        case '$':
            outBuilder.appendChar('$');
            break;
        }

        outBuilder.appendChar(c);
    }
}

// Writes a "<output-file>: <dep> <dep...>" line to the stream.
// When outputPath is empty (output to stdout), "-" is used as the make target placeholder.
// writtenStdoutSentinel prevents duplicate "-: ..." lines across multiple call sites.
//
// Each dependency is written as " <escaped-path>" (separator first) and the statement is
// newline-terminated, so the count of dependencies need not be known in advance — the module
// dependencies below are filtered as they are emitted rather than pre-collected into a list.
static void _writeDependencyStatement(
    Stream& stream,
    EndToEndCompileRequest* compileRequest,
    const String& outputPath,
    bool& writtenStdoutSentinel)
{
    StringBuilder builder;
    if (outputPath.getLength() == 0)
    {
        if (writtenStdoutSentinel)
            return;
        writtenStdoutSentinel = true;
        _writeString(stream, "-");
    }
    else
    {
        _escapeDependencyString(outputPath.begin(), builder);
        _writeString(stream, builder.begin());
        builder.clear();
    }
    _writeString(stream, ":");

    auto writeDependency = [&](const char* path)
    {
        builder.clear();
        _escapeDependencyString(path, builder);
        _writeString(stream, " ");
        _writeString(stream, builder.begin());
    };

    // Track the paths already written so a module whose `.slang-module` identity coincides with a
    // listed source (a module imported from source) is not emitted twice.
    HashSet<String> alreadyListedPaths;
    int dependencyCount = compileRequest->getDependencyFileCount();
    for (int dependencyIndex = 0; dependencyIndex < dependencyCount; ++dependencyIndex)
    {
        const char* dependencyPath = compileRequest->getDependencyFilePath(dependencyIndex);
        alreadyListedPaths.add(dependencyPath);
        writeDependency(dependencyPath);
    }

    // A compiled `.slang-module` an `import` loads is recorded only as a module dependency, never a
    // file dependency, so `-depfile` would otherwise omit it and a consumer would miss the rebuild
    // edge to the importer. Append each such module file, skipping any path already listed as a
    // source. Draw the modules from the same unspecialized program the file dependencies come from
    // (`getDependencyFilePath`), so the module set and the dedup set share one dependency closure.
    auto program = compileRequest->getUnspecializedGlobalAndEntryPointsComponentType();
    for (auto module : program->getModuleDependencies())
    {
        // Classify on the file path; a source-loaded module has a `.slang` path already emitted.
        const char* filePath = module->getFilePath();
        if (!filePath || !UnownedStringSlice(filePath).endsWith(toSlice(".slang-module")))
            continue;
        // Emit the same identity representation `getDependencyFilePath` uses for file dependencies.
        const char* emitPath = module->getUniqueIdentity();
        if (!emitPath)
            emitPath = filePath;
        if (alreadyListedPaths.add(String(emitPath)))
            writeDependency(emitPath);
    }

    _writeString(stream, "\n");
}

// Writes a file with dependency info, with one line in the output file per compile product.
SlangResult writeDependencyFile(EndToEndCompileRequest* compileRequest)
{
    if (compileRequest->m_dependencyOutputPath.getLength() == 0)
        return SLANG_OK;

    FileStream stream;
    SLANG_RETURN_ON_FAIL(stream.init(
        compileRequest->m_dependencyOutputPath,
        FileMode::Create,
        FileAccess::Write,
        FileShare::ReadWrite));

    auto linkage = compileRequest->getLinkage();
    auto program = compileRequest->getSpecializedGlobalAndEntryPointsComponentType();

    bool writtenStdoutSentinel = false;

    // Iterate over all the targets and their outputs
    for (const auto& targetReq : linkage->targets)
    {
        if (compileRequest->getTargetOptionSet(targetReq).getBoolOption(
                CompilerOptionName::GenerateWholeProgram))
        {
            RefPtr<EndToEndCompileRequest::TargetInfo> targetInfo;
            if (compileRequest->m_targetInfos.tryGetValue(targetReq, targetInfo))
            {
                _writeDependencyStatement(
                    stream,
                    compileRequest,
                    targetInfo->wholeTargetOutputPath,
                    writtenStdoutSentinel);
            }
        }
        else
        {
            Index entryPointCount = program->getEntryPointCount();
            for (Index entryPointIndex = 0; entryPointIndex < entryPointCount; ++entryPointIndex)
            {
                RefPtr<EndToEndCompileRequest::TargetInfo> targetInfo;
                if (compileRequest->m_targetInfos.tryGetValue(targetReq, targetInfo))
                {
                    String outputPath;
                    if (targetInfo->entryPointOutputPaths.tryGetValue(entryPointIndex, outputPath))
                    {
                        _writeDependencyStatement(
                            stream,
                            compileRequest,
                            outputPath,
                            writtenStdoutSentinel);
                    }
                }
            }
        }
    }

    // When the output is a binary module, linkage->targets can be empty. So
    // we need to do their dependencies separately.
    if (compileRequest->m_containerFormat == ContainerFormat::SlangModule)
    {
        _writeDependencyStatement(
            stream,
            compileRequest,
            compileRequest->m_containerOutputPath,
            writtenStdoutSentinel);
    }

    return SLANG_OK;
}

} // namespace Slang
