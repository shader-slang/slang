#include "slang-structural-ray-tracing.h"

#include "slang-ast-builder.h"
#include "slang-ast-decl.h"
#include "slang-lookup.h"
#include "slang-module.h"
#include "slang-syntax.h"

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

static GenericTypeConstraintDecl* _findAssociatedTypeConstraint(AssocTypeDecl* associatedType)
{
    if (!associatedType)
        return nullptr;
    auto parentInterface = as<InterfaceDecl>(associatedType->parentDecl);
    if (!parentInterface)
        return nullptr;
    for (auto constraint : parentInterface->getDirectMemberDeclsOfType<GenericTypeConstraintDecl>())
    {
        auto subType = as<DeclRefType>(constraint->sub.type);
        if (subType && subType->getDeclRef().getDecl() == associatedType)
            return constraint;
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
    if (text == "record")
        return StructuralRayTracingStageInputOperationKind::Record;
    if (text == "attributes")
        return StructuralRayTracingStageInputOperationKind::HitAttributes;
    if (text == "barycentricCoord")
        return StructuralRayTracingStageInputOperationKind::TriangleBarycentricCoord;
    if (text == "frontFacing")
        return StructuralRayTracingStageInputOperationKind::TriangleFrontFacing;
    if (text == "parameter")
        return StructuralRayTracingStageInputOperationKind::CurveParameter;
    if (text == "minDistance")
        return StructuralRayTracingStageInputOperationKind::RayTMin;
    if (text == "distance")
        return StructuralRayTracingStageInputOperationKind::RayTCurrent;
    if (text == "time")
        return StructuralRayTracingStageInputOperationKind::RayTime;
    if (text == "rayFlags")
        return StructuralRayTracingStageInputOperationKind::RayFlags;
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
    if (text == "instanceIndex")
        return StructuralRayTracingStageInputOperationKind::InstanceIndex;
    if (text == "instanceID")
        return StructuralRayTracingStageInputOperationKind::InstanceID;
    if (text == "objectToWorld")
        return StructuralRayTracingStageInputOperationKind::ObjectToWorld;
    if (text == "worldToObject")
        return StructuralRayTracingStageInputOperationKind::WorldToObject;
    if (text == "dispatchRaysIndex")
        return StructuralRayTracingStageInputOperationKind::DispatchRaysIndex;
    if (text == "dispatchRaysDimensions")
        return StructuralRayTracingStageInputOperationKind::DispatchRaysDimensions;
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
    m_motionTypes[0] = as<AggTypeDecl>(_findNamedDecl(module, "NoMotion"));
    m_motionTypes[1] = as<AggTypeDecl>(_findNamedDecl(module, "PrimitiveMotion"));
    m_motionTypes[2] = as<AggTypeDecl>(_findNamedDecl(module, "InstanceMotion"));
    m_motionTypes[3] = as<AggTypeDecl>(_findNamedDecl(module, "PrimitiveAndInstanceMotion"));
    m_stagePlaceholderTypes[int(StructuralRayTracingStageKind::ClosestHit)] =
        as<AggTypeDecl>(_findNamedDecl(module, "NoClosestHit"));
    m_stagePlaceholderTypes[int(StructuralRayTracingStageKind::AnyHit)] =
        as<AggTypeDecl>(_findNamedDecl(module, "NoAnyHit"));
    m_stagePlaceholderTypes[int(StructuralRayTracingStageKind::Intersection)] =
        as<AggTypeDecl>(_findNamedDecl(module, "NoIntersection"));

    m_associatedTypeRequirements[int(StructuralRayTracingAssociatedTypeKind::TracePayload)] =
        _findAssociatedTypeRequirement(module, "ITraceContext", "Payload");
    m_associatedTypeRequirements[int(StructuralRayTracingAssociatedTypeKind::TraceMotion)] =
        _findAssociatedTypeRequirement(module, "ITraceContext", "Motion");
    m_associatedTypeRequirements[int(StructuralRayTracingAssociatedTypeKind::HitTraceContext)] =
        _findAssociatedTypeRequirement(module, "IHitContext", "TraceContext");
    m_associatedTypeRequirements[int(StructuralRayTracingAssociatedTypeKind::HitPrimitive)] =
        _findAssociatedTypeRequirement(module, "IHitContext", "Primitive");
    m_associatedTypeRequirements[int(StructuralRayTracingAssociatedTypeKind::HitRecord)] =
        _findAssociatedTypeRequirement(module, "IHitContext", "Record");
    m_associatedTypeRequirements[int(StructuralRayTracingAssociatedTypeKind::PrimitiveAttributes)] =
        _findAssociatedTypeRequirement(module, "IIntersectionPrimitive", "Attributes");
    m_associatedTypeRequirements[int(StructuralRayTracingAssociatedTypeKind::MissTraceContext)] =
        _findAssociatedTypeRequirement(module, "IMissGroupContext", "TraceContext");
    m_associatedTypeRequirements[int(StructuralRayTracingAssociatedTypeKind::MissRecord)] =
        _findAssociatedTypeRequirement(module, "IMissGroupContext", "Record");
    m_associatedTypeRequirements[int(
        StructuralRayTracingAssociatedTypeKind::CallableTraceContext)] =
        _findAssociatedTypeRequirement(module, "ICallableGroupContext", "TraceContext");
    m_associatedTypeRequirements[int(StructuralRayTracingAssociatedTypeKind::CallableData)] =
        _findAssociatedTypeRequirement(module, "ICallableGroupContext", "CallableData");
    m_associatedTypeRequirements[int(StructuralRayTracingAssociatedTypeKind::CallableRecord)] =
        _findAssociatedTypeRequirement(module, "ICallableGroupContext", "Record");
    m_associatedTypeRequirements[int(StructuralRayTracingAssociatedTypeKind::ProgramTraceContext)] =
        _findAssociatedTypeRequirement(module, "ITraceProgramLayout", "TraceContext");
    m_associatedTypeRequirements[int(StructuralRayTracingAssociatedTypeKind::ProgramHitGroups)] =
        _findAssociatedTypeRequirement(module, "ITraceProgramLayout", "HitGroups");
    m_associatedTypeRequirements[int(StructuralRayTracingAssociatedTypeKind::ProgramMissGroups)] =
        _findAssociatedTypeRequirement(module, "ITraceProgramLayout", "MissGroups");
    m_associatedTypeRequirements[int(
        StructuralRayTracingAssociatedTypeKind::ProgramCallableGroups)] =
        _findAssociatedTypeRequirement(module, "ITraceProgramLayout", "CallableGroups");
    m_associatedTypeRequirements[int(StructuralRayTracingAssociatedTypeKind::HitGroupSlot)] =
        _findAssociatedTypeRequirement(module, "IHitGroup", "Slot");
    m_associatedTypeRequirements[int(StructuralRayTracingAssociatedTypeKind::HitGroupContext)] =
        _findAssociatedTypeRequirement(module, "IHitGroup", "Context");
    m_associatedTypeRequirements[int(StructuralRayTracingAssociatedTypeKind::HitGroupClosestHit)] =
        _findAssociatedTypeRequirement(module, "IHitGroup", "ClosestHit");
    m_associatedTypeRequirements[int(StructuralRayTracingAssociatedTypeKind::HitGroupAnyHit)] =
        _findAssociatedTypeRequirement(module, "IHitGroup", "AnyHit");
    m_associatedTypeRequirements[int(
        StructuralRayTracingAssociatedTypeKind::HitGroupIntersection)] =
        _findAssociatedTypeRequirement(module, "IHitGroup", "Intersection");
    m_associatedTypeRequirements[int(StructuralRayTracingAssociatedTypeKind::MissGroupSlot)] =
        _findAssociatedTypeRequirement(module, "IMissGroup", "Slot");
    m_associatedTypeRequirements[int(StructuralRayTracingAssociatedTypeKind::MissGroupContext)] =
        _findAssociatedTypeRequirement(module, "IMissGroup", "Context");
    m_associatedTypeRequirements[int(StructuralRayTracingAssociatedTypeKind::MissGroupMiss)] =
        _findAssociatedTypeRequirement(module, "IMissGroup", "Miss");
    m_associatedTypeRequirements[int(StructuralRayTracingAssociatedTypeKind::CallableGroupSlot)] =
        _findAssociatedTypeRequirement(module, "ICallableGroup", "Slot");
    m_associatedTypeRequirements[int(
        StructuralRayTracingAssociatedTypeKind::CallableGroupContext)] =
        _findAssociatedTypeRequirement(module, "ICallableGroup", "Context");
    m_associatedTypeRequirements[int(
        StructuralRayTracingAssociatedTypeKind::CallableGroupCallable)] =
        _findAssociatedTypeRequirement(module, "ICallableGroup", "Callable");

    for (int i = 0; i < int(StructuralRayTracingAssociatedTypeKind::Count); ++i)
    {
        m_associatedTypeConstraintRequirements[i] =
            _findAssociatedTypeConstraint(m_associatedTypeRequirements[i]);
    }

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
    if (auto curveDataType = as<AggTypeDecl>(_findNamedDecl(module, "CurveData")))
        _registerStageInputOperations(curveDataType, m_stageInputOperations);
    for (int i = 0; i < int(StructuralRayTracingMetadataKind::Count); ++i)
    {
        auto kind = StructuralRayTracingMetadataKind(i);
        m_metadataInterfaces[i] =
            as<InterfaceDecl>(_findNamedDecl(module, _getMetadataInterfaceName(kind)));
    }
    return true;
}

bool StructuralRayTracingDeclRegistry::isTrustedModule(Module* module) const
{
    return module && module->getModuleDecl() == m_trustedModuleDecl;
}

AssocTypeDecl* StructuralRayTracingDeclRegistry::getAssociatedTypeRequirement(
    StructuralRayTracingAssociatedTypeKind kind) const
{
    auto index = int(kind);
    if (index < 0 || index >= int(StructuralRayTracingAssociatedTypeKind::Count))
        return nullptr;
    return m_associatedTypeRequirements[index];
}

Type* StructuralRayTracingDeclRegistry::resolveAssociatedType(
    ASTBuilder* astBuilder,
    SubtypeWitness* witness,
    StructuralRayTracingAssociatedTypeKind kind) const
{
    if (!witness)
        return nullptr;

    auto requirement = getAssociatedTypeRequirement(kind);
    if (!requirement)
        return nullptr;

    auto requirementWitness = tryLookUpRequirementWitness(astBuilder, witness, requirement);
    if (requirementWitness.getFlavor() == RequirementWitness::Flavor::val)
        return as<Type>(requirementWitness.getVal()->resolve());
    if (requirementWitness.getFlavor() == RequirementWitness::Flavor::declRef)
    {
        auto type = DeclRefType::create(astBuilder, requirementWitness.getDeclRef());
        return type ? as<Type>(type->resolve()) : nullptr;
    }
    return nullptr;
}

SubtypeWitness* StructuralRayTracingDeclRegistry::resolveAssociatedTypeConstraint(
    ASTBuilder* astBuilder,
    SubtypeWitness* witness,
    StructuralRayTracingAssociatedTypeKind kind) const
{
    if (!witness)
        return nullptr;

    auto index = int(kind);
    if (index < 0 || index >= int(StructuralRayTracingAssociatedTypeKind::Count))
        return nullptr;
    auto requirement = m_associatedTypeConstraintRequirements[index];
    if (!requirement)
        return nullptr;

    auto requirementWitness = tryLookUpRequirementWitness(astBuilder, witness, requirement);
    if (requirementWitness.getFlavor() == RequirementWitness::Flavor::val)
        return as<SubtypeWitness>(requirementWitness.getVal()->resolve());
    return nullptr;
}

bool StructuralRayTracingDeclRegistry::tryGetShaderGroupSlotIndex(
    ASTBuilder* astBuilder,
    Type* slotType,
    int64_t& outIndex) const
{
    auto lookupResult = lookUpMember(
        astBuilder,
        nullptr,
        astBuilder->getNamePool()->getName("index"),
        slotType,
        nullptr,
        LookupMask::Value,
        LookupOptions::IgnoreBaseInterfaces);
    if (lookupResult.isValid() && !lookupResult.isOverloaded())
    {
        if (auto slotIndexDeclRef = lookupResult.item.declRef.as<VarDeclBase>())
        {
            auto slotIndexDecl = slotIndexDeclRef.getDecl();
            if (auto constantValue = as<ConstantIntVal>(slotIndexDecl->val))
            {
                outIndex = constantValue->getValue();
                return true;
            }
            if (auto value = slotIndexDecl->val)
            {
                value =
                    as<IntVal>(value->substitute(astBuilder, SubstitutionSet(slotIndexDeclRef)));
                if (auto constantValue = as<ConstantIntVal>(value ? value->resolve() : nullptr))
                {
                    outIndex = constantValue->getValue();
                    return true;
                }
            }
        }
    }
    return false;
}

bool StructuralRayTracingDeclRegistry::isStagePlaceholder(
    StructuralRayTracingStageKind kind,
    Type* type) const
{
    auto index = int(kind);
    if (index < 0 || index >= int(StructuralRayTracingStageKind::Count) ||
        !m_stagePlaceholderTypes[index])
    {
        return false;
    }
    type = type ? as<Type>(type->resolve()) : nullptr;
    auto declRefType = as<DeclRefType>(type);
    return declRefType && declRefType->getDeclRef().getDecl() == m_stagePlaceholderTypes[index];
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

StructuralRayTracingMotionKind StructuralRayTracingDeclRegistry::getMotionKind(
    Type* motionType) const
{
    motionType = motionType ? as<Type>(motionType->resolve()) : nullptr;
    auto declRefType = as<DeclRefType>(motionType);
    auto motionDecl = declRefType ? declRefType->getDeclRef().as<AggTypeDecl>().getDecl() : nullptr;
    for (UInt i = 0; i < SLANG_COUNT_OF(m_motionTypes); ++i)
    {
        if (motionDecl == m_motionTypes[i])
            return StructuralRayTracingMotionKind(i);
    }
    return StructuralRayTracingMotionKind::Invalid;
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

InterfaceDecl* StructuralRayTracingDeclRegistry::getMetadataInterface(
    StructuralRayTracingMetadataKind kind) const
{
    auto index = int(kind);
    if (index < 0 || index >= int(StructuralRayTracingMetadataKind::Count))
        return nullptr;
    return m_metadataInterfaces[index];
}

StructuralRayTracingStageInputOperationKind StructuralRayTracingDeclRegistry::
    getStageInputOperationKind(FunctionDeclBase* functionDecl) const
{
    if (auto kind = m_stageInputOperations.tryGetValue(functionDecl))
        return *kind;
    return StructuralRayTracingStageInputOperationKind::Count;
}

static bool _isRayTracerMethod(
    FunctionDeclBase* functionDecl,
    ModuleDecl* trustedModuleDecl,
    AggTypeDecl* rayTracerType,
    UnownedStringSlice expectedName)
{
    if (!functionDecl || !trustedModuleDecl || !rayTracerType || !functionDecl->getName() ||
        functionDecl->getName()->text.getUnownedSlice() != expectedName)
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
    if (moduleDecl != trustedModuleDecl || !extensionDecl)
        return false;

    auto targetType = as<DeclRefType>(extensionDecl->targetType.type);
    return targetType && targetType->getDeclRef().getDecl() == rayTracerType;
}

bool StructuralRayTracingDeclRegistry::isTraceMethod(FunctionDeclBase* functionDecl) const
{
    return _isRayTracerMethod(functionDecl, m_trustedModuleDecl, m_rayTracerType, toSlice("trace"));
}

bool StructuralRayTracingDeclRegistry::isCallShaderMethod(FunctionDeclBase* functionDecl) const
{
    return _isRayTracerMethod(
        functionDecl,
        m_trustedModuleDecl,
        m_rayTracerType,
        toSlice("callShader"));
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

void StructuralRayTracingDeclRegistry::registerFunctionCall(
    FunctionDeclBase* caller,
    FunctionDeclBase* callee,
    SourceLoc callLoc)
{
    if (!caller || !callee || !isInitialized())
        return;

    m_functionCallees.getOrAddValue(caller, HashSet<FunctionDeclBase*>()).add(callee);
    if (isTraceMethod(callee) || isCallShaderMethod(callee))
        m_structuralProgramCallers.add(caller);
    if (isCallShaderMethod(callee))
        m_callShaderCallers[caller] = callLoc;
}

bool StructuralRayTracingDeclRegistry::functionReachesStructuralTrace(
    FunctionDeclBase* function) const
{
    if (!function)
        return false;

    HashSet<FunctionDeclBase*> visited;
    List<FunctionDeclBase*> workList;
    workList.add(function);
    for (Index i = 0; i < workList.getCount(); ++i)
    {
        auto current = workList[i];
        if (!visited.add(current))
            continue;
        if (m_structuralProgramCallers.contains(current))
            return true;
        if (auto callees = m_functionCallees.tryGetValue(current))
        {
            for (auto callee : *callees)
                workList.add(callee);
        }
    }
    return false;
}

bool StructuralRayTracingDeclRegistry::findReachableCallShader(
    FunctionDeclBase* function,
    SourceLoc& outCallLoc) const
{
    if (!function)
        return false;

    HashSet<FunctionDeclBase*> visited;
    List<FunctionDeclBase*> workList;
    workList.add(function);
    for (Index i = 0; i < workList.getCount(); ++i)
    {
        auto current = workList[i];
        if (!visited.add(current))
            continue;
        if (auto callLoc = m_callShaderCallers.tryGetValue(current))
        {
            outCallLoc = *callLoc;
            return true;
        }
        if (auto callees = m_functionCallees.tryGetValue(current))
        {
            for (auto callee : *callees)
                workList.add(callee);
        }
    }
    return false;
}

} // namespace Slang
