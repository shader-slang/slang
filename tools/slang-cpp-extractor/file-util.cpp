#include "file-util.h"

#include "../../source/core/slang-io.h"

namespace CppExtract {
using namespace Slang;

/* static */SlangResult FileUtil::readAllText(const Slang::String& fileName, DiagnosticSink* sink, String& outRead)
{
    RefPtr<FileStream> stream = new FileStream;
    SlangResult res = stream->init(fileName, FileMode::Open, FileAccess::Read, FileShare::ReadWrite);

    if (SLANG_FAILED(res))
    {
        if (sink)
        {
            sink->diagnose(SourceLoc(), CPPDiagnostics::cannotOpenFile, fileName);
        }
        return res;
    }

    try
    {
        StreamReader reader(stream);
        outRead = reader.ReadToEnd();
    }
    catch (IOException&)
    {
        if (sink)
        {
            sink->diagnose(SourceLoc(), CPPDiagnostics::cannotOpenFile, fileName);
        }
    }
    return SLANG_OK;
}

/* static */SlangResult FileUtil::writeAllText(const Slang::String& fileName, DiagnosticSink* sink, const UnownedStringSlice& text)
{
    try
    {
        // TODO(JS):
        // There is an optimization/behavior here,that checks if the contents has changed. It only writes if it hasn't
        // That might not be what you want (both because of extra work of read, the file modified stamp or other reasons, file is write only etc)
        // NOTE! That this also does the work of the comparison after it is decoded, but the *bytes* might actually be different.

        if (File::exists(fileName))
        {
            String existingText;
            if (readAllText(fileName, nullptr, existingText) == SLANG_OK)
            {
                if (existingText == text)
                    return SLANG_OK;
            }
        }

        RefPtr<FileStream> stream = new FileStream;
        SlangResult res = stream->init(fileName, FileMode::Create);

        if (SLANG_FAILED(res))
        {
            if (sink)
            {
                sink->diagnose(SourceLoc(), CPPDiagnostics::cannotOpenFile, fileName);
            }
        }

        StreamWriter writer(stream);
        writer.Write(text);
    }
    catch (const IOException&)
    {
        if (sink)
        {
            sink->diagnose(SourceLoc(), CPPDiagnostics::cannotOpenFile, fileName);
        }
        return SLANG_FAIL;
    }

    return SLANG_OK;
}

/* static */ void FileUtil::indent(Index indentCount, StringBuilder& out)
{
    for (Index i = 0; i < indentCount; ++i)
    {
        out << CPP_EXTRACT_INDENT_STRING;
    }
}

} // namespace CppExtract
