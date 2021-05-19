#include "slang-command-line-args.h"

#include "../core/slang-process-util.h"
#include "../core/slang-string-escape-util.h"

#include "slang-core-diagnostics.h"

namespace Slang {

void CommandLineArgs::setArgs(const char*const* args, size_t argCount)
{
    m_args.clear();

    const SourceLoc startLoc = m_sourceManager->getNextRangeStart();

    StringBuilder buf;
   
    auto escapeHandler = ProcessUtil::getEscapeHandler();

    for (size_t i = 0; i < argCount; ++i)
    {
        const Index offset = buf.getLength();

        const char* srcArg = args[i];

        Arg dstArg;
        dstArg.loc = startLoc + offset;
        dstArg.value = srcArg;

        m_args.add(dstArg);

        // Write the string escaped if necessary
        StringEscapeUtil::appendMaybeQuoted(escapeHandler, dstArg.value.getUnownedSlice(), buf);

        // Put a space between the args
        buf << " ";
    }

    SourceFile* sourceFile = m_sourceManager->createSourceFileWithString(PathInfo::makeUnknown(), buf.ProduceString());
    m_sourceView = m_sourceManager->createSourceView(sourceFile, nullptr, SourceLoc::fromRaw(0));

    SLANG_ASSERT(m_sourceView->getRange().begin == startLoc);
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

                         CommandLineReader

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

String CommandLineReader::getPreviousValue() const
{
    SLANG_ASSERT(m_index > 0);
    if (m_index > 0)
    {
        const auto& prevArg = (*m_args)[m_index - 1];
        return prevArg.value;
    }
    else
    {
        return String();
    }
}

SlangResult CommandLineReader::expectArg(String& outArg)
{
    if (hasArg())
    {
        outArg = m_args->m_args[m_index++].value;
        return SLANG_OK;
    }
    else
    {
        m_sink->diagnose(peekLoc(), MiscDiagnostics::expectedArgumentForOption, getPreviousValue());
        return SLANG_FAIL;
    }
}

SlangResult CommandLineReader::expectArg(CommandLineArg& outArg)
{
    if (hasArg())
    {
        outArg = peekArg();
        advance();
        return SLANG_OK;
    }
    else
    {
        m_sink->diagnose(peekLoc(), MiscDiagnostics::expectedArgumentForOption, getPreviousValue());
        return SLANG_FAIL;
    }
}


} // namespace Slang
