// parse-diagnostic-util.h

#ifndef PARSE_DIAGNOSTIC_UTIL_H
#define PARSE_DIAGNOSTIC_UTIL_H

#include "../../source/core/slang-string-util.h"
#include "../../source/core/slang-downstream-compiler.h"
#include "../../source/core/slang-string.h"

#include "../../slang-com-ptr.h"

struct ParseDiagnosticUtil
{
    struct OutputInfo
    {
        int resultCode;
        Slang::String stdError;
        Slang::String stdOut;
    };

        /// We need a way to identify Slang. Ideally we'd have an enum that included SLANG
        /// But that doesn't really work with SlangPassThrough, although perhaps makes
        /// some sort of sense as a 'DownstreamCompiler'.
    struct CompilerIdentity
    {
        typedef CompilerIdentity ThisType;

        enum Type
        {
            Unknown,
            Slang,
            DownstreamCompiler,
        };

        static CompilerIdentity make(Type type, SlangPassThrough downstreamCompiler) { CompilerIdentity ident; ident.m_type = type; ident.m_downstreamCompiler = downstreamCompiler; return ident; }
        static CompilerIdentity make(SlangPassThrough downstreamCompiler) { return make(Type::DownstreamCompiler, downstreamCompiler); }
        static CompilerIdentity makeSlang() { return make(Type::Slang, SLANG_PASS_THROUGH_NONE); }

        bool operator==(const ThisType& rhs) const { return m_type == rhs.m_type && m_downstreamCompiler == rhs.m_downstreamCompiler; }
        bool operator!=(const ThisType& rhs) const { return !(*this == rhs); }

        Type m_type = Type::Unknown; 
        SlangPassThrough m_downstreamCompiler = SLANG_PASS_THROUGH_NONE;
    };

    typedef uint32_t EqualityFlags;
    struct EqualityFlag
    {
        enum Enum : EqualityFlags
        {
            IgnoreLineNos = 0x1,
        };
    };

    typedef SlangResult (*LineParser)(const Slang::UnownedStringSlice& line, Slang::List<Slang::UnownedStringSlice>& lineSlices, Slang::DownstreamDiagnostic& outDiagnostic);

    static LineParser getLineParser(const CompilerIdentity& compilerIdentity);

        /// Given a path, that holds line number and potentially column number in () after path, writes result into outDiagnostic
    static SlangResult splitPathLocation(const Slang::UnownedStringSlice& pathLocation, Slang::DownstreamDiagnostic& outDiagnostic);

        /// For FXC lines
    static SlangResult parseFXCLine(const Slang::UnownedStringSlice& line, Slang::List<Slang::UnownedStringSlice>& lineSlices, Slang::DownstreamDiagnostic& outDiagnostic);

        /// For DXC lines
    static SlangResult parseDXCLine(const Slang::UnownedStringSlice& line, Slang::List<Slang::UnownedStringSlice>& lineSlices, Slang::DownstreamDiagnostic& outDiagnostic);

        /// For GLSL lines
    static SlangResult parseGlslangLine(const Slang::UnownedStringSlice& line, Slang::List<Slang::UnownedStringSlice>& lineSlices, Slang::DownstreamDiagnostic& outDiagnostic);

        /// For a 'generic' (as in uses DownstreamCompiler mechanism) line parsing
    static SlangResult parseGenericLine(const Slang::UnownedStringSlice& line, Slang::List<Slang::UnownedStringSlice>& lineSlices, Slang::DownstreamDiagnostic& outDiagnostic);

        /// For parsing diagnostics from Slang
    static SlangResult parseSlangLine(const Slang::UnownedStringSlice& line, Slang::List<Slang::UnownedStringSlice>& lineSlices, Slang::DownstreamDiagnostic& outDiagnostic);

        /// Parse diagnostics into output text
    static SlangResult parseDiagnostics(const Slang::UnownedStringSlice& inText, Slang::List<Slang::DownstreamDiagnostic>& outDiagnostics);

        /// Given the file output style used by tests, get components of the output into Diagnostic
    static SlangResult parseOutputInfo(const Slang::UnownedStringSlice& in, OutputInfo& out);

        /// Given a line split it into slices - taking into account compiler output, path considerations, and potentially line prefixing
    static SlangResult splitDiagnosticLine(const CompilerIdentity& compilerIdentity, const Slang::UnownedStringSlice& line, Slang::UnownedStringSlice& linePrefix, Slang::List<Slang::UnownedStringSlice>& outSlices);

        /// Give text of diagnostic determine which compiler the output is from
    static SlangResult identifyCompiler(const Slang::UnownedStringSlice& in, CompilerIdentity& outIdentity);

        /// Determines if equal taking into account flags
    static bool areEqual(const Slang::UnownedStringSlice& a, const Slang::UnownedStringSlice& b, EqualityFlags flags);
};

#endif // PARSE_DIAGNOSTIC_UTIL_H
