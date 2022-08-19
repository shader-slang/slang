#ifndef SLANG_DOWNSTREAM_DEP1_H
#define SLANG_DOWNSTREAM_DEP1_H


#include "slang-downstream-compiler.h"

namespace Slang
{

// (DEPRECIATED)

// Compiler description
struct DownstreamCompilerDesc_Dep1
{
    SlangPassThrough type;      ///< The type of the compiler

    /// TODO(JS): Would probably be better if changed to SemanticVersion, but not trivial to change
    // because this type is part of the DownstreamCompiler interface, which is used with `slang-llvm`.
    Int majorVersion;           ///< Major version (interpretation is type specific)
    Int minorVersion;           ///< Minor version (interpretation is type specific)
};

struct DownstreamDiagnostic_Dep1
{
    enum class Severity
    {
        Unknown,
        Info,
        Warning,
        Error,
        CountOf,
    };
    enum class Stage
    {
        Compile,
        Link,
    };

    Severity severity = Severity::Unknown;          ///< The severity of error
    Stage stage = Stage::Compile;                   ///< The stage the error came from
    String text;                                    ///< The text of the error
    String code;                                    ///< The compiler specific error code
    String filePath;                                ///< The path the error originated from
    Int fileLine = 0;                               ///< The line number the error came from
};

struct DownstreamDiagnostics_Dep1
{
    typedef DownstreamDiagnostic_Dep1 Diagnostic;

    String rawDiagnostics;

    SlangResult result = SLANG_OK;
    List<Diagnostic> diagnostics;
};

class DownstreamCompileResult_Dep1 : public RefObject
{
public:
    SLANG_CLASS_GUID(0xdfc5d318, 0x8675, 0x40ef, { 0xbd, 0x7b, 0x4, 0xa4, 0xff, 0x66, 0x11, 0x30 })

    virtual SlangResult getHostCallableSharedLibrary(ComPtr<ISlangSharedLibrary>& outLibrary) { SLANG_UNUSED(outLibrary); return SLANG_FAIL; }
    virtual SlangResult getBinary(ComPtr<ISlangBlob>& outBlob) { SLANG_UNUSED(outBlob); return SLANG_FAIL; }

    const DownstreamDiagnostics_Dep1& getDiagnostics() const { return m_diagnostics; }

    /// Ctor
    DownstreamCompileResult_Dep1(const DownstreamDiagnostics_Dep1& diagnostics) :
        m_diagnostics(diagnostics)
    {}

protected:
    DownstreamDiagnostics_Dep1 m_diagnostics;
};

class DownstreamCompiler_Dep1: public RefObject
{
public:
    typedef RefObject Super;

        /// Get the desc of this compiler
    const DownstreamCompilerDesc_Dep1& getDesc() const { return m_desc;  }
        /// Compile using the specified options. The result is in resOut
    virtual SlangResult compile(const DownstreamCompileOptions& options, RefPtr<DownstreamCompileResult_Dep1>& outResult) = 0;
        /// Some compilers have support converting a binary blob into disassembly. Output disassembly is held in the output blob
    virtual SlangResult disassemble(SlangCompileTarget sourceBlobTarget, const void* blob, size_t blobSize, ISlangBlob** out);

        /// True if underlying compiler uses file system to communicate source
    virtual bool isFileBased() = 0;

protected:

    DownstreamCompilerDesc_Dep1 m_desc;
};

class DownstreamCompilerAdapter_Dep1 : public DownstreamCompilerBase
{
public:
    // IDownstreamCompiler
    virtual SLANG_NO_THROW const Desc& SLANG_MCALL getDesc() SLANG_OVERRIDE { return m_desc; }
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL compile(const CompileOptions& options, IArtifact** outArtifact) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL disassemble(SlangCompileTarget sourceBlobTarget, const void* blob, size_t blobSize, ISlangBlob** out) SLANG_OVERRIDE { return m_dep->disassemble(sourceBlobTarget, blob, blobSize, out); }
    virtual SLANG_NO_THROW bool SLANG_MCALL isFileBased() SLANG_OVERRIDE { return m_dep->isFileBased(); }

    DownstreamCompilerAdapter_Dep1(DownstreamCompiler_Dep1* dep);

protected:

    DownstreamCompilerDesc m_desc;

    RefPtr<DownstreamCompiler_Dep1> m_dep;
};

struct DownstreamUtil_Dep1
{
    static SlangResult getDownstreamSharedLibrary(DownstreamCompileResult_Dep1* downstreamResult, ComPtr<ISlangSharedLibrary>& outSharedLibrary);
};

}

#endif
