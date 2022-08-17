// slang-artifact-associated-impl.h
#ifndef SLANG_ARTIFACT_ASSOCIATED_IMPL_H
#define SLANG_ARTIFACT_ASSOCIATED_IMPL_H

#include "../../slang-com-helper.h"
#include "../../slang-com-ptr.h"

#include "../core/slang-com-object.h"
#include "../core/slang-memory-arena.h"

#include "slang-artifact-associated.h"

#include "slang-artifact-util.h"

namespace Slang
{

class DiagnosticsImpl : public ComBaseObject, public IDiagnostics
{
public:
    SLANG_COM_BASE_IUNKNOWN_ALL

    // ICastable
    SLANG_NO_THROW void* SLANG_MCALL castAs(const Guid& guid) SLANG_OVERRIDE;
    // IDiagnostic
    SLANG_NO_THROW virtual const Diagnostic* SLANG_MCALL getAt(Index i) SLANG_OVERRIDE { return &m_diagnostics[i]; }
    SLANG_NO_THROW virtual Count SLANG_MCALL getCount() SLANG_OVERRIDE { return m_diagnostics.getCount(); }
    SLANG_NO_THROW virtual void SLANG_MCALL add(const Diagnostic& diagnostic) SLANG_OVERRIDE; 
    SLANG_NO_THROW virtual void SLANG_MCALL removeAt(Index i) SLANG_OVERRIDE { m_diagnostics.removeAt(i); }
    SLANG_NO_THROW virtual SlangResult SLANG_MCALL getResult() SLANG_OVERRIDE { return m_result; }
    SLANG_NO_THROW virtual void SLANG_MCALL setResult(SlangResult res) SLANG_OVERRIDE { m_result = res; }
    SLANG_NO_THROW virtual void SLANG_MCALL setRaw(const ZeroTerminatedCharSlice& slice) SLANG_OVERRIDE;
    SLANG_NO_THROW virtual ZeroTerminatedCharSlice SLANG_MCALL getRaw() SLANG_OVERRIDE { return m_raw; }
    SLANG_NO_THROW virtual void SLANG_MCALL reset() SLANG_OVERRIDE;
    SLANG_NO_THROW virtual Count SLANG_MCALL getCountAtLeastSeverity(Severity severity) SLANG_OVERRIDE;
    SLANG_NO_THROW virtual Count SLANG_MCALL getCountBySeverity(Severity severity) SLANG_OVERRIDE;
    SLANG_NO_THROW virtual bool SLANG_MCALL hasOfAtLeastSeverity(Severity severity) SLANG_OVERRIDE;
    SLANG_NO_THROW virtual Count SLANG_MCALL getCountByStage(Stage stage, Count outCounts[Int(Severity::CountOf)]) SLANG_OVERRIDE;
    SLANG_NO_THROW virtual void SLANG_MCALL removeBySeverity(Severity severity) SLANG_OVERRIDE;
    SLANG_NO_THROW virtual void SLANG_MCALL maybeAddNote(const ZeroTerminatedCharSlice& in) SLANG_OVERRIDE;
    SLANG_NO_THROW virtual void SLANG_MCALL requireErrorDiagnostic() SLANG_OVERRIDE;
    SLANG_NO_THROW virtual void SLANG_MCALL appendSummary(ISlangBlob** outBlob) SLANG_OVERRIDE;
    SLANG_NO_THROW virtual void SLANG_MCALL appendSimplifiedSummary(ISlangBlob** outBlob) SLANG_OVERRIDE;

protected:
    void* getInterface(const Guid& uuid);
    void* getObject(const Guid& uuid);

    ArtifactSliceAllocator m_allocator;

    List<Diagnostic> m_diagnostics;
    SlangResult m_result = SLANG_OK;
    
    // Raw diagnostics
    ZeroTerminatedCharSlice m_raw;
};

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!! PostEmitMetadataImpl !!!!!!!!!!!!!!!!!!!!!!!!!! */

struct ShaderBindingRange
{
    slang::ParameterCategory category = slang::ParameterCategory::None;
    UInt spaceIndex = 0;
    UInt registerIndex = 0;
    UInt registerCount = 0; // 0 for unsized

    bool isInfinite() const
    {
        return registerCount == 0;
    }

    bool containsBinding(slang::ParameterCategory _category, UInt _spaceIndex, UInt _registerIndex) const
    {
        return category == _category
            && spaceIndex == _spaceIndex
            && registerIndex <= _registerIndex
            && (isInfinite() || registerCount + registerIndex > _registerIndex);
    }

    bool intersectsWith(const ShaderBindingRange& other) const
    {
        if (category != other.category || spaceIndex != other.spaceIndex)
            return false;

        const bool leftIntersection = (registerIndex < other.registerIndex + other.registerCount) || other.isInfinite();
        const bool rightIntersection = (other.registerIndex < registerIndex + registerCount) || isInfinite();

        return leftIntersection && rightIntersection;
    }

    bool adjacentTo(const ShaderBindingRange& other) const
    {
        if (category != other.category || spaceIndex != other.spaceIndex)
            return false;

        const bool leftIntersection = (registerIndex <= other.registerIndex + other.registerCount) || other.isInfinite();
        const bool rightIntersection = (other.registerIndex <= registerIndex + registerCount) || isInfinite();

        return leftIntersection && rightIntersection;
    }

    void mergeWith(const ShaderBindingRange other)
    {
        UInt newRegisterIndex = Math::Min(registerIndex, other.registerIndex);

        if (other.isInfinite())
            registerCount = 0;
        else if (!isInfinite())
            registerCount = Math::Max(registerIndex + registerCount, other.registerIndex + other.registerCount) - newRegisterIndex;

        registerIndex = newRegisterIndex;
    }

    static bool isUsageTracked(slang::ParameterCategory category)
    {
        switch (category)
        {
        case slang::ConstantBuffer:
        case slang::ShaderResource:
        case slang::UnorderedAccess:
        case slang::SamplerState:
            return true;
        default:
            return false;
        }
    }
};

class PostEmitMetadataImpl : public ComBaseObject, public IPostEmitMetadata
{
public:
    SLANG_CLASS_GUID(0x6f82509f, 0xe48b, 0x4b83, { 0xa3, 0x84, 0x5d, 0x70, 0x83, 0x19, 0x83, 0xcc })

    SLANG_COM_BASE_IUNKNOWN_ALL

    // ICastable
    SLANG_NO_THROW void* SLANG_MCALL castAs(const Guid& guid) SLANG_OVERRIDE;
    
    // IPostEmitMetadata
    SLANG_NO_THROW virtual Slice<ShaderBindingRange> SLANG_MCALL getUsedBindingRanges() SLANG_OVERRIDE;
    
    void* getInterface(const Guid& uuid);
    void* getObject(const Guid& uuid);

    List<ShaderBindingRange> m_usedBindings;
};

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ArtifactDiagnosticsUtil !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

struct ArtifactDiagnosticsUtil
{
    typedef IDiagnostics::Severity Severity;
    typedef IDiagnostics::Diagnostic ArtifactDiagnostic;

        /// Given severity return as text
    static UnownedStringSlice getSeverityText(Severity severity);

        /// Given a path, that holds line number and potentially column number in () after path, writes result into outDiagnostic
    static SlangResult splitPathLocation(ArtifactSliceAllocator& allocator, const UnownedStringSlice& pathLocation, ArtifactDiagnostic& outDiagnostic);

        /// Split the line (separated by :), where a path is at pathIndex 
    static SlangResult splitColonDelimitedLine(const UnownedStringSlice& line, Int pathIndex, List<UnownedStringSlice>& outSlices);

    typedef SlangResult(*LineParser)(ArtifactSliceAllocator& allocator, const UnownedStringSlice& line, List<UnownedStringSlice>& lineSlices, ArtifactDiagnostic& outDiagnostic);

        /// Given diagnostics in inText that are colon delimited, use lineParser to do per line parsing.
    static SlangResult parseColonDelimitedDiagnostics(ArtifactSliceAllocator& allocator, const UnownedStringSlice& inText, Int pathIndex, LineParser lineParser, List<ArtifactDiagnostic>& outDiagnostics);

        /// Maybe add a note
    static void maybeAddNote(ArtifactSliceAllocator& allocator, const UnownedStringSlice& in, List<ArtifactDiagnostic>& ioDiagnostics);

};

} // namespace Slang

#endif
