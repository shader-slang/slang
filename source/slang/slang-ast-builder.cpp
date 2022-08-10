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
            m_sliceToTypeMap.Add(UnownedStringSlice(info->m_name), info);
            Name* name = m_namePool->getName(String(info->m_name));
            m_nameToTypeMap.Add(name, info);
        }
    }
}

const ReflectClassInfo* SharedASTBuilder::findClassInfo(const UnownedStringSlice& slice)
{
    const ReflectClassInfo* typeInfo;
    return m_sliceToTypeMap.TryGetValue(slice, typeInfo) ? typeInfo : nullptr;
}

SyntaxClass<NodeBase> SharedASTBuilder::findSyntaxClass(const UnownedStringSlice& slice)
{
    const ReflectClassInfo* typeInfo;
    if (m_sliceToTypeMap.TryGetValue(slice, typeInfo))
    {
        return SyntaxClass<NodeBase>(typeInfo);
    }
    return SyntaxClass<NodeBase>();
}

const ReflectClassInfo* SharedASTBuilder::findClassInfo(Name* name)
{
    const ReflectClassInfo* typeInfo;
    return m_nameToTypeMap.TryGetValue(name, typeInfo) ? typeInfo : nullptr;
}

SyntaxClass<NodeBase> SharedASTBuilder::findSyntaxClass(Name* name)
{
    const ReflectClassInfo* typeInfo;
    if (m_nameToTypeMap.TryGetValue(name, typeInfo))
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
    auto type = DeclRefType::create(m_astBuilder, DeclRef<Decl>(decl, nullptr));
    m_builtinTypes[Index(modifier->tag)] = type;
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
    return m_magicDecls[name].GetValue();
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
    auto declRef = getBuiltinDeclRef(magicTypeName, makeConstArrayViewSingle<Val*>(typeParam));
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
    ArrayExpressionType* arrayType = create<ArrayExpressionType>();
    arrayType->baseType = elementType;
    arrayType->arrayLength = elementCount;
    return arrayType;
}

VectorExpressionType* ASTBuilder::getVectorType(
    Type*    elementType,
    IntVal*  elementCount)
{
    auto vectorGenericDecl = as<GenericDecl>(m_sharedASTBuilder->findMagicDecl("Vector"));
    
    auto vectorTypeDecl = vectorGenericDecl->inner;

    auto substitutions = create<GenericSubstitution>();
    substitutions->genericDecl = vectorGenericDecl;
    substitutions->args.add(elementType);
    substitutions->args.add(elementCount);

    auto declRef = DeclRef<Decl>(vectorTypeDecl, substitutions);

    return as<VectorExpressionType>(DeclRefType::create(this, declRef));
}

DifferentialPairType* ASTBuilder::getDifferentialPairType(Type* valueType, Witness* conformanceWitness)
{
    auto genericDecl = dynamicCast<GenericDecl>(m_sharedASTBuilder->findMagicDecl("DifferentialPairType"));

    auto typeDecl = genericDecl->inner;

    auto substitutions = create<GenericSubstitution>();
    substitutions->genericDecl = genericDecl;
    substitutions->args.add(valueType);
    substitutions->args.add(conformanceWitness);

    auto declRef = DeclRef<Decl>(typeDecl, substitutions);
    auto rsType = DeclRefType::create(this, declRef);

    return as<DifferentialPairType>(rsType);
}

DeclRef<InterfaceDecl> ASTBuilder::getDifferentiableInterface()
{
    DeclRef<InterfaceDecl> declRef;
    declRef.decl = dynamicCast<InterfaceDecl>(m_sharedASTBuilder->findMagicDecl("DifferentiableType"));
    return declRef;
}

DeclRef<Decl> ASTBuilder::getBuiltinDeclRef(const char* builtinMagicTypeName, ConstArrayView<Val*> genericArgs)
{
    DeclRef<Decl> declRef;
    declRef.decl = m_sharedASTBuilder->findMagicDecl(builtinMagicTypeName);
    if (auto genericDecl = as<GenericDecl>(declRef.decl))
    {
        if (genericArgs.getCount())
        {
            auto substitutions = create<GenericSubstitution>();
            substitutions->genericDecl = genericDecl;
            for (auto arg : genericArgs)
                substitutions->args.add(arg);
            declRef.substitutions = substitutions;
        }
        declRef.decl = genericDecl->inner;
    }
    else
    {
        SLANG_ASSERT(genericArgs.getCount() == 0);
    }
    return declRef;
}

Type* ASTBuilder::getAndType(Type* left, Type* right)
{
    auto type = create<AndType>();
    type->left = left;
    type->right = right;
    return type;
}

Type* ASTBuilder::getModifiedType(Type* base, Count modifierCount, Val* const* modifiers)
{
    auto type = create<ModifiedType>();
    type->base = base;
    type->modifiers.addRange(modifiers, modifierCount);
    return type;
}

NodeBase* ASTBuilder::_getOrCreateImpl(NodeDesc const& desc, NodeCreateFunc createFunc, void* createFuncUserData)
{
    if(auto found = m_cachedNodes.TryGetValue(desc))
        return *found;

    auto node = createFunc(this, desc, createFuncUserData);

    auto operandCount = desc.operandCount;
    NodeBase** operandsCopy = m_arena.allocateAndZeroArray<NodeBase*>(desc.operandCount);
    for(Index i = 0; i < operandCount; ++i)
        operandsCopy[i] = desc.operands[i];

    NodeDesc descCopy = desc;
    descCopy.operands = operandsCopy;
    m_cachedNodes.Add(descCopy, node);

    return node;
}


Val* ASTBuilder::getUNormModifierVal()
{
    return _getOrCreate<UNormModifierVal>();
}

Val* ASTBuilder::getSNormModifierVal()
{
    return _getOrCreate<SNormModifierVal>();
}

TypeType* ASTBuilder::getTypeType(Type* type)
{
    return create<TypeType>(type);
}

bool ASTBuilder::NodeDesc::operator==(NodeDesc const& that) const
{
    if(type != that.type) return false;
    if(operandCount != that.operandCount) return false;
    for(Index i = 0; i < operandCount; ++i)
    {
        // Note: we are comparing the operands directly for identity
        // (pointer equality) rather than doing the `Val`-level
        // equality check.
        //
        // The rationale here is that nodes that will be created
        // via a `NodeDesc` *should* all be going through the
        // deduplication path anyway, as should their operands.
        // 
        if(operands[i] != that.operands[i]) return false;
    }
    return true;
}
HashCode ASTBuilder::NodeDesc::getHashCode() const
{
    Hasher hasher;
    hasher.hashValue(Int(type));
    hasher.hashValue(operandCount);
    for(Index i = 0; i < operandCount; ++i)
    {
        // Note: we are hashing the raw pointer value rather
        // than the content of the value node. This is done
        // to match the semantics implemented for `==` on
        // `NodeDesc`.
        //
        hasher.hashValue((void*) operands[i]);
    }
    return hasher.getResult();
}

} // namespace Slang
