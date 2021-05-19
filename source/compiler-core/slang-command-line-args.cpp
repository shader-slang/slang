#include "slang-command-line-args.h"

#include "../core/slang-process-util.h"
#include "../core/slang-string-escape-util.h"

#include "slang-core-diagnostics.h"

namespace Slang {

void CommandLineArgs::setArgs(const char*const* args, size_t argCount)
{
    m_args.clear();

    SourceManager* sourceManager = m_context->getSourceManager();

    const SourceLoc startLoc = sourceManager->getNextRangeStart();

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

    SourceFile* sourceFile = sourceManager->createSourceFileWithString(PathInfo::makeUnknown(), buf.ProduceString());
    SourceView* sourceView = sourceManager->createSourceView(sourceFile, nullptr, SourceLoc::fromRaw(0));

    SLANG_UNUSED(sourceView);
    SLANG_ASSERT(sourceView->getRange().begin == startLoc);
}

bool CommandLineArgs::hasArgs(const char*const* args, Index count) const
{
    if (m_args.getCount() != count)
    {
        return false;
    }

    for (Index i = 0; i < count; ++i)
    {
        if (m_args[i].value != args[i])
        {
            return false;
        }
    }

    return true;
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

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

                         DownstreamArgs

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

Index DownstreamArgs::addName(const String& name)
{
    Index index = m_names.indexOf(name);
    if (index < 0)
    {
        index = m_names.getCount();
        m_names.add(name);

        CommandLineArgs args(m_context);
        m_args.add(args);
    }
    return index;
}

Index DownstreamArgs::_findOrAddName(SourceLoc loc, const UnownedStringSlice& name, Flags flags, DiagnosticSink* sink)
{
    if (name.getLength() <= 0)
    {
        sink->diagnose(loc, MiscDiagnostics::downstreamToolNameNotDefined);
        return -1;
    }

    if (flags & Flag::AllowNewNames)
    {
        return addName(name);
    }

    Index index = findName(name);
    if (index >= 0)
    {
        return index;
    }

    sink->diagnose(loc, MiscDiagnostics::downstreamNameNotKnown);
    return -1;
}

SlangResult DownstreamArgs::stripDownstreamArgs(CommandLineArgs& ioArgs, Flags flags, DiagnosticSink* sink)
{
    CommandLineReader reader(&ioArgs, sink);

    while (reader.hasArg())
    {
        const CommandLineArg& arg = reader.peekArg();

        if (arg.value.startsWith("-X"))
        {
            if (arg.value.endsWith("..."))
            {
                const UnownedStringSlice name = arg.value.getUnownedSlice().subString(2, arg.value.getLength() - 5);
                const Index nameIndex = _findOrAddName(arg.loc, name, flags, sink);
                if (nameIndex < 0)
                {
                    return SLANG_FAIL;
                }

                Index depth = 1;
                const Index startIndex = reader.getIndex();

                Int index = startIndex + 1;
                const Int count = ioArgs.m_args.getCount();

                for (; index < count; ++index)
                {
                    const auto& curArg = ioArgs.m_args[index];

                    if (curArg.value == "-X.")
                    {
                        depth--;
                        // If we are at end of scope we are done
                        if (depth <= 0)
                        {
                            break;
                        }
                    }
                    else if (curArg.value.startsWith("-X") && curArg.value.endsWith("..."))
                    {
                        depth++;
                    }
                }

                // We don't care if its 1, as we allow the main scope to be left open
                if (depth > 1)
                {
                    sink->diagnose(arg.loc, MiscDiagnostics::unbalancedDownstreamArguments);
                    return SLANG_FAIL;
                }

                // We are either at end of scope or at end of list
                SLANG_ASSERT(depth <= 0 || index >= count);

                // Add all of these args
                CommandLineArgs& args = m_args[nameIndex];

                // Copy the values in the range
                args.m_args.addRange(ioArgs.m_args.getBuffer() + startIndex + 1, index - (startIndex + 1));

                // If we aren't at the end, we must be pointing to -X., so skip that
                index += Index(index < count);
                // Remove the range. The readers position, needs to be fixed though
                ioArgs.m_args.removeRange(startIndex, index - startIndex);

                // The reader should be at startIndex, and so doesn't need fixing
                SLANG_ASSERT(reader.getIndex() == startIndex);
            }
            else if (arg.value == "-X.")
            {
                sink->diagnose(arg.loc, MiscDiagnostics::closeOfUnopenDownstreamArgs);
                return SLANG_FAIL;
            }
            else
            {
                // Extract the name
                UnownedStringSlice name = arg.value.getUnownedSlice().tail(2);
                const Index nameIndex = _findOrAddName(arg.loc, name, flags, sink);
                if (nameIndex < 0)
                {
                    return SLANG_FAIL;
                }

                reader.advance();

                CommandLineArg nextArg;
                SLANG_RETURN_ON_FAIL(reader.expectArg(nextArg));

                ioArgs.m_args.add(nextArg);
            }
        }
    }

    return SLANG_OK;
}

} // namespace Slang
