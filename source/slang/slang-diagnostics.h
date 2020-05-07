#ifndef RASTER_RENDERER_COMPILE_ERROR_H
#define RASTER_RENDERER_COMPILE_ERROR_H

#include "../core/slang-basic.h"
#include "../core/slang-writer.h"

#include "slang-source-loc.h"
#include "slang-token.h"

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
        SourceLoc loc;
        int ErrorID;
        Severity severity;

        Diagnostic()
        {
            ErrorID = -1;
        }
        Diagnostic(
            const String & msg,
            int id,
            const SourceLoc & pos,
            Severity severity)
            : severity(severity)
        {
            Message = msg;
            ErrorID = id;
            loc = pos;
        }
    };

    class Name;

    //enum class CodeGenTarget;

    //enum class Stage : SlangStage;
    //enum class ProfileVersion;

    void printDiagnosticArg(StringBuilder& sb, char const* str);

    void printDiagnosticArg(StringBuilder& sb, int32_t val);
    void printDiagnosticArg(StringBuilder& sb, uint32_t val);

    void printDiagnosticArg(StringBuilder& sb, int64_t val);
    void printDiagnosticArg(StringBuilder& sb, uint64_t val);

    void printDiagnosticArg(StringBuilder& sb, double val);

    void printDiagnosticArg(StringBuilder& sb, Slang::String const& str);
    void printDiagnosticArg(StringBuilder& sb, Slang::UnownedStringSlice const& str);
    void printDiagnosticArg(StringBuilder& sb, Name* name);

    void printDiagnosticArg(StringBuilder& sb, TokenType tokenType);
    void printDiagnosticArg(StringBuilder& sb, Token const& token);
    
    
    template<typename T>
    void printDiagnosticArg(StringBuilder& sb, RefPtr<T> ptr)
    {
        printDiagnosticArg(sb, ptr.Ptr());
    }

    inline SourceLoc const& getDiagnosticPos(SourceLoc const& pos) { return pos;  }

    SourceLoc const& getDiagnosticPos(Token const& token);
    

    template<typename T>
    SourceLoc getDiagnosticPos(RefPtr<T> const& ptr)
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
        DiagnosticSink(SourceManager* sourceManager)
            : sourceManager(sourceManager)
        {}

        struct Flag 
        {
            enum Enum: uint32_t
            {
                VerbosePath = 0x1,              ///< Will display a more verbose path (if available) - such as a canonical or absolute path
            };
        };
        typedef uint32_t Flags;

        StringBuilder outputBuffer;
//            List<Diagnostic> diagnostics;
        int errorCount = 0;
        int internalErrorLocsNoted = 0;

        ISlangWriter* writer                        = nullptr;
        Flags flags                                 = 0;

        // The source manager to use when mapping source locations to file+line info
        SourceManager*  sourceManager = nullptr;

/*
        void Error(int id, const String & msg, const SourceLoc & pos)
        {
            diagnostics.Add(Diagnostic(msg, id, pos, Severity::Error));
            errorCount++;
        }

        void Warning(int id, const String & msg, const SourceLoc & pos)
        {
            diagnostics.Add(Diagnostic(msg, id, pos, Severity::Warning));
        }
*/
        int GetErrorCount() { return errorCount; }

        void diagnoseDispatch(SourceLoc const& pos, DiagnosticInfo const& info)
        {
            diagnoseImpl(pos, info, 0, nullptr);
        }

        void diagnoseDispatch(SourceLoc const& pos, DiagnosticInfo const& info, DiagnosticArg const& arg0)
        {
            DiagnosticArg const* args[] = { &arg0 };
            diagnoseImpl(pos, info, 1, args);
        }

        void diagnoseDispatch(SourceLoc const& pos, DiagnosticInfo const& info, DiagnosticArg const& arg0, DiagnosticArg const& arg1)
        {
            DiagnosticArg const* args[] = { &arg0, &arg1 };
            diagnoseImpl(pos, info, 2, args);
        }

        void diagnoseDispatch(SourceLoc const& pos, DiagnosticInfo const& info, DiagnosticArg const& arg0, DiagnosticArg const& arg1, DiagnosticArg const& arg2)
        {
            DiagnosticArg const* args[] = { &arg0, &arg1, &arg2 };
            diagnoseImpl(pos, info, 3, args);
        }

        void diagnoseDispatch(SourceLoc const& pos, DiagnosticInfo const& info, DiagnosticArg const& arg0, DiagnosticArg const& arg1, DiagnosticArg const& arg2, DiagnosticArg const& arg3)
        {
            DiagnosticArg const* args[] = { &arg0, &arg1, &arg2, &arg3 };
            diagnoseImpl(pos, info, 4, args);
        }

        template<typename P, typename... Args>
        void diagnose(P const& pos, DiagnosticInfo const& info, Args const&... args )
        {
            diagnoseDispatch(getDiagnosticPos(pos), info, args...);
        }

        void diagnoseImpl(SourceLoc const& pos, DiagnosticInfo const& info, int argCount, DiagnosticArg const* const* args);

        // Add a diagnostic with raw text
        // (used when we get errors from a downstream compiler)
        void diagnoseRaw(
            Severity    severity,
            char const* message);
        void diagnoseRaw(
            Severity    severity,
            const UnownedStringSlice& message);

            /// During propagation of an exception for an internal
            /// error, note that this source location was involved
        void noteInternalErrorLoc(SourceLoc const& loc);

        SlangResult getBlobIfNeeded(ISlangBlob** outBlob);
    };

        /// An `ISlangWriter` that writes directly to a diagnostic sink.
    class DiagnosticSinkWriter : public AppendBufferWriter
    {
    public:
        typedef AppendBufferWriter Super;

        DiagnosticSinkWriter(DiagnosticSink* sink)
            : Super(WriterFlag::IsStatic)
            , m_sink(sink)
        {}

        // ISlangWriter
        SLANG_NO_THROW virtual SlangResult SLANG_MCALL write(const char* chars, size_t numChars) SLANG_OVERRIDE
        {
            m_sink->diagnoseRaw(Severity::Note, UnownedStringSlice(chars, chars+numChars));
            return SLANG_OK;
        }

    private:
        DiagnosticSink* m_sink = nullptr;
    };

    DiagnosticInfo const* findDiagnosticByName(UnownedStringSlice const& name);

    namespace Diagnostics
    {
#define DIAGNOSTIC(id, severity, name, messageFormat) extern const DiagnosticInfo name;
#include "slang-diagnostic-defs.h"
    }
}

#ifdef _DEBUG
#define SLANG_INTERNAL_ERROR(sink, pos) \
    (sink)->diagnose(Slang::SourceLoc(__LINE__, 0, 0, __FILE__), Slang::Diagnostics::internalCompilerError)
#define SLANG_UNIMPLEMENTED(sink, pos, what) \
    (sink)->diagnose(Slang::SourceLoc(__LINE__, 0, 0, __FILE__), Slang::Diagnostics::unimplemented, what)

#else
#define SLANG_INTERNAL_ERROR(sink, pos) \
    (sink)->diagnose(pos, Slang::Diagnostics::internalCompilerError)
#define SLANG_UNIMPLEMENTED(sink, pos, what) \
    (sink)->diagnose(pos, Slang::Diagnostics::unimplemented, what)

#endif

#define SLANG_DIAGNOSE_UNEXPECTED(sink, pos, message) \
    (sink)->diagnose(pos, Slang::Diagnostics::unexpected, message)

#endif
