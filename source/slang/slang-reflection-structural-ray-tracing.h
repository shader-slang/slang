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
    Index functionIndex = 0;
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
    Index functionIndex = 0;
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
    Index functionIndex = 0;
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

    /// Number of physical slots required by this payload's sparse Metal IFT.
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

    /// Payloads are ordered by first use in hit groups, then first use in miss shaders.
    List<RefPtr<StructuralRayTracingPayloadReflection>> payloads;
    List<RefPtr<StructuralRayTracingCallableShaderReflection>> callableShaders;

    /// Metal descriptor fields in their physical argument-buffer order. Other targets have none.
    List<DescriptorResource> descriptorResources;
};

class StructuralRayTracingReflectionData : public RefObject
{
public:
    List<RefPtr<StructuralRayTracingProgramSchemaReflection>> programSchemas;
};

StructuralRayTracingProgramSchemaReflection* findStructuralRayTracingProgramSchemaReflection(
    ProgramLayout* programLayout,
    const char* name);

} // namespace Slang
