// parse-diagnostic-util.h

#ifndef PARSE_DIAGNOSTIC_UTIL_H
#define PARSE_DIAGNOSTIC_UTIL_H

#include "../../source/core/slang-string-util.h"
#include "../../source/core/slang-downstream-compiler.h"
#include "../../source/core/slang-string.h"

#include "../../slang-com-ptr.h"

struct ParseDiagnosticUtil
{
    static SlangResult splitPathLocation(const Slang::UnownedStringSlice& pathLocation, Slang::DownstreamDiagnostic& outDiagnostic);

    static SlangResult parseFXCLine(const Slang::UnownedStringSlice& line, Slang::List<Slang::UnownedStringSlice>& lineSlices, Slang::DownstreamDiagnostic& outDiagnostic);

    static SlangResult parseDXCLine(const Slang::UnownedStringSlice& line, Slang::List<Slang::UnownedStringSlice>& lineSlices, Slang::DownstreamDiagnostic& outDiagnostic);

    static SlangResult parseGlslangLine(const Slang::UnownedStringSlice& line, Slang::List<Slang::UnownedStringSlice>& lineSlices, Slang::DownstreamDiagnostic& outDiagnostic);

    static SlangResult parseGenericLine(const Slang::UnownedStringSlice& line, Slang::List<Slang::UnownedStringSlice>& lineSlices, Slang::DownstreamDiagnostic& outDiagnostic);

    static SlangResult splitCompilerDiagnosticLine(SlangPassThrough downstreamCompiler, const Slang::UnownedStringSlice& line, Slang::UnownedStringSlice& linePrefix, Slang::List<Slang::UnownedStringSlice>& outSlices);

    static SlangResult parseDiagnostics(const Slang::UnownedStringSlice& inText, Slang::List<Slang::DownstreamDiagnostic>& outDiagnostics);

};

#endif // PARSE_DIAGNOSTIC_UTIL_H
