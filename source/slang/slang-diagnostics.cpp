// slang-diagnostics.cpp
#include "slang-diagnostics.h"

#include "slang-name.h"

#include "../core/slang-memory-arena.h"
#include "../core/slang-dictionary.h"
#include "../core/slang-string-util.h"
#include "../core/slang-name-convention-util.h"

#include <assert.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#undef WIN32_LEAN_AND_MEAN
#undef NOMINMAX
#include <d3dcompiler.h>
#endif

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

static void formatDiagnostic(const HumaneSourceLoc& humaneLoc, Diagnostic const& diagnostic, StringBuilder& outBuilder)
{
    outBuilder << humaneLoc.pathInfo.foundPath;
    outBuilder << "(";
    outBuilder << Int32(humaneLoc.line);
    outBuilder << "): ";

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
        formatDiagnostic(humaneLoc, diagnostic, sb);

        {
            SourceView* currentView = sourceView;

            while (currentView && currentView->getInitiatingSourceLoc().isValid() && currentView->getSourceFile()->getPathInfo().type == PathInfo::Type::TokenPaste)
            {
                SourceView* initiatingView = sourceManager->findSourceView(currentView->getInitiatingSourceLoc());
                if (initiatingView == nullptr)
                {
                    break;
                }

                const DiagnosticInfo& diagnosticInfo = Diagnostics::seeTokenPasteLocation;

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
                formatDiagnostic(pasteHumaneLoc, initiationDiagnostic, sb);

                // Make the initiatingView the current view
                currentView = initiatingView;
            }
        }
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
            formatDiagnostic(actualHumaneLoc, diagnostic, sb);
        }
    }
}

void DiagnosticSink::diagnoseImpl(SourceLoc const& pos, DiagnosticInfo const& info, int argCount, DiagnosticArg const* const* args)
{
    StringBuilder sb;
    formatDiagnosticMessage(sb, info.messageFormat, argCount, args);

    Diagnostic diagnostic;
    diagnostic.ErrorID = info.id;
    diagnostic.Message = sb.ProduceString();
    diagnostic.loc = pos;
    diagnostic.severity = info.severity;

    if (diagnostic.severity >= Severity::Error)
    {
        m_errorCount++;
    }

    // Did the client supply a callback for us to use?
    if( writer )
    {
        // If so, pass the error string along to them
        StringBuilder messageBuilder;
        formatDiagnostic(this, diagnostic, messageBuilder);

        writer->write(messageBuilder.getBuffer(), messageBuilder.getLength());
    }
    else
    {
        // If the user doesn't have a callback, then just
        // collect our diagnostic messages into a buffer
        formatDiagnostic(this, diagnostic, outputBuffer);
    }

    if (diagnostic.severity >= Severity::Fatal)
    {
        // TODO: figure out a better policy for aborting compilation
        throw AbortCompilationException();
    }
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

    if (severity >= Severity::Fatal)
    {
        // TODO: figure out a better policy for aborting compilation
        throw InvalidOperationException();
    }
}

namespace Diagnostics
{
#define DIAGNOSTIC(id, severity, name, messageFormat) const DiagnosticInfo name = { id, Severity::severity, #name, messageFormat };
#include "slang-diagnostic-defs.h"
#undef DIAGNOSTIC
}

static const DiagnosticInfo* const kAllDiagnostics[] =
{
#define DIAGNOSTIC(id, severity, name, messageFormat) &Diagnostics::name, 
#include "slang-diagnostic-defs.h"
#undef DIAGNOSTIC
};

class DiagnosticsLookup : public RefObject
{
public:
    const DiagnosticInfo* findDiagostic(const UnownedStringSlice& slice) const
    {
        const Index* indexPtr = m_map.TryGetValue(slice);
        return indexPtr ? kAllDiagnostics[*indexPtr] : nullptr;
    }
    Index _findDiagnosticIndex(const UnownedStringSlice& slice) const
    {
        const Index* indexPtr = m_map.TryGetValue(slice);
        return indexPtr ? *indexPtr : 0;
    }
    static DiagnosticsLookup* getSingleton()
    {
        static RefPtr<DiagnosticsLookup> singleton = new DiagnosticsLookup;
        return singleton;
    }

protected:
    void _add(const char* name, Index index)
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
    void _addAlias(const char* name, const char* diagnosticName)
    {
        const Index index = _findDiagnosticIndex(UnownedStringSlice(diagnosticName));
        SLANG_ASSERT(index >= 0);
        if (index >= 0)
        {
            _add(name, index);
        }
    }
    DiagnosticsLookup();

    StringBuilder m_work;
    Dictionary<UnownedStringSlice, Index> m_map;
    MemoryArena m_arena;
};

DiagnosticsLookup::DiagnosticsLookup():
    m_arena(2048)
{
    // TODO: We should eventually have a more formal system for associating individual
    // diagnostics, or groups of diagnostics, with user-exposed names for use when
    // enabling/disabling warnings (or turning warnings into errors, etc.).
    //
    // For now we build a map from diagnostic name to it's entry. Two entries are typically
    // added - the 'original name' as associated with the diagnostic in lowerCamel, and
    // a dashified version.
    
    for (Index i = 0; i < SLANG_COUNT_OF(kAllDiagnostics); ++i)
    {
        const DiagnosticInfo* diagnostic = kAllDiagnostics[i];
        _add(diagnostic->name, i);
    }

    // Add any aliases
    _addAlias("overlappingBindings", "parameterBindingsOverlap");
}

DiagnosticInfo const* findDiagnosticByName(UnownedStringSlice const& name)
{
    return DiagnosticsLookup::getSingleton()->findDiagostic(name);
}


} // namespace Slang
