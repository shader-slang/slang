// slang-diagnostic-sink.cpp
#include "slang-diagnostic-sink.h"

#include "slang-name.h"
#include "slang-core-diagnostics.h"
#include "slang-name-convention-util.h"

#include "../core/slang-memory-arena.h"
#include "../core/slang-dictionary.h"
#include "../core/slang-string-util.h"
#include "../core/slang-char-util.h"

namespace Slang {

void printDiagnosticArg(StringBuilder& sb, char const* str)
{
    sb << str;
}

void printDiagnosticArg(StringBuilder& sb, int32_t val)
{
    sb << val;
}

void printDiagnosticArg(StringBuilder& sb, uint32_t val)
{
    sb << val;
}

void printDiagnosticArg(StringBuilder& sb, int64_t val)
{
    sb << val;
}

void printDiagnosticArg(StringBuilder& sb, uint64_t val)
{
    sb << val;
}

void printDiagnosticArg(StringBuilder& sb, double val)
{
    sb << val;
}

void printDiagnosticArg(StringBuilder& sb, Slang::String const& str)
{
    sb << str;
}

void printDiagnosticArg(StringBuilder& sb, Slang::UnownedStringSlice const& str)
{
    sb.append(str);
}


void printDiagnosticArg(StringBuilder& sb, Name* name)
{
    sb << getText(name);
}


void printDiagnosticArg(StringBuilder& sb, TokenType tokenType)
{
    sb << TokenTypeToString(tokenType);
}

void printDiagnosticArg(StringBuilder& sb, Token const& token)
{
    sb << token.getContent();
}

SourceLoc const& getDiagnosticPos(Token const& token)
{
    return token.loc;
}

// Take the format string for a diagnostic message, along with its arguments, and turn it into a
static void formatDiagnosticMessage(StringBuilder& sb, char const* format, int argCount, DiagnosticArg const* const* args)
{
    char const* spanBegin = format;
    for(;;)
    {
        char const* spanEnd = spanBegin;
        while (int c = *spanEnd)
        {
            if (c == '$')
                break;
            spanEnd++;
        }

        sb.Append(spanBegin, int(spanEnd - spanBegin));
        if (!*spanEnd)
            return;

        SLANG_ASSERT(*spanEnd == '$');
        spanEnd++;
        int d = *spanEnd++;
        switch (d)
        {
        // A double dollar sign `$$` is used to emit a single `$` 
        case '$':
            sb.Append('$');
            break;

        // A single digit means to emit the corresponding argument.
        // TODO: support more than 10 arguments, and add options
        // to control formatting, etc.
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
            {
                int index = d - '0';
                if (index >= argCount)
                {
                    // TODO(tfoley): figure out what a good policy will be for "panic" situations like this
                    throw InvalidOperationException("too few arguments for diagnostic message");
                }
                else
                {
                    DiagnosticArg const* arg = args[index];
                    arg->printFunc(sb, arg->data);
                }
            }
            break;

        default:
            throw InvalidOperationException("invalid diagnostic message format");
            break;
        }

        spanBegin = spanEnd;
    }
}

static void formatDiagnostic(const HumaneSourceLoc& humaneLoc, Diagnostic const& diagnostic, DiagnosticSink::Flags flags, StringBuilder& outBuilder)
{
    if (flags & DiagnosticSink::Flag::HumaneLoc)
    {
        outBuilder << humaneLoc.pathInfo.foundPath;
        outBuilder << "(";
        outBuilder << Int32(humaneLoc.line);
        outBuilder << "): ";
    }

    outBuilder << getSeverityName(diagnostic.severity);

    if (diagnostic.ErrorID >= 0)
    {
        outBuilder << " ";
        outBuilder << diagnostic.ErrorID;
    }

    outBuilder << ": ";
    outBuilder << diagnostic.Message;
    outBuilder << "\n";
}

static void _replaceTabWithSpaces(const UnownedStringSlice& slice, Int tabSize, StringBuilder& out)
{
    const char* start = slice.begin();
    const char*const end = slice.end();

    const Index startLength = out.getLength();

    for (const char* cur = start; cur < end; cur++)
    {
        if (*cur == '\t')
        {
            if (start < cur)
            {
                out.append(start, cur);
            }

            // The amount of spaces we add depends on the current position.
            const Index lastPosition = out.getLength() - startLength;
            Index tabPosition = lastPosition;

            // Strip the tabPosition so it's back to the tab stop
            // Special case if tabSize is a power of 2
            if ((tabSize & (tabSize - 1)) == 0)
            {
                tabPosition = tabPosition & ~Index(tabSize - 1);
            }
            else
            {
                tabPosition -= tabPosition % tabSize;
            }

            // Move to next tab
            tabPosition += tabSize;

            // The amount of spaces to simulate the tab
            const Index spacesCount = tabPosition - lastPosition;

            // Add the spaces
            out.appendRepeatedChar(' ', spacesCount);

            // Set the start at the first character past
            start = cur + 1;
        }
    }

    if (start < end)
    {
        out.append(start, end);
    }
}

// Given multi-line text, and a position within the text (as a pointer into the memory of text)
// extract the line that contains pos
static UnownedStringSlice _extractLineContainingPosition(const UnownedStringSlice& text, const char* pos)
{
    SLANG_ASSERT(text.isMemoryContained(pos));

    const char*const contentStart = text.begin();
    const char*const contentEnd = text.end();

    // We want to determine the start of the line, and the end of the line
    const char* start = pos;
    for (; start > contentStart; --start)
    {
        const char c = *start;
        if (c == '\n' || c == '\r')
        {
            // We want the character after, but we can only do this if not already at pos
            start += int(start < pos);
            break;
        }
    }
    const char* end = pos;
    for (; end < contentEnd; ++end)
    {
        const char c = *end;
        if (c == '\n' || c == '\r')
        {
            break;
        }
    }

    return UnownedStringSlice(start, end);
}

static void _reduceLength(Index startIndex, StringBuilder& ioBuf)
{
    StringBuilder buf;
    buf << "...";
    buf.append(ioBuf.getUnownedSlice().tail(startIndex));
    ioBuf = buf;
}

static void _sourceLocationNoteDiagnostic(DiagnosticSink* sink, SourceView* sourceView, SourceLoc sourceLoc, StringBuilder& sb)
{
    SourceFile* sourceFile = sourceView->getSourceFile();
    if (!sourceFile)
    {
        return;
    }

    UnownedStringSlice content = sourceFile->getContent();

    // Make sure the offset is within content.
    // This is important because it's possible to have a 'SourceFile' that doesn't contain any content
    // (for example when reconstructed via serialization with just line offsets, the actual source text 'content' isn't available).
    const int offset = sourceView->getRange().getOffset(sourceLoc);
    if (offset < 0 || offset >= content.getLength())
    {
        return;
    }

    // Work out the position of the SourceLoc in the source
    const char*const pos = content.begin() + offset;

    UnownedStringSlice line = _extractLineContainingPosition(content, pos);

    // Trim any trailing white space
    line = UnownedStringSlice(line.begin(), line.trim().end());

    // TODO(JS): The tab size should ideally be configurable from command line.
    // For now just go with 4.
    const Index tabSize = 4;

    StringBuilder sourceLine;
    StringBuilder caretLine;

    // First work out the sourceLine
    _replaceTabWithSpaces(line, tabSize, sourceLine);
    
    // Now the caretLine which appears underneath the sourceLine
    {
        // Produce the text up to the caret position (at pos), taking into account tabs
        _replaceTabWithSpaces(UnownedStringSlice(line.begin(), pos), tabSize, caretLine);

        // Now make all spaces
        const Index length = caretLine.getLength();
        caretLine.Clear();
        caretLine.appendRepeatedChar(' ', length);

        Index caretIndex = caretLine.getLength();

        // Add caret
        caretLine << "^";

        auto lexer = sink->getSourceLocationLexer();
        if (lexer)
        {
            UnownedStringSlice token = lexer(UnownedStringSlice(pos, line.end()));

            if (token.getLength() > 1)
            {
                caretLine.appendRepeatedChar('~', token.getLength() - 1);
            }
        }

        const Index maxLength = sink->getSourceLineMaxLength();
        if (maxLength > 0)
        {
            Index endIndex = lexer ? caretLine.getLength() : (caretIndex + (maxLength / 4));

            if (endIndex > maxLength)
            {
                Index startIndex = endIndex - (maxLength - 3);

                _reduceLength(startIndex, sourceLine);
                _reduceLength(startIndex, caretLine);
            }

            if (sourceLine.getLength() > maxLength)
            {
                StringBuilder buf;
                buf.append(sourceLine.getUnownedSlice().head(maxLength - 3));
                buf << "...";
                sourceLine = buf;
            }
        }
    }

    // We could have handling here for if the line is too long, that we surround the important section
    // will ellipsis for example.
    // For now we just output.

    sb << sourceLine << "\n";
    sb << caretLine << "\n";
}


static void formatDiagnostic(
    DiagnosticSink*     sink,
    Diagnostic const&   diagnostic,
    StringBuilder&      sb)
{
    auto sourceManager = sink->getSourceManager();

    SourceView* sourceView = nullptr;
    HumaneSourceLoc humaneLoc;
    const auto sourceLoc = diagnostic.loc;
    {
        sourceView = sourceManager->findSourceViewRecursively(sourceLoc);
        if (sourceView)
        {
            humaneLoc = sourceView->getHumaneLoc(sourceLoc);
        }
        formatDiagnostic(humaneLoc, diagnostic, sink->getFlags(), sb);

        {
            SourceView* currentView = sourceView;

            while (currentView && currentView->getInitiatingSourceLoc().isValid() && currentView->getSourceFile()->getPathInfo().type == PathInfo::Type::TokenPaste)
            {
                SourceView* initiatingView = sourceManager->findSourceView(currentView->getInitiatingSourceLoc());
                if (initiatingView == nullptr)
                {
                    break;
                }

                const DiagnosticInfo& diagnosticInfo = MiscDiagnostics::seeTokenPasteLocation;

                // Turn the message format into a message. For the moment it assumes no parameters.
                StringBuilder msg;
                formatDiagnosticMessage(msg, diagnosticInfo.messageFormat, 0, nullptr);

                // Set up the diagnostic.
                Diagnostic initiationDiagnostic;
                initiationDiagnostic.ErrorID = diagnosticInfo.id;
                initiationDiagnostic.Message = msg.ProduceString();
                initiationDiagnostic.loc = sourceView->getInitiatingSourceLoc();
                initiationDiagnostic.severity = diagnosticInfo.severity;

                // TODO(JS):
                // Not 100%  clear what the best sourceLoc type is most useful here - we will go with default for now
                HumaneSourceLoc pasteHumaneLoc = initiatingView->getHumaneLoc(sourceView->getInitiatingSourceLoc());

                // Okay we should output where the token paste took place
                formatDiagnostic(pasteHumaneLoc, initiationDiagnostic, sink->getFlags(), sb);

                // Make the initiatingView the current view
                currentView = initiatingView;
            }
        }
    }

    // We don't don't output source line information if this is a 'note' as a note is extra information for one
    // of the other main severity types, and so the information should already be output on the initial line
    if (sourceView && sink->isFlagSet(DiagnosticSink::Flag::SourceLocationLine) && diagnostic.severity != Severity::Note)
    {
        _sourceLocationNoteDiagnostic(sink, sourceView, sourceLoc, sb);
    }

    if (sourceView && sink->isFlagSet(DiagnosticSink::Flag::VerbosePath))
    {
        auto actualHumaneLoc = sourceView->getHumaneLoc(diagnostic.loc, SourceLocType::Actual);

        // Look up the path verbosely (will get the canonical path if necessary)
        actualHumaneLoc.pathInfo.foundPath = sourceView->getSourceFile()->calcVerbosePath();

        // Only output if it's actually different
        if (actualHumaneLoc.pathInfo.foundPath != humaneLoc.pathInfo.foundPath ||
            actualHumaneLoc.line != humaneLoc.line ||
            actualHumaneLoc.column != humaneLoc.column)
        { 
            formatDiagnostic(actualHumaneLoc, diagnostic, sink->getFlags(), sb);
        }
    }
}

void DiagnosticSink::init(SourceManager* sourceManager, SourceLocationLexer sourceLocationLexer)
{
    m_errorCount = 0;
    m_internalErrorLocsNoted = 0;

    m_sourceManager = sourceManager;
    m_sourceLocationLexer = sourceLocationLexer;
    m_sourceLineMaxLength = 0;

    m_flags = Flag::HumaneLoc;

    // If we have a source location lexer, we'll by default enable source location output
    if (sourceLocationLexer)
    {
        setFlag(Flag::SourceLocationLine);
    }
}

void DiagnosticSink::noteInternalErrorLoc(SourceLoc const& loc)
{
    // Don't consider invalid source locations.
    if (!loc.isValid())
        return;

    if (m_parentSink)
    {
        m_parentSink->noteInternalErrorLoc(loc);
    }

    // If this is the first source location being noted,
    // then emit a message to help the user isolate what
    // code might have confused the compiler.
    if (m_internalErrorLocsNoted == 0)
    {
        diagnose(loc, MiscDiagnostics::noteLocationOfInternalError);
    }
    m_internalErrorLocsNoted++;
}

SlangResult DiagnosticSink::getBlobIfNeeded(ISlangBlob** outBlob)
{
    // If the client doesn't want an output blob, there is nothing to do.
    //
    if (!outBlob) return SLANG_OK;

    // For outputBuffer to be valid and hold diagnostics, writer must not be set
    SLANG_ASSERT(writer == nullptr);

    // If there were no errors, and there was no diagnostic output, there is nothing to do.
    if (getErrorCount() == 0 && outputBuffer.getLength() == 0)
    {
        return SLANG_OK;
    }

    Slang::ComPtr<ISlangBlob> blob = Slang::StringUtil::createStringBlob(outputBuffer);
    *outBlob = blob.detach();

    return SLANG_OK;
}

void DiagnosticSink::diagnoseImpl(DiagnosticInfo const& info, const UnownedStringSlice& formattedMessage)
{
    if (info.severity >= Severity::Error)
    {
        m_errorCount++;
    }

    if (writer)
    {
        writer->write(formattedMessage.begin(), formattedMessage.getLength());
    }
    else
    {
        outputBuffer.append(formattedMessage);
    }

    if (m_parentSink)
    {
        m_parentSink->diagnoseImpl(info, formattedMessage);
    }

    if (info.severity >= Severity::Fatal)
    {
        // TODO: figure out a better policy for aborting compilation
        throw AbortCompilationException();
    }
}

void DiagnosticSink::diagnoseImpl(SourceLoc const& pos, DiagnosticInfo const& info, int argCount, DiagnosticArg const* const* args)
{
    StringBuilder messageBuilder;
    {
        StringBuilder sb;
        formatDiagnosticMessage(sb, info.messageFormat, argCount, args);

        Diagnostic diagnostic;
        diagnostic.ErrorID = info.id;
        diagnostic.Message = sb.ProduceString();
        diagnostic.loc = pos;
        diagnostic.severity = info.severity;

        // If so, pass the error string along to them
        formatDiagnostic(this, diagnostic, messageBuilder);
    }

    diagnoseImpl(info, messageBuilder.getUnownedSlice());
}

void DiagnosticSink::diagnoseRaw(
    Severity    severity,
    char const* message)
{
    return diagnoseRaw(severity, UnownedStringSlice(message));
}

void DiagnosticSink::diagnoseRaw(
    Severity    severity,
    const UnownedStringSlice& message)
{
    if (severity >= Severity::Error)
    {
        m_errorCount++;
    }

    // Did the client supply a callback for us to use?
    if(writer)
    {
        // If so, pass the error string along to them
        writer->write(message.begin(), message.getLength());
    }
    else
    {
        // If the user doesn't have a callback, then just
        // collect our diagnostic messages into a buffer
        outputBuffer.append(message);
    }

    if (m_parentSink)
    {
        m_parentSink->diagnoseRaw(severity, message);
    }

    if (severity >= Severity::Fatal)
    {
        // TODO: figure out a better policy for aborting compilation
        throw InvalidOperationException();
    }
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! DiagnosticLookup !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

void DiagnosticsLookup::_add(const char* name, Index index)
{
    UnownedStringSlice nameSlice(name);
    m_map.Add(nameSlice, index);

    // Add a dashed version (KababCase)
    {
        m_work.Clear();

        NameConventionUtil::convert(NameConvention::Camel, nameSlice, CharCase::Lower, NameConvention::Kabab, m_work);

        UnownedStringSlice dashSlice(m_arena.allocateString(m_work.getBuffer(), m_work.getLength()), m_work.getLength());

        m_map.AddIfNotExists(dashSlice, index);
    }
}
void DiagnosticsLookup::addAlias(const char* name, const char* diagnosticName)
{
    const Index index = _findDiagnosticIndex(UnownedStringSlice(diagnosticName));
    SLANG_ASSERT(index >= 0);
    if (index >= 0)
    {
        _add(name, index);
    }
}

Index DiagnosticsLookup::add(const DiagnosticInfo* info)
{
    // Check it's not already added
    SLANG_ASSERT(m_diagnostics.indexOf(info) < 0);

    const Index index = m_diagnostics.getCount();

    _add(info->name, index);

    m_diagnostics.add(info);
    return index;
}

void DiagnosticsLookup::add(const DiagnosticInfo*const* infos, Index infosCount)
{
    for (Index i = 0; i < infosCount; ++i)
    {
        add(infos[i]);
    }
}

DiagnosticsLookup::DiagnosticsLookup():
    m_arena(kArenaInitialSize)
{
}

DiagnosticsLookup::DiagnosticsLookup(const DiagnosticInfo*const* diagnostics, Index diagnosticsCount) :
    m_arena(kArenaInitialSize)
{
    m_diagnostics.addRange(diagnostics, diagnosticsCount);

    // TODO: We should eventually have a more formal system for associating individual
    // diagnostics, or groups of diagnostics, with user-exposed names for use when
    // enabling/disabling warnings (or turning warnings into errors, etc.).
    //
    // For now we build a map from diagnostic name to it's entry. Two entries are typically
    // added - the 'original name' as associated with the diagnostic in lowerCamel, and
    // a dashified version.

    for (Index i = 0; i < diagnosticsCount; ++i)
    {
        const DiagnosticInfo* diagnostic = diagnostics[i];
        _add(diagnostic->name, i);
    }
}

} // namespace Slang
