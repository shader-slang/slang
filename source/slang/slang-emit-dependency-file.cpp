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

// A compiled `.slang-module` an `import` loads is recorded only as a module dependency, never a
// file dependency, so `-depfile` omits it and consumers miss the rebuild edge to the importer.
// Append each such module dependency; `alreadyListedPaths` skips a path already written (both the
// file dependencies and any module path repeated across the closure).
static void _collectExtraModuleDependencyPaths(
    EndToEndCompileRequest* compileRequest,
    List<String>& outPaths)
{
    // Use the same unspecialized program the file dependencies come from (`getDependencyFilePath`),
    // so the module set and the dedup set are drawn from one consistent dependency closure.
    auto program = compileRequest->getUnspecializedGlobalAndEntryPointsComponentType();

    HashSet<String> alreadyListedPaths;
    int fileDependencyCount = compileRequest->getDependencyFileCount();
    for (int i = 0; i < fileDependencyCount; ++i)
        alreadyListedPaths.add(compileRequest->getDependencyFilePath(i));

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
        String emitPathString(emitPath);
        if (alreadyListedPaths.add(emitPathString))
            outPaths.add(emitPathString);
    }
}

// Writes a "<output-file>: <dep> <dep...>" line to the stream.
// When outputPath is empty (output to stdout), "-" is used as the make target placeholder.
// writtenStdoutSentinel prevents duplicate "-: ..." lines across multiple call sites.
static void _writeDependencyStatement(
    Stream& stream,
    EndToEndCompileRequest* compileRequest,
    const String& outputPath,
    const List<String>& extraModuleDependencyPaths,
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
    _writeString(stream, ": ");

    int dependencyCount = compileRequest->getDependencyFileCount();
    Index extraCount = extraModuleDependencyPaths.getCount();
    Index totalCount = Index(dependencyCount) + extraCount;
    Index writtenCount = 0;
    for (int dependencyIndex = 0; dependencyIndex < dependencyCount; ++dependencyIndex)
    {
        builder.clear();
        _escapeDependencyString(compileRequest->getDependencyFilePath(dependencyIndex), builder);
        _writeString(stream, builder.begin());
        _writeString(stream, (++writtenCount < totalCount) ? " " : "\n");
    }
    for (auto& extraPath : extraModuleDependencyPaths)
    {
        builder.clear();
        _escapeDependencyString(extraPath.begin(), builder);
        _writeString(stream, builder.begin());
        _writeString(stream, (++writtenCount < totalCount) ? " " : "\n");
    }
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

    List<String> extraModuleDependencyPaths;
    _collectExtraModuleDependencyPaths(compileRequest, extraModuleDependencyPaths);

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
                    extraModuleDependencyPaths,
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
                            extraModuleDependencyPaths,
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
            extraModuleDependencyPaths,
            writtenStdoutSentinel);
    }

    return SLANG_OK;
}

} // namespace Slang
