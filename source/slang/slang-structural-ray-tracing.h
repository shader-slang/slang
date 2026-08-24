#pragma once

#include "core/slang-dictionary.h"
#include "slang-compiler-fwd.h"

namespace Slang
{

class InterfaceDecl;
class FunctionDeclBase;

enum class StructuralRayTracingStageKind
{
    ClosestHit,
    AnyHit,
    Intersection,
    Miss,
    Callable,
    Count,
};

class StructuralRayTracingDeclRegistry
{
public:
    bool registerTrustedModule(
        Module* module,
        StructuralRayTracingStageKind* outMissingStage = nullptr);

    InterfaceDecl* getStageInterface(StructuralRayTracingStageKind kind) const;
    StructuralRayTracingStageKind getStageKind(InterfaceDecl* interfaceDecl) const;

    FunctionDeclBase* getStageInvokeRequirement(StructuralRayTracingStageKind kind) const;
    void registerStageImplementation(
        FunctionDeclBase* implementation,
        StructuralRayTracingStageKind kind);
    StructuralRayTracingStageKind getStageKind(FunctionDeclBase* implementation) const;

private:
    InterfaceDecl* m_stageInterfaces[int(StructuralRayTracingStageKind::Count)] = {};
    FunctionDeclBase* m_stageInvokeRequirements[int(StructuralRayTracingStageKind::Count)] = {};
    Dictionary<FunctionDeclBase*, StructuralRayTracingStageKind> m_stageImplementations;
};

const char* getStructuralRayTracingStageInterfaceName(StructuralRayTracingStageKind kind);

} // namespace Slang
