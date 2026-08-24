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

static AssocTypeDecl* _findAssociatedTypeRequirement(
    Module* module,
    const char* interfaceName,
    const char* requirementName)
{
    auto interfaceDecl = as<InterfaceDecl>(_findNamedDecl(module, interfaceName));
    if (!interfaceDecl)
        return nullptr;
    for (auto member : interfaceDecl->getDirectMemberDeclsOfType<AssocTypeDecl>())
    {
        if (member->getName() && member->getName()->text == requirementName)
            return member;
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
    if (text == "payload")
        return StructuralRayTracingStageInputOperationKind::Payload;
    if (text == "data")
        return StructuralRayTracingStageInputOperationKind::CallableData;
    if (text == "attributes")
        return StructuralRayTracingStageInputOperationKind::HitAttributes;
    if (text == "barycentricCoord")
        return StructuralRayTracingStageInputOperationKind::TriangleBarycentricCoord;
    if (text == "frontFacing")
        return StructuralRayTracingStageInputOperationKind::TriangleFrontFacing;
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
    ContainerDecl* container,
    Dictionary<FunctionDeclBase*, StructuralRayTracingStageInputOperationKind>& operations)
{
    for (auto member : container->getDirectMemberDecls())
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

static void _registerStageInputExtensionOperations(
    ContainerDecl* container,
    AggTypeDecl* const* inputTypes,
    Dictionary<FunctionDeclBase*, StructuralRayTracingStageInputOperationKind>& operations)
{
    for (auto member : container->getDirectMemberDecls())
    {
        auto candidate = member;
        if (auto genericDecl = as<GenericDecl>(candidate))
            candidate = genericDecl->inner;

        if (auto extensionDecl = as<ExtensionDecl>(candidate))
        {
            auto targetType = as<DeclRefType>(extensionDecl->targetType.type);
            auto targetDecl = targetType ? targetType->getDeclRef().getDecl() : nullptr;
            for (int i = 0; i < int(StructuralRayTracingStageKind::Count); ++i)
            {
                if (targetDecl == inputTypes[i])
                {
                    _registerStageInputOperations(extensionDecl, operations);
                    break;
                }
            }
        }

        if (auto childContainer = as<ContainerDecl>(candidate))
            _registerStageInputExtensionOperations(childContainer, inputTypes, operations);
    }
}

bool StructuralRayTracingDeclRegistry::registerTrustedModule(
    Module* module,
    StructuralRayTracingStageKind* outMissingStage)
{
    m_trustedModuleDecl = module->getModuleDecl();
    m_rayTracerType = as<AggTypeDecl>(_findNamedDecl(module, "RayTracer"));
    m_trianglePrimitiveType = as<AggTypeDecl>(_findNamedDecl(module, "TrianglePrimitive"));
    m_curvePrimitiveType = as<AggTypeDecl>(_findNamedDecl(module, "CurvePrimitive"));

    m_associatedTypeRequirements[int(StructuralRayTracingAssociatedTypeKind::TracePayload)] =
        _findAssociatedTypeRequirement(module, "ITraceContext", "Payload");
    m_associatedTypeRequirements[int(StructuralRayTracingAssociatedTypeKind::HitTraceContext)] =
        _findAssociatedTypeRequirement(module, "IHitContext", "TraceContext");
    m_associatedTypeRequirements[int(StructuralRayTracingAssociatedTypeKind::HitPrimitive)] =
        _findAssociatedTypeRequirement(module, "IHitContext", "Primitive");
    m_associatedTypeRequirements[int(StructuralRayTracingAssociatedTypeKind::PrimitiveAttributes)] =
        _findAssociatedTypeRequirement(module, "IIntersectionPrimitive", "Attributes");
    m_associatedTypeRequirements[int(StructuralRayTracingAssociatedTypeKind::MissTraceContext)] =
        _findAssociatedTypeRequirement(module, "IMissGroupContext", "TraceContext");
    m_associatedTypeRequirements[int(StructuralRayTracingAssociatedTypeKind::CallableData)] =
        _findAssociatedTypeRequirement(module, "ICallableGroupContext", "CallableData");

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
    _registerStageInputExtensionOperations(
        module->getModuleDecl(),
        inputTypes,
        m_stageInputOperations);
    if (auto triangleDataType = as<AggTypeDecl>(_findNamedDecl(module, "TriangleData")))
        _registerStageInputOperations(triangleDataType, m_stageInputOperations);
    for (int i = 0; i < int(StructuralRayTracingMetadataKind::Count); ++i)
    {
        auto kind = StructuralRayTracingMetadataKind(i);
        m_metadataInterfaces[i] =
            as<InterfaceDecl>(_findNamedDecl(module, _getMetadataInterfaceName(kind)));
    }
    return true;
}

AssocTypeDecl* StructuralRayTracingDeclRegistry::getAssociatedTypeRequirement(
    StructuralRayTracingAssociatedTypeKind kind) const
{
    auto index = int(kind);
    if (index < 0 || index >= int(StructuralRayTracingAssociatedTypeKind::Count))
        return nullptr;
    return m_associatedTypeRequirements[index];
}

StructuralRayTracingHitAttributesKind StructuralRayTracingDeclRegistry::getHitAttributesKind(
    Type* primitiveType) const
{
    primitiveType = primitiveType ? as<Type>(primitiveType->resolve()) : nullptr;
    auto declRefType = as<DeclRefType>(primitiveType);
    auto primitiveDecl =
        declRefType ? declRefType->getDeclRef().as<AggTypeDecl>().getDecl() : nullptr;
    if (primitiveDecl == m_trianglePrimitiveType)
        return StructuralRayTracingHitAttributesKind::Triangle;
    if (primitiveDecl == m_curvePrimitiveType)
        return StructuralRayTracingHitAttributesKind::Curve;
    return primitiveDecl ? StructuralRayTracingHitAttributesKind::Custom
                         : StructuralRayTracingHitAttributesKind::None;
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

bool StructuralRayTracingDeclRegistry::isTraceMethod(FunctionDeclBase* functionDecl) const
{
    if (!functionDecl || !m_trustedModuleDecl || !m_rayTracerType || !functionDecl->getName() ||
        functionDecl->getName()->text != "trace")
    {
        return false;
    }

    ExtensionDecl* extensionDecl = nullptr;
    ModuleDecl* moduleDecl = nullptr;
    for (auto parent = functionDecl->parentDecl; parent; parent = parent->parentDecl)
    {
        if (!extensionDecl)
            extensionDecl = as<ExtensionDecl>(parent);
        if (auto candidateModule = as<ModuleDecl>(parent))
        {
            moduleDecl = candidateModule;
            break;
        }
    }
    if (moduleDecl != m_trustedModuleDecl || !extensionDecl)
        return false;

    auto targetType = as<DeclRefType>(extensionDecl->targetType.type);
    return targetType && targetType->getDeclRef().getDecl() == m_rayTracerType;
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
