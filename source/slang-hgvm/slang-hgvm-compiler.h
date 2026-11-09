// hgbc-compiler.h
#pragma once

#include "core/slang-string.h"
#include "slang-com-ptr.h"
#include "slang.h"

#include <memory>

namespace Slang::HGVM
{

/// Caches Slang sessions by compiler options and ordered search paths for HGBC hosts.
/// A null entryPointName selects all concrete [host] entry points in the source module.
/// A compiler instance represents a source snapshot; create a new instance to reload changed files.
class HGBCCompiler : public Slang::RefObject
{
public:
    /// Uses the supplied filesystem for source paths and imports. The filesystem and its contents
    /// belong to this compiler's source snapshot; a null filesystem uses the native filesystem.
    explicit HGBCCompiler(
        slang::IGlobalSession* globalSession,
        ISlangFileSystemExt* fileSystem = nullptr);
    ~HGBCCompiler();

    SlangResult compile(
        const char* sourcePath,
        const char* entryPointName,
        const char* const* additionalSearchPaths,
        SlangInt additionalSearchPathCount,
        SlangDebugInfoLevel debugInfoLevel,
        SlangDiagnosticColor diagnosticColor,
        Slang::ComPtr<slang::IBlob>& outCode,
        Slang::String& outDiagnostics,
        bool reportCheckpointIntermediates = false);

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

/// Compiles selected host entry points to HGBC using a temporary compiler instance.
SlangResult compileHGBCProgram(
    const char* sourcePath,
    const char* entryPointName,
    const char* const* additionalSearchPaths,
    SlangInt additionalSearchPathCount,
    SlangDebugInfoLevel debugInfoLevel,
    SlangDiagnosticColor diagnosticColor,
    Slang::ComPtr<slang::IBlob>& outCode,
    Slang::String& outDiagnostics,
    bool reportCheckpointIntermediates = false);

} // namespace Slang::HGVM
