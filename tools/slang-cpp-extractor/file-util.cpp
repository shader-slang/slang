#include "file-util.h"

#include "../../source/core/slang-io.h"

namespace CppExtract {
using namespace Slang;

/* static */SlangResult FileUtil::readAllText(const Slang::String& fileName, DiagnosticSink* sink, String& outRead)
{
    try
    {
        StreamReader reader(new FileStream(fileName, FileMode::Open, FileAccess::Read, FileShare::ReadWrite));
        outRead = reader.ReadToEnd();
    }
    catch (const IOException&)
    {
        if (sink)
        {
            sink->diagnose(SourceLoc(), CPPDiagnostics::cannotOpenFile, fileName);
        }
        return SLANG_FAIL;
    }
    catch (...)
    {
        if (sink)
        {
            sink->diagnose(SourceLoc(), CPPDiagnostics::cannotOpenFile, fileName);
        }
        return SLANG_FAIL;
    }

    return SLANG_OK;
}

/* static */SlangResult FileUtil::writeAllText(const Slang::String& fileName, DiagnosticSink* sink, const UnownedStringSlice& text)
{
    try
    {
        if (File::exists(fileName))
        {
            String existingText;

            if (readAllText(fileName, nullptr, existingText) == SLANG_OK)
            {
                if (existingText == text)
                    return SLANG_OK;
            }
        }
        StreamWriter writer(new FileStream(fileName, FileMode::Create));
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
