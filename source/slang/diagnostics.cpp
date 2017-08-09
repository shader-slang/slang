// Diagnostics.cpp
#include "Diagnostics.h"

#include "Syntax.h"

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

void printDiagnosticArg(StringBuilder& sb, int str)
{
    sb << str;
}

void printDiagnosticArg(StringBuilder& sb, UInt val)
{
    // TODO: make this robust
    sb << (int) val;
}

void printDiagnosticArg(StringBuilder& sb, Slang::String const& str)
{
    sb << str;
}

void printDiagnosticArg(StringBuilder& sb, Decl* decl)
{
    sb << decl->Name.Content;
}

void printDiagnosticArg(StringBuilder& sb, Type* type)
{
    sb << type->ToString();
}

void printDiagnosticArg(StringBuilder& sb, TypeExp const& type)
{
    sb << type.type->ToString();
}

void printDiagnosticArg(StringBuilder& sb, QualType const& type)
{
    sb << type.type->ToString();
}

void printDiagnosticArg(StringBuilder& sb, TokenType tokenType)
{
    sb << TokenTypeToString(tokenType);
}

void printDiagnosticArg(StringBuilder& sb, Token const& token)
{
    sb << token.Content;
}

CodePosition const& getDiagnosticPos(SyntaxNode const* syntax)
{
    return syntax->Position;
}

CodePosition const& getDiagnosticPos(Token const& token)
{
    return token.Position;
}

CodePosition const& getDiagnosticPos(TypeExp const& typeExp)
{
    return typeExp.exp->Position;
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

        SLANG_API(*spanEnd == '$');
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

static void formatDiagnostic(
    StringBuilder&      sb,
    Diagnostic const&   diagnostic)
{
    sb << diagnostic.Position.FileName;
    sb << "(";
    sb << diagnostic.Position.Line;
    sb << "): ";
    sb << getSeverityName(diagnostic.severity);
    sb << " ";
    sb << diagnostic.ErrorID;
    sb << ": ";
    sb << diagnostic.Message;
    sb << "\n";
}

void DiagnosticSink::diagnoseImpl(CodePosition const& pos, DiagnosticInfo const& info, int argCount, DiagnosticArg const* const* args)
{
    StringBuilder sb;
    formatDiagnosticMessage(sb, info.messageFormat, argCount, args);

    Diagnostic diagnostic;
    diagnostic.ErrorID = info.id;
    diagnostic.Message = sb.ProduceString();
    diagnostic.Position = pos;
    diagnostic.severity = info.severity;

    if (diagnostic.severity >= Severity::Error)
    {
        errorCount++;
    }

    // Did the client supply a callback for us to use?
    if( callback )
    {
        // If so, pass the error string along to them
        StringBuilder messageBuilder;
        formatDiagnostic(messageBuilder, diagnostic);

        callback(messageBuilder.ProduceString().begin(), callbackUserData);
    }
    else
    {
        // If the user doesn't have a callback, then just
        // collect our diagnostic messages into a buffer
        formatDiagnostic(outputBuffer, diagnostic);
    }

    if (diagnostic.severity >= Severity::Fatal)
    {
        // TODO: figure out a better policy for aborting compilation
        throw InvalidOperationException();
    }
}

void DiagnosticSink::diagnoseRaw(
    Severity    severity,
    char const* message)
{
    if (severity >= Severity::Error)
    {
        errorCount++;
    }

    // Did the client supply a callback for us to use?
    if( callback )
    {
        // If so, pass the error string along to them
        callback(message, callbackUserData);
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
#include "diagnostic-defs.h"
}


} // namespace Slang
