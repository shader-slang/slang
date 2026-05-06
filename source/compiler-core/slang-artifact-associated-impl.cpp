// slang-artifact-associated-impl.cpp
#include "slang-artifact-associated-impl.h"

#include "../core/slang-array-view.h"
#include "../core/slang-char-util.h"
#include "../core/slang-file-system.h"
#include "../core/slang-io.h"
#include "../core/slang-type-text-util.h"
#include "slang-artifact-diagnostic-util.h"

namespace Slang
{

/* !!!!!!!!!!!!!!!!!!!!!!!!!!! ArtifactDiagnostics !!!!!!!!!!!!!!!!!!!!!!!!!!! */

ArtifactDiagnostics::ArtifactDiagnostics(const ThisType& rhs)
    : ComBaseObject()
    , m_result(rhs.m_result)
    , m_diagnostics(rhs.m_diagnostics)
    , m_raw(rhs.m_raw.getLength() + 1)
{
    // We need to be careful with raw, we want a new *copy* not a non atomic ref counting
    // In initialization we should have enough space
    m_raw.append(rhs.m_raw.getUnownedSlice());

    // Reallocate all the strings
    for (auto& diagnostic : m_diagnostics)
    {
        diagnostic.filePath = m_allocator.allocate(diagnostic.filePath);
        diagnostic.code = m_allocator.allocate(diagnostic.code);
        diagnostic.text = m_allocator.allocate(diagnostic.text);
    }
}

void* ArtifactDiagnostics::clone(const Guid& guid)
{
    ThisType* copy = new ThisType(*this);
    if (auto ptr = copy->castAs(guid))
    {
        return ptr;
    }
    // If the cast fails, we delete the item.
    delete copy;
    return nullptr;
}

void* ArtifactDiagnostics::getInterface(const Guid& guid)
{
    if (guid == ISlangUnknown::getTypeGuid() || guid == ICastable::getTypeGuid() ||
        guid == IClonable::getTypeGuid() || guid == IArtifactDiagnostics::getTypeGuid())
    {
        return static_cast<IArtifactDiagnostics*>(this);
    }
    return nullptr;
}

void* ArtifactDiagnostics::getObject(const Guid& guid)
{
    SLANG_UNUSED(guid);
    return nullptr;
}

void* ArtifactDiagnostics::castAs(const Guid& guid)
{
    if (auto intf = getInterface(guid))
    {
        return intf;
    }
    return getObject(guid);
}

void ArtifactDiagnostics::reset()
{
    m_diagnostics.clear();
    m_raw.clear();
    m_result = SLANG_OK;

    m_allocator.deallocateAll();
}

void ArtifactDiagnostics::add(const Diagnostic& inDiagnostic)
{
    Diagnostic diagnostic(inDiagnostic);

    diagnostic.text = m_allocator.allocate(inDiagnostic.text);
    diagnostic.code = m_allocator.allocate(inDiagnostic.code);
    diagnostic.filePath = m_allocator.allocate(inDiagnostic.filePath);

    m_diagnostics.add(diagnostic);
}

void ArtifactDiagnostics::setRaw(const CharSlice& slice)
{
    m_raw.clear();
    m_raw << asStringSlice(slice);
}

void ArtifactDiagnostics::appendRaw(const CharSlice& slice)
{
    if (m_raw.getLength() && m_raw[m_raw.getLength() - 1] != '\n')
    {
        m_raw.appendChar('\n');
    }
    m_raw << asStringSlice(slice);
}

Count ArtifactDiagnostics::getCountAtLeastSeverity(Diagnostic::Severity severity)
{
    Index count = 0;
    for (const auto& msg : m_diagnostics)
    {
        count += Index(Index(msg.severity) >= Index(severity));
    }
    return count;
}

Count ArtifactDiagnostics::getCountBySeverity(Diagnostic::Severity severity)
{
    Index count = 0;
    for (const auto& msg : m_diagnostics)
    {
        count += Index(msg.severity == severity);
    }
    return count;
}

bool ArtifactDiagnostics::hasOfAtLeastSeverity(Diagnostic::Severity severity)
{
    for (const auto& msg : m_diagnostics)
    {
        if (Index(msg.severity) >= Index(severity))
        {
            return true;
        }
    }
    return false;
}

Count ArtifactDiagnostics::getCountByStage(
    Diagnostic::Stage stage,
    Count outCounts[Int(Diagnostic::Severity::CountOf)])
{
    Int count = 0;
    ::memset(outCounts, 0, sizeof(Index) * Int(Diagnostic::Severity::CountOf));
    for (const auto& diagnostic : m_diagnostics)
    {
        if (diagnostic.stage == stage)
        {
            count++;
            outCounts[Index(diagnostic.severity)]++;
        }
    }
    return count++;
}

void ArtifactDiagnostics::removeBySeverity(Diagnostic::Severity severity)
{
    Index count = m_diagnostics.getCount();
    for (Index i = 0; i < count; ++i)
    {
        if (m_diagnostics[i].severity == severity)
        {
            m_diagnostics.removeAt(i);
            i--;
            count--;
        }
    }
}

void ArtifactDiagnostics::maybeAddNote(const CharSlice& in)
{
    ArtifactDiagnosticUtil::maybeAddNote(asStringSlice(in), this);
}

void ArtifactDiagnostics::requireErrorDiagnostic()
{
    // If we find an error, we don't need to add a generic diagnostic
    for (const auto& msg : m_diagnostics)
    {
        if (Index(msg.severity) >= Index(Diagnostic::Severity::Error))
        {
            return;
        }
    }

    Diagnostic diagnostic;
    diagnostic.severity = Diagnostic::Severity::Error;
    diagnostic.text = m_allocator.allocate(m_raw);

    // Add the diagnostic
    m_diagnostics.add(diagnostic);
}

/* static */ UnownedStringSlice _getSeverityText(ArtifactDiagnostic::Severity severity)
{
    typedef ArtifactDiagnostic::Severity Severity;
    switch (severity)
    {
    default:
        return UnownedStringSlice::fromLiteral("Unknown");
    case Severity::Info:
        return UnownedStringSlice::fromLiteral("Info");
    case Severity::Warning:
        return UnownedStringSlice::fromLiteral("Warning");
    case Severity::Error:
        return UnownedStringSlice::fromLiteral("Error");
    }
}

static void _appendCounts(
    const Index counts[Int(ArtifactDiagnostic::Severity::CountOf)],
    StringBuilder& out)
{
    typedef ArtifactDiagnostic::Severity Severity;

    for (Index i = 0; i < Int(Severity::CountOf); i++)
    {
        if (counts[i] > 0)
        {
            out << _getSeverityText(Severity(i)) << "(" << counts[i] << ") ";
        }
    }
}

static void _appendSimplified(
    const Index counts[Int(ArtifactDiagnostic::Severity::CountOf)],
    StringBuilder& out)
{
    typedef ArtifactDiagnostic::Severity Severity;
    for (Index i = 0; i < Int(Severity::CountOf); i++)
    {
        if (counts[i] > 0)
        {
            out << _getSeverityText(Severity(i)) << " ";
        }
    }
}

void ArtifactDiagnostics::calcSummary(ISlangBlob** outBlob)
{
    StringBuilder buf;

    Index counts[Int(Diagnostic::Severity::CountOf)];
    if (getCountByStage(Diagnostic::Stage::Compile, counts) > 0)
    {
        buf << "Compile: ";
        _appendCounts(counts, buf);
        buf << "\n";
    }
    if (getCountByStage(Diagnostic::Stage::Link, counts) > 0)
    {
        buf << "Link: ";
        _appendCounts(counts, buf);
        buf << "\n";
    }

    *outBlob = StringBlob::moveCreate(buf).detach();
}

void ArtifactDiagnostics::calcSimplifiedSummary(ISlangBlob** outBlob)
{
    StringBuilder buf;

    Index counts[Int(Diagnostic::Severity::CountOf)];
    if (getCountByStage(Diagnostic::Stage::Compile, counts) > 0)
    {
        buf << "Compile: ";
        _appendSimplified(counts, buf);
        buf << "\n";
    }
    if (getCountByStage(Diagnostic::Stage::Link, counts) > 0)
    {
        buf << "Link: ";
        _appendSimplified(counts, buf);
        buf << "\n";
    }

    *outBlob = StringBlob::moveCreate(buf).detach();
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ArtifactPostEmitMetadata !!!!!!!!!!!!!!!!!!!!!!!!!!! */

void* ArtifactPostEmitMetadata::getInterface(const Guid& guid)
{
    if (guid == ISlangUnknown::getTypeGuid() || guid == ICastable::getTypeGuid() ||
        guid == IArtifactPostEmitMetadata::getTypeGuid())
    {
        return static_cast<IArtifactPostEmitMetadata*>(this);
    }
    if (guid == slang::IMetadata::getTypeGuid())
        return static_cast<slang::IMetadata*>(this);
    if (guid == slang::ICoverageTracingMetadata::getTypeGuid())
    {
        return static_cast<slang::ICoverageTracingMetadata*>(this);
    }
    if (guid == slang::ISyntheticResourceMetadata::getTypeGuid())
    {
        return static_cast<slang::ISyntheticResourceMetadata*>(this);
    }
    if (guid == slang::ICooperativeTypesMetadata::getTypeGuid())
        return static_cast<slang::ICooperativeTypesMetadata*>(this);
    return nullptr;
}

void* ArtifactPostEmitMetadata::getObject(const Guid& uuid)
{
    if (uuid == getTypeGuid())
    {
        return this;
    }
    return nullptr;
}

void* ArtifactPostEmitMetadata::castAs(const Guid& guid)
{
    if (auto ptr = getInterface(guid))
    {
        return ptr;
    }
    return getObject(guid);
}

Slice<ShaderBindingRange> ArtifactPostEmitMetadata::getUsedBindingRanges()
{
    return Slice<ShaderBindingRange>(m_usedBindings.getBuffer(), m_usedBindings.getCount());
}

Slice<String> ArtifactPostEmitMetadata::getExportedFunctionMangledNames()
{
    return Slice<String>(
        m_exportedFunctionMangledNames.getBuffer(),
        m_exportedFunctionMangledNames.getCount());
}

SlangResult ArtifactPostEmitMetadata::isParameterLocationUsed(
    SlangParameterCategory category,
    SlangUInt spaceIndex,
    SlangUInt registerIndex,
    bool& outUsed)
{
    for (const auto& range : getUsedBindingRanges())
    {
        if (range.containsBinding((slang::ParameterCategory)category, spaceIndex, registerIndex))
        {
            outUsed = true;
            return SLANG_OK;
        }
    }

    outUsed = false;
    return SLANG_OK;
}

const char* ArtifactPostEmitMetadata::getDebugBuildIdentifier()
{
    return m_debugBuildIdentifier.getBuffer();
}

uint32_t ArtifactPostEmitMetadata::getCounterCount()
{
    return (uint32_t)m_coverageEntries.getCount();
}

// ABI versioning: minimum `structSize` we accept for the v1 shape of
// `CoverageEntryInfo` / `CoverageBufferInfo`. These constants are
// frozen at the offsets of the LAST field shipped in v1 (file+line for
// the entry struct, space+binding for the buffer struct) and **must
// not be updated when fields are added later** — that's the whole
// point of `structSize` versioning. Newer callers with larger structs
// pass through; older callers feeding a future v2+ implementation
// will likewise be accepted, with the impl writing only the v1 fields
// the caller has space for. (When v2 fields are added, write them
// conditionally based on `outInfo->structSize >= offsetof(struct, newField)
// + sizeof(newField)`.)
static constexpr size_t kCoverageEntryInfoV1MinSize =
    offsetof(slang::CoverageEntryInfo, line) + sizeof(uint32_t);
static constexpr size_t kCoverageBufferInfoV1MinSize =
    offsetof(slang::CoverageBufferInfo, binding) + sizeof(int32_t);
static constexpr size_t kSyntheticResourceInfoV1MinSize =
    offsetof(slang::SyntheticResourceInfo, featureTag) + sizeof(const char*);
static constexpr size_t kSyntheticResourceDescriptorBindingInfoV1MinSize =
    offsetof(slang::SyntheticResourceDescriptorBindingInfo, binding) + sizeof(int32_t);
static constexpr size_t kSyntheticResourceUniformBindingInfoV1MinSize =
    offsetof(slang::SyntheticResourceUniformBindingInfo, uniformStride) + sizeof(int32_t);

SlangResult ArtifactPostEmitMetadata::getEntryInfo(
    uint32_t index,
    slang::CoverageEntryInfo* outInfo)
{
    if (!outInfo)
        return SLANG_E_INVALID_ARG;
    if (outInfo->structSize < kCoverageEntryInfoV1MinSize)
        return SLANG_E_INVALID_ARG;
    if (index >= (uint32_t)m_coverageEntries.getCount())
        return SLANG_E_INVALID_ARG;
    auto& entry = m_coverageEntries[index];
    // Surface an empty source-file path as a null pointer so callers
    // can branch on attributability without separately checking the
    // length. An empty file string would otherwise look like a
    // valid (but empty) path to consumers.
    outInfo->file = entry.file.getLength() ? entry.file.getBuffer() : nullptr;
    outInfo->line = entry.line;
    return SLANG_OK;
}

SlangResult ArtifactPostEmitMetadata::getBufferInfo(slang::CoverageBufferInfo* outInfo)
{
    if (!outInfo)
        return SLANG_E_INVALID_ARG;
    if (outInfo->structSize < kCoverageBufferInfoV1MinSize)
        return SLANG_E_INVALID_ARG;
    outInfo->space = m_coverageBufferSpace;
    outInfo->binding = m_coverageBufferBinding;
    return SLANG_OK;
}

uint32_t ArtifactPostEmitMetadata::getResourceCount()
{
    return (uint32_t)m_syntheticResources.getCount();
}

SlangResult ArtifactPostEmitMetadata::findResourceIndexByID(uint32_t id, uint32_t* outIndex)
{
    if (!outIndex)
        return SLANG_E_INVALID_ARG;

    for (Index i = 0; i < m_syntheticResources.getCount(); ++i)
    {
        if (m_syntheticResources[i].id == id)
        {
            *outIndex = uint32_t(i);
            return SLANG_OK;
        }
    }

    return SLANG_E_INVALID_ARG;
}

SlangResult ArtifactPostEmitMetadata::getResourceInfo(
    uint32_t index,
    slang::SyntheticResourceInfo* outInfo)
{
    if (!outInfo)
        return SLANG_E_INVALID_ARG;
    if (outInfo->structSize < kSyntheticResourceInfoV1MinSize)
        return SLANG_E_INVALID_ARG;
    if (index >= (uint32_t)m_syntheticResources.getCount())
        return SLANG_E_INVALID_ARG;

    auto& record = m_syntheticResources[index];
    outInfo->id = record.id;
    outInfo->bindingType = record.bindingType;
    outInfo->arraySize = record.arraySize;
    outInfo->scope = record.scope;
    outInfo->access = record.access;
    outInfo->entryPointIndex = record.entryPointIndex;
    outInfo->space = record.space;
    outInfo->binding = record.binding;
    outInfo->uniformOffset = record.uniformOffset;
    outInfo->uniformStride = record.uniformStride;
    outInfo->debugName = record.debugName.getLength() ? record.debugName.getBuffer() : nullptr;
    outInfo->featureTag = record.featureTag.getLength() ? record.featureTag.getBuffer() : nullptr;
    return SLANG_OK;
}

SlangResult ArtifactPostEmitMetadata::getResourceDescriptorBindingInfo(
    uint32_t index,
    slang::SyntheticResourceDescriptorBindingInfo* outInfo)
{
    if (!outInfo)
        return SLANG_E_INVALID_ARG;
    if (outInfo->structSize < kSyntheticResourceDescriptorBindingInfoV1MinSize)
        return SLANG_E_INVALID_ARG;
    if (index >= (uint32_t)m_syntheticResources.getCount())
        return SLANG_E_INVALID_ARG;

    auto& record = m_syntheticResources[index];
    outInfo->space = record.space;
    outInfo->binding = record.binding;
    return SLANG_OK;
}

SlangResult ArtifactPostEmitMetadata::getResourceUniformBindingInfo(
    uint32_t index,
    slang::SyntheticResourceUniformBindingInfo* outInfo)
{
    if (!outInfo)
        return SLANG_E_INVALID_ARG;
    if (outInfo->structSize < kSyntheticResourceUniformBindingInfoV1MinSize)
        return SLANG_E_INVALID_ARG;
    if (index >= (uint32_t)m_syntheticResources.getCount())
        return SLANG_E_INVALID_ARG;

    auto& record = m_syntheticResources[index];
    outInfo->uniformOffset = record.uniformOffset;
    outInfo->uniformStride = record.uniformStride;
    return SLANG_OK;
}

SlangUInt ArtifactPostEmitMetadata::getCooperativeMatrixTypeCount()
{
    return SlangUInt(m_cooperativeMatrixTypes.getCount());
}

SlangResult ArtifactPostEmitMetadata::getCooperativeMatrixTypeByIndex(
    SlangUInt index,
    slang::CooperativeMatrixType* outType)
{
    if (!outType)
        return SLANG_E_INVALID_ARG;

    if (index >= SlangUInt(m_cooperativeMatrixTypes.getCount()))
        return SLANG_E_INVALID_ARG;

    *outType = m_cooperativeMatrixTypes[Index(index)];
    return SLANG_OK;
}

SlangUInt ArtifactPostEmitMetadata::getCooperativeMatrixCombinationCount()
{
    return SlangUInt(m_cooperativeMatrixCombinations.getCount());
}

SlangResult ArtifactPostEmitMetadata::getCooperativeMatrixCombinationByIndex(
    SlangUInt index,
    slang::CooperativeMatrixCombination* outCombination)
{
    if (!outCombination)
        return SLANG_E_INVALID_ARG;

    if (index >= SlangUInt(m_cooperativeMatrixCombinations.getCount()))
        return SLANG_E_INVALID_ARG;

    *outCombination = m_cooperativeMatrixCombinations[Index(index)];
    return SLANG_OK;
}

SlangUInt ArtifactPostEmitMetadata::getCooperativeVectorTypeCount()
{
    return SlangUInt(m_cooperativeVectorTypes.getCount());
}

SlangResult ArtifactPostEmitMetadata::getCooperativeVectorTypeByIndex(
    SlangUInt index,
    slang::CooperativeVectorTypeUsageInfo* outType)
{
    if (!outType)
        return SLANG_E_INVALID_ARG;

    if (index >= SlangUInt(m_cooperativeVectorTypes.getCount()))
        return SLANG_E_INVALID_ARG;

    *outType = m_cooperativeVectorTypes[Index(index)];
    return SLANG_OK;
}

SlangUInt ArtifactPostEmitMetadata::getCooperativeVectorCombinationCount()
{
    return SlangUInt(m_cooperativeVectorCombinations.getCount());
}

SlangResult ArtifactPostEmitMetadata::getCooperativeVectorCombinationByIndex(
    SlangUInt index,
    slang::CooperativeVectorCombination* outCombination)
{
    if (!outCombination)
        return SLANG_E_INVALID_ARG;

    if (index >= SlangUInt(m_cooperativeVectorCombinations.getCount()))
        return SLANG_E_INVALID_ARG;

    *outCombination = m_cooperativeVectorCombinations[Index(index)];
    return SLANG_OK;
}


} // namespace Slang
