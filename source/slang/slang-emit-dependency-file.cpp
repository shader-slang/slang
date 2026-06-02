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

// Writes a line to the file stream, formatted like this:
//   <output-file>: <dependency-file> <dependency-file...>
//
// When outputPath is empty (output going to stdout), "-" is used as the target placeholder.
// writtenStdoutSentinel guards against emitting duplicate "-: ..." lines when multiple
// targets or entry points share the same empty output path (e.g. two -target flags with no -o).
static void _writeDependencyStatement(
    Stream& stream,
    EndToEndCompileRequest* compileRequest,
    const String& outputPath,
    bool& writtenStdoutSentinel)
{
    StringBuilder builder;
    if (outputPath.getLength() == 0)
    {
        // No output file — output goes to stdout. Emit the sentinel only once even if
        // called multiple times (once per target/entry-point) with an empty path.
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
    for (int dependencyIndex = 0; dependencyIndex < dependencyCount; ++dependencyIndex)
    {
        builder.clear();
        _escapeDependencyString(compileRequest->getDependencyFilePath(dependencyIndex), builder);
        _writeString(stream, builder.begin());
        _writeString(stream, (dependencyIndex + 1 < dependencyCount) ? " " : "\n");
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
