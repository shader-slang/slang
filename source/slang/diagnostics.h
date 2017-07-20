#ifndef RASTER_RENDERER_COMPILE_ERROR_H
#define RASTER_RENDERER_COMPILE_ERROR_H

#include "../core/basic.h"

#include "source-loc.h"
#include "token.h"

#include "../../slang.h"

namespace Slang
{
    enum class Severity
    {
        Note,
        Warning,
        Error,
        Fatal,
        Internal,
    };

    // TODO(tfoley): move this into a source file...
    inline const char* getSeverityName(Severity severity)
    {
        switch (severity)
        {
        case Severity::Note:        return "note";
        case Severity::Warning:     return "warning";
        case Severity::Error:       return "error";
        case Severity::Fatal:       return "fatal error";
        case Severity::Internal:    return "internal error";
        default:                    return "unknown error";
        }
    }

    // A structure to be used in static data describing different
    // diagnostic messages.
    struct DiagnosticInfo
    {
        int id;
        Severity severity;
        char const* messageFormat;
    };

    class Diagnostic
    {
    public:
        String Message;
        CodePosition Position;
        int ErrorID;
        Severity severity;

        Diagnostic()
        {
            ErrorID = -1;
        }
        Diagnostic(
            const String & msg,
            int id,
            const CodePosition & pos,
            Severity severity)
            : severity(severity)
        {
            Message = msg;
            ErrorID = id;
            Position = pos;
        }
    };

    class Decl;
    class Type;
    class ExpressionType;
    class ILType;
    class StageAttribute;
    struct TypeExp;
    struct QualType;

    void printDiagnosticArg(StringBuilder& sb, char const* str);
    void printDiagnosticArg(StringBuilder& sb, int val);
    void printDiagnosticArg(StringBuilder& sb, UInt val);
    void printDiagnosticArg(StringBuilder& sb, Slang::String const& str);
    void printDiagnosticArg(StringBuilder& sb, Decl* decl);
    void printDiagnosticArg(StringBuilder& sb, Type* type);
    void printDiagnosticArg(StringBuilder& sb, ExpressionType* type);
    void printDiagnosticArg(StringBuilder& sb, TypeExp const& type);
    void printDiagnosticArg(StringBuilder& sb, QualType const& type);
    void printDiagnosticArg(StringBuilder& sb, TokenType tokenType);
    void printDiagnosticArg(StringBuilder& sb, Token const& token);

    template<typename T>
    void printDiagnosticArg(StringBuilder& sb, RefPtr<T> ptr)
    {
        printDiagnosticArg(sb, ptr.Ptr());
    }

    inline CodePosition const& getDiagnosticPos(CodePosition const& pos) { return pos;  }

    class SyntaxNode;
    class ShaderClosure;
    CodePosition const& getDiagnosticPos(SyntaxNode const* syntax);
    CodePosition const& getDiagnosticPos(Token const& token);
    CodePosition const& getDiagnosticPos(TypeExp const& typeExp);

    template<typename T>
    CodePosition getDiagnosticPos(RefPtr<T> const& ptr)
    {
        return getDiagnosticPos(ptr.Ptr());
    }

    struct DiagnosticArg
    {
        void* data;
        void (*printFunc)(StringBuilder&, void*);

        template<typename T>
        struct Helper
        {
            static void printFunc(StringBuilder& sb, void* data) { printDiagnosticArg(sb, *(T*)data); }
        };

        template<typename T>
        DiagnosticArg(T const& arg)
            : data((void*)&arg)
            , printFunc(&Helper<T>::printFunc)
        {}
    };

    class DiagnosticSink
    {
    public:
        StringBuilder outputBuffer;
//            List<Diagnostic> diagnostics;
        int errorCount = 0;

        SlangDiagnosticCallback callback            = nullptr;
        void*                   callbackUserData    = nullptr;

/*
        void Error(int id, const String & msg, const CodePosition & pos)
        {
            diagnostics.Add(Diagnostic(msg, id, pos, Severity::Error));
            errorCount++;
        }

        void Warning(int id, const String & msg, const CodePosition & pos)
        {
            diagnostics.Add(Diagnostic(msg, id, pos, Severity::Warning));
        }
*/
        int GetErrorCount() { return errorCount; }

        void diagnoseDispatch(CodePosition const& pos, DiagnosticInfo const& info)
        {
            diagnoseImpl(pos, info, 0, NULL);
        }

        void diagnoseDispatch(CodePosition const& pos, DiagnosticInfo const& info, DiagnosticArg const& arg0)
        {
            DiagnosticArg const* args[] = { &arg0 };
            diagnoseImpl(pos, info, 1, args);
        }

        void diagnoseDispatch(CodePosition const& pos, DiagnosticInfo const& info, DiagnosticArg const& arg0, DiagnosticArg const& arg1)
        {
            DiagnosticArg const* args[] = { &arg0, &arg1 };
            diagnoseImpl(pos, info, 2, args);
        }

        void diagnoseDispatch(CodePosition const& pos, DiagnosticInfo const& info, DiagnosticArg const& arg0, DiagnosticArg const& arg1, DiagnosticArg const& arg2)
        {
            DiagnosticArg const* args[] = { &arg0, &arg1, &arg2 };
            diagnoseImpl(pos, info, 3, args);
        }

        void diagnoseDispatch(CodePosition const& pos, DiagnosticInfo const& info, DiagnosticArg const& arg0, DiagnosticArg const& arg1, DiagnosticArg const& arg2, DiagnosticArg const& arg3)
        {
            DiagnosticArg const* args[] = { &arg0, &arg1, &arg2, &arg3 };
            diagnoseImpl(pos, info, 4, args);
        }

        template<typename P, typename... Args>
        void diagnose(P const& pos, DiagnosticInfo const& info, Args const&... args )
        {
            diagnoseDispatch(getDiagnosticPos(pos), info, args...);
        }

        void diagnoseImpl(CodePosition const& pos, DiagnosticInfo const& info, int argCount, DiagnosticArg const* const* args);

        // Add a diagnostic with raw text
        // (used when we get errors from a downstream compiler)
        void diagnoseRaw(
            Severity    severity,
            char const* message);
    };

    namespace Diagnostics
    {
#define DIAGNOSTIC(id, severity, name, messageFormat) extern const DiagnosticInfo name;
#include "diagnostic-defs.h"
    }
}

#ifdef _DEBUG
#define SLANG_INTERNAL_ERROR(sink, pos) \
    (sink)->diagnose(Slang::CodePosition(__LINE__, 0, 0, __FILE__), Slang::Diagnostics::internalCompilerError)
#define SLANG_UNIMPLEMENTED(sink, pos, what) \
    (sink)->diagnose(Slang::CodePosition(__LINE__, 0, 0, __FILE__), Slang::Diagnostics::unimplemented, what)

#else
#define SLANG_INTERNAL_ERROR(sink, pos) \
    (sink)->diagnose(pos, Slang::Diagnostics::internalCompilerError)
#define SLANG_UNIMPLEMENTED(sink, pos, what) \
    (sink)->diagnose(pos, Slang::Diagnostics::unimplemented, what)

#endif

#define SLANG_DIAGNOSE_UNEXPECTED(sink, pos, message) \
    (sink)->diagnose(pos, Slang::Diagnostics::unexpected, message)

#endif
