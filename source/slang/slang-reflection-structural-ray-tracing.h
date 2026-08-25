#pragma once

#include "core/slang-basic.h"
#include "slang-structural-ray-tracing.h"

namespace Slang
{

class ProgramLayout;
class Type;

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
    int64_t slot = 0;
    Type* groupType = nullptr;
    Type* contextType = nullptr;
    Type* recordType = nullptr;
    Type* primitiveType = nullptr;
    Type* intersectionAttributesType = nullptr;
    RefPtr<StructuralRayTracingStageReflection> closestHit;
    RefPtr<StructuralRayTracingStageReflection> anyHit;
    RefPtr<StructuralRayTracingStageReflection> intersection;
};

class StructuralRayTracingMissGroupReflection : public RefObject
{
public:
    int64_t slot = 0;
    Type* groupType = nullptr;
    Type* contextType = nullptr;
    Type* recordType = nullptr;
    RefPtr<StructuralRayTracingStageReflection> miss;
};

class StructuralRayTracingCallableGroupReflection : public RefObject
{
public:
    int64_t slot = 0;
    Type* groupType = nullptr;
    Type* contextType = nullptr;
    Type* recordType = nullptr;
    Type* callableDataType = nullptr;
    RefPtr<StructuralRayTracingStageReflection> callable;
};

class StructuralRayTracingProgramLayoutReflection : public RefObject
{
public:
    Type* layoutType = nullptr;
    Type* traceContextType = nullptr;
    List<RefPtr<StructuralRayTracingHitGroupReflection>> hitGroups;
    List<RefPtr<StructuralRayTracingMissGroupReflection>> missGroups;
    List<RefPtr<StructuralRayTracingCallableGroupReflection>> callableGroups;
};

class StructuralRayTracingReflectionData : public RefObject
{
public:
    List<RefPtr<StructuralRayTracingProgramLayoutReflection>> programLayouts;
};

StructuralRayTracingProgramLayoutReflection* findStructuralRayTracingProgramLayoutReflection(
    ProgramLayout* programLayout,
    const char* name);

} // namespace Slang
