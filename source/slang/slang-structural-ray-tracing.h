#pragma once

#include "slang-compiler-fwd.h"

namespace Slang
{

class InterfaceDecl;

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

private:
    InterfaceDecl* m_stageInterfaces[int(StructuralRayTracingStageKind::Count)] = {};
};

const char* getStructuralRayTracingStageInterfaceName(StructuralRayTracingStageKind kind);

} // namespace Slang
