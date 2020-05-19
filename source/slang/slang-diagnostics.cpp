// slang-diagnostics.cpp
#include "slang-diagnostics.h"

#include "slang-name.h"

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
    auto sourceManager = sink->sourceManager;

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
    }
     
    if (sourceView && (sink->flags & DiagnosticSink::Flag::VerbosePath))
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
        errorCount++;
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
        errorCount++;
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
#define DIAGNOSTIC(id, severity, name, messageFormat) const DiagnosticInfo name = { id, Severity::severity, messageFormat };
#include "slang-diagnostic-defs.h"
}

DiagnosticInfo const* findDiagnosticByName(UnownedStringSlice const& name)
{
    // TODO: We should eventually have a more formal system for associating individual
    // diagnostics, or groups of diagnostics, with user-exposed names for use when
    // enabling/disabling warnings (or turning warnings into errors, etc.).
    //
    // For now we build an ad hoc mapping from string names to corresponding single
    // diagnostics (not groups).
    //
    static const struct
    {
        char const*             name;
        DiagnosticInfo const*   diagnostic;
    } kDiagnostics[] =
    {
        { "overlapping-bindings", &Diagnostics::parameterBindingsOverlap },
    };

    for( auto& entry : kDiagnostics )
    {
        if( name == entry.name )
            return entry.diagnostic;
    }
    return nullptr;
}


} // namespace Slang
