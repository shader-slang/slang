// slang-compiler.h
#ifndef SLANG_COMPILER_H_INCLUDED
#define SLANG_COMPILER_H_INCLUDED

//
// This file provides an umbrella header that ties together
// the headers for a bunch of the core types used by the
// Slang compiler implementation: the global session, session,
// modules, entry points, etc.
//
// Note: this file used to be a kind of kitchen-sink header
// with thousands of lines of declarations, and even though
// those declarations have migrated to their own files, this
// header has been otherwise left as-is to avoid breaking
// all of the code that `#include`s it.
//
// Please avoid adding new declarations in here without a clear
// motivation for *why* they belong here.
//

#include "../compiler-core/slang-artifact-representation-impl.h"
#include "../compiler-core/slang-command-line-args.h"
#include "../compiler-core/slang-downstream-compiler-util.h"
#include "../compiler-core/slang-downstream-compiler.h"
#include "../compiler-core/slang-include-system.h"
#include "../compiler-core/slang-name.h"
#include "../compiler-core/slang-source-embed-util.h"
#include "../compiler-core/slang-spirv-core-grammar.h"
#include "../core/slang-basic.h"
#include "../core/slang-command-options.h"
#include "../core/slang-crypto.h"
#include "../core/slang-file-system.h"
#include "../core/slang-shared-library.h"
#include "../core/slang-std-writers.h"
#include "slang-base-type-info.h"
#include "slang-capability.h"
#include "slang-code-gen.h"
#include "slang-com-ptr.h"
#include "slang-compile-request.h"
#include "slang-compiler-api.h"
#include "slang-compiler-options.h"
#include "slang-content-assist-info.h"
#include "slang-diagnostics.h"
#include "slang-end-to-end-request.h"
#include "slang-global-session.h"
#include "slang-hlsl-to-vulkan-layout-options.h"
#include "slang-linkable-impls.h"
#include "slang-linkable.h"
#include "slang-module.h"
#include "slang-pass-through.h"
#include "slang-preprocessor.h"
#include "slang-profile.h"
#include "slang-serialize-ir-types.h"
#include "slang-session.h"
#include "slang-syntax.h"
#include "slang-target.h"
#include "slang-translation-unit.h"
#include "slang.h"

#include <chrono>

namespace Slang
{
struct PathInfo;
struct IncludeHandler;
struct ModuleChunk;

class ProgramLayout;
class PtrType;
class TargetProgram;
class TargetRequest;
class TypeLayout;
class Artifact;

enum class CompilerMode
{
    ProduceLibrary,
    ProduceShader,
    GenerateChoice
};


enum class LineDirectiveMode : SlangLineDirectiveModeIntegral
{
    Default = SLANG_LINE_DIRECTIVE_MODE_DEFAULT,
    None = SLANG_LINE_DIRECTIVE_MODE_NONE,
    Standard = SLANG_LINE_DIRECTIVE_MODE_STANDARD,
    GLSL = SLANG_LINE_DIRECTIVE_MODE_GLSL,
    SourceMap = SLANG_LINE_DIRECTIVE_MODE_SOURCE_MAP,
};

enum class ResultFormat
{
    None,
    Text,
    Binary,
};

// When storing the layout for a matrix-type
// value, we need to know whether it has been
// laid out with row-major or column-major
// storage.
//
enum MatrixLayoutMode : SlangMatrixLayoutModeIntegral
{
    kMatrixLayoutMode_RowMajor = SLANG_MATRIX_LAYOUT_ROW_MAJOR,
    kMatrixLayoutMode_ColumnMajor = SLANG_MATRIX_LAYOUT_COLUMN_MAJOR,
};

enum class DebugInfoLevel : SlangDebugInfoLevelIntegral
{
    None = SLANG_DEBUG_INFO_LEVEL_NONE,
    Minimal = SLANG_DEBUG_INFO_LEVEL_MINIMAL,
    Standard = SLANG_DEBUG_INFO_LEVEL_STANDARD,
    Maximal = SLANG_DEBUG_INFO_LEVEL_MAXIMAL,
};

enum class DebugInfoFormat : SlangDebugInfoFormatIntegral
{
    Default = SLANG_DEBUG_INFO_FORMAT_DEFAULT,
    C7 = SLANG_DEBUG_INFO_FORMAT_C7,
    Pdb = SLANG_DEBUG_INFO_FORMAT_PDB,

    Stabs = SLANG_DEBUG_INFO_FORMAT_STABS,
    Coff = SLANG_DEBUG_INFO_FORMAT_COFF,
    Dwarf = SLANG_DEBUG_INFO_FORMAT_DWARF,

    CountOf = SLANG_DEBUG_INFO_FORMAT_COUNT_OF,
};

enum class OptimizationLevel : SlangOptimizationLevelIntegral
{
    None = SLANG_OPTIMIZATION_LEVEL_NONE,
    Default = SLANG_OPTIMIZATION_LEVEL_DEFAULT,
    High = SLANG_OPTIMIZATION_LEVEL_HIGH,
    Maximal = SLANG_OPTIMIZATION_LEVEL_MAXIMAL,
};

struct CodeGenContext;
class EndToEndCompileRequest;
class FrontEndCompileRequest;
class Linkage;
class Module;
class TranslationUnitRequest;


class SourceFile;


enum class FloatingPointMode : SlangFloatingPointModeIntegral
{
    Default = SLANG_FLOATING_POINT_MODE_DEFAULT,
    Fast = SLANG_FLOATING_POINT_MODE_FAST,
    Precise = SLANG_FLOATING_POINT_MODE_PRECISE,
};

enum class FloatingPointDenormalMode : SlangFpDenormalModeIntegral
{
    Any = SLANG_FP_DENORM_MODE_ANY,
    Preserve = SLANG_FP_DENORM_MODE_PRESERVE,
    FlushToZero = SLANG_FP_DENORM_MODE_FTZ,
};


// Compute the "effective" profile to use when outputting the given entry point
// for the chosen code-generation target.
//
// The stage of the effective profile will always come from the entry point, while
// the profile version (aka "shader model") will be computed as follows:
//
// - If the entry point and target belong to the same profile family, then take
//   the latest version between the two (e.g., if the entry point specified `ps_5_1`
//   and the target specifies `sm_5_0` then use `sm_5_1` as the version).
//
// - If the entry point and target disagree on the profile family, always use the
//   profile family and version from the target.
//
Profile getEffectiveProfile(EntryPoint* entryPoint, TargetRequest* target);


/// Get the build tag string
const char* getBuildTagString();


//

void checkTranslationUnit(
    TranslationUnitRequest* translationUnit,
    LoadedModuleDictionary& loadedModules);

// Look for a module that matches the given name:
// either one we've loaded already, or one we
// can find vai the search paths available to us.
//
// Needed by import declaration checking.
//
RefPtr<Module> findOrImportModule(
    Linkage* linkage,
    Name* name,
    SourceLoc const& loc,
    DiagnosticSink* sink,
    const LoadedModuleDictionary* additionalLoadedModules);

SlangResult passthroughDownstreamDiagnostics(
    DiagnosticSink* sink,
    IDownstreamCompiler* compiler,
    IArtifact* artifact);


// helpers for error/warning reporting
enum class DiagnosticCategory
{
    None = 0,
    Capability = 1 << 0,
};
template<typename P, typename... Args>
bool maybeDiagnose(
    DiagnosticSink* sink,
    CompilerOptionSet& optionSet,
    DiagnosticCategory errorType,
    P const& pos,
    DiagnosticInfo const& info,
    Args const&... args)
{
    if ((int)errorType & (int)DiagnosticCategory::Capability &&
        optionSet.getBoolOption(CompilerOptionName::IgnoreCapabilities))
        return false;
    return sink->diagnose(pos, info, args...);
}

template<typename P, typename... Args>
bool maybeDiagnoseWarningOrError(
    DiagnosticSink* sink,
    CompilerOptionSet& optionSet,
    DiagnosticCategory errorType,
    P const& pos,
    DiagnosticInfo const& warningInfo,
    DiagnosticInfo const& errorInfo,
    Args const&... args)
{
    if ((int)errorType & (int)DiagnosticCategory::Capability &&
        optionSet.getBoolOption(CompilerOptionName::RestrictiveCapabilityCheck))
    {
        return maybeDiagnose(sink, optionSet, errorType, pos, errorInfo, args...);
    }
    else
    {
        return maybeDiagnose(sink, optionSet, errorType, pos, warningInfo, args...);
    }
}

bool isValidSlangLanguageVersion(SlangLanguageVersion version);
bool isValidGLSLVersion(int version);

} // namespace Slang

#endif
