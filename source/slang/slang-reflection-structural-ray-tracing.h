#pragma once

#include "core/slang-basic.h"
#include "slang-structural-ray-tracing.h"

namespace Slang
{

class ProgramLayout;
class Type;
class TypeLayout;

/// Describes one concrete structural stage and the physical entry-point name generated for it.
class StructuralRayTracingStageReflection : public RefObject
{
public:
    StructuralRayTracingStageKind stageKind = StructuralRayTracingStageKind::Count;
    Type* type = nullptr;
    String entryPointName;
};

class StructuralRayTracingHitGroupReflection : public RefObject
{
public:
    /// Identifies this group within the hit-group function table for its payload partition.
    /// Schema-free declaration catalogue entries retain -1 because no schema selected a slot.
    Index functionIndex = -1;
    Type* groupType = nullptr;
    Type* contextType = nullptr;
    Type* recordType = nullptr;
    /// Layout the host uses for the application data stored after this group's record header.
    TypeLayout* recordTypeLayout = nullptr;
    Type* primitiveType = nullptr;
    Type* intersectionAttributesType = nullptr;
    /// True when whole-program open-section completion selected this group.
    bool isLinked = false;
    /// Exact target symbol installed in this group's closest-hit table slot.
    ///
    /// This remains populated for a synthesized Metal no-op even though `closestHit` is null and
    /// therefore preserves the source-level `NoClosestHit` contract.
    String closestHitEntryPointName;
    RefPtr<StructuralRayTracingStageReflection> closestHit;
    RefPtr<StructuralRayTracingStageReflection> anyHit;
    RefPtr<StructuralRayTracingStageReflection> intersection;
};

class StructuralRayTracingMissShaderReflection : public RefObject
{
public:
    /// Identifies this shader within the miss function table for its payload partition.
    /// Schema-free declaration catalogue entries retain -1 because no schema selected a slot.
    Index functionIndex = -1;
    Type* shaderType = nullptr;
    Type* contextType = nullptr;
    Type* recordType = nullptr;
    /// Layout the host uses for the application data stored after this shader's record header.
    TypeLayout* recordTypeLayout = nullptr;
    /// True when whole-program open-section completion selected this shader.
    bool isLinked = false;
    RefPtr<StructuralRayTracingStageReflection> miss;
};

class StructuralRayTracingCallableShaderReflection : public RefObject
{
public:
    /// Identifies this shader within the schema-wide callable function table.
    /// Schema-free declaration catalogue entries retain -1 because no schema selected a slot.
    Index functionIndex = -1;
    Type* shaderType = nullptr;
    Type* contextType = nullptr;
    Type* recordType = nullptr;
    /// Layout the host uses for the application data stored after this shader's record header.
    TypeLayout* recordTypeLayout = nullptr;
    Type* callableDataType = nullptr;
    /// True when whole-program open-section completion selected this shader.
    bool isLinked = false;
    RefPtr<StructuralRayTracingStageReflection> callable;
};

enum class StructuralRayTracingMetalIntersectionFunctionImplementationKind
{
    ExportedFunction,
    OpaqueTriangle,
    OpaqueCurve,
};

/// Describes one installed entry in a payload partition's Metal intersection-function table.
class StructuralRayTracingIntersectionFunctionReflection : public RefObject
{
public:
    Index intersectionFunctionTableIndex = -1;
    StructuralRayTracingMetalCandidateKind geometryKind =
        StructuralRayTracingMetalCandidateKind::Count;
    StructuralRayTracingMetalIntersectionFunctionImplementationKind implementationKind =
        StructuralRayTracingMetalIntersectionFunctionImplementationKind::ExportedFunction;
    String entryPointName;
};

class StructuralRayTracingPayloadReflection : public RefObject
{
public:
    Type* payloadType = nullptr;
    /// Target layout of the payload value carried by this partition.
    TypeLayout* typeLayout = nullptr;
    /// Native host pipeline ABI requirement for this payload, in bytes.
    size_t nativePayloadSize = 0;

    /// Number of physical slots required by this payload's fixed-index Metal IFT.
    Index intersectionFunctionTableSize = 0;
    List<RefPtr<StructuralRayTracingIntersectionFunctionReflection>> intersectionFunctions;

    /// Entries retain their source-list order after filtering for this payload type.
    List<RefPtr<StructuralRayTracingHitGroupReflection>> hitGroups;
    List<RefPtr<StructuralRayTracingMissShaderReflection>> missShaders;
};

class StructuralRayTracingProgramSchemaReflection : public RefObject
{
public:
    struct DescriptorResource
    {
        StructuralRayTracingDescriptorResourceKind kind =
            StructuralRayTracingDescriptorResourceKind::Count;
        /// Payload index for per-payload resources, or -1 for schema-wide resources.
        Index payloadIndex = -1;
        String name;
    };

    /// Stable compiler-owned identity shared with target metadata and generated Metal symbols.
    ///
    /// This can differ from the spelling accepted by `findTraceProgramSchema`, notably for a
    /// specialized generic schema whose stable identity includes a mangled specialization suffix.
    String name;
    Type* schemaType = nullptr;
    Type* traceContextType = nullptr;
    /// These flags describe the source schema; the entry lists below are already link-finalized.
    bool hitGroupSectionOpen = false;
    bool missShaderSectionOpen = false;
    bool callableShaderSectionOpen = false;

    /// Fixed Metal record-buffer strides in bytes, including each record's 16-byte header.
    /// These remain zero for targets without a compiler-owned structural record buffer.
    size_t hitRecordStride = 0;
    size_t missRecordStride = 0;
    size_t callableRecordStride = 0;
    /// Native hit-attribute ABI requirement, including any target minimum, in bytes.
    size_t maxNativeHitAttributeSize = 0;
    /// Fixed compiler-owned Metal record header, or zero on other targets.
    size_t metalRecordHeaderSize = 0;

    /// Payloads are ordered by first use in hit groups, then first use in miss shaders.
    List<RefPtr<StructuralRayTracingPayloadReflection>> payloads;
    List<RefPtr<StructuralRayTracingCallableShaderReflection>> callableShaders;

    /// Logical Metal descriptor resources. Their semantic kind and payload partition determine
    /// the physical argument-buffer index through the shared compiler ABI mapping.
    /// Other targets have none.
    List<DescriptorResource> descriptorResources;
};

/// Owns structural entry declarations that are visible to one ordinary component program.
///
/// These objects deliberately do not belong to a trace-program schema. They let a host discover
/// the concrete hit, miss, and callable declarations from which it may construct schemas or native
/// pipeline records without requesting schema finalization. Consequently their function indices
/// remain -1, `isLinked` remains false, and they never contain schema-specific Metal symbols.
class StructuralRayTracingEntryCatalogueReflection : public RefObject
{
public:
    List<RefPtr<StructuralRayTracingHitGroupReflection>> hitGroups;
    List<RefPtr<StructuralRayTracingMissShaderReflection>> missShaders;
    List<RefPtr<StructuralRayTracingCallableShaderReflection>> callableShaders;
};

class StructuralRayTracingReflectionData : public RefObject
{
public:
    /// The schema-free catalogue has its own entry objects. A later schema query can assign slots
    /// and target symbols without mutating the declarations previously returned to the host.
    RefPtr<StructuralRayTracingEntryCatalogueReflection> entryCatalogue;
    List<RefPtr<StructuralRayTracingProgramSchemaReflection>> programSchemas;
};

StructuralRayTracingEntryCatalogueReflection* getStructuralRayTracingEntryCatalogueReflection(
    ProgramLayout* programLayout);

StructuralRayTracingProgramSchemaReflection* findStructuralRayTracingProgramSchemaReflection(
    ProgramLayout* programLayout,
    const char* name);

} // namespace Slang
