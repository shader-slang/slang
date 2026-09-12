#include "slang-structural-ray-tracing.h"

#include "slang-ast-builder.h"
#include "slang-ast-decl.h"
#include "slang-check-impl.h"
#include "slang-lookup.h"
#include "slang-mangle.h"
#include "slang-module.h"
#include "slang-syntax.h"

namespace Slang
{

String getStructuralRayTracingMetalDescriptorResourceName(
    StructuralRayTracingDescriptorResourceKind kind,
    Index payloadIndex,
    Index payloadCount)
{
    StringBuilder result;
    switch (kind)
    {
    case StructuralRayTracingDescriptorResourceKind::IntersectionFunctionTable:
        result << "intersectionFunctions";
        break;
    case StructuralRayTracingDescriptorResourceKind::MissVisibleFunctionTable:
        result << "missFunctions";
        break;
    case StructuralRayTracingDescriptorResourceKind::ClosestHitVisibleFunctionTable:
        result << "closestHitFunctions";
        break;
    case StructuralRayTracingDescriptorResourceKind::CallableVisibleFunctionTable:
        result << "callableFunctions";
        break;
    case StructuralRayTracingDescriptorResourceKind::Records:
        result << "records";
        break;
    default:
        SLANG_UNEXPECTED("invalid structural ray-tracing descriptor resource kind");
    }

    if (payloadIndex >= 0 && payloadCount > 1)
        result << payloadIndex;
    return result.produceString();
}

static String _getStructuralRayTracingSourceDeclName(Decl* decl)
{
    if (!decl)
        return String();

    auto leafName = decl->getName();
    if (!leafName || leafName->text.getLength() == 0)
        return String();

    auto parentDecl = decl->parentDecl;
    if (auto genericParentDecl = as<GenericDecl>(parentDecl))
        parentDecl = genericParentDecl->parentDecl;
    if (auto fileParentDecl = as<FileDecl>(parentDecl))
        parentDecl = fileParentDecl->parentDecl;
    if (auto moduleParentDecl = as<ModuleDecl>(parentDecl))
        parentDecl = moduleParentDecl->parentDecl;

    auto parentName = _getStructuralRayTracingSourceDeclName(parentDecl);
    if (parentName.getLength() == 0)
        return leafName->text;

    StringBuilder result;
    result << parentName << "." << leafName->text;
    return result.produceString();
}

static bool _hasStructuralRayTracingGenericSubstitution(DeclRefBase* declRef)
{
    // Consider `GenericMiss<uint>` and an ordinary `Miss`. The first decl-ref contains a
    // `GenericAppDeclRef` carrying `uint`, while the second has no generic substitution at all.
    // Check that semantic representation directly instead of trying to recognize generic syntax
    // in a printed or mangled name.
    bool result = false;
    SubstitutionSet(declRef).forEachGenericSubstitution([&](GenericDecl*, Val::OperandView<Val>)
                                                        { result = true; });
    return result;
}

String getStructuralRayTracingSourceTypeName(ASTBuilder* astBuilder, Type* type)
{
    auto declRefType = as<DeclRefType>(type ? type->resolve() : nullptr);
    if (!declRefType)
        return String();

    auto sourceName = _getStructuralRayTracingSourceDeclName(declRefType->getDeclRef().getDecl());
    if (sourceName.getLength() == 0 ||
        !_hasStructuralRayTracingGenericSubstitution(declRefType->getDeclRef().declRefBase))
    {
        return sourceName;
    }

    // `GenericMiss<uint>` and `GenericMiss<float>` share one declaration path, but their
    // canonical semantic types have distinct mangled identities. Hash that existing identity only
    // to keep the public target symbol compact; the compiler never parses a mangled spelling to
    // rediscover either the declaration or its substitutions.
    auto canonicalType = type->getCanonicalType();
    auto mangledTypeName = getMangledTypeName(astBuilder, canonicalType);
    StringBuilder result;
    result << sourceName << getHashedName(mangledTypeName.getUnownedSlice());
    return result.produceString();
}

bool isSemanticallyEmptyStructuralRayTracingPayload(ASTBuilder* astBuilder, Type* type)
{
    type = type ? type->getCanonicalType() : nullptr;
    auto structType = as<DeclRefType>(type);
    auto structDeclRef =
        structType ? structType->getDeclRef().as<StructDecl>() : DeclRef<StructDecl>();
    auto structDecl = structDeclRef.getDecl();
    if (!structDecl || !structDecl->hasBody || structDecl->aliasedType)
        return false;

    // Built-in and magic types such as `uint` and `vector<T, N>` use source-level struct
    // declarations to describe compiler-known types, but their value storage is not represented by
    // ordinary fields on those declarations. Only an ordinary user struct can be an implicit
    // zero-storage payload; otherwise scalar and vector payload accesses are incorrectly rejected
    // as accesses to an empty payload.
    if (structDecl->findModifier<BuiltinTypeModifier>() ||
        structDecl->findModifier<MagicTypeModifier>() ||
        structDecl->findModifier<IntrinsicTypeModifier>())
    {
        return false;
    }

    if (getFields(astBuilder, structDeclRef, MemberFilterStyle::Instance).isNonEmpty())
        return false;

    if (auto baseStructType = findBaseStructType(astBuilder, structDeclRef))
        return isSemanticallyEmptyStructuralRayTracingPayload(astBuilder, baseStructType);
    return true;
}

static String _encodeStructuralRayTracingSymbolName(UnownedStringSlice logicalName)
{
    static const UnownedStringSlice kEncodedPrefix = toSlice("__slang_structural_rt_");
    StringBuilder result;
    result << kEncodedPrefix;
    for (auto c : logicalName)
    {
        auto byte = uint8_t(c);
        result.appendChar("0123456789abcdef"[byte >> 4]);
        result.appendChar("0123456789abcdef"[byte & 0xf]);
    }
    return result.produceString();
}

String getStructuralRayTracingEntryPointName(UnownedStringSlice sourceTypeName)
{
    // Consider `Miss` and `Stages.Miss`. Keeping `Miss` unchanged preserves the public names used
    // by existing structural programs. `Stages.Miss` cannot be emitted as a CUDA or C-like symbol,
    // while C-like targets reserve the entry-point name `main`, so encode every UTF-8 byte of those
    // names after a compiler-reserved prefix. We also encode source names that start with the
    // prefix; consequently a user-written identifier cannot collide with an encoded qualified
    // name.
    static const UnownedStringSlice kEncodedPrefix = toSlice("__slang_structural_rt_");
    bool isSimpleIdentifier =
        sourceTypeName.getLength() != 0 && !sourceTypeName.startsWith(kEncodedPrefix) &&
        sourceTypeName != toSlice("main") &&
        ((sourceTypeName[0] >= 'A' && sourceTypeName[0] <= 'Z') ||
         (sourceTypeName[0] >= 'a' && sourceTypeName[0] <= 'z') || sourceTypeName[0] == '_');
    for (Index i = 1; isSimpleIdentifier && i < sourceTypeName.getLength(); ++i)
    {
        auto c = sourceTypeName[i];
        isSimpleIdentifier =
            (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_';
    }
    if (isSimpleIdentifier)
        return String(sourceTypeName);

    return _encodeStructuralRayTracingSymbolName(sourceTypeName);
}

enum class StructuralRayTracingMetalFunctionRole
{
    Miss,
    ClosestHit,
    Callable,
    Candidate,
};

static void _appendStructuralRayTracingMetalNamePart(StringBuilder& key, UnownedStringSlice name)
{
    // Length-prefix source names so the key remains injective even if a future source spelling can
    // contain one of the separators used by this private ABI format.
    key << "|" << name.getLength() << ":" << name;
}

static UnownedStringSlice _getStructuralRayTracingMetalCandidateKey(
    StructuralRayTracingMetalCandidateKind candidateKind)
{
    switch (candidateKind)
    {
    case StructuralRayTracingMetalCandidateKind::Triangle:
        return toSlice("triangle");
    case StructuralRayTracingMetalCandidateKind::BoundingBox:
        return toSlice("boundingBox");
    case StructuralRayTracingMetalCandidateKind::Curve:
        return toSlice("curve");
    default:
        SLANG_UNEXPECTED("invalid structural ray-tracing Metal candidate kind");
    }
}

// Forms the compiler-owned ABI key shared by Metal synthesis and reflection. The version belongs
// in the encoded name: changing a generated function's ABI can then produce a new physical symbol
// without making hosts guess which convention a library used.
static String _getStructuralRayTracingMetalFunctionName(
    StructuralRayTracingMetalFunctionRole role,
    UnownedStringSlice schemaSourceTypeName,
    Index payloadIndex,
    Index functionIndex,
    UnownedStringSlice groupSourceTypeName,
    UnownedStringSlice stageSourceTypeName,
    StructuralRayTracingMetalCandidateKind candidateKind =
        StructuralRayTracingMetalCandidateKind::Count)
{
    SLANG_RELEASE_ASSERT(schemaSourceTypeName.getLength() != 0);

    StringBuilder key;
    key << "metal.v1";
    switch (role)
    {
    case StructuralRayTracingMetalFunctionRole::Miss:
        SLANG_RELEASE_ASSERT(
            payloadIndex >= 0 && functionIndex >= 0 && stageSourceTypeName.getLength() != 0);
        key << "|miss";
        _appendStructuralRayTracingMetalNamePart(key, schemaSourceTypeName);
        key << "|" << payloadIndex << "|" << functionIndex;
        _appendStructuralRayTracingMetalNamePart(key, stageSourceTypeName);
        break;
    case StructuralRayTracingMetalFunctionRole::ClosestHit:
        SLANG_RELEASE_ASSERT(
            payloadIndex >= 0 && functionIndex >= 0 && groupSourceTypeName.getLength() != 0 &&
            stageSourceTypeName.getLength() != 0);
        key << "|closestHit";
        _appendStructuralRayTracingMetalNamePart(key, schemaSourceTypeName);
        key << "|" << payloadIndex << "|" << functionIndex;
        _appendStructuralRayTracingMetalNamePart(key, groupSourceTypeName);
        _appendStructuralRayTracingMetalNamePart(key, stageSourceTypeName);
        break;
    case StructuralRayTracingMetalFunctionRole::Callable:
        SLANG_RELEASE_ASSERT(functionIndex >= 0 && stageSourceTypeName.getLength() != 0);
        key << "|callable";
        _appendStructuralRayTracingMetalNamePart(key, schemaSourceTypeName);
        key << "|" << functionIndex;
        _appendStructuralRayTracingMetalNamePart(key, stageSourceTypeName);
        break;
    case StructuralRayTracingMetalFunctionRole::Candidate:
        SLANG_RELEASE_ASSERT(
            payloadIndex >= 0 && candidateKind != StructuralRayTracingMetalCandidateKind::Count);
        key << "|candidate";
        _appendStructuralRayTracingMetalNamePart(key, schemaSourceTypeName);
        key << "|" << payloadIndex << "|"
            << _getStructuralRayTracingMetalCandidateKey(candidateKind);
        break;
    default:
        SLANG_UNEXPECTED("invalid structural ray-tracing Metal function role");
    }
    return _encodeStructuralRayTracingSymbolName(key.getUnownedSlice());
}

String getStructuralRayTracingMetalMissFunctionName(
    UnownedStringSlice schemaSourceTypeName,
    Index payloadIndex,
    Index functionIndex,
    UnownedStringSlice stageSourceTypeName)
{
    return _getStructuralRayTracingMetalFunctionName(
        StructuralRayTracingMetalFunctionRole::Miss,
        schemaSourceTypeName,
        payloadIndex,
        functionIndex,
        UnownedStringSlice(),
        stageSourceTypeName);
}

String getStructuralRayTracingMetalClosestHitFunctionName(
    UnownedStringSlice schemaSourceTypeName,
    Index payloadIndex,
    Index functionIndex,
    UnownedStringSlice groupSourceTypeName,
    UnownedStringSlice stageSourceTypeName)
{
    return _getStructuralRayTracingMetalFunctionName(
        StructuralRayTracingMetalFunctionRole::ClosestHit,
        schemaSourceTypeName,
        payloadIndex,
        functionIndex,
        groupSourceTypeName,
        stageSourceTypeName);
}

String getStructuralRayTracingMetalNoOpClosestHitFunctionName(
    UnownedStringSlice schemaSourceTypeName,
    Index payloadIndex,
    Index functionIndex,
    UnownedStringSlice groupSourceTypeName)
{
    // This spelling is not user-visible source metadata. It is only the stable final component of
    // the compiler-owned physical name and deliberately lives beside the shared naming algorithm.
    return _getStructuralRayTracingMetalFunctionName(
        StructuralRayTracingMetalFunctionRole::ClosestHit,
        schemaSourceTypeName,
        payloadIndex,
        functionIndex,
        groupSourceTypeName,
        UnownedStringSlice::fromLiteral("NoClosestHit"));
}

String getStructuralRayTracingMetalCallableFunctionName(
    UnownedStringSlice schemaSourceTypeName,
    Index functionIndex,
    UnownedStringSlice stageSourceTypeName)
{
    return _getStructuralRayTracingMetalFunctionName(
        StructuralRayTracingMetalFunctionRole::Callable,
        schemaSourceTypeName,
        -1,
        functionIndex,
        UnownedStringSlice(),
        stageSourceTypeName);
}

String getStructuralRayTracingMetalCandidateDispatcherName(
    UnownedStringSlice schemaSourceTypeName,
    Index payloadIndex,
    StructuralRayTracingMetalCandidateKind candidateKind)
{
    return _getStructuralRayTracingMetalFunctionName(
        StructuralRayTracingMetalFunctionRole::Candidate,
        schemaSourceTypeName,
        payloadIndex,
        -1,
        UnownedStringSlice(),
        UnownedStringSlice(),
        candidateKind);
}

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
    case StructuralRayTracingMetadataKind::HitGroup:
        return "IHitGroup";
    case StructuralRayTracingMetadataKind::HitGroupList:
        return "IHitGroupList";
    case StructuralRayTracingMetadataKind::MissShaderList:
        return "IMissShaderList";
    case StructuralRayTracingMetadataKind::CallableShaderList:
        return "ICallableShaderList";
    case StructuralRayTracingMetadataKind::TraceProgramSchema:
        return "ITraceProgramSchema";
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

/// Returns the sole ordinary type parameter, or null if the generic shape is not unary.
static GenericTypeParamDecl* _getOnlyGenericTypeParameter(GenericDecl* genericDecl)
{
    GenericTypeParamDecl* result = nullptr;
    if (!genericDecl)
        return result;
    for (auto member : genericDecl->getDirectMemberDecls())
    {
        auto parameter = as<GenericTypeParamDecl>(member);
        if (!parameter)
        {
            if (isGenericParam(member))
                return nullptr;
            continue;
        }
        if (result)
            return nullptr;
        result = parameter;
    }
    return result;
}

/// Finds the unique source constraint that proves `subtype : interfaceDecl`.
static GenericTypeConstraintDecl* _findConformanceConstraint(
    GenericDecl* genericDecl,
    Type* subtype,
    InterfaceDecl* interfaceDecl)
{
    GenericTypeConstraintDecl* result = nullptr;
    if (!genericDecl || !subtype || !interfaceDecl)
        return result;
    for (auto member : genericDecl->getDirectMemberDecls())
    {
        auto constraint = as<GenericTypeConstraintDecl>(member);
        if (!constraint || constraint->isEqualityConstraint || !constraint->sub.type ||
            !constraint->sub.type->equals(subtype))
            continue;
        auto superType = isDeclRefTypeOf<InterfaceDecl>(
            constraint->sup.type ? constraint->sup.type->resolve() : nullptr);
        if (!superType || superType.getDecl() != interfaceDecl)
            continue;
        if (result)
            return nullptr;
        result = constraint;
    }
    return result;
}

/// Returns the source type of one checked associated-type access.
///
/// Consider the trusted declaration `CallableContext.TraceContext == Schema.TraceContext`.
/// Semantic checking represents each endpoint with a `LookupDeclRef`: the declaration identifies
/// the exact associated-type requirement and `getLookupSource()` identifies either
/// `CallableContext` or `Schema`. Reading those two semantic roles is more robust than resolving
/// the type (which may erase the access through the equality) or recognizing its printed name.
static Type* _getAssociatedTypeLookupSource(Type* type, AssocTypeDecl* requirement)
{
    auto associatedType = isDeclRefTypeOf<AssocTypeDecl>(type);
    if (!associatedType || associatedType.getDecl() != requirement)
        return nullptr;

    auto lookupDeclRef = as<LookupDeclRef>(associatedType.declRefBase);
    return lookupDeclRef ? lookupDeclRef->getLookupSource() : nullptr;
}

/// Returns whether `type` is the exact associated-type access `source.requirement`.
static bool _isAssociatedTypeAccess(Type* type, AssocTypeDecl* requirement, Type* source)
{
    auto lookupSource = _getAssociatedTypeLookupSource(type, requirement);
    return lookupSource && source && lookupSource->equals(source);
}

/// Returns whether `type` is `Schema.TraceContext.requirement` for the trusted schema parameter.
static bool _isSchemaTraceContextAssociatedTypeAccess(
    Type* type,
    AssocTypeDecl* requirement,
    AssocTypeDecl* programTraceContextRequirement,
    Type* schemaType)
{
    auto traceContextType = _getAssociatedTypeLookupSource(type, requirement);
    return traceContextType &&
           _isAssociatedTypeAccess(traceContextType, programTraceContextRequirement, schemaType);
}

/// Extracts the endpoint paired with `Schema.TraceContext.requirement` by an equality.
///
/// Equality constraints are symmetric, so the standard module may spell the associated access on
/// either side without changing this compiler contract.
static Type* _getSchemaTraceContextEqualityOtherType(
    GenericTypeConstraintDecl* constraint,
    AssocTypeDecl* requirement,
    AssocTypeDecl* programTraceContextRequirement,
    Type* schemaType)
{
    if (!constraint || !constraint->isEqualityConstraint)
        return nullptr;

    if (_isSchemaTraceContextAssociatedTypeAccess(
            constraint->sub.type,
            requirement,
            programTraceContextRequirement,
            schemaType))
    {
        return constraint->sup.type;
    }
    if (_isSchemaTraceContextAssociatedTypeAccess(
            constraint->sup.type,
            requirement,
            programTraceContextRequirement,
            schemaType))
    {
        return constraint->sub.type;
    }
    return nullptr;
}

/// Returns whether an equality connects the callable and schema trace-context roles exactly.
static bool _isCallableSchemaTraceContextEquality(
    GenericTypeConstraintDecl* constraint,
    AssocTypeDecl* stageTraceContextRequirement,
    Type* callableContextType,
    AssocTypeDecl* programTraceContextRequirement,
    Type* schemaType)
{
    if (!constraint || !constraint->isEqualityConstraint)
        return false;

    auto isCallableTraceContext = [&](Type* type)
    { return _isAssociatedTypeAccess(type, stageTraceContextRequirement, callableContextType); };
    auto isSchemaTraceContext = [&](Type* type)
    { return _isAssociatedTypeAccess(type, programTraceContextRequirement, schemaType); };
    return (isCallableTraceContext(constraint->sub.type) &&
            isSchemaTraceContext(constraint->sup.type)) ||
           (isCallableTraceContext(constraint->sup.type) &&
            isSchemaTraceContext(constraint->sub.type));
}

/// Returns whether `type` is the compiler's native two-level acceleration-structure handle.
static bool _isNativeAccelerationStructureType(Type* type)
{
    return as<RaytracingAccelerationStructureType>(type ? type->resolve() : nullptr) != nullptr;
}

/// Returns whether `type` is `genericType<valueParameter>` with no other generic arguments.
///
/// Consider the parameter `MultiLevelAccelerationStructure<maxLevelCount>` on
/// `trace<let maxLevelCount>()`. The checked type carries the multi-level type's own generic
/// application, whose value operand must be a `DeclRefIntVal` for this method parameter. Checking
/// that existing semantic operand prevents a different value parameter or constant depth from
/// being accepted as the trusted signature.
static bool _isGenericTypeAppliedToValueParameter(
    ASTBuilder* astBuilder,
    Type* type,
    AggTypeDecl* genericType,
    GenericValueParamDecl* valueParameter)
{
    auto declRefType = as<DeclRefType>(type);
    auto genericDecl = as<GenericDecl>(genericType ? genericType->parentDecl : nullptr);
    if (!declRefType || declRefType->getDeclRef().getDecl() != genericType || !genericDecl ||
        genericDecl->inner != genericType || !valueParameter)
    {
        return false;
    }

    GenericValueParamDecl* genericValueParameter = nullptr;
    for (auto member : genericDecl->getDirectMemberDecls())
    {
        if (auto parameter = as<GenericValueParamDecl>(member))
        {
            if (genericValueParameter)
                return false;
            genericValueParameter = parameter;
        }
        else if (isGenericParam(member))
        {
            return false;
        }
    }
    auto application =
        SubstitutionSet(declRefType->getDeclRef()).findGenericAppDeclRef(genericDecl);
    auto argumentIndex = getGenericArgumentIndex(genericDecl, genericValueParameter);
    auto expectedValue = astBuilder->getDeclRefVal(makeDeclRef(valueParameter));
    return application && application->getArgCount() == getGenericArgumentCount(genericDecl) &&
           argumentIndex >= 0 && argumentIndex < application->getArgCount() && expectedValue &&
           application->getArg(argumentIndex)->equals(expectedValue);
}

/// Holds semantic roles decoded from one trusted `RayTracer<Schema>` extension signature.
struct _StructuralRayTracingRayTracerExtensionInfo
{
    StructuralRayTracingRayTracerMethodInfo methodInfo;
    GenericTypeConstraintDecl* accelerationStructureConstraint = nullptr;
    Type* accelerationStructureType = nullptr;
    GenericTypeConstraintDecl* motionConstraint = nullptr;
    Type* motionType = nullptr;
};

/// Decodes the semantic roles in one checked `extension<Schema> RayTracer<Schema>` signature.
///
/// The target application identifies the extension's schema parameter semantically. The matching
/// source constraint then identifies its witness argument even when other extension constraints
/// are inserted before or after it. The topology and motion equalities are retained separately so
/// the containing overload family can validate the exact ABI contract it selects.
static bool _tryGetRayTracerMethodInfo(
    ExtensionDecl* extensionDecl,
    AggTypeDecl* rayTracerType,
    InterfaceDecl* traceProgramSchemaInterface,
    AssocTypeDecl* programTraceContextRequirement,
    AssocTypeDecl* accelerationStructureRequirement,
    AssocTypeDecl* motionRequirement,
    _StructuralRayTracingRayTracerExtensionInfo& outInfo)
{
    auto rayTracerGenericDecl =
        as<GenericDecl>(rayTracerType ? rayTracerType->parentDecl : nullptr);
    auto extensionGenericDecl =
        as<GenericDecl>(extensionDecl ? extensionDecl->parentDecl : nullptr);
    if (!rayTracerGenericDecl || rayTracerGenericDecl->inner != rayTracerType ||
        !extensionGenericDecl || extensionGenericDecl->inner != extensionDecl)
    {
        return false;
    }

    auto rayTracerSchemaParameter = _getOnlyGenericTypeParameter(rayTracerGenericDecl);
    auto targetType = as<DeclRefType>(
        extensionDecl->targetType.type ? extensionDecl->targetType.type->resolve() : nullptr);
    if (!rayTracerSchemaParameter || !targetType ||
        targetType->getDeclRef().getDecl() != rayTracerType)
    {
        return false;
    }

    auto rayTracerApplication =
        SubstitutionSet(targetType->getDeclRef()).findGenericAppDeclRef(rayTracerGenericDecl);
    auto targetSchemaArgumentIndex =
        getGenericArgumentIndex(rayTracerGenericDecl, rayTracerSchemaParameter);
    if (!rayTracerApplication || targetSchemaArgumentIndex < 0 ||
        targetSchemaArgumentIndex >= rayTracerApplication->getArgCount() ||
        rayTracerApplication->getArgCount() != getGenericArgumentCount(rayTracerGenericDecl))
    {
        return false;
    }

    auto schemaType = as<Type>(rayTracerApplication->getArg(targetSchemaArgumentIndex)->resolve());
    auto schemaDeclRefType = as<DeclRefType>(schemaType);
    auto schemaParameter = schemaDeclRefType
                               ? as<GenericTypeParamDecl>(schemaDeclRefType->getDeclRef().getDecl())
                               : nullptr;
    if (!schemaParameter || schemaParameter->parentDecl != extensionGenericDecl)
        return false;

    auto schemaConstraint =
        _findConformanceConstraint(extensionGenericDecl, schemaType, traceProgramSchemaInterface);
    auto schemaTypeArgumentIndex = getGenericArgumentIndex(extensionGenericDecl, schemaParameter);
    auto schemaWitnessArgumentIndex =
        getGenericArgumentIndex(extensionGenericDecl, schemaConstraint);
    if (!schemaConstraint || schemaTypeArgumentIndex < 0 || schemaWitnessArgumentIndex < 0)
        return false;

    for (auto member : extensionGenericDecl->getDirectMemberDecls())
    {
        if (isGenericParam(member))
        {
            if (member != schemaParameter)
                return false;
            continue;
        }

        if (!isGenericConstraintParameterDecl(member))
            continue;
        auto constraint = as<GenericTypeConstraintDecl>(member);
        if (!constraint)
            return false;
        if (constraint == schemaConstraint)
            continue;

        if (auto accelerationStructureType = _getSchemaTraceContextEqualityOtherType(
                constraint,
                accelerationStructureRequirement,
                programTraceContextRequirement,
                schemaType))
        {
            if (outInfo.accelerationStructureConstraint)
                return false;
            outInfo.accelerationStructureConstraint = constraint;
            outInfo.accelerationStructureType = accelerationStructureType;
            continue;
        }
        if (auto motionType = _getSchemaTraceContextEqualityOtherType(
                constraint,
                motionRequirement,
                programTraceContextRequirement,
                schemaType))
        {
            if (outInfo.motionConstraint)
                return false;
            outInfo.motionConstraint = constraint;
            outInfo.motionType = motionType;
            continue;
        }
        return false;
    }

    outInfo.methodInfo.extensionDecl = extensionDecl;
    outInfo.methodInfo.schemaGenericDecl = extensionGenericDecl;
    outInfo.methodInfo.schemaTypeParameter = schemaParameter;
    outInfo.methodInfo.schemaConstraint = schemaConstraint;
    outInfo.methodInfo.schemaTypeArgumentIndex = schemaTypeArgumentIndex;
    outInfo.methodInfo.schemaWitnessArgumentIndex = schemaWitnessArgumentIndex;
    return true;
}

/// Returns whether `type` directly names the trusted aggregate declaration.
static bool _isDirectTypeOf(Type* type, Decl* expectedDecl)
{
    auto declRef = isDeclRefTypeOf<Decl>(type ? type->resolve() : nullptr);
    return declRef && declRef.getDecl() == expectedDecl;
}

/// Returns whether `type` is `TraceProgramDescriptor` specialized with `schemaType`.
static bool _isTraceProgramDescriptorForSchema(
    Type* type,
    AggTypeDecl* traceProgramDescriptorType,
    Type* schemaType)
{
    auto descriptorType = as<DeclRefType>(type ? type->resolve() : nullptr);
    auto descriptorGenericDecl = as<GenericDecl>(
        traceProgramDescriptorType ? traceProgramDescriptorType->parentDecl : nullptr);
    auto descriptorSchemaParameter = _getOnlyGenericTypeParameter(descriptorGenericDecl);
    if (!descriptorType || descriptorType->getDeclRef().getDecl() != traceProgramDescriptorType ||
        !descriptorGenericDecl || descriptorGenericDecl->inner != traceProgramDescriptorType ||
        !descriptorSchemaParameter)
    {
        return false;
    }

    auto descriptorApplication =
        SubstitutionSet(descriptorType->getDeclRef()).findGenericAppDeclRef(descriptorGenericDecl);
    auto schemaArgumentIndex =
        getGenericArgumentIndex(descriptorGenericDecl, descriptorSchemaParameter);
    if (!descriptorApplication || schemaArgumentIndex < 0 ||
        schemaArgumentIndex >= descriptorApplication->getArgCount() ||
        descriptorApplication->getArgCount() != getGenericArgumentCount(descriptorGenericDecl))
    {
        return false;
    }
    auto descriptorSchema = as<Type>(descriptorApplication->getArg(schemaArgumentIndex)->resolve());
    return descriptorSchema && descriptorSchema->equals(schemaType);
}

/// Holds semantic roles while one trusted trace overload is being validated.
struct _StructuralRayTracingTraceMethodCandidate
{
    FunctionDeclBase* method = nullptr;
    StructuralRayTracingTraceMethodInfo info;
    GenericTypeParamDecl* payloadGenericParameter = nullptr;
    GenericValueParamDecl* maxLevelCountParameter = nullptr;
    GenericTypeConstraintDecl* accelerationStructureConstraint = nullptr;
    bool usesMultiLevelAccelerationStructure = false;
};

/// Validates the trusted callable-dispatch method and records its generic and value roles.
///
/// `CallableContext : ICallableContext` identifies the witness used to resolve `CallableData`;
/// parameter types and directions then distinguish the callable index, descriptor, and data
/// without relying on declaration order.
static bool _tryGetCallShaderMethodInfo(
    ASTBuilder* astBuilder,
    FunctionDeclBase* functionDecl,
    AggTypeDecl* traceProgramDescriptorType,
    Type* schemaType,
    InterfaceDecl* callableContextInterface,
    AssocTypeDecl* stageTraceContextRequirement,
    AssocTypeDecl* programTraceContextRequirement,
    AssocTypeDecl* callableDataRequirement,
    StructuralRayTracingCallShaderMethodInfo& outInfo)
{
    if (!functionDecl || !functionDecl->returnType.type ||
        !functionDecl->returnType.type->equals(astBuilder->getVoidType()))
    {
        return false;
    }

    auto methodGenericDecl = as<GenericDecl>(functionDecl->parentDecl);
    if (!methodGenericDecl || methodGenericDecl->inner != functionDecl)
        return false;
    auto callableContextParameter = _getOnlyGenericTypeParameter(methodGenericDecl);
    if (!callableContextParameter)
        return false;
    auto callableContextType =
        DeclRefType::create(astBuilder, makeDeclRef(callableContextParameter));
    auto callableContextConstraint = _findConformanceConstraint(
        methodGenericDecl,
        callableContextType,
        callableContextInterface);
    auto callableContextTypeArgumentIndex =
        getGenericArgumentIndex(methodGenericDecl, callableContextParameter);
    auto callableContextWitnessArgumentIndex =
        getGenericArgumentIndex(methodGenericDecl, callableContextConstraint);
    if (!callableContextConstraint || callableContextTypeArgumentIndex < 0 ||
        callableContextWitnessArgumentIndex < 0)
    {
        return false;
    }

    GenericTypeConstraintDecl* traceContextEqualityConstraint = nullptr;
    for (auto member : methodGenericDecl->getDirectMemberDecls())
    {
        if (isGenericConstraintParameterDecl(member))
        {
            if (member != callableContextConstraint)
            {
                auto constraint = as<GenericTypeConstraintDecl>(member);
                if (!_isCallableSchemaTraceContextEquality(
                        constraint,
                        stageTraceContextRequirement,
                        callableContextType,
                        programTraceContextRequirement,
                        schemaType))
                {
                    // Other method constraints stay in the checked generic application. Lowering
                    // only needs the callable-context conformance and trace-context equality roles.
                    continue;
                }
                if (traceContextEqualityConstraint)
                {
                    return false;
                }
                traceContextEqualityConstraint = constraint;
            }
        }
        else if (isGenericParam(member) && member != callableContextParameter)
            return false;
    }
    // The equality witness is part of the trusted semantic signature, but its serialized position
    // is deliberately not part of this registry API.
    if (!traceContextEqualityConstraint)
        return false;

    auto parameters = functionDecl->getParameters();
    for (Index parameterIndex = 0; parameterIndex < parameters.getCount(); ++parameterIndex)
    {
        auto parameter = parameters[parameterIndex];
        auto parameterType = parameter->type.type;
        bool isInOut = parameter->hasModifier<InOutModifier>();
        bool isOut = parameter->hasModifier<OutModifier>();
        bool hasReferenceDirection =
            parameter->hasModifier<RefModifier>() || parameter->hasModifier<BorrowModifier>();
        bool hasNonInputDirection = isInOut || isOut || hasReferenceDirection;

        if (_isTraceProgramDescriptorForSchema(
                parameterType,
                traceProgramDescriptorType,
                schemaType))
        {
            if (outInfo.descriptorParameterIndex >= 0 || hasNonInputDirection)
                return false;
            outInfo.descriptorParameterIndex = parameterIndex;
        }
        else if (isInOut)
        {
            if (outInfo.dataParameterIndex >= 0 || hasReferenceDirection ||
                !_isAssociatedTypeAccess(
                    parameterType,
                    callableDataRequirement,
                    callableContextType))
            {
                return false;
            }
            outInfo.dataParameterIndex = parameterIndex;
        }
        else if (parameterType && parameterType->equals(astBuilder->getUIntType()))
        {
            if (outInfo.callableIndexParameterIndex >= 0 || hasNonInputDirection)
                return false;
            outInfo.callableIndexParameterIndex = parameterIndex;
        }
        else
        {
            return false;
        }
    }
    if (outInfo.callableIndexParameterIndex < 0 || outInfo.descriptorParameterIndex < 0 ||
        outInfo.dataParameterIndex < 0)
    {
        return false;
    }

    outInfo.methodGenericDecl = methodGenericDecl;
    outInfo.callableContextTypeParameter = callableContextParameter;
    outInfo.callableContextConstraint = callableContextConstraint;
    outInfo.callableContextTypeArgumentIndex = callableContextTypeArgumentIndex;
    outInfo.callableContextWitnessArgumentIndex = callableContextWitnessArgumentIndex;
    return true;
}

/// Validates one trace overload and records each source parameter and generic-argument role.
///
/// For example, the payload role is the unique `inout` parameter whose type is owned by the
/// method's generic declaration. A multi-level overload must additionally bind
/// `Schema.TraceContext.AccelerationStructure` to the exact
/// `MultiLevelAccelerationStructure<maxLevelCount>` parameter type. Neither role is inferred from
/// a parameter's name or position.
static bool _tryGetTraceMethodCandidate(
    ASTBuilder* astBuilder,
    FunctionDeclBase* functionDecl,
    AggTypeDecl* rayTraversalDescType,
    AggTypeDecl* traceProgramDescriptorType,
    AssocTypeDecl* accelerationStructureRequirement,
    AssocTypeDecl* programTraceContextRequirement,
    AggTypeDecl* multiLevelAccelerationStructureType,
    Type* schemaType,
    _StructuralRayTracingTraceMethodCandidate& outCandidate)
{
    if (!functionDecl || !functionDecl->returnType.type ||
        !functionDecl->returnType.type->equals(astBuilder->getVoidType()))
    {
        return false;
    }

    auto methodGenericDecl = as<GenericDecl>(functionDecl->parentDecl);
    if (methodGenericDecl && methodGenericDecl->inner != functionDecl)
        return false;

    if (methodGenericDecl)
    {
        for (auto member : methodGenericDecl->getDirectMemberDecls())
        {
            if (auto typeParameter = as<GenericTypeParamDecl>(member))
            {
                if (outCandidate.payloadGenericParameter)
                    return false;
                outCandidate.payloadGenericParameter = typeParameter;
            }
            else if (as<GenericTypePackParamDecl>(member) || as<GenericValuePackParamDecl>(member))
            {
                return false;
            }
            else if (auto valueParameter = as<GenericValueParamDecl>(member))
            {
                if (outCandidate.maxLevelCountParameter || !valueParameter->type.type ||
                    !valueParameter->type.type->equals(astBuilder->getIntType()))
                {
                    return false;
                }
                outCandidate.maxLevelCountParameter = valueParameter;
            }
            else if (isGenericConstraintParameterDecl(member))
            {
                auto constraint = as<GenericTypeConstraintDecl>(member);
                if (!constraint || outCandidate.accelerationStructureConstraint)
                    return false;
                outCandidate.accelerationStructureConstraint = constraint;
            }
        }
    }

    auto payloadType =
        outCandidate.payloadGenericParameter
            ? DeclRefType::create(astBuilder, makeDeclRef(outCandidate.payloadGenericParameter))
            : nullptr;

    auto parameters = functionDecl->getParameters();
    for (Index parameterIndex = 0; parameterIndex < parameters.getCount(); ++parameterIndex)
    {
        auto parameter = parameters[parameterIndex];
        auto parameterType = parameter->type.type;
        bool isInOut = parameter->hasModifier<InOutModifier>();
        bool isOut = parameter->hasModifier<OutModifier>();
        bool hasReferenceDirection =
            parameter->hasModifier<RefModifier>() || parameter->hasModifier<BorrowModifier>();
        bool hasNonInputDirection = isInOut || isOut || hasReferenceDirection;

        if (_isDirectTypeOf(parameterType, rayTraversalDescType))
        {
            if (outCandidate.info.traversalDescParameterIndex >= 0 || hasNonInputDirection)
            {
                return false;
            }
            outCandidate.info.traversalDescParameterIndex = parameterIndex;
        }
        else if (_isTraceProgramDescriptorForSchema(
                     parameterType,
                     traceProgramDescriptorType,
                     schemaType))
        {
            if (outCandidate.info.descriptorParameterIndex >= 0 || hasNonInputDirection)
            {
                return false;
            }
            outCandidate.info.descriptorParameterIndex = parameterIndex;
        }
        else if (isInOut)
        {
            if (outCandidate.info.payloadParameterIndex >= 0 || hasReferenceDirection ||
                !payloadType || !parameterType || !parameterType->equals(payloadType))
            {
                return false;
            }
            outCandidate.info.payloadParameterIndex = parameterIndex;
        }
        else if (_isSchemaTraceContextAssociatedTypeAccess(
                     parameterType,
                     accelerationStructureRequirement,
                     programTraceContextRequirement,
                     schemaType))
        {
            if (outCandidate.info.accelerationStructureParameterIndex >= 0 || hasNonInputDirection)
            {
                return false;
            }
            outCandidate.info.accelerationStructureParameterIndex = parameterIndex;
        }
        else if (_isGenericTypeAppliedToValueParameter(
                     astBuilder,
                     parameterType,
                     multiLevelAccelerationStructureType,
                     outCandidate.maxLevelCountParameter))
        {
            if (outCandidate.info.accelerationStructureParameterIndex >= 0 || hasNonInputDirection)
            {
                return false;
            }
            outCandidate.info.accelerationStructureParameterIndex = parameterIndex;
            outCandidate.usesMultiLevelAccelerationStructure = true;
        }
        else
        {
            return false;
        }
    }

    if (outCandidate.info.traversalDescParameterIndex < 0 ||
        outCandidate.info.accelerationStructureParameterIndex < 0 ||
        outCandidate.info.descriptorParameterIndex < 0)
    {
        return false;
    }

    if (payloadType)
    {
        if (outCandidate.info.payloadParameterIndex < 0)
            return false;
        outCandidate.info.kind = StructuralRayTracingTraceMethodKind::ExplicitPayload;
        outCandidate.info.payloadGenericArgumentIndex =
            getGenericArgumentIndex(methodGenericDecl, outCandidate.payloadGenericParameter);
        if (outCandidate.info.payloadGenericArgumentIndex < 0)
            return false;
    }
    else
    {
        if (outCandidate.info.payloadParameterIndex >= 0)
            return false;
        outCandidate.info.kind = StructuralRayTracingTraceMethodKind::ImplicitEmptyPayload;
    }

    auto accelerationStructureType =
        parameters[outCandidate.info.accelerationStructureParameterIndex]->type.type;
    if (outCandidate.usesMultiLevelAccelerationStructure)
    {
        auto equalityOtherType = _getSchemaTraceContextEqualityOtherType(
            outCandidate.accelerationStructureConstraint,
            accelerationStructureRequirement,
            programTraceContextRequirement,
            schemaType);
        if (!outCandidate.maxLevelCountParameter || !equalityOtherType ||
            !accelerationStructureType || !equalityOtherType->equals(accelerationStructureType))
        {
            return false;
        }
    }
    else if (outCandidate.maxLevelCountParameter || outCandidate.accelerationStructureConstraint)
    {
        return false;
    }

    outCandidate.method = functionDecl;
    outCandidate.info.methodGenericDecl = methodGenericDecl;
    return true;
}

/// Pairs the empty-payload overload with its payload-taking implementation.
///
/// The generated mapping is indexed by the payload overload's checked generic-argument layout.
/// Thus `<Payload, let N>` and `<let N, Payload>` produce different mappings without changing any
/// lowering consumer.
static bool _pairTraceMethodCandidates(
    _StructuralRayTracingTraceMethodCandidate& implicitPayload,
    _StructuralRayTracingTraceMethodCandidate& explicitPayload)
{
    if (!implicitPayload.method || !explicitPayload.method ||
        implicitPayload.info.kind != StructuralRayTracingTraceMethodKind::ImplicitEmptyPayload ||
        explicitPayload.info.kind != StructuralRayTracingTraceMethodKind::ExplicitPayload ||
        implicitPayload.usesMultiLevelAccelerationStructure !=
            explicitPayload.usesMultiLevelAccelerationStructure ||
        bool(implicitPayload.maxLevelCountParameter) !=
            bool(explicitPayload.maxLevelCountParameter) ||
        bool(implicitPayload.accelerationStructureConstraint) !=
            bool(explicitPayload.accelerationStructureConstraint))
    {
        return false;
    }

    auto implicitParameters = implicitPayload.method->getParameters();
    auto explicitParameters = explicitPayload.method->getParameters();
    auto implicitAccelerationType =
        implicitParameters[implicitPayload.info.accelerationStructureParameterIndex]->type.type;
    auto explicitAccelerationType =
        explicitParameters[explicitPayload.info.accelerationStructureParameterIndex]->type.type;
    if (implicitPayload.maxLevelCountParameter)
    {
        if (!implicitPayload.accelerationStructureConstraint ||
            !explicitPayload.accelerationStructureConstraint ||
            !implicitPayload.maxLevelCountParameter->type.type ||
            !explicitPayload.maxLevelCountParameter->type.type ||
            !implicitPayload.maxLevelCountParameter->type.type->equals(
                explicitPayload.maxLevelCountParameter->type.type))
        {
            return false;
        }
    }
    else if (
        !implicitAccelerationType || !explicitAccelerationType ||
        !implicitAccelerationType->equals(explicitAccelerationType))
    {
        return false;
    }

    auto explicitGenericDecl = explicitPayload.info.methodGenericDecl;
    auto explicitArgumentCount = getGenericArgumentCount(explicitGenericDecl);
    auto implicitArgumentCount = getGenericArgumentCount(implicitPayload.info.methodGenericDecl);
    if (explicitArgumentCount <= 0)
        return false;

    implicitPayload.info.pairedPayloadMethod = explicitPayload.method;
    implicitPayload.info.pairedPayloadGenericArguments.setCount(explicitArgumentCount);
    List<bool> initializedArguments;
    initializedArguments.setCount(explicitArgumentCount);
    for (Index i = 0; i < explicitArgumentCount; ++i)
        initializedArguments[i] = false;

    auto setArgument = [&](Index targetIndex,
                           StructuralRayTracingPairedTraceArgumentSourceKind kind,
                           Index sourceIndex)
    {
        if (targetIndex < 0 || targetIndex >= explicitArgumentCount ||
            initializedArguments[targetIndex] ||
            (kind == StructuralRayTracingPairedTraceArgumentSourceKind::
                         ImplicitEmptyPayloadMethodArgument &&
             (sourceIndex < 0 || sourceIndex >= implicitArgumentCount)))
        {
            return false;
        }
        auto& argument = implicitPayload.info.pairedPayloadGenericArguments[targetIndex];
        argument.kind = kind;
        argument.argumentIndex = sourceIndex;
        initializedArguments[targetIndex] = true;
        return true;
    };

    if (!setArgument(
            explicitPayload.info.payloadGenericArgumentIndex,
            StructuralRayTracingPairedTraceArgumentSourceKind::PayloadType,
            -1))
    {
        return false;
    }

    if (explicitPayload.maxLevelCountParameter)
    {
        if (!setArgument(
                getGenericArgumentIndex(
                    explicitGenericDecl,
                    explicitPayload.maxLevelCountParameter),
                StructuralRayTracingPairedTraceArgumentSourceKind::
                    ImplicitEmptyPayloadMethodArgument,
                getGenericArgumentIndex(
                    implicitPayload.info.methodGenericDecl,
                    implicitPayload.maxLevelCountParameter)))
        {
            return false;
        }
    }

    if (explicitPayload.accelerationStructureConstraint)
    {
        if (!setArgument(
                getGenericArgumentIndex(
                    explicitGenericDecl,
                    explicitPayload.accelerationStructureConstraint),
                StructuralRayTracingPairedTraceArgumentSourceKind::
                    ImplicitEmptyPayloadMethodArgument,
                getGenericArgumentIndex(
                    implicitPayload.info.methodGenericDecl,
                    implicitPayload.accelerationStructureConstraint)))
        {
            return false;
        }
    }

    for (auto initialized : initializedArguments)
    {
        if (!initialized)
            return false;
    }
    return true;
}

/// Validates and registers all compiler-owned methods on trusted `RayTracer` extensions.
static bool _registerRayTracerMethods(
    ASTBuilder* astBuilder,
    ContainerDecl* container,
    AggTypeDecl* rayTracerType,
    InterfaceDecl* traceProgramSchemaInterface,
    AggTypeDecl* rayTraversalDescType,
    AggTypeDecl* traceProgramDescriptorType,
    AssocTypeDecl* accelerationStructureRequirement,
    AssocTypeDecl* motionRequirement,
    AssocTypeDecl* stageTraceContextRequirement,
    AssocTypeDecl* programTraceContextRequirement,
    AggTypeDecl* multiLevelAccelerationStructureType,
    AggTypeDecl* const* motionTypes,
    InterfaceDecl* callableContextInterface,
    AssocTypeDecl* callableDataRequirement,
    Dictionary<FunctionDeclBase*, StructuralRayTracingTraceMethodInfo>& traceMethods,
    Dictionary<FunctionDeclBase*, StructuralRayTracingRayTracerMethodInfo>& rayTracerMethods,
    Dictionary<FunctionDeclBase*, StructuralRayTracingCallShaderMethodInfo>& callShaderMethods)
{
    for (auto member : container->getDirectMemberDecls())
    {
        auto candidate = member;
        if (auto genericDecl = as<GenericDecl>(candidate))
            candidate = genericDecl->inner;

        if (auto extensionDecl = as<ExtensionDecl>(candidate))
        {
            auto targetType = as<DeclRefType>(extensionDecl->targetType.type);
            if (targetType && targetType->getDeclRef().getDecl() == rayTracerType)
            {
                _StructuralRayTracingRayTracerExtensionInfo rayTracerExtensionInfo;
                if (!_tryGetRayTracerMethodInfo(
                        extensionDecl,
                        rayTracerType,
                        traceProgramSchemaInterface,
                        programTraceContextRequirement,
                        accelerationStructureRequirement,
                        motionRequirement,
                        rayTracerExtensionInfo))
                {
                    return false;
                }
                auto& rayTracerMethodInfo = rayTracerExtensionInfo.methodInfo;
                auto schemaType = DeclRefType::create(
                    astBuilder,
                    makeDeclRef(rayTracerMethodInfo.schemaTypeParameter));

                _StructuralRayTracingTraceMethodCandidate payloadMethod;
                _StructuralRayTracingTraceMethodCandidate noPayloadMethod;
                FunctionDeclBase* callShaderMethod = nullptr;
                StructuralRayTracingCallShaderMethodInfo callShaderMethodInfo;
                Index traceMethodCount = 0;
                for (auto extensionMember : extensionDecl->getDirectMemberDecls())
                {
                    auto methodCandidate = extensionMember;
                    if (auto genericDecl = as<GenericDecl>(methodCandidate))
                        methodCandidate = genericDecl->inner;
                    auto functionDecl = as<FunctionDeclBase>(methodCandidate);
                    if (!functionDecl || !functionDecl->getName())
                        continue;

                    if (functionDecl->getName()->text == "trace")
                    {
                        _StructuralRayTracingTraceMethodCandidate traceMethod;
                        if (!_tryGetTraceMethodCandidate(
                                astBuilder,
                                functionDecl,
                                rayTraversalDescType,
                                traceProgramDescriptorType,
                                accelerationStructureRequirement,
                                programTraceContextRequirement,
                                multiLevelAccelerationStructureType,
                                schemaType,
                                traceMethod))
                        {
                            return false;
                        }
                        ++traceMethodCount;
                        if (traceMethod.info.kind ==
                            StructuralRayTracingTraceMethodKind::ExplicitPayload)
                        {
                            if (payloadMethod.method)
                                return false;
                            payloadMethod = traceMethod;
                        }
                        else
                        {
                            if (noPayloadMethod.method)
                                return false;
                            noPayloadMethod = traceMethod;
                        }
                    }
                    else if (functionDecl->getName()->text == "callShader")
                    {
                        if (callShaderMethod || !_tryGetCallShaderMethodInfo(
                                                    astBuilder,
                                                    functionDecl,
                                                    traceProgramDescriptorType,
                                                    schemaType,
                                                    callableContextInterface,
                                                    stageTraceContextRequirement,
                                                    programTraceContextRequirement,
                                                    callableDataRequirement,
                                                    callShaderMethodInfo))
                        {
                            return false;
                        }
                        callShaderMethod = functionDecl;
                    }
                }

                if (traceMethodCount)
                {
                    if (traceMethodCount != 2 || callShaderMethod ||
                        !_pairTraceMethodCandidates(noPayloadMethod, payloadMethod) ||
                        !rayTracerExtensionInfo.motionConstraint)
                    {
                        return false;
                    }

                    bool hasKnownMotionType = false;
                    for (Index i = 0; i < 4; ++i)
                    {
                        if (_isDirectTypeOf(rayTracerExtensionInfo.motionType, motionTypes[i]))
                        {
                            hasKnownMotionType = true;
                            break;
                        }
                    }
                    if (!hasKnownMotionType)
                        return false;

                    if (payloadMethod.usesMultiLevelAccelerationStructure)
                    {
                        if (rayTracerExtensionInfo.accelerationStructureConstraint)
                            return false;
                    }
                    else if (
                        !rayTracerExtensionInfo.accelerationStructureConstraint ||
                        !_isNativeAccelerationStructureType(
                            rayTracerExtensionInfo.accelerationStructureType))
                    {
                        return false;
                    }
                    traceMethods[payloadMethod.method] = payloadMethod.info;
                    traceMethods[noPayloadMethod.method] = noPayloadMethod.info;
                    rayTracerMethods[payloadMethod.method] = rayTracerMethodInfo;
                    rayTracerMethods[noPayloadMethod.method] = rayTracerMethodInfo;
                }
                else
                {
                    if (!callShaderMethod ||
                        rayTracerExtensionInfo.accelerationStructureConstraint ||
                        rayTracerExtensionInfo.motionConstraint)
                    {
                        // The callable-only extension has no topology or motion equality.
                        // Accepting one here would let an unrelated extension constraint
                        // masquerade as part of the compiler-owned callable signature.
                        return false;
                    }
                    callShaderMethods[callShaderMethod] = callShaderMethodInfo;
                    rayTracerMethods[callShaderMethod] = rayTracerMethodInfo;
                }
            }
        }

        if (auto childContainer = as<ContainerDecl>(candidate))
        {
            if (!_registerRayTracerMethods(
                    astBuilder,
                    childContainer,
                    rayTracerType,
                    traceProgramSchemaInterface,
                    rayTraversalDescType,
                    traceProgramDescriptorType,
                    accelerationStructureRequirement,
                    motionRequirement,
                    stageTraceContextRequirement,
                    programTraceContextRequirement,
                    multiLevelAccelerationStructureType,
                    motionTypes,
                    callableContextInterface,
                    callableDataRequirement,
                    traceMethods,
                    rayTracerMethods,
                    callShaderMethods))
            {
                return false;
            }
        }
    }
    return true;
}

bool StructuralRayTracingDeclRegistry::registerTrustedModule(
    Module* module,
    StructuralRayTracingStageKind* outMissingStage)
{
    m_trustedModuleDecl = module->getModuleDecl();
    m_intersectionStageInterface = as<InterfaceDecl>(_findNamedDecl(module, "IIntersectionStage"));
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

    m_associatedTypeRequirements[int(
        StructuralRayTracingAssociatedTypeKind::TraceAccelerationStructure)] =
        _findAssociatedTypeRequirement(module, "ITraceContext", "AccelerationStructure");
    m_associatedTypeRequirements[int(StructuralRayTracingAssociatedTypeKind::TraceMotion)] =
        _findAssociatedTypeRequirement(module, "ITraceContext", "Motion");
    m_associatedTypeRequirements[int(StructuralRayTracingAssociatedTypeKind::StageTraceContext)] =
        _findAssociatedTypeRequirement(module, "IStageContext", "TraceContext");
    m_associatedTypeRequirements[int(StructuralRayTracingAssociatedTypeKind::StageRecord)] =
        _findAssociatedTypeRequirement(module, "IStageContext", "Record");
    m_associatedTypeRequirements[int(
        StructuralRayTracingAssociatedTypeKind::PayloadContextPayload)] =
        _findAssociatedTypeRequirement(module, "IPayloadContext", "Payload");
    m_associatedTypeRequirements[int(StructuralRayTracingAssociatedTypeKind::HitPrimitive)] =
        _findAssociatedTypeRequirement(module, "IHitContext", "Primitive");
    m_associatedTypeRequirements[int(StructuralRayTracingAssociatedTypeKind::PrimitiveAttributes)] =
        _findAssociatedTypeRequirement(module, "IIntersectionPrimitive", "Attributes");
    m_associatedTypeRequirements[int(StructuralRayTracingAssociatedTypeKind::CallableData)] =
        _findAssociatedTypeRequirement(module, "ICallableContext", "CallableData");
    m_associatedTypeRequirements[int(StructuralRayTracingAssociatedTypeKind::ProgramTraceContext)] =
        _findAssociatedTypeRequirement(module, "ITraceProgramSchema", "TraceContext");
    m_associatedTypeRequirements[int(StructuralRayTracingAssociatedTypeKind::ProgramHitGroups)] =
        _findAssociatedTypeRequirement(module, "ITraceProgramSchema", "HitGroups");
    m_associatedTypeRequirements[int(StructuralRayTracingAssociatedTypeKind::ProgramMissShaders)] =
        _findAssociatedTypeRequirement(module, "ITraceProgramSchema", "MissShaders");
    m_associatedTypeRequirements[int(
        StructuralRayTracingAssociatedTypeKind::ProgramCallableShaders)] =
        _findAssociatedTypeRequirement(module, "ITraceProgramSchema", "CallableShaders");
    m_associatedTypeRequirements[int(StructuralRayTracingAssociatedTypeKind::HitGroupContext)] =
        _findAssociatedTypeRequirement(module, "IHitGroup", "Context");
    m_associatedTypeRequirements[int(StructuralRayTracingAssociatedTypeKind::HitGroupClosestHit)] =
        _findAssociatedTypeRequirement(module, "IHitGroup", "ClosestHit");
    m_associatedTypeRequirements[int(StructuralRayTracingAssociatedTypeKind::HitGroupAnyHit)] =
        _findAssociatedTypeRequirement(module, "IHitGroup", "AnyHit");
    m_associatedTypeRequirements[int(
        StructuralRayTracingAssociatedTypeKind::HitGroupIntersection)] =
        _findAssociatedTypeRequirement(module, "IHitGroup", "Intersection");
    m_associatedTypeRequirements[int(
        StructuralRayTracingAssociatedTypeKind::ClosestHitShaderContext)] =
        _findAssociatedTypeRequirement(module, "IClosestHitShader", "Context");
    m_associatedTypeRequirements[int(StructuralRayTracingAssociatedTypeKind::AnyHitShaderContext)] =
        _findAssociatedTypeRequirement(module, "IAnyHitShader", "Context");
    m_associatedTypeRequirements[int(
        StructuralRayTracingAssociatedTypeKind::IntersectionStageContext)] =
        _findAssociatedTypeRequirement(module, "IIntersectionStage", "Context");
    m_associatedTypeRequirements[int(StructuralRayTracingAssociatedTypeKind::MissShaderContext)] =
        _findAssociatedTypeRequirement(module, "IMissShader", "Context");
    m_associatedTypeRequirements[int(
        StructuralRayTracingAssociatedTypeKind::CallableShaderContext)] =
        _findAssociatedTypeRequirement(module, "ICallableShader", "Context");

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
    auto traceProgramSchemaInterface =
        as<InterfaceDecl>(_findNamedDecl(module, "ITraceProgramSchema"));
    auto rayTraversalDescType = as<AggTypeDecl>(_findNamedDecl(module, "RayTraversalDesc"));
    auto traceProgramDescriptorType =
        as<AggTypeDecl>(_findNamedDecl(module, "TraceProgramDescriptor"));
    auto accelerationStructureRequirement = getAssociatedTypeRequirement(
        StructuralRayTracingAssociatedTypeKind::TraceAccelerationStructure);
    auto motionRequirement =
        getAssociatedTypeRequirement(StructuralRayTracingAssociatedTypeKind::TraceMotion);
    auto stageTraceContextRequirement =
        getAssociatedTypeRequirement(StructuralRayTracingAssociatedTypeKind::StageTraceContext);
    auto programTraceContextRequirement =
        getAssociatedTypeRequirement(StructuralRayTracingAssociatedTypeKind::ProgramTraceContext);
    auto multiLevelAccelerationStructureType =
        as<AggTypeDecl>(_findNamedDecl(module, "MultiLevelAccelerationStructure"));
    auto callableContextInterface = as<InterfaceDecl>(_findNamedDecl(module, "ICallableContext"));
    auto callableDataRequirement =
        getAssociatedTypeRequirement(StructuralRayTracingAssociatedTypeKind::CallableData);
    // These declarations form a private compiler/standard-module contract. Validate the complete
    // contract once here so later checking and lowering can consume semantic roles directly.
    bool registeredRayTracerMethods = _registerRayTracerMethods(
        module->getASTBuilder(),
        module->getModuleDecl(),
        m_rayTracerType,
        traceProgramSchemaInterface,
        rayTraversalDescType,
        traceProgramDescriptorType,
        accelerationStructureRequirement,
        motionRequirement,
        stageTraceContextRequirement,
        programTraceContextRequirement,
        multiLevelAccelerationStructureType,
        m_motionTypes,
        callableContextInterface,
        callableDataRequirement,
        m_traceMethods,
        m_rayTracerMethods,
        m_callShaderMethods);
    // A missing stage interface uses the recoverable `outMissingStage` load diagnostic above. A
    // malformed trace/callable signature has no corresponding user-actionable diagnostic: this
    // module is compiler-shipped and trusted, so such a mismatch means the compiler and its
    // standard module were built from incompatible sources.
    SLANG_RELEASE_ASSERT(
        registeredRayTracerMethods && m_traceMethods.getCount() != 0 &&
        m_callShaderMethods.getCount() != 0);
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

static SubtypeWitness* _projectStructuralRayTracingWitnessToInterface(
    ASTBuilder* astBuilder,
    SharedSemanticsContext* sharedSemantics,
    SubtypeWitness* witness,
    InterfaceDecl* targetInterface)
{
    if (!astBuilder || !sharedSemantics || !witness || !targetInterface)
        return nullptr;

    // Consider a concrete `HitContext : IHitContext`, where `IHitContext` inherits
    // `IPayloadContext`, which inherits `IStageContext`. The checked `HitContext : IHitContext`
    // witness cannot directly answer a requirement declared by `IStageContext`. Project that
    // exact witness through the checked interface-inheritance path. This matters when the context
    // has another route to `IStageContext`: a fresh subtype query could select that route instead
    // of the `IHitContext` proof supplied by the caller.
    auto targetType = DeclRefType::create(astBuilder, makeDeclRef(targetInterface));
    return sharedSemantics->tryProjectInterfaceSubtypeWitness(witness, targetType);
}

static SubtypeWitness* _projectStructuralRayTracingWitnessToRequirement(
    ASTBuilder* astBuilder,
    SharedSemanticsContext* sharedSemantics,
    SubtypeWitness* witness,
    AssocTypeDecl* requirement)
{
    auto declaringInterface = requirement ? as<InterfaceDecl>(requirement->parentDecl) : nullptr;
    SLANG_RELEASE_ASSERT(declaringInterface);
    return _projectStructuralRayTracingWitnessToInterface(
        astBuilder,
        sharedSemantics,
        witness,
        declaringInterface);
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

    auto trustedModule = m_trustedModuleDecl ? m_trustedModuleDecl->module : nullptr;
    auto sharedSemantics =
        trustedModule ? trustedModule->getLinkage()->getSemanticsForReflection() : nullptr;
    witness = _projectStructuralRayTracingWitnessToRequirement(
        astBuilder,
        sharedSemantics,
        witness,
        requirement);
    if (!witness)
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

    auto associatedType = getAssociatedTypeRequirement(kind);
    auto trustedModule = m_trustedModuleDecl ? m_trustedModuleDecl->module : nullptr;
    auto sharedSemantics =
        trustedModule ? trustedModule->getLinkage()->getSemanticsForReflection() : nullptr;
    witness = _projectStructuralRayTracingWitnessToRequirement(
        astBuilder,
        sharedSemantics,
        witness,
        associatedType);
    if (!witness)
        return nullptr;

    auto requirementWitness = tryLookUpRequirementWitness(astBuilder, witness, requirement);
    if (requirementWitness.getFlavor() == RequirementWitness::Flavor::val)
        return as<SubtypeWitness>(requirementWitness.getVal()->resolve());
    return nullptr;
}

StructuralRayTracingEntryPack getStructuralRayTracingEntryPack(
    ASTBuilder* astBuilder,
    Type* entryListType)
{
    StructuralRayTracingEntryPack result;
    if (auto declRefType = as<DeclRefType>(entryListType))
    {
        if (auto genericApp = SubstitutionSet(declRefType->getDeclRef()).findGenericAppDeclRef())
        {
            // A sealed entry-list type has one concrete type pack and, for a non-empty list, one
            // matching conformance-witness pack. Select them by semantic role instead of relying
            // on their positions among the generic arguments. Both IR lowering and reflection
            // consume this exact checked representation.
            for (auto argument : genericApp->getArgs())
            {
                auto resolvedArgument = argument->resolve();
                if (auto typePack = as<ConcreteTypePack>(resolvedArgument))
                    result.types = typePack;
                else if (auto witnessPack = as<TypePackSubtypeWitness>(resolvedArgument))
                    result.witnesses = witnessPack;
            }
        }
    }
    if (!result.types)
        result.types = astBuilder->getTypePack(ArrayView<Type*>());
    SLANG_RELEASE_ASSERT(
        !result.witnesses || result.witnesses->getCount() == result.types->getTypeCount());
    return result;
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
    if (interfaceDecl == m_intersectionStageInterface)
        return StructuralRayTracingStageKind::Intersection;
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

StructuralRayTracingTraceMethodKind StructuralRayTracingDeclRegistry::getTraceMethodKind(
    FunctionDeclBase* functionDecl) const
{
    if (auto info = getTraceMethodInfo(functionDecl))
        return info->kind;
    return StructuralRayTracingTraceMethodKind::None;
}

const StructuralRayTracingTraceMethodInfo* StructuralRayTracingDeclRegistry::getTraceMethodInfo(
    FunctionDeclBase* functionDecl) const
{
    return m_traceMethods.tryGetValue(functionDecl);
}

const StructuralRayTracingRayTracerMethodInfo* StructuralRayTracingDeclRegistry::
    getRayTracerMethodInfo(FunctionDeclBase* functionDecl) const
{
    return m_rayTracerMethods.tryGetValue(functionDecl);
}

const StructuralRayTracingCallShaderMethodInfo* StructuralRayTracingDeclRegistry::
    getCallShaderMethodInfo(FunctionDeclBase* functionDecl) const
{
    return m_callShaderMethods.tryGetValue(functionDecl);
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
