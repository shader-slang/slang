#include "slang-structural-ray-tracing.h"

#include "slang-ast-builder.h"
#include "slang-ast-decl.h"
#include "slang-module.h"

namespace Slang
{

const char* getStructuralRayTracingStageInterfaceName(StructuralRayTracingStageKind kind)
{
    switch (kind)
    {
    case StructuralRayTracingStageKind::ClosestHit:
        return "IClosestHitShader";
    case StructuralRayTracingStageKind::AnyHit:
        return "IAnyHitShader";
    case StructuralRayTracingStageKind::Intersection:
        return "IIntersectionShader";
    case StructuralRayTracingStageKind::Miss:
        return "IMissShader";
    case StructuralRayTracingStageKind::Callable:
        return "ICallableShader";
    default:
        return nullptr;
    }
}

static const char* _getStageInputTypeName(StructuralRayTracingStageKind kind)
{
    switch (kind)
    {
    case StructuralRayTracingStageKind::ClosestHit:
        return "ClosestHitInput";
    case StructuralRayTracingStageKind::AnyHit:
        return "AnyHitInput";
    case StructuralRayTracingStageKind::Intersection:
        return "IntersectionInput";
    case StructuralRayTracingStageKind::Miss:
        return "MissInput";
    case StructuralRayTracingStageKind::Callable:
        return "CallableInput";
    default:
        return nullptr;
    }
}

static const char* _getMetadataInterfaceName(StructuralRayTracingMetadataKind kind)
{
    switch (kind)
    {
    case StructuralRayTracingMetadataKind::ShaderGroupSlot:
        return "IShaderGroupSlot";
    case StructuralRayTracingMetadataKind::HitGroup:
        return "IHitGroup";
    case StructuralRayTracingMetadataKind::MissGroup:
        return "IMissGroup";
    case StructuralRayTracingMetadataKind::CallableGroup:
        return "ICallableGroup";
    case StructuralRayTracingMetadataKind::HitGroupList:
        return "IHitGroupList";
    case StructuralRayTracingMetadataKind::MissGroupList:
        return "IMissGroupList";
    case StructuralRayTracingMetadataKind::CallableGroupList:
        return "ICallableGroupList";
    case StructuralRayTracingMetadataKind::TraceProgramLayout:
        return "ITraceProgramLayout";
    default:
        return nullptr;
    }
}

static Decl* _findNamedDeclInContainer(
    ContainerDecl* container,
    Name* rtName,
    Name* declName,
    bool insideRayTracingNamespace)
{
    for (auto decl : container->getDirectMemberDecls())
    {
        bool insideNamespace = insideRayTracingNamespace;
        if (auto namespaceDecl = as<NamespaceDecl>(decl))
            insideNamespace = insideNamespace || namespaceDecl->getName() == rtName;

        auto candidate = decl;
        if (auto genericDecl = as<GenericDecl>(candidate))
            candidate = genericDecl->inner;
        if (insideNamespace && candidate->getName() == declName)
            return candidate;

        if (auto childContainer = as<ContainerDecl>(decl))
        {
            if (auto result =
                    _findNamedDeclInContainer(childContainer, rtName, declName, insideNamespace))
            {
                return result;
            }
        }
    }
    return nullptr;
}

static Decl* _findNamedDecl(Module* module, const char* name)
{
    auto namePool = module->getASTBuilder()->getNamePool();
    return _findNamedDeclInContainer(
        module->getModuleDecl(),
        namePool->getName("rt"),
        namePool->getName(name),
        false);
}

static InterfaceDecl* _findStageInterface(Module* module, StructuralRayTracingStageKind kind)
{
    return as<InterfaceDecl>(
        _findNamedDecl(module, getStructuralRayTracingStageInterfaceName(kind)));
}

static AggTypeDecl* _findStageInputType(Module* module, StructuralRayTracingStageKind kind)
{
    return as<AggTypeDecl>(_findNamedDecl(module, _getStageInputTypeName(kind)));
}

static FunctionDeclBase* _findStageInvokeRequirement(InterfaceDecl* interfaceDecl)
{
    for (auto member : interfaceDecl->getDirectMemberDecls())
    {
        auto candidate = member;
        if (auto genericDecl = as<GenericDecl>(candidate))
            candidate = genericDecl->inner;
        if (auto functionDecl = as<FunctionDeclBase>(candidate))
        {
            if (functionDecl->getName() && functionDecl->getName()->text == "invoke")
                return functionDecl;
        }
    }
    return nullptr;
}

static StructuralRayTracingStageInputOperationKind _getStageInputOperationKind(
    FunctionDeclBase* functionDecl)
{
    Decl* namedDecl = functionDecl;
    if (as<AccessorDecl>(functionDecl))
        namedDecl = as<PropertyDecl>(functionDecl->parentDecl);
    auto name = namedDecl ? namedDecl->getName() : nullptr;
    if (!name)
        return StructuralRayTracingStageInputOperationKind::Count;

    auto text = name->text.getUnownedSlice();
    if (text == "distance")
        return StructuralRayTracingStageInputOperationKind::RayTCurrent;
    if (text == "hitKind")
        return StructuralRayTracingStageInputOperationKind::HitKind;
    if (text == "worldSpaceOrigin")
        return StructuralRayTracingStageInputOperationKind::WorldRayOrigin;
    if (text == "worldSpaceDirection")
        return StructuralRayTracingStageInputOperationKind::WorldRayDirection;
    if (text == "objectSpaceRay")
        return StructuralRayTracingStageInputOperationKind::ObjectSpaceRay;
    if (text == "primitiveIndex")
        return StructuralRayTracingStageInputOperationKind::PrimitiveIndex;
    if (text == "geometryIndex")
        return StructuralRayTracingStageInputOperationKind::GeometryIndex;
    if (text == "ignoreHit")
        return StructuralRayTracingStageInputOperationKind::IgnoreHit;
    if (text == "acceptHitAndEndSearch")
        return StructuralRayTracingStageInputOperationKind::AcceptHitAndEndSearch;
    if (text == "reportHit")
    {
        return functionDecl->getParameters().getCount() == 2
                   ? StructuralRayTracingStageInputOperationKind::ReportHit
                   : StructuralRayTracingStageInputOperationKind::ReportHitWithKind;
    }
    return StructuralRayTracingStageInputOperationKind::Count;
}

static void _registerStageInputOperations(
    AggTypeDecl* inputType,
    Dictionary<FunctionDeclBase*, StructuralRayTracingStageInputOperationKind>& operations)
{
    for (auto member : inputType->getDirectMemberDecls())
    {
        if (auto propertyDecl = as<PropertyDecl>(member))
        {
            for (auto accessor : propertyDecl->getDirectMemberDeclsOfType<AccessorDecl>())
            {
                auto kind = _getStageInputOperationKind(accessor);
                if (kind != StructuralRayTracingStageInputOperationKind::Count)
                    operations[accessor] = kind;
            }
        }
        else if (auto functionDecl = as<FunctionDeclBase>(member))
        {
            auto kind = _getStageInputOperationKind(functionDecl);
            if (kind != StructuralRayTracingStageInputOperationKind::Count)
                operations[functionDecl] = kind;
        }
    }
}

bool StructuralRayTracingDeclRegistry::registerTrustedModule(
    Module* module,
    StructuralRayTracingStageKind* outMissingStage)
{
    InterfaceDecl* interfaces[int(StructuralRayTracingStageKind::Count)] = {};
    AggTypeDecl* inputTypes[int(StructuralRayTracingStageKind::Count)] = {};
    FunctionDeclBase* invokeRequirements[int(StructuralRayTracingStageKind::Count)] = {};
    for (int i = 0; i < int(StructuralRayTracingStageKind::Count); ++i)
    {
        auto kind = StructuralRayTracingStageKind(i);
        interfaces[i] = _findStageInterface(module, kind);
        inputTypes[i] = _findStageInputType(module, kind);
        if (interfaces[i])
            invokeRequirements[i] = _findStageInvokeRequirement(interfaces[i]);
        if (!interfaces[i] || !inputTypes[i] || !invokeRequirements[i])
        {
            if (outMissingStage)
                *outMissingStage = kind;
            return false;
        }
    }

    for (int i = 0; i < int(StructuralRayTracingStageKind::Count); ++i)
    {
        m_stageInterfaces[i] = interfaces[i];
        m_stageInputTypes[i] = inputTypes[i];
        m_stageInvokeRequirements[i] = invokeRequirements[i];
        _registerStageInputOperations(inputTypes[i], m_stageInputOperations);
    }
    for (int i = 0; i < int(StructuralRayTracingMetadataKind::Count); ++i)
    {
        auto kind = StructuralRayTracingMetadataKind(i);
        m_metadataInterfaces[i] =
            as<InterfaceDecl>(_findNamedDecl(module, _getMetadataInterfaceName(kind)));
    }
    return true;
}

InterfaceDecl* StructuralRayTracingDeclRegistry::getStageInterface(
    StructuralRayTracingStageKind kind) const
{
    auto index = int(kind);
    if (index < 0 || index >= int(StructuralRayTracingStageKind::Count))
        return nullptr;
    return m_stageInterfaces[index];
}

StructuralRayTracingStageKind StructuralRayTracingDeclRegistry::getStageKind(
    InterfaceDecl* interfaceDecl) const
{
    if (!interfaceDecl)
        return StructuralRayTracingStageKind::Count;
    for (int i = 0; i < int(StructuralRayTracingStageKind::Count); ++i)
    {
        if (m_stageInterfaces[i] == interfaceDecl)
            return StructuralRayTracingStageKind(i);
    }
    return StructuralRayTracingStageKind::Count;
}

AggTypeDecl* StructuralRayTracingDeclRegistry::getStageInputType(
    StructuralRayTracingStageKind kind) const
{
    auto index = int(kind);
    if (index < 0 || index >= int(StructuralRayTracingStageKind::Count))
        return nullptr;
    return m_stageInputTypes[index];
}

StructuralRayTracingStageKind StructuralRayTracingDeclRegistry::getStageInputKind(
    AggTypeDecl* typeDecl) const
{
    if (!typeDecl)
        return StructuralRayTracingStageKind::Count;
    for (int i = 0; i < int(StructuralRayTracingStageKind::Count); ++i)
    {
        if (m_stageInputTypes[i] == typeDecl)
            return StructuralRayTracingStageKind(i);
    }
    return StructuralRayTracingStageKind::Count;
}

StructuralRayTracingMetadataKind StructuralRayTracingDeclRegistry::getMetadataKind(
    InterfaceDecl* interfaceDecl) const
{
    if (!interfaceDecl)
        return StructuralRayTracingMetadataKind::Count;
    for (int i = 0; i < int(StructuralRayTracingMetadataKind::Count); ++i)
    {
        if (m_metadataInterfaces[i] == interfaceDecl)
            return StructuralRayTracingMetadataKind(i);
    }
    return StructuralRayTracingMetadataKind::Count;
}

StructuralRayTracingStageInputOperationKind StructuralRayTracingDeclRegistry::
    getStageInputOperationKind(FunctionDeclBase* functionDecl) const
{
    if (auto kind = m_stageInputOperations.tryGetValue(functionDecl))
        return *kind;
    return StructuralRayTracingStageInputOperationKind::Count;
}

FunctionDeclBase* StructuralRayTracingDeclRegistry::getStageInvokeRequirement(
    StructuralRayTracingStageKind kind) const
{
    auto index = int(kind);
    if (index < 0 || index >= int(StructuralRayTracingStageKind::Count))
        return nullptr;
    return m_stageInvokeRequirements[index];
}

void StructuralRayTracingDeclRegistry::registerStageImplementation(
    FunctionDeclBase* implementation,
    StructuralRayTracingStageKind kind)
{
    if (implementation && kind != StructuralRayTracingStageKind::Count)
        m_stageImplementations[implementation] = kind;
}

StructuralRayTracingStageKind StructuralRayTracingDeclRegistry::getStageKind(
    FunctionDeclBase* implementation) const
{
    if (!implementation)
        return StructuralRayTracingStageKind::Count;
    for (int i = 0; i < int(StructuralRayTracingStageKind::Count); ++i)
    {
        if (m_stageInvokeRequirements[i] == implementation)
            return StructuralRayTracingStageKind(i);
    }
    if (auto kind = m_stageImplementations.tryGetValue(implementation))
        return *kind;
    return StructuralRayTracingStageKind::Count;
}

bool StructuralRayTracingDeclRegistry::registerAPIUse(
    Module* module,
    RayTracingAPIFamily family,
    Decl* decl,
    Decl** outOtherDecl)
{
    *outOtherDecl = nullptr;
    if (!module || !decl)
        return false;

    auto& usage = m_apiUsage.getOrAddValue(module, RayTracingAPIUsage());
    auto& currentDecl =
        family == RayTracingAPIFamily::Structural ? usage.structuralDecl : usage.legacyDecl;
    auto otherDecl =
        family == RayTracingAPIFamily::Structural ? usage.legacyDecl : usage.structuralDecl;
    if (!currentDecl)
        currentDecl = decl;
    if (!otherDecl || usage.diagnosed)
        return false;

    usage.diagnosed = true;
    *outOtherDecl = otherDecl;
    return true;
}

} // namespace Slang
