#include "slang-structural-ray-tracing.h"

#include "slang-ast-builder.h"
#include "slang-ast-decl.h"
#include "slang-check-impl.h"
#include "slang-ir-insts.h"
#include "slang-lookup.h"
#include "slang-mangle.h"
#include "slang-module.h"
#include "slang-syntax.h"

namespace Slang
{

bool isCoreLegacyRayTracingPipelineMethod(FunctionDeclBase* functionDecl)
{
    if (!functionDecl || !functionDecl->getName())
        return false;

    auto name = functionDecl->getName()->text.getUnownedSlice();
    if (name != "TraceRay" && name != "TraceMotionRay" && name != "CallShader")
        return false;

    for (auto parent = functionDecl->parentDecl; parent; parent = parent->parentDecl)
    {
        if (as<AggTypeDecl>(parent))
            return false;
        if (auto moduleDecl = as<ModuleDecl>(parent))
            return moduleDecl->hasModifier<FromCoreModuleModifier>();
    }
    return false;
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

// The annotation already identifies the AST type that DeclRefType::create constructs. Reading
// its class here avoids constructing a type merely to classify a declaration during checking.
static ASTNodeType _getMagicTypeClass(Decl* declaration)
{
    auto modifier = declaration ? declaration->findModifier<MagicTypeModifier>() : nullptr;
    return modifier && modifier->magicNodeType ? modifier->magicNodeType.getTag()
                                               : ASTNodeType::CountOf;
}

bool isStructuralRayTracingDeclaration(Decl* declaration)
{
    if (auto genericDecl = as<GenericDecl>(declaration))
        declaration = genericDecl->inner;
    if (!declaration)
        return false;
    if (declaration->hasModifier<RayTracingTraceAttribute>() ||
        declaration->hasModifier<RayTracingCallShaderAttribute>())
        return true;

    // An extension carries its target's schema parameters, but arbitrary methods declared inside
    // it do not become structural operations. Their own generic arguments still need checking.
    if (auto extensionDecl = as<ExtensionDecl>(declaration))
    {
        auto targetType = as<DeclRefType>(extensionDecl->targetType.type);
        declaration = targetType ? targetType->getDeclRef().getDecl() : nullptr;
    }
    if (!declaration)
        return false;
    auto magicModifier = declaration->findModifier<MagicTypeModifier>();
    if (magicModifier && magicModifier->magicNodeType &&
        magicModifier->magicNodeType.isSubClassOf<ModuleBuiltinType>())
        return true;
    auto intrinsicModifier = declaration->findModifier<IntrinsicTypeModifier>();
    return intrinsicModifier && intrinsicModifier->irOp == kIROp_TraceProgramDescriptorType;
}

StructuralRayTracingStageKind getStructuralRayTracingStageKind(InterfaceDecl* interfaceDecl)
{
    switch (_getMagicTypeClass(interfaceDecl))
    {
    case ASTNodeType::ClosestHitShaderType:
        return StructuralRayTracingStageKind::ClosestHit;
    case ASTNodeType::AnyHitShaderType:
        return StructuralRayTracingStageKind::AnyHit;
    case ASTNodeType::IntersectionShaderType:
    case ASTNodeType::IntersectionStageType:
        return StructuralRayTracingStageKind::Intersection;
    case ASTNodeType::MissShaderType:
        return StructuralRayTracingStageKind::Miss;
    case ASTNodeType::CallableShaderType:
        return StructuralRayTracingStageKind::Callable;
    default:
        return StructuralRayTracingStageKind::Count;
    }
}

bool isExecutableStructuralRayTracingStageInterface(InterfaceDecl* interfaceDecl)
{
    auto modifier = interfaceDecl ? interfaceDecl->findModifier<MagicTypeModifier>() : nullptr;
    return modifier && modifier->magicNodeType &&
           modifier->magicNodeType.isSubClassOf<RayTracingStageInterfaceType>();
}

StructuralRayTracingStageKind getStructuralRayTracingStageInputKind(AggTypeDecl* typeDecl)
{
    switch (_getMagicTypeClass(typeDecl))
    {
    case ASTNodeType::ClosestHitInputType:
        return StructuralRayTracingStageKind::ClosestHit;
    case ASTNodeType::AnyHitInputType:
        return StructuralRayTracingStageKind::AnyHit;
    case ASTNodeType::IntersectionInputType:
        return StructuralRayTracingStageKind::Intersection;
    case ASTNodeType::MissInputType:
        return StructuralRayTracingStageKind::Miss;
    case ASTNodeType::CallableInputType:
        return StructuralRayTracingStageKind::Callable;
    default:
        return StructuralRayTracingStageKind::Count;
    }
}

StructuralRayTracingMetadataKind getStructuralRayTracingMetadataKind(InterfaceDecl* interfaceDecl)
{
    switch (_getMagicTypeClass(interfaceDecl))
    {
    case ASTNodeType::RayTracingHitGroupType:
        return StructuralRayTracingMetadataKind::HitGroup;
    case ASTNodeType::RayTracingHitGroupListType:
        return StructuralRayTracingMetadataKind::HitGroupList;
    case ASTNodeType::RayTracingMissShaderListType:
        return StructuralRayTracingMetadataKind::MissShaderList;
    case ASTNodeType::RayTracingCallableShaderListType:
        return StructuralRayTracingMetadataKind::CallableShaderList;
    case ASTNodeType::RayTracingProgramSchemaType:
        return StructuralRayTracingMetadataKind::TraceProgramSchema;
    default:
        return StructuralRayTracingMetadataKind::Count;
    }
}

StructuralRayTracingAssociatedTypeKind getStructuralRayTracingAssociatedTypeKind(
    AssocTypeDecl* requirement)
{
    if (!requirement || !requirement->getName())
        return StructuralRayTracingAssociatedTypeKind::Count;
    auto name = requirement->getName()->text.getUnownedSlice();
    switch (_getMagicTypeClass(requirement->parentDecl))
    {
    case ASTNodeType::RayTracingTraceContextType:
        if (name == "AccelerationStructure")
            return StructuralRayTracingAssociatedTypeKind::TraceAccelerationStructure;
        if (name == "Motion")
            return StructuralRayTracingAssociatedTypeKind::TraceMotion;
        break;
    case ASTNodeType::RayTracingStageContextType:
        if (name == "TraceContext")
            return StructuralRayTracingAssociatedTypeKind::StageTraceContext;
        if (name == "Record")
            return StructuralRayTracingAssociatedTypeKind::StageRecord;
        break;
    case ASTNodeType::RayTracingPayloadContextType:
        if (name == "Payload")
            return StructuralRayTracingAssociatedTypeKind::PayloadContextPayload;
        break;
    case ASTNodeType::RayTracingHitContextType:
        if (name == "Primitive")
            return StructuralRayTracingAssociatedTypeKind::HitPrimitive;
        break;
    case ASTNodeType::RayTracingIntersectionPrimitiveType:
        if (name == "Attributes")
            return StructuralRayTracingAssociatedTypeKind::PrimitiveAttributes;
        break;
    case ASTNodeType::RayTracingCallableContextType:
        if (name == "CallableData")
            return StructuralRayTracingAssociatedTypeKind::CallableData;
        break;
    case ASTNodeType::RayTracingProgramSchemaType:
        if (name == "TraceContext")
            return StructuralRayTracingAssociatedTypeKind::ProgramTraceContext;
        if (name == "HitGroups")
            return StructuralRayTracingAssociatedTypeKind::ProgramHitGroups;
        if (name == "MissShaders")
            return StructuralRayTracingAssociatedTypeKind::ProgramMissShaders;
        if (name == "CallableShaders")
            return StructuralRayTracingAssociatedTypeKind::ProgramCallableShaders;
        break;
    case ASTNodeType::RayTracingHitGroupType:
        if (name == "Context")
            return StructuralRayTracingAssociatedTypeKind::HitGroupContext;
        if (name == "ClosestHit")
            return StructuralRayTracingAssociatedTypeKind::HitGroupClosestHit;
        if (name == "AnyHit")
            return StructuralRayTracingAssociatedTypeKind::HitGroupAnyHit;
        if (name == "Intersection")
            return StructuralRayTracingAssociatedTypeKind::HitGroupIntersection;
        break;
    case ASTNodeType::ClosestHitShaderType:
        if (name == "Context")
            return StructuralRayTracingAssociatedTypeKind::ClosestHitShaderContext;
        break;
    case ASTNodeType::AnyHitShaderType:
        if (name == "Context")
            return StructuralRayTracingAssociatedTypeKind::AnyHitShaderContext;
        break;
    case ASTNodeType::IntersectionStageType:
        if (name == "Context")
            return StructuralRayTracingAssociatedTypeKind::IntersectionStageContext;
        break;
    case ASTNodeType::MissShaderType:
        if (name == "Context")
            return StructuralRayTracingAssociatedTypeKind::MissShaderContext;
        break;
    case ASTNodeType::CallableShaderType:
        if (name == "Context")
            return StructuralRayTracingAssociatedTypeKind::CallableShaderContext;
        break;
    default:
        break;
    }
    return StructuralRayTracingAssociatedTypeKind::Count;
}

// Finds a requirement in the interface supplied by the caller's witness and projects that same
// witness to its owner. Consider `HitContext : IHitContext`, with `IHitContext : IPayloadContext`
// and `IPayloadContext : IStageContext`. Resolving `Record` needs the inherited IStageContext
// requirement. The checked interface facets identify that declaration; projecting the original
// witness preserves this path even when HitContext has another IStageContext conformance.
static AssocTypeDecl* _findStructuralAssociatedRequirement(
    SubtypeWitness*& witness,
    StructuralRayTracingAssociatedTypeKind kind)
{
    witness = witness ? as<SubtypeWitness>(witness->resolve()) : nullptr;
    if (!witness)
        return nullptr;
    auto interfaceType = as<DeclRefType>(witness->getSup()->resolve());
    auto interfaceDecl =
        interfaceType ? interfaceType->getDeclRef().as<InterfaceDecl>() : DeclRef<InterfaceDecl>();
    if (!interfaceDecl)
        return nullptr;
    auto module = getModule(interfaceDecl.getDecl());
    SLANG_RELEASE_ASSERT(module);
    auto sharedSemantics = module->getLinkage()->getSemanticsForReflection();
    for (auto facet : sharedSemantics->getInheritanceInfo(interfaceType).facets)
    {
        auto owner = facet->origin.declRef.as<InterfaceDecl>();
        if (!owner)
            continue;
        for (auto requirement : owner.getDecl()->getDirectMemberDeclsOfType<AssocTypeDecl>())
        {
            if (getStructuralRayTracingAssociatedTypeKind(requirement) != kind)
                continue;
            witness = sharedSemantics->tryProjectInterfaceSubtypeWitness(witness, facet->getType());
            return witness ? requirement : nullptr;
        }
    }
    return nullptr;
}

Type* resolveStructuralRayTracingAssociatedType(
    ASTBuilder* astBuilder,
    SubtypeWitness* witness,
    StructuralRayTracingAssociatedTypeKind kind)
{
    auto requirement = _findStructuralAssociatedRequirement(witness, kind);
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

// Names the interface contract needed to continue resolving one associated-type requirement.
// The role matters when a declaration adds unrelated constraints on the same associated type:
// `Context : IExtra` must not replace the `Context : IPayloadContext` proof for a miss stage.
static ASTNodeType _getAssociatedTypeConstraintClass(StructuralRayTracingAssociatedTypeKind kind)
{
    switch (kind)
    {
    case StructuralRayTracingAssociatedTypeKind::StageTraceContext:
    case StructuralRayTracingAssociatedTypeKind::ProgramTraceContext:
        return ASTNodeType::RayTracingTraceContextType;
    case StructuralRayTracingAssociatedTypeKind::HitPrimitive:
        return ASTNodeType::RayTracingIntersectionPrimitiveType;
    case StructuralRayTracingAssociatedTypeKind::ProgramHitGroups:
        return ASTNodeType::RayTracingHitGroupListType;
    case StructuralRayTracingAssociatedTypeKind::ProgramMissShaders:
        return ASTNodeType::RayTracingMissShaderListType;
    case StructuralRayTracingAssociatedTypeKind::ProgramCallableShaders:
        return ASTNodeType::RayTracingCallableShaderListType;
    case StructuralRayTracingAssociatedTypeKind::HitGroupClosestHit:
        return ASTNodeType::ClosestHitShaderType;
    case StructuralRayTracingAssociatedTypeKind::HitGroupAnyHit:
        return ASTNodeType::AnyHitShaderType;
    case StructuralRayTracingAssociatedTypeKind::HitGroupIntersection:
        return ASTNodeType::IntersectionStageType;
    case StructuralRayTracingAssociatedTypeKind::HitGroupContext:
    case StructuralRayTracingAssociatedTypeKind::ClosestHitShaderContext:
    case StructuralRayTracingAssociatedTypeKind::AnyHitShaderContext:
    case StructuralRayTracingAssociatedTypeKind::IntersectionStageContext:
        return ASTNodeType::RayTracingHitContextType;
    case StructuralRayTracingAssociatedTypeKind::MissShaderContext:
        return ASTNodeType::RayTracingPayloadContextType;
    case StructuralRayTracingAssociatedTypeKind::CallableShaderContext:
        return ASTNodeType::RayTracingCallableContextType;
    default:
        return ASTNodeType::CountOf;
    }
}

SubtypeWitness* resolveStructuralRayTracingAssociatedTypeConstraint(
    ASTBuilder* astBuilder,
    SubtypeWitness* witness,
    StructuralRayTracingAssociatedTypeKind kind)
{
    auto expectedInterfaceClass = _getAssociatedTypeConstraintClass(kind);
    if (expectedInterfaceClass == ASTNodeType::CountOf)
        return nullptr;
    auto associatedType = _findStructuralAssociatedRequirement(witness, kind);
    if (!associatedType)
        return nullptr;
    auto owner = as<InterfaceDecl>(associatedType->parentDecl);
    SLANG_RELEASE_ASSERT(owner);
    auto sharedSemantics = getModule(owner)->getLinkage()->getSemanticsForReflection();
    for (auto constraint : owner->getDirectMemberDeclsOfType<GenericTypeConstraintDecl>())
    {
        if (constraint->isEqualityConstraint)
            continue;
        auto subType = isDeclRefTypeOf<AssocTypeDecl>(constraint->sub.type);
        if (!subType || subType.getDecl() != associatedType)
            continue;
        auto requirementWitness = tryLookUpRequirementWitness(astBuilder, witness, constraint);
        if (requirementWitness.getFlavor() != RequirementWitness::Flavor::val)
            continue;
        auto constraintWitness = as<SubtypeWitness>(requirementWitness.getVal()->resolve());
        if (!constraintWitness)
            continue;

        // A bound may refine the required context interface. Follow its checked interface facets
        // to recognize that contract, but return the original requirement witness so later
        // associated-type projection keeps the conformance path selected by this declaration.
        for (auto facet : sharedSemantics->getInheritanceInfo(constraintWitness->getSup()).facets)
        {
            auto interfaceDecl = facet->origin.declRef.as<InterfaceDecl>();
            if (_getMagicTypeClass(interfaceDecl.getDecl()) == expectedInterfaceClass)
                return constraintWitness;
        }
    }
    return nullptr;
}

StructuralRayTracingHitAttributesKind getStructuralRayTracingHitAttributesKind(Type* primitiveType)
{
    primitiveType = primitiveType ? as<Type>(primitiveType->resolve()) : nullptr;
    if (as<TrianglePrimitiveType>(primitiveType))
        return StructuralRayTracingHitAttributesKind::Triangle;
    if (as<CurvePrimitiveType>(primitiveType))
        return StructuralRayTracingHitAttributesKind::Curve;
    return isDeclRefTypeOf<AggTypeDecl>(primitiveType)
               ? StructuralRayTracingHitAttributesKind::Custom
               : StructuralRayTracingHitAttributesKind::None;
}

bool isStructuralRayTracingPayloadStageInputAccessor(FunctionDeclBase* functionDecl)
{
    auto propertyDecl = functionDecl ? as<PropertyDecl>(functionDecl->parentDecl) : nullptr;
    return propertyDecl && propertyDecl->hasModifier<RayTracingPayloadAttribute>();
}

StructuralRayTracingTraceMethodKind getStructuralRayTracingTraceMethodKind(
    FunctionDeclBase* functionDecl)
{
    return getStructuralRayTracingTraceMethodInfo(functionDecl).kind;
}

StructuralRayTracingTraceMethodInfo getStructuralRayTracingTraceMethodInfo(
    FunctionDeclBase* functionDecl)
{
    StructuralRayTracingTraceMethodInfo result;
    if (!functionDecl || !functionDecl->hasModifier<RayTracingTraceAttribute>())
        return result;
    result.kind = StructuralRayTracingTraceMethodKind::ImplicitEmptyPayload;
    Index parameterIndex = 0;
    for (auto parameter : functionDecl->getParameters())
    {
        if (parameter->hasModifier<RayTracingPayloadAttribute>())
        {
            result.kind = StructuralRayTracingTraceMethodKind::ExplicitPayload;
            result.payloadParameterIndex = parameterIndex;
            break;
        }
        ++parameterIndex;
    }
    return result;
}

bool isStructuralRayTracingTraceMethod(FunctionDeclBase* functionDecl)
{
    return getStructuralRayTracingTraceMethodKind(functionDecl) !=
           StructuralRayTracingTraceMethodKind::None;
}

bool isStructuralRayTracingCallShaderMethod(FunctionDeclBase* functionDecl)
{
    return functionDecl && functionDecl->hasModifier<RayTracingCallShaderAttribute>();
}

FunctionDeclBase* getStructuralRayTracingStageInvokeRequirement(InterfaceDecl* interfaceDecl)
{
    if (!isExecutableStructuralRayTracingStageInterface(interfaceDecl))
        return nullptr;
    for (auto member : interfaceDecl->getDirectMemberDeclsOfType<FunctionDeclBase>())
    {
        if (member->getName() && member->getName()->text == "invoke")
            return member;
    }
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
            // A concrete entry-list specialization has a type pack and a matching conformance-
            // witness pack, both empty for an empty list. Select them by semantic role instead of
            // relying on their positions among the generic arguments. Both IR lowering and
            // reflection consume this exact checked representation.
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
    // Dependent list types have no concrete entries until specialization.
    if (!result.types)
        result.types = astBuilder->getTypePack(ArrayView<Type*>());
    SLANG_RELEASE_ASSERT(
        !result.witnesses || result.witnesses->getCount() == result.types->getTypeCount());
    return result;
}


} // namespace Slang
