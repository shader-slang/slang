// slang-diagnostic-sink.cpp
#include "slang-diagnostic-sink.h"

#include "compiler-core/slang-rich-diagnostics-render.h"
#include "core/slang-char-util.h"
#include "core/slang-dictionary.h"
#include "core/slang-memory-arena.h"
#include "core/slang-string-util.h"
#include "slang-core-diagnostics.h"
#include "slang-name-convention-util.h"
#include "slang-name.h"

namespace Slang
{

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

SourceLoc getDiagnosticPos(Token const& token)
{
    return token.loc;
}

// Take the format string for a diagnostic message, along with its arguments, and turn it into a
static void formatDiagnosticMessage(
    StringBuilder& sb,
    char const* format,
    std::size_t argCount,
    DiagnosticArg const* args)
{
    char const* spanBegin = format;
    for (;;)
    {
        char const* spanEnd = spanBegin;
        while (int c = *spanEnd)
        {
            if (c == '$')
                break;
            spanEnd++;
        }

        sb.append(spanBegin, int(spanEnd - spanBegin));
        if (!*spanEnd)
            return;

        SLANG_ASSERT(*spanEnd == '$');
        spanEnd++;
        int d = *spanEnd++;
        switch (d)
        {
        // A double dollar sign `$$` is used to emit a single `$`
        case '$':
            sb.append('$');
            break;

        // A single digit means to emit the corresponding argument.
        // TODO: support more than 10 arguments, and add options
        // to control formatting, etc.
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            {
                int index = d - '0';
                if (index >= Index(argCount))
                {
                    // TODO(tfoley): figure out what a good policy will be for "panic" situations
                    // like this
                    SLANG_INVALID_OPERATION("too few arguments for diagnostic message");
                }
                else
                {
                    DiagnosticArg const& arg = args[index];
                    arg.printFunc(sb, arg.data);
                }
            }
            break;

        default:
            SLANG_INVALID_OPERATION("invalid diagnostic message format");
            break;
        }

        spanBegin = spanEnd;
    }
}

static void formatDiagnostic(
    const HumaneSourceLoc& humaneLoc,
    Diagnostic const& diagnostic,
    DiagnosticSink::Flags flags,
    StringBuilder& outBuilder)
{
    if (flags & DiagnosticSink::Flag::HumaneLoc)
    {
        outBuilder << humaneLoc.pathInfo.foundPath;
        outBuilder << "(";
        outBuilder << Int32(humaneLoc.line);
        if (flags & DiagnosticSink::Flag::LanguageServer)
        {
            outBuilder << ", " << humaneLoc.column;
        }
        outBuilder << "): ";
    }

    outBuilder << getSeverityName(diagnostic.severity);

    if ((flags & DiagnosticSink::Flag::LanguageServer) || diagnostic.ErrorID >= 0)
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
    const char* const end = slice.end();

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
static UnownedStringSlice _extractLineContainingPosition(
    const UnownedStringSlice& text,
    const char* pos)
{
    SLANG_ASSERT(text.isMemoryContained(pos));

    const char* const contentStart = text.begin();
    const char* const contentEnd = text.end();

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

static void _reduceLength(Index startIndex, const UnownedStringSlice& prefix, StringBuilder& ioBuf)
{
    StringBuilder buf;
    buf << prefix;
    buf.append(ioBuf.getUnownedSlice().tail(startIndex));
    ioBuf = buf;
}

static void _sourceLocationNoteDiagnostic(
    DiagnosticSink* sink,
    SourceView* sourceView,
    SourceLoc sourceLoc,
    StringBuilder& sb)
{
    SourceFile* sourceFile = sourceView->getSourceFile();
    if (!sourceFile)
    {
        return;
    }

    // Check if the source file has actual content available.
    // This is important because it's possible to have a 'SourceFile' that doesn't contain any
    // content (for example when reconstructed via serialization with just line offsets, the actual
    // source text 'content' isn't available).
    if (!sourceFile->hasContent())
    {
        return;
    }

    UnownedStringSlice content = sourceFile->getContent();

    // Make sure the offset is within content.
    const int offset = sourceView->getRange().getOffset(sourceLoc);
    if (offset < 0 || offset >= content.getLength())
    {
        return;
    }

    // Work out the position of the SourceLoc in the source
    const char* const pos = content.begin() + offset;

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
        caretLine.clear();
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
            const UnownedStringSlice ellipsis = UnownedStringSlice::fromLiteral("...");
            const UnownedStringSlice spaces = UnownedStringSlice::fromLiteral("   ");
            SLANG_ASSERT(ellipsis.getLength() == spaces.getLength());

            // We use the caretLine length if we have a lexer, because it will have underscores such
            // that it's end is the end of the item at issue. If we don't have the lexer, we
            // guesstimate using 1/4 of the maximum length
            const Index endIndex = lexer ? caretLine.getLength() : (caretIndex + (maxLength / 4));

            if (endIndex > maxLength)
            {
                const Index startIndex = endIndex - (maxLength - ellipsis.getLength());

                _reduceLength(startIndex, ellipsis, sourceLine);
                _reduceLength(startIndex, spaces, caretLine);
            }

            if (sourceLine.getLength() > maxLength)
            {
                StringBuilder buf;
                buf.append(sourceLine.getUnownedSlice().head(maxLength - ellipsis.getLength()));
                buf << ellipsis;
                sourceLine = buf;
            }
        }
    }

    // We could have handling here for if the line is too long, that we surround the important
    // section will ellipsis for example. For now we just output.

    sb << sourceLine << "\n";
    sb << caretLine << "\n";
}

// Output the length of the token at `sourceLoc`. This is used by language server.
static void _tokenLengthNoteDiagnostic(
    DiagnosticSink* sink,
    SourceView* sourceView,
    SourceLoc sourceLoc,
    StringBuilder& sb)
{
    SourceFile* sourceFile = sourceView->getSourceFile();
    if (!sourceFile)
    {
        return;
    }

    // Check if the source file has actual content available.
    // This is important because it's possible to have a 'SourceFile' that doesn't contain any
    // content (for example when reconstructed via serialization with just line offsets, the actual
    // source text 'content' isn't available).
    if (!sourceFile->hasContent())
    {
        return;
    }

    UnownedStringSlice content = sourceFile->getContent();

    // Make sure the offset is within content.
    const int offset = sourceView->getRange().getOffset(sourceLoc);
    if (offset < 0 || offset >= content.getLength())
    {
        return;
    }

    // Work out the position of the SourceLoc in the source
    const char* const pos = content.begin() + offset;

    UnownedStringSlice line = _extractLineContainingPosition(content, pos);

    // Trim any trailing white space
    line = UnownedStringSlice(line.begin(), line.trim().end());

    auto lexer = sink->getSourceLocationLexer();
    if (lexer)
    {
        UnownedStringSlice token = lexer(UnownedStringSlice(pos, line.end()));

        if (token.getLength() > 1)
        {
            sb << "^+" << token.getLength() << "\n";
        }
    }
}

// Use the shared depth limit from SourceManager (see slang-source-loc.h). The cap makes loop
// termination unconditional under adversarial or malformed input; non-pathological programs won't
// reach it. Using the same constant as kMaxMacroExpansionUnmapDepth ensures the chain walk never
// requests more expansion-unmapping steps than findSourceViewThroughExpansion can provide.
static constexpr int kMaxMacroExpansionDiagnosticDepth = SourceManager::kMaxMacroExpansionDepth;

/// Walk the mixed macro-expansion/token-paste provenance chain rooted at `primaryLoc` and append a
/// note for each level. This produces the "expanded from macro 'X'" and "see token-paste location"
/// notes shown below the primary diagnostic message.
///
/// The chain has two kinds of steps:
///  - Macro-expansion step: currentLoc falls in a per-invocation expansion range registered with
///    SourceManager::registerMacroExpansion. The entry gives us the call-site loc for the note
///    and the macro name for the message.
///  - Token-paste step: currentLoc belongs to a TokenPaste SourceView (created when ## pastes two
///    tokens). The view's initiating loc points to where the ## operator appeared.
///
/// At each step the un-remapped call-site/initiating loc is used as the note span so the renderer
/// has a real source location. The walk continues from the original (pre-unmap) loc so that the
/// next iteration can find the next entry in the chain.
///
/// Preconditions: sm must not be null; primaryLoc must be valid.
/// Call sites must guard with `if (sm && primaryLoc.isValid())` before calling.
static void appendMacroExpansionNotes(
    SourceManager* sm,
    SourceLoc primaryLoc,
    List<DiagnosticNote>& notes)
{
    SLANG_ASSERT(sm);
    SLANG_ASSERT(primaryLoc.isValid());

    SourceLoc currentLoc = primaryLoc;
    for (int depth = 0; depth < kMaxMacroExpansionDiagnosticDepth; ++depth)
    {
        if (const auto* entry = sm->findMacroExpansion(currentLoc))
        {
            // Macro-expansion step: emit an "expanded from macro 'X'" note at the call site.
            // The call-site loc may itself be in an expansion range, so unmap it to get a real
            // source location that the renderer can display.
            const String& macroName = entry->macroName;
            DiagnosticArg arg(macroName);
            StringBuilder msg;
            formatDiagnosticMessage(
                msg,
                MiscDiagnostics::seeExpandedFromMacro.messageFormat,
                1,
                &arg);
            DiagnosticNote note;
            note.message = msg.produceString();
            note.diagnosticInfo = &MiscDiagnostics::seeExpandedFromMacro;
            SourceLoc spanLoc = entry->callSiteLoc;
            sm->findSourceViewThroughExpansion(spanLoc); // unmap in-place; ignore returned view
            note.span.range = SourceRange{spanLoc, spanLoc + (Int)macroName.getLength()};
            notes.add(std::move(note));
            // Walk from the original (pre-unmap) call-site loc so the next iteration sees it
            // in the context of the outer expansion chain.
            currentLoc = entry->callSiteLoc;
        }
        else
        {
            // Token-paste step: check if currentLoc is in a TokenPaste SourceView. If so, emit a
            // "see token-paste location" note pointing to the ## operator's source location.
            SourceView* currentView = sm->findSourceViewRecursively(currentLoc);
            if (!currentView || !currentView->getInitiatingSourceLoc().isValid())
                break; // no initiating loc — end of chain
            if (currentView->getSourceFile()->getPathInfo().type != PathInfo::Type::TokenPaste)
                break; // not a token-paste view — end of chain

            SourceLoc initiatingLoc = currentView->getInitiatingSourceLoc();
            // The initiating loc may be in an expansion range; unmap it so the note span is valid.
            if (!sm->findSourceViewThroughExpansion(initiatingLoc))
                break; // initiating loc unmaps to nowhere — stop rather than show a bad span

            StringBuilder msg;
            formatDiagnosticMessage(
                msg,
                MiscDiagnostics::seeTokenPasteLocation.messageFormat,
                0,
                nullptr);
            DiagnosticNote note;
            note.message = msg.produceString();
            note.diagnosticInfo = &MiscDiagnostics::seeTokenPasteLocation;
            note.span.range = SourceRange{initiatingLoc};
            notes.add(std::move(note));
            // Walk from the pre-unmap initiating loc so the next iteration can find an expansion
            // side-table entry if the token-paste itself was inside a macro invocation.
            currentLoc = currentView->getInitiatingSourceLoc();
        }
    }
}

/// Format `diagnostic` — including its "expanded from macro" / "see token-paste" expansion notes
/// — to a human-readable string in `sb`.
///
/// This is the text-rendering entry point for the legacy Diagnostic path (as opposed to the rich
/// GenericDiagnostic path used by diagnoseRichImpl). It shares the expansion-note logic with the
/// rich path via appendMacroExpansionNotes so that both renderers always show the same chain.
static void formatDiagnosticWithExpansionChain(
    DiagnosticSink* sink,
    Diagnostic const& diagnostic,
    StringBuilder& sb)
{
    auto sourceManager = sink->getSourceManager();

    // Resolve the primary diagnostic location through any macro expansion remapping.
    // findSourceViewThroughExpansion updates sourceLoc in-place to the definition-file loc;
    // sourceLoc and sourceView must be used together thereafter (both refer to the unmapped loc).
    SourceView* sourceView = nullptr;
    HumaneSourceLoc humaneLoc;
    auto sourceLoc = diagnostic.loc;
    if (sourceManager)
    {
        sourceView = sourceManager->findSourceViewThroughExpansion(sourceLoc);
        if (sourceView)
            humaneLoc = sourceView->getHumaneLoc(sourceLoc);
    }

    // Emit the primary diagnostic line.
    formatDiagnostic(humaneLoc, diagnostic, sink->getFlags(), sb);

    // Emit "expanded from macro 'X'" / "see token-paste location" notes for each level in the
    // provenance chain. appendMacroExpansionNotes is also called by the rich (machine-readable)
    // path — both go through it so the two renderers always show the same chain.
    if (sourceManager && diagnostic.loc.isValid())
    {
        List<DiagnosticNote> notes;
        appendMacroExpansionNotes(sourceManager, diagnostic.loc, notes);
        for (const auto& note : notes)
        {
            // Resolve the note's span location to a humaneLoc for text rendering.
            // The span loc was already unmapped by appendMacroExpansionNotes.
            HumaneSourceLoc noteHumaneLoc = sourceManager->getHumaneLoc(note.span.range.begin);

            Diagnostic noteDiag;
            // Use the originating diagnostic's id from DiagnosticNote so "expanded from macro"
            // and "see token-paste location" notes are stamped with their own ids. Both are
            // currently -1 (anonymous notes), but carrying the id explicitly makes the text
            // path future-proof if they are ever assigned distinct codes.
            noteDiag.ErrorID = note.diagnosticInfo ? note.diagnosticInfo->id
                                                   : MiscDiagnostics::seeExpandedFromMacro.id;
            noteDiag.Message = note.message;
            noteDiag.loc = note.span.range.begin;
            noteDiag.severity = Severity::Note;
            formatDiagnostic(noteHumaneLoc, noteDiag, sink->getFlags(), sb);
        }
    }

    // Language-server extras: token length hint and source-line annotation.
    if (sourceView && sink->isFlagSet(DiagnosticSink::Flag::LanguageServer))
        _tokenLengthNoteDiagnostic(sink, sourceView, sourceLoc, sb);

    if (sourceView && sink->isFlagSet(DiagnosticSink::Flag::SourceLocationLine) &&
        diagnostic.loc.isValid())
        _sourceLocationNoteDiagnostic(sink, sourceView, sourceLoc, sb);

    if (sourceView && sink->isFlagSet(DiagnosticSink::Flag::VerbosePath))
    {
        auto actualHumaneLoc = sourceView->getHumaneLoc(sourceLoc, SourceLocType::Actual);

        // Look up the path verbosely (will get the canonical path if necessary)
        actualHumaneLoc.pathInfo.foundPath = sourceView->getSourceFile()->calcVerbosePath();

        // Only output if it's actually different
        if (actualHumaneLoc.pathInfo.foundPath != humaneLoc.pathInfo.foundPath ||
            actualHumaneLoc.line != humaneLoc.line || actualHumaneLoc.column != humaneLoc.column)
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

void DiagnosticSink::reset()
{
    m_errorCount = 0;
    m_internalErrorLocsNoted = 0;

    outputBuffer.clear();
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
    if (!outBlob)
        return SLANG_OK;

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

bool DiagnosticSink::diagnoseImpl(
    DiagnosticInfo const& info,
    const UnownedStringSlice& formattedMessage)
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
        std::string message(formattedMessage.begin(), formattedMessage.end());
        SLANG_ABORT_COMPILATION(message.c_str());
    }
    return true;
}

bool DiagnosticSink::diagnoseRichImpl(
    const GenericDiagnostic& diagnostic,
    const DiagnosticInfo* info)
{
    return diagnoseRichImpl(diagnostic, info, getSourceManager());
}

bool DiagnosticSink::diagnoseRichImpl(
    const GenericDiagnostic& diagnostic,
    const DiagnosticInfo* info,
    SourceManager* sourceManager)
{
    // Check for severity overrides (e.g., from -Wno-xxx flags)
    Severity effectiveSeverity = diagnostic.severity;
    if (info)
    {
        effectiveSeverity = getEffectiveMessageSeverity(*info, diagnostic.primarySpan.range.begin);
    }

    // If the diagnostic has been disabled, don't emit it
    if (effectiveSeverity == Severity::Disable)
        return false;

    // Create a copy with the effective severity for rendering
    GenericDiagnostic effectiveDiagnostic = diagnostic;
    effectiveDiagnostic.severity = effectiveSeverity;

    // Append "expanded from macro 'X'" / "see token-paste location" notes for any provenance chain
    // rooted at the primary location. The guard is required because sourceManager may be null when
    // the sink was created without one (e.g. for command-line error messages), and because a
    // diagnostic may genuinely have no source location (e.g. internal compiler errors).
    if (sourceManager && effectiveDiagnostic.primarySpan.range.begin.isValid())
    {
        appendMacroExpansionNotes(
            sourceManager,
            effectiveDiagnostic.primarySpan.range.begin,
            effectiveDiagnostic.notes);
    }

    if (effectiveSeverity >= Severity::Error)
    {
        m_errorCount++;
    }

    String message;
    if (isFlagSet(Flag::MachineReadableDiagnostics))
    {
        message = renderDiagnosticMachineReadable(
            getSourceLocationLexer(),
            sourceManager,
            effectiveDiagnostic);
    }
    else
    {
        message = renderDiagnostic(
            getSourceLocationLexer(),
            sourceManager,
            {.enableTerminalColors = shouldEnableTerminalColors(),
             .enableUnicode = shouldEnableUnicode()},
            effectiveDiagnostic);
    }

    if (writer)
    {
        writer->write(message.begin(), message.getLength());
    }
    else
    {
        outputBuffer.append(message);
    }

    // Route to parent sink so it can render with its own settings (its own severity overrides,
    // writer, and color mode). The source manager is passed explicitly because the parent may own
    // a different SourceManager that cannot resolve locs from this compilation (e.g. command-line
    // source locations live in a separate SourceManager).
    //
    // We pass the original (un-decorated) `diagnostic` rather than `effectiveDiagnostic` for two
    // reasons:
    //  1. The parent should apply its own severity mapping (its own -warnings-as-errors state may
    //     differ from the child's), so it must receive the original severity, not the child's
    //     effective severity.
    //  2. effectiveDiagnostic.notes already contains expansion notes appended by this sink; if we
    //     passed it, the parent's diagnoseRichImpl would call appendMacroExpansionNotes a second
    //     time on top of the already-appended notes, producing duplicate "expanded from macro"
    //     entries. Passing the original diagnostic lets the parent derive its own notes
    //     independently.
    //
    // The reason 2 is safe (i.e. the parent can re-derive the same notes without loss):
    // appendMacroExpansionNotes is a pure function of (primarySpan.range.begin, sourceManager).
    // Given the same primary loc and the same SourceManager, it always walks the same expansion
    // chain and produces the same sequence of "expanded from macro" and "see token-paste location"
    // notes. So the parent's independent call produces exactly one copy of the note set — no fewer
    // (the chain is the same) and no more (only one call happens). This determinism invariant means
    // we can safely let the parent re-derive the notes rather than forwarding them pre-built.
    if (m_parentSink)
    {
        m_parentSink->diagnoseRichImpl(diagnostic, info, sourceManager);
    }

    if (effectiveSeverity >= Severity::Fatal)
    {
        std::string msg(message.begin(), message.end());
        SLANG_ABORT_COMPILATION(msg.c_str());
    }
    return true;
}

// Fallback to diagnose from the old diagnostic messages
bool DiagnosticSink::diagnoseRichImpl(
    SourceLoc const& loc,
    DiagnosticInfo const& info,
    std::size_t argCount,
    DiagnosticArg const* args)
{
    StringBuilder sb;
    formatDiagnosticMessage(sb, info.messageFormat, argCount, args);

    GenericDiagnostic diagnostic;
    diagnostic.code = info.id;
    diagnostic.severity = info.severity;
    diagnostic.message = sb.produceString();

    diagnostic.primarySpan.range = SourceRange{loc};
    diagnostic.primarySpan.message = "";

    return diagnoseRichImpl(diagnostic, &info);
}

Severity DiagnosticSink::getEffectiveMessageSeverity(
    DiagnosticInfo const& info,
    SourceLoc const& location)
{
    Severity effectiveSeverity = info.severity;

    if (effectiveSeverity <= Severity::Warning && m_sourceWarningStateTracker)
    {
        effectiveSeverity = m_sourceWarningStateTracker->consumeWarningSeverity(
            location,
            info.id,
            effectiveSeverity);
    }

    Severity* pSeverityOverride = m_severityOverrides.tryGetValue(info.id);

    // See if there is an override
    if (pSeverityOverride)
    {
        // Override the current severity, but don't allow lowering it if it's Error or Fatal
        if (effectiveSeverity < Severity::Error || *pSeverityOverride >= effectiveSeverity)
            effectiveSeverity = *pSeverityOverride;
    }
    else if (effectiveSeverity == Severity::Warning && !isWarningLevelEnabled(info.level))
    {
        // The warning belongs to an opt-in group (-Wall/-Wextra/-Wpedantic) that has not been
        // enabled, so it is suppressed. An explicit per-id override (-W<name>/-Wno-<name>) takes
        // precedence over this group gating, which is why it lives in the `else` branch.
        effectiveSeverity = Severity::Disable;
    }

    if (isFlagSet(Flag::TreatWarningsAsErrors) && effectiveSeverity == Severity::Warning)
        effectiveSeverity = Severity::Error;

    return effectiveSeverity;
}

bool DiagnosticSink::diagnoseImpl(
    SourceLoc const& pos,
    DiagnosticInfo info,
    std::size_t argCount,
    DiagnosticArg const* args)
{
    // Override the severity in the 'info' structure to pass it further into formatDiagnostics
    info.severity = getEffectiveMessageSeverity(info, pos);

    if (info.severity == Severity::Disable)
        return false;

    StringBuilder messageBuilder;
    {
        StringBuilder sb;
        formatDiagnosticMessage(sb, info.messageFormat, argCount, args);

        Diagnostic diagnostic;
        diagnostic.ErrorID = info.id;
        diagnostic.Message = sb.produceString();
        diagnostic.loc = pos;
        diagnostic.severity = info.severity;

        // If so, pass the error string along to them
        formatDiagnosticWithExpansionChain(this, diagnostic, messageBuilder);
    }

    return diagnoseImpl(info, messageBuilder.getUnownedSlice());
}

void DiagnosticSink::diagnoseRaw(Severity severity, char const* message)
{
    return diagnoseRaw(severity, UnownedStringSlice(message));
}

void DiagnosticSink::diagnoseRaw(Severity severity, const UnownedStringSlice& message)
{
    if (severity >= Severity::Error)
    {
        m_errorCount++;
    }

    // Did the client supply a callback for us to use?
    if (writer)
    {
        // If so, pass the error string along to them.
        writer->write(message.begin(), message.getLength());
    }
    else
    {
        // If the user doesn't have a callback, then just
        // collect our diagnostic messages into a buffer.
        outputBuffer.append(message);
    }

    if (m_parentSink)
    {
        m_parentSink->diagnoseRaw(severity, message);
    }

    if (severity >= Severity::Fatal)
    {
        // TODO: figure out a better policy for aborting compilation
        SLANG_ABORT_COMPILATION("");
    }
}

void DiagnosticSink::overrideDiagnosticSeverity(
    int diagnosticId,
    Severity overrideSeverity,
    const DiagnosticInfo* info)
{
    if (info)
    {
        SLANG_ASSERT(info->id == diagnosticId);

        // If the override is the same as the default, we can just remove the override -- but only
        // for the always-on Default group. For a warning in an opt-in group (-Wall/-Wextra/
        // -Wpedantic), an explicit override back to its nominal Warning severity is meaningful: it
        // force-enables the warning even though its group is not enabled, so the override must be
        // kept (an absent override would let group gating suppress it).
        if (info->severity == overrideSeverity && info->level == WarningLevel::Default)
        {
            m_severityOverrides.remove(diagnosticId);
            return;
        }
    }

    // Set the override
    m_severityOverrides[diagnosticId] = overrideSeverity;
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! DiagnosticLookup
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

Index DiagnosticsLookup::_findDiagnosticIndexByExactName(const UnownedStringSlice& slice) const
{
    const Index* indexPtr = m_nameMap.tryGetValue(slice);
    return indexPtr ? *indexPtr : -1;
}

void DiagnosticsLookup::_addName(const char* name, Index diagnosticIndex)
{
    UnownedStringSlice nameSlice(name);
    m_nameMap.add(nameSlice, diagnosticIndex);
}

void DiagnosticsLookup::addAlias(const char* name, const char* diagnosticName)
{
    const Index index = _findDiagnosticIndexByExactName(UnownedStringSlice(diagnosticName));
    SLANG_ASSERT(index >= 0);
    if (index >= 0)
    {
        _addName(name, index);
    }
}

const DiagnosticInfo* DiagnosticsLookup::getDiagnosticById(Int id) const
{
    const auto indexPtr = m_idMap.tryGetValue(id);
    return indexPtr ? m_diagnostics[*indexPtr] : nullptr;
}

const DiagnosticInfo* DiagnosticsLookup::findDiagnosticByExactName(
    const UnownedStringSlice& slice) const
{
    const Index* indexPtr = m_nameMap.tryGetValue(slice);
    return indexPtr ? m_diagnostics[*indexPtr] : nullptr;
}

const DiagnosticInfo* DiagnosticsLookup::findDiagnosticByName(const UnownedStringSlice& slice) const
{
    const auto convention = NameConventionUtil::inferConventionFromText(slice);
    switch (convention)
    {
    case NameConvention::Invalid:
        return nullptr;
    case NameConvention::LowerCamel:
        return findDiagnosticByExactName(slice);
    default:
        break;
    }

    StringBuilder buf;
    NameConventionUtil::convert(getNameStyle(convention), slice, NameConvention::LowerCamel, buf);

    return findDiagnosticByExactName(buf.getUnownedSlice());
}

Index DiagnosticsLookup::add(const DiagnosticInfo* info)
{
    // Check it's not already added
    SLANG_ASSERT(m_diagnostics.indexOf(info) < 0);

    const Index diagnosticIndex = m_diagnostics.getCount();
    m_diagnostics.add(info);

    _addName(info->name, diagnosticIndex);
    m_idMap.addIfNotExists(info->id, diagnosticIndex);

    return diagnosticIndex;
}

void DiagnosticsLookup::add(const DiagnosticInfo* const* infos, Index infosCount)
{
    for (Index i = 0; i < infosCount; ++i)
    {
        add(infos[i]);
    }
}

DiagnosticsLookup::DiagnosticsLookup()
    : m_arena(kArenaInitialSize)
{
}

DiagnosticsLookup::DiagnosticsLookup(
    const DiagnosticInfo* const* diagnostics,
    Index diagnosticsCount)
    : m_arena(kArenaInitialSize)
{
    // TODO: We should eventually have a more formal system for associating individual
    // diagnostics, or groups of diagnostics, with user-exposed names for use when
    // enabling/disabling warnings (or turning warnings into errors, etc.).
    //
    // For now we build a map from diagnostic name to it's entry.

    add(diagnostics, diagnosticsCount);
}

void outputExceptionDiagnostic(
    const AbortCompilationException& exception,
    DiagnosticSink& sink,
    slang::IBlob** outDiagnostics)
{
    sink.diagnoseRaw(Severity::Error, exception.Message.getUnownedSlice());
    sink.getBlobIfNeeded(outDiagnostics);
}

void outputExceptionDiagnostic(
    const Exception& exception,
    DiagnosticSink& sink,
    slang::IBlob** outDiagnostics)
{
    try
    {
        sink.diagnoseRaw(Severity::Internal, exception.Message.getUnownedSlice());
    }
    catch (const AbortCompilationException&)
    {
        // Catch and ignore the AbortCompilationException that diagnoseRaw throws
        // for Internal severity to prevent exception leak from loadModule
    }
    sink.getBlobIfNeeded(outDiagnostics);
}

void outputExceptionDiagnostic(DiagnosticSink& sink, slang::IBlob** outDiagnostics)
{
    try
    {
        sink.diagnoseRaw(Severity::Fatal, "An unknown exception occurred");
    }
    catch (const AbortCompilationException&)
    {
        // Catch and ignore the AbortCompilationException that diagnoseRaw throws
        // for Fatal severity to prevent exception leak from loadModule
    }
    sink.getBlobIfNeeded(outDiagnostics);
}

} // namespace Slang
