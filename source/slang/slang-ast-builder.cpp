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

PtrTypeBase* ASTBuilder::getPtrType(Type* valueType, char const* ptrTypeName)
{
    auto genericDecl = dynamicCast<GenericDecl>(m_sharedASTBuilder->findMagicDecl(ptrTypeName));
    return getPtrType(valueType, genericDecl);
}

PtrTypeBase* ASTBuilder::getPtrType(Type* valueType, GenericDecl* genericDecl)
{
    auto typeDecl = genericDecl->inner;

    auto substitutions = create<GenericSubstitution>();
    substitutions->genericDecl = genericDecl;
    substitutions->args.add(valueType);

    auto declRef = DeclRef<Decl>(typeDecl, substitutions);
    auto rsType = DeclRefType::create(this, declRef);
    return as<PtrTypeBase>(rsType);
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

Type* ASTBuilder::getAndType(Type* left, Type* right)
{
    auto type = create<AndType>();
    type->left = left;
    type->right = right;
    return type;
}

TypeType* ASTBuilder::getTypeType(Type* type)
{
    return create<TypeType>(type);
}


} // namespace Slang
