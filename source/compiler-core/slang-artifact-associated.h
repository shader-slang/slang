// slang-artifact-associated.h
#ifndef SLANG_ARTIFACT_ASSOCIATED_H
#define SLANG_ARTIFACT_ASSOCIATED_H

#include "slang-artifact.h"

namespace Slang
{

struct ArtifactDiagnostic
{
    typedef ArtifactDiagnostic ThisType;

    enum class Severity : uint8_t
    {
        Unknown,
        Info,
        Warning,
        Error,
        CountOf,
    };
    enum class Stage : uint8_t
    {
        Compile,
        Link,
    };

    struct Location
    {
        typedef Location ThisType;
        bool operator==(const ThisType& rhs) const
        {
            return line == rhs.line && column == rhs.column;
        }
        bool operator!=(const ThisType& rhs) const { return !(*this == rhs); }

        Int line = 0;   ///< One indexed line number. 0 if not defined
        Int column = 0; ///< One indexed *character (not byte)* column number. 0 if not defined
    };

    bool operator==(const ThisType& rhs) const
    {
        return severity == rhs.severity && stage == rhs.stage && text == rhs.text &&
               code == rhs.code && location == rhs.location;
    }
    bool operator!=(const ThisType& rhs) const { return !(*this == rhs); }

    Severity severity = Severity::Unknown; ///< The severity of error
    Stage stage = Stage::Compile;          ///< The stage the error came from
    TerminatedCharSlice text;              ///< The text of the error
    TerminatedCharSlice code;              ///< The compiler specific error code
    TerminatedCharSlice filePath;          ///< The path the error originated from
    Location location;                     ///< The location of the diagnostic in the filePath
};

/* Artifact diagnostics interface.

IArtifactDiagnostics are added as associated types on an IArtifact typically.
*/
class IArtifactDiagnostics : public IClonable
{
public:
    SLANG_COM_INTERFACE(
        0x91f9b857,
        0xcd6b,
        0x45ca,
        {0x8e, 0x3, 0x8f, 0xa3, 0x3c, 0x5c, 0xf0, 0x1a});

    typedef ArtifactDiagnostic Diagnostic;

    /// Get the diagnostic at the index
    SLANG_NO_THROW virtual const Diagnostic* SLANG_MCALL getAt(Index i) = 0;
    /// Get the amount of diangostics
    SLANG_NO_THROW virtual Count SLANG_MCALL getCount() = 0;
    /// Add a diagnostic
    SLANG_NO_THROW virtual void SLANG_MCALL add(const Diagnostic& diagnostic) = 0;
    /// Remove the diagnostic at the index
    SLANG_NO_THROW virtual void SLANG_MCALL removeAt(Index i) = 0;

    /// Get raw diagnostics information
    SLANG_NO_THROW virtual TerminatedCharSlice SLANG_MCALL getRaw() = 0;
    /// Set the raw diagnostic info
    SLANG_NO_THROW virtual void SLANG_MCALL setRaw(const CharSlice& slice) = 0;
    /// Append to the raw diagnostic
    SLANG_NO_THROW virtual void SLANG_MCALL appendRaw(const CharSlice& slice) = 0;

    /// Get the result for a compilation
    SLANG_NO_THROW virtual SlangResult SLANG_MCALL getResult() = 0;
    /// Set the result
    SLANG_NO_THROW virtual void SLANG_MCALL setResult(SlangResult res) = 0;

    /// Reset all state
    SLANG_NO_THROW virtual void SLANG_MCALL reset() = 0;

    /// Count the number of diagnostics which have 'severity' or greater
    SLANG_NO_THROW virtual Count SLANG_MCALL
    getCountAtLeastSeverity(Diagnostic::Severity severity) = 0;

    /// Get the number of diagnostics by severity
    SLANG_NO_THROW virtual Count SLANG_MCALL getCountBySeverity(Diagnostic::Severity severity) = 0;

    /// True if there are any diagnostics of severity or worse
    SLANG_NO_THROW virtual bool SLANG_MCALL hasOfAtLeastSeverity(Diagnostic::Severity severity) = 0;

    /// Stores in outCounts, the amount of diagnostics for the stage of each severity
    SLANG_NO_THROW virtual Count SLANG_MCALL getCountByStage(
        Diagnostic::Stage stage,
        Count outCounts[Int(Diagnostic::Severity::CountOf)]) = 0;

    /// Remove all diagnostics of the type
    SLANG_NO_THROW virtual void SLANG_MCALL removeBySeverity(Diagnostic::Severity severity) = 0;

    /// Add a note
    SLANG_NO_THROW virtual void SLANG_MCALL maybeAddNote(const CharSlice& in) = 0;

    /// If there are no error diagnostics, adds a generic error diagnostic
    SLANG_NO_THROW virtual void SLANG_MCALL requireErrorDiagnostic() = 0;

    /// Creates summary text and place in outBlob
    SLANG_NO_THROW virtual void SLANG_MCALL calcSummary(ISlangBlob** outBlob) = 0;
    /// Creates a simplified summary text and places it in out blob
    SLANG_NO_THROW virtual void SLANG_MCALL calcSimplifiedSummary(ISlangBlob** outBlob) = 0;
};

struct ShaderBindingRange;

struct UniformParamUsage
{
    // Parent CB or parameter block binding identity. Together
    // (parentSpace, parentBindingIndex) uniquely identifies which
    // uniform bearing parameter the byte ranges below belong to, so
    // multiple CBs in the same register space stay disambiguated.
    // Each entry in usedRanges has category=Uniform,
    // spaceIndex=parentSpace, registerIndex=byte offset within the
    // parent, and registerCount=byte size.
    UInt parentSpace;
    UInt parentBindingIndex;
    List<ShaderBindingRange> usedRanges;
    // True when the IR pass deliberately did not analyze this param
    // (unbounded uniform element type, etc.). usedRanges is empty in
    // that case and any byte query against this parent should yield
    // SLANG_E_NOT_AVAILABLE.
    bool isUntracked;
};

// Result of resolving a uniform bearing param's parent binding identity.
// found is false when the param carries neither a constant buffer nor a
// descriptor slot binding, in which case there is no parent to scope its
// byte ranges against.
struct UniformParentBinding
{
    UInt space;
    UInt bindingIndex;
    bool found;
};

// Choose the (space, bindingIndex) that identifies a uniform bearing
// param's parent constant buffer or parameter block. A constant buffer
// can expose a ConstantBuffer binding (D3D b register), a
// DescriptorTableSlot binding (Vulkan/SPIR-V descriptor), or both.
//
// The IR pass that records byte ranges and the reflection emitter that
// reports them must agree on this key, or the ranges silently fail to
// match. Both route through this one rule: prefer the ConstantBuffer
// binding when present, otherwise the DescriptorTableSlot binding,
// otherwise report not found. Presence is decided by the caller (the
// layout actually carries an offset for that category), never by
// treating a zero offset as absent: a zero offset is a real binding
// (register b0, descriptor binding 0).
inline UniformParentBinding selectUniformParentBinding(
    bool hasConstantBuffer,
    UInt constantBufferSpace,
    UInt constantBufferIndex,
    bool hasDescriptorTableSlot,
    UInt descriptorTableSlotSpace,
    UInt descriptorTableSlotIndex)
{
    if (hasConstantBuffer)
        return {constantBufferSpace, constantBufferIndex, true};
    if (hasDescriptorTableSlot)
        return {descriptorTableSlotSpace, descriptorTableSlotIndex, true};
    return {0, 0, false};
}

class IArtifactPostEmitMetadata : public slang::IMetadata
{
public:
    SLANG_COM_INTERFACE(
        0x5d03bce9,
        0xafb1,
        0x4fc8,
        {0xa4, 0x6f, 0x3c, 0xe0, 0x7b, 0x6, 0x1b, 0x1b});

    /// Get the binding ranges
    SLANG_NO_THROW virtual Slice<ShaderBindingRange> SLANG_MCALL getUsedBindingRanges() = 0;

    /// Get the list of functions that were exported in the linked IR
    SLANG_NO_THROW virtual Slice<String> SLANG_MCALL getExportedFunctionMangledNames() = 0;

    /// Get the debug build identifier for a base and debug spirv pair
    SLANG_NO_THROW virtual const char* SLANG_MCALL getDebugBuildIdentifier() = 0;

    /// Per uniform bearing parameter, a scoped list of byte ranges that
    /// reachable code touched. See UniformParamUsage for the scoping
    /// contract.
    SLANG_NO_THROW virtual Slice<UniformParamUsage> SLANG_MCALL getUniformParamUsage() = 0;
};

} // namespace Slang

#endif
