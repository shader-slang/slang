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

};

#endif // PARSE_DIAGNOSTIC_UTIL_H
