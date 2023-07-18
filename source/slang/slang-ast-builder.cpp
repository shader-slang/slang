// slang-ast-builder.cpp
#include "slang-ast-builder.h"
#include <assert.h>

#include "slang-compiler.h"

namespace Slang {

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! SharedASTBuilder !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

SharedASTBuilder::SharedASTBuilder()
{    
}

void SharedASTBuilder::init(Session* session)
{
    m_namePool = session->getNamePool();

    // Save the associated session
    m_session = session;

    // We just want as a place to store allocations of shared types
    {
        RefPtr<ASTBuilder> astBuilder(new ASTBuilder);
        astBuilder->m_sharedASTBuilder = this;
        m_astBuilder = astBuilder.detach();
    }

    // Clear the built in types
    memset(m_builtinTypes, 0, sizeof(m_builtinTypes));

    // Create common shared types
    m_errorType = m_astBuilder->create<ErrorType>();
    m_bottomType = m_astBuilder->create<BottomType>();
    m_initializerListType = m_astBuilder->create<InitializerListType>();
    m_overloadedType = m_astBuilder->create<OverloadGroupType>();

    // We can just iterate over the class pointers.
    // NOTE! That this adds the names of the abstract classes too(!)
    for (Index i = 0; i < Index(ASTNodeType::CountOf); ++i)
    {
        const ReflectClassInfo* info = ASTClassInfo::getInfo(ASTNodeType(i));
        if (info)
        {
            m_sliceToTypeMap.add(UnownedStringSlice(info->m_name), info);
            Name* name = m_namePool->getName(String(info->m_name));
            m_nameToTypeMap.add(name, info);
        }
    }
}

const ReflectClassInfo* SharedASTBuilder::findClassInfo(const UnownedStringSlice& slice)
{
    const ReflectClassInfo* typeInfo;
    return m_sliceToTypeMap.tryGetValue(slice, typeInfo) ? typeInfo : nullptr;
}

SyntaxClass<NodeBase> SharedASTBuilder::findSyntaxClass(const UnownedStringSlice& slice)
{
    const ReflectClassInfo* typeInfo;
    if (m_sliceToTypeMap.tryGetValue(slice, typeInfo))
    {
        return SyntaxClass<NodeBase>(typeInfo);
    }
    return SyntaxClass<NodeBase>();
}

const ReflectClassInfo* SharedASTBuilder::findClassInfo(Name* name)
{
    const ReflectClassInfo* typeInfo;
    return m_nameToTypeMap.tryGetValue(name, typeInfo) ? typeInfo : nullptr;
}

SyntaxClass<NodeBase> SharedASTBuilder::findSyntaxClass(Name* name)
{
    const ReflectClassInfo* typeInfo;
    if (m_nameToTypeMap.tryGetValue(name, typeInfo))
    {
        return SyntaxClass<NodeBase>(typeInfo);
    }
    return SyntaxClass<NodeBase>();
}

Type* SharedASTBuilder::getStringType()
{
    if (!m_stringType)
    {
        auto stringTypeDecl = findMagicDecl("StringType");
        m_stringType = DeclRefType::create(m_astBuilder, makeDeclRef<Decl>(stringTypeDecl));
    }
    return m_stringType;
}

Type* SharedASTBuilder::getNativeStringType()
{
    if (!m_nativeStringType)
    {
        auto nativeStringTypeDecl = findMagicDecl("NativeStringType");
        m_nativeStringType = DeclRefType::create(m_astBuilder, makeDeclRef<Decl>(nativeStringTypeDecl));
    }
    return m_nativeStringType;
}

Type* SharedASTBuilder::getEnumTypeType()
{
    if (!m_enumTypeType)
    {
        auto enumTypeTypeDecl = findMagicDecl("EnumTypeType");
        m_enumTypeType = DeclRefType::create(m_astBuilder, makeDeclRef<Decl>(enumTypeTypeDecl));
    }
    return m_enumTypeType;
}

Type* SharedASTBuilder::getDynamicType()
{
    if (!m_dynamicType)
    {
        auto dynamicTypeDecl = findMagicDecl("DynamicType");
        m_dynamicType = DeclRefType::create(m_astBuilder, makeDeclRef<Decl>(dynamicTypeDecl));
    }
    return m_dynamicType;
}

Type* SharedASTBuilder::getNullPtrType()
{
    if (!m_nullPtrType)
    {
        auto nullPtrTypeDecl = findMagicDecl("NullPtrType");
        m_nullPtrType = DeclRefType::create(m_astBuilder, makeDeclRef<Decl>(nullPtrTypeDecl));
    }
    return m_nullPtrType;
}

Type* SharedASTBuilder::getNoneType()
{
    if (!m_noneType)
    {
        auto noneTypeDecl = findMagicDecl("NoneType");
        m_noneType = DeclRefType::create(m_astBuilder, makeDeclRef<Decl>(noneTypeDecl));
    }
    return m_noneType;
}

Type* SharedASTBuilder::getDiffInterfaceType()
{
    if (!m_diffInterfaceType)
    {
        auto decl = findMagicDecl("DifferentiableType");
        m_diffInterfaceType = DeclRefType::create(m_astBuilder, makeDeclRef<Decl>(decl));
    }
    return m_diffInterfaceType;
}

SharedASTBuilder::~SharedASTBuilder()
{
    // Release built in types..
    for (Index i = 0; i < SLANG_COUNT_OF(m_builtinTypes); ++i)
    {
        m_builtinTypes[i] = nullptr;
    }

    if (m_astBuilder)
    {
        m_astBuilder->releaseReference();
    }
}

void SharedASTBuilder::registerBuiltinDecl(Decl* decl, BuiltinTypeModifier* modifier)
{
    auto type = DeclRefType::create(m_astBuilder, makeDeclRef<Decl>(decl));
    m_builtinTypes[Index(modifier->tag)] = type;
}

void SharedASTBuilder::registerBuiltinRequirementDecl(Decl* decl, BuiltinRequirementModifier* modifier)
{
    m_builtinRequirementDecls[modifier->kind] = decl;
}

void SharedASTBuilder::registerMagicDecl(Decl* decl, MagicTypeModifier* modifier)
{
    // In some cases the modifier will have been applied to the
    // "inner" declaration of a `GenericDecl`, but what we
    // actually want to register is the generic itself.
    //
    auto declToRegister = decl;
    if (auto genericDecl = as<GenericDecl>(decl->parentDecl))
        declToRegister = genericDecl;

    m_magicDecls[modifier->magicName] = declToRegister;
}

Decl* SharedASTBuilder::findMagicDecl(const String& name)
{
    return m_magicDecls[name].getValue();
}

Decl* SharedASTBuilder::tryFindMagicDecl(const String& name)
{
    if (m_magicDecls.containsKey(name))
    {
        return m_magicDecls[name].getValue();
    }
    else
    {
        return nullptr;
    }
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ASTBuilder !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

ASTBuilder::ASTBuilder(SharedASTBuilder* sharedASTBuilder, const String& name):
    m_sharedASTBuilder(sharedASTBuilder),
    m_name(name),
    m_id(sharedASTBuilder->m_id++),
    m_arena(2048)
{
    SLANG_ASSERT(sharedASTBuilder);
}

ASTBuilder::ASTBuilder():
    m_sharedASTBuilder(nullptr),
    m_id(-1),
    m_arena(2048)
{
    m_name = "SharedASTBuilder::m_astBuilder";
}

ASTBuilder::~ASTBuilder()
{
    for (NodeBase* node : m_dtorNodes)
    {
        const ReflectClassInfo* info = ASTClassInfo::getInfo(node->astNodeType);
        SLANG_ASSERT(info->m_destructorFunc);
        info->m_destructorFunc(node);
    }
}

NodeBase* ASTBuilder::createByNodeType(ASTNodeType nodeType)
{
    const ReflectClassInfo* info = ASTClassInfo::getInfo(nodeType);
    
    auto createFunc = info->m_createFunc;
    SLANG_ASSERT(createFunc);
    if (!createFunc)
    {
        return nullptr;
    }

    return (NodeBase*)createFunc(this);
}

Type* ASTBuilder::getSpecializedBuiltinType(Type* typeParam, char const* magicTypeName)
{
    auto declRef = getBuiltinDeclRef(magicTypeName, typeParam);
    auto rsType = DeclRefType::create(this, declRef);
    return rsType;
}

PtrType* ASTBuilder::getPtrType(Type* valueType)
{
    return dynamicCast<PtrType>(getPtrType(valueType, "PtrType"));
}

// Construct the type `Out<valueType>`
OutType* ASTBuilder::getOutType(Type* valueType)
{
    return dynamicCast<OutType>(getPtrType(valueType, "OutType"));
}

InOutType* ASTBuilder::getInOutType(Type* valueType)
{
    return dynamicCast<InOutType>(getPtrType(valueType, "InOutType"));
}

RefType* ASTBuilder::getRefType(Type* valueType)
{
    return dynamicCast<RefType>(getPtrType(valueType, "RefType"));
}

OptionalType* ASTBuilder::getOptionalType(Type* valueType)
{
    auto rsType = getSpecializedBuiltinType(valueType, "OptionalType");
    return as<OptionalType>(rsType);
}

PtrTypeBase* ASTBuilder::getPtrType(Type* valueType, char const* ptrTypeName)
{
    return as<PtrTypeBase>(getSpecializedBuiltinType(valueType, ptrTypeName));
}

ArrayExpressionType* ASTBuilder::getArrayType(Type* elementType, IntVal* elementCount)
{
    if (!elementCount)
        elementCount = getIntVal(getIntType(), kUnsizedArrayMagicLength);

    auto result = getOrCreate<ArrayExpressionType>(elementType, elementCount);
    if (!result->declRef.getDecl())
    {
        auto arrayGenericDecl = as<GenericDecl>(m_sharedASTBuilder->findMagicDecl("ArrayType"));
        auto arrayTypeDecl = arrayGenericDecl->inner;
        auto substitutions = getOrCreateGenericSubstitution(nullptr, arrayGenericDecl, elementType, elementCount);
        result->declRef = getSpecializedDeclRef<Decl>(arrayTypeDecl, substitutions);
    }
    return result;
}

VectorExpressionType* ASTBuilder::getVectorType(
    Type*    elementType,
    IntVal*  elementCount)
{
    auto result = getOrCreate<VectorExpressionType>(elementType, elementCount);
    if (!result->declRef.getDecl())
    {
        auto vectorGenericDecl = as<GenericDecl>(m_sharedASTBuilder->findMagicDecl("Vector"));
        auto vectorTypeDecl = vectorGenericDecl->inner;
        auto substitutions = getOrCreateGenericSubstitution(nullptr, vectorGenericDecl, elementType, elementCount);
        result->declRef = getSpecializedDeclRef<Decl>(vectorTypeDecl, substitutions);
    }
    return result;
}

DifferentialPairType* ASTBuilder::getDifferentialPairType(
    Type* valueType,
    Witness* primalIsDifferentialWitness)
{
    auto genericDecl = dynamicCast<GenericDecl>(m_sharedASTBuilder->findMagicDecl("DifferentialPairType"));

    auto typeDecl = genericDecl->inner;

    auto substitutions = getOrCreateGenericSubstitution(
        nullptr,
        genericDecl,
        valueType,
        primalIsDifferentialWitness);

    auto declRef = getSpecializedDeclRef<Decl>(typeDecl, substitutions);
    auto rsType = DeclRefType::create(this, declRef);

    return as<DifferentialPairType>(rsType);
}

DeclRef<InterfaceDecl> ASTBuilder::getDifferentiableInterfaceDecl()
{
    DeclRef<InterfaceDecl> declRef = DeclRef<InterfaceDecl>(getBuiltinDeclRef("DifferentiableType", nullptr));
    return declRef;
}

bool ASTBuilder::isDifferentiableInterfaceAvailable()
{
    return (m_sharedASTBuilder->tryFindMagicDecl("DifferentiableType") != nullptr);
}

MeshOutputType* ASTBuilder::getMeshOutputTypeFromModifier(
    HLSLMeshShaderOutputModifier* modifier,
    Type* elementType,
    IntVal* maxElementCount)
{
    SLANG_ASSERT(modifier);
    SLANG_ASSERT(elementType);
    SLANG_ASSERT(maxElementCount);

    const char* declName
        = as<HLSLVerticesModifier>(modifier) ? "VerticesType"
        : as<HLSLIndicesModifier>(modifier) ? "IndicesType"
        : as<HLSLPrimitivesModifier>(modifier) ? "PrimitivesType"
        : (SLANG_UNEXPECTED("Unhandled mesh output modifier"), nullptr);
    auto genericDecl = dynamicCast<GenericDecl>(m_sharedASTBuilder->findMagicDecl(declName));

    auto typeDecl = genericDecl->inner;

    auto substitutions = getOrCreateGenericSubstitution(
        nullptr,
        genericDecl,
        elementType,
        maxElementCount);

    auto declRef = getSpecializedDeclRef<Decl>(typeDecl, substitutions);
    auto rsType = DeclRefType::create(this, declRef);

    return as<MeshOutputType>(rsType);
}

Type* ASTBuilder::getDifferentiableInterfaceType()
{
    return DeclRefType::create(this, getDifferentiableInterfaceDecl());
}

DeclRef<Decl> ASTBuilder::getBuiltinDeclRef(const char* builtinMagicTypeName, Val* genericArg)
{
    auto decl = m_sharedASTBuilder->findMagicDecl(builtinMagicTypeName);
    if (auto genericDecl = as<GenericDecl>(decl))
    {
        decl = genericDecl->inner;
        Substitutions* subst = nullptr;
        if (genericArg)
        {
            subst = getOrCreateGenericSubstitution(nullptr, genericDecl, genericArg);
        }
        return getSpecializedDeclRef(decl, subst);
    }
    else
    {
        SLANG_ASSERT(!genericArg);
    }
    return makeDeclRef(decl);
}

Type* ASTBuilder::getAndType(Type* left, Type* right)
{
    auto type = getOrCreate<AndType>(left, right);
    return type;
}

Type* ASTBuilder::getModifiedType(Type* base, Count modifierCount, Val* const* modifiers)
{
    auto type = create<ModifiedType>();
    type->base = base;
    type->modifiers.addRange(modifiers, modifierCount);
    return type;
}

Val* ASTBuilder::getUNormModifierVal()
{
    return getOrCreate<UNormModifierVal>();
}

Val* ASTBuilder::getSNormModifierVal()
{
    return getOrCreate<SNormModifierVal>();
}

Val* ASTBuilder::getNoDiffModifierVal()
{
    return getOrCreate<NoDiffModifierVal>();
}

Type* ASTBuilder::getFuncType(List<Type*> parameters, Type* result)
{
    auto errorType = getOrCreate<BottomType>();
    return getOrCreate<FuncType>(parameters, result, errorType);
}

Type* ASTBuilder::getTupleType(List<Type*>& types)
{
    return getOrCreate<TupleType>(types);
}

TypeType* ASTBuilder::getTypeType(Type* type)
{
    return getOrCreate<TypeType>(type);
}

TypeEqualityWitness* ASTBuilder::getTypeEqualityWitness(
    Type* type)
{
    return getOrCreate<TypeEqualityWitness>(type);
}


SubtypeWitness* ASTBuilder::getDeclaredSubtypeWitness(
    Type*                   subType,
    Type*                   superType,
    DeclRef<Decl> const&    declRef)
{
    auto witness = getOrCreate<DeclaredSubtypeWitness>(
        subType, superType, declRef.declRefBase);
    return witness;
}

SubtypeWitness* ASTBuilder::getTransitiveSubtypeWitness(
    SubtypeWitness* aIsSubtypeOfBWitness,
    SubtypeWitness* bIsSubtypeOfCWitness)
{
top:
    // Our job is to take the witnesses that `a <: b` and `b <: c`
    // and produce a valid witness that `a <: c`
    //
    // There are some special cases we want to handle, in order
    // to simplify logic elesewhere in the compiler. For example,
    // if either of the input witnesses is a type *equality* witness,
    // then the other witness can be returned as-is.
    //
    // If `a == b`, then the `b <: c` witness is also a witness of `a <: c`.
    //
    if(as<TypeEqualityWitness>(aIsSubtypeOfBWitness))
    {
        return bIsSubtypeOfCWitness;
    }

    // Similarly, if `b == c`, then the `a <: b` witness is a witness for `a <: c`
    //
    if (as<TypeEqualityWitness>(bIsSubtypeOfCWitness))
    {
        return aIsSubtypeOfBWitness;
    }

    // HACK: There is downstream code generation logic that assumes that
    // a `TransitiveSubtypeWitness` will never have a transitive witness
    // as its `b <: c` witness. If we are at risk of creating such a witness here,
    // we will shift things around to make that not be the case:
    //
    if (auto bIsTransitiveSubtypeOfCWitness = as<TransitiveSubtypeWitness>(bIsSubtypeOfCWitness))
    {
        // Let's call the intermediate type here `x`, we know that the `b <: c`
        // witness is based on witnesses that `b <: x` and `x <: c`:
        //
        auto bIsSubtypeOfXWitness = bIsTransitiveSubtypeOfCWitness->subToMid;
        auto xIsSubtypeOfCWitness = bIsTransitiveSubtypeOfCWitness->midToSup;

        // We can recursively call this operation to produce a witness that
        // `a <: x`, based on the witnesses we already have for `a <: b` and `b <: x`:
        //
        auto aIsSubtypeOfXWitness = getTransitiveSubtypeWitness(
            aIsSubtypeOfBWitness,
            bIsSubtypeOfXWitness);

        // Now we can perform a "tail recursive" call to this function (via `goto`
        // to combine the `a <: x` witness with our `x <: c` witness:
        //
        aIsSubtypeOfBWitness = aIsSubtypeOfXWitness;
        bIsSubtypeOfCWitness = xIsSubtypeOfCWitness;
        goto top;
    }

    auto aType = aIsSubtypeOfBWitness->sub;
    auto cType = bIsSubtypeOfCWitness->sup;

    // If the right-hand side is a conjunction witness for `B <: C`
    // of the form `(B <: X)&(B <: Y)`, then we have it that `C = X&Y`
    // and we'd rather form a conjunction witness for `A <: C`
    // that is of the form `(A <: X)&(A <: Y)`.
    //
    if(auto bIsSubtypeOfXAndY = as<ConjunctionSubtypeWitness>(bIsSubtypeOfCWitness))
    {
        auto bIsSubtypeOfXWitness = bIsSubtypeOfXAndY->getLeftWitness();
        auto bIsSubtypeOfYWitness = bIsSubtypeOfXAndY->getRightWitness();

        return getConjunctionSubtypeWitness(
            aType,
            cType,
            getTransitiveSubtypeWitness(aIsSubtypeOfBWitness, bIsSubtypeOfXWitness),
            getTransitiveSubtypeWitness(aIsSubtypeOfBWitness, bIsSubtypeOfYWitness));
    }

    // If the right-hand witness `R` is of the form `extract(i, W)`, then
    // `W` is a witness that `B <: X&Y&...` for some conjunction, where `C`
    // is one component of that conjunction.
    //
    if(auto bIsSubtypeViaExtraction = as<ExtractFromConjunctionSubtypeWitness>(bIsSubtypeOfCWitness))
    {
        // We decompose the witness `extract(i, W)` to get both
        // the witness `W` that `B <: X&Y&...` as well as the index
        // `i` of `C` within the conjunction.
        //
        auto bIsSubtypeOfConjunction = bIsSubtypeViaExtraction->conjunctionWitness;
        auto indexOfCInConjunction = bIsSubtypeViaExtraction->indexInConjunction;

        // We lift the extraction to the outside of the composition, by
        // forming a witness for `A <: C` that is of the form
        // `extract(i, L . W )`, where `L` is the left-hand witnes (for `A <: B`).
        // The composition `L . W` is a witness that `A <: X&Y&...`, and
        // the `i`th component of it should be a witness that `A <: C`.
        //
        return getExtractFromConjunctionSubtypeWitness(
            aType,
            cType,
            getTransitiveSubtypeWitness(aIsSubtypeOfBWitness, bIsSubtypeOfConjunction),
            indexOfCInConjunction);
    }

    // If none of the above special cases applied, then we are just going to create
    // a `TransitiveSubtypeWitness` directly.
    //
    // TODO: Identify other cases that we can potentially simplify.
    // It is particularly notable that we do not have simplification rules that
    // detect when the left-hand side of a composition has some particular
    // structure. This may be fine, or it may not; we should write down a more
    // formal set of rules for the allowed structure of our witnesses to
    // guarantee that our simplifications are sufficient.

    TransitiveSubtypeWitness* transitiveWitness = getOrCreateWithDefaultCtor<TransitiveSubtypeWitness>(
        aType,
        cType,
        aIsSubtypeOfBWitness,
        bIsSubtypeOfCWitness);
    transitiveWitness->sub = aType;
    transitiveWitness->sup = cType;
    transitiveWitness->subToMid = aIsSubtypeOfBWitness;
    transitiveWitness->midToSup = bIsSubtypeOfCWitness;

    return transitiveWitness;
}

ThisTypeSubtypeWitness* ASTBuilder::getThisTypeSubtypeWitness(Type* subType, Type* superType)
{
    return getOrCreate<ThisTypeSubtypeWitness>(subType, superType);
}

SubtypeWitness* ASTBuilder::getExtractFromConjunctionSubtypeWitness(
    Type*           subType,
    Type*           superType,
    SubtypeWitness* conjunctionWitness,
    int             indexOfSuperTypeInConjunction)
{
    // We are taking a witness `W` for `S <: L&R` and
    // using it to produce a witness for `S <: L`
    // or `S <: R`.

    // If it turns out that the witness `W` is itself
    // formed as a conjuction of witnesses: `(S <: L) & (S <: R)`,
    // then we can simply re-use the appropriate sub-witness.
    //
    if (auto conjWitness = as<ConjunctionSubtypeWitness>(conjunctionWitness))
    {
        return conjWitness->getComponentWitness(indexOfSuperTypeInConjunction);
    }

    // TODO: Are there other simplification cases we should be paying attention
    // to here? For example:
    //
    // * What if the original witness is transitive?

    auto witness = getOrCreateWithDefaultCtor<ExtractFromConjunctionSubtypeWitness>(
        subType,
        superType,
        conjunctionWitness,
        indexOfSuperTypeInConjunction);

    witness->sub = subType;
    witness->sup = superType;
    witness->conjunctionWitness = conjunctionWitness;
    witness->indexInConjunction = indexOfSuperTypeInConjunction;
    return witness;
}

SubtypeWitness* ASTBuilder::getConjunctionSubtypeWitness(
    Type*           sub,
    Type*           lAndR,
    SubtypeWitness* subIsLWitness,
    SubtypeWitness* subIsRWitness)
{
    // If a conjunction witness for `S <: L&R` is being formed,
    // where the constituent witnesses for `S <: L` and `S <: R`
    // are themselves extractions of the first and second
    // components, respectively, of a single witness `W`, then
    // we can simply use `W` as-is.
    //
    auto lExtract = as<ExtractFromConjunctionSubtypeWitness>(subIsLWitness);
    auto rExtract = as<ExtractFromConjunctionSubtypeWitness>(subIsRWitness);
    if(lExtract && rExtract)
    {
        if (lExtract->indexInConjunction == 0
            && rExtract->indexInConjunction == 1)
        {
            auto lInner = lExtract->conjunctionWitness;
            auto rInner = rExtract->conjunctionWitness;
            if (lInner == rInner)
            {
                return lInner;
            }
        }
    }

    // TODO: Depending on how we decide our canonicalized witnesses
    // should be structured, we could detect the case where the
    // `S <: L` and `S <: R` witnesses are both transitive compositions
    // of the form `X . A` and `X . B`, such that we *could* form
    // a composition around a conjunction - that is, produce
    // `X . (A & B)` rather than `(X . A) & (X . B)`.
    //
    // For now we are favoring putting the composition (transitive
    // witness) deeper, so that we have more chances to expose a
    // conjunction witness at higher levels.

    auto witness = getOrCreateWithDefaultCtor<ConjunctionSubtypeWitness>(
        sub,
        lAndR,
        subIsLWitness,
        subIsRWitness);
    witness->componentWitnesses[0] = subIsLWitness;
    witness->componentWitnesses[1] = subIsRWitness;
    witness->sub = sub;
    witness->sup = lAndR;
    return witness;
}

bool ASTBuilder::NodeDesc::operator==(NodeDesc const& that) const
{
    if (hashCode != that.hashCode) return false;
    if(type != that.type) return false;
    if(operands.getCount() != that.operands.getCount()) return false;
    for(Index i = 0; i < operands.getCount(); ++i)
    {
        // Note: we are comparing the operands directly for identity
        // (pointer equality) rather than doing the `Val`-level
        // equality check.
        //
        // The rationale here is that nodes that will be created
        // via a `NodeDesc` *should* all be going through the
        // deduplication path anyway, as should their operands.
        // 
        if (operands[i].values.nodeOperand != that.operands[i].values.nodeOperand) return false;
    }
    return true;
}

void ASTBuilder::NodeDesc::init()
{
    Hasher hasher;
    hasher.hashValue(Int(type));
    for(Index i = 0; i < operands.getCount(); ++i)
    {
        // Note: we are hashing the raw pointer value rather
        // than the content of the value node. This is done
        // to match the semantics implemented for `==` on
        // `NodeDesc`.
        //
        hasher.hashValue(operands[i].values.nodeOperand);
    }
    hashCode = hasher.getResult();
}

DeclRef<Decl> _getSpecializedDeclRef(ASTBuilder* builder, Decl* decl, Substitutions* subst)
{
    return builder->getSpecializedDeclRef(decl, subst);
}

} // namespace Slang
