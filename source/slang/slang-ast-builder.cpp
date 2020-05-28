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
    RefPtr<ASTBuilder> astBuilder(new ASTBuilder);

    astBuilder->m_sharedASTBuilder = this;

    memset(m_builtinTypes, 0, sizeof(m_builtinTypes));

    m_errorType = m_astBuilder->create<ErrorType>();
    m_initializerListType = m_astBuilder->create<InitializerListType>();
    m_overloadedType = m_astBuilder->create<OverloadGroupType>();

    m_astBuilder = astBuilder.detach();

    // We can just iterate over the class pointers.
    // NOTE! That this adds the names of the abstract classes too(!)
    for (Index i = 0; i < Index(ASTNodeType::CountOf); ++i)
    {
        const ReflectClassInfo* info = ReflectClassInfo::getInfo(ASTNodeType(i));
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

SyntaxClass<RefObject> SharedASTBuilder::findSyntaxClass(const UnownedStringSlice& slice)
{
    const ReflectClassInfo* typeInfo;
    if (m_sliceToTypeMap.TryGetValue(slice, typeInfo))
    {
        return SyntaxClass<RefObject>(typeInfo);
    }
    return SyntaxClass<RefObject>();
}

const ReflectClassInfo* SharedASTBuilder::findClassInfo(Name* name)
{
    const ReflectClassInfo* typeInfo;
    return m_nameToTypeMap.TryGetValue(name, typeInfo) ? typeInfo : nullptr;
}

SyntaxClass<RefObject> SharedASTBuilder::findSyntaxClass(Name* name)
{
    const ReflectClassInfo* typeInfo;
    if (m_nameToTypeMap.TryGetValue(name, typeInfo))
    {
        return SyntaxClass<RefObject>(typeInfo);
    }
    return SyntaxClass<RefObject>();
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

SharedASTBuilder::~SharedASTBuilder()
{
    // Release built in types..
    for (Index i = 0; i < SLANG_COUNT_OF(m_builtinTypes); ++i)
    {
        m_builtinTypes[i].setNull();
    }

    if (m_astBuilder)
    {
        m_astBuilder->releaseReference();
    }
}

void SharedASTBuilder::registerBuiltinDecl(RefPtr<Decl> decl, RefPtr<BuiltinTypeModifier> modifier)
{
    auto type = DeclRefType::create(m_astBuilder, DeclRef<Decl>(decl.Ptr(), nullptr));
    m_builtinTypes[Index(modifier->tag)] = type;
}

void SharedASTBuilder::registerMagicDecl(RefPtr<Decl> decl, RefPtr<MagicTypeModifier> modifier)
{
    // In some cases the modifier will have been applied to the
    // "inner" declaration of a `GenericDecl`, but what we
    // actually want to register is the generic itself.
    //
    auto declToRegister = decl;
    if (auto genericDecl = as<GenericDecl>(decl->parentDecl))
        declToRegister = genericDecl;

    m_magicDecls[modifier->name] = declToRegister.Ptr();
}

RefPtr<Decl> SharedASTBuilder::findMagicDecl(const String&   name)
{
    return m_magicDecls[name].GetValue();
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ASTBuilder !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

ASTBuilder::ASTBuilder(SharedASTBuilder* sharedASTBuilder):
    m_sharedASTBuilder(sharedASTBuilder)
{
    SLANG_ASSERT(sharedASTBuilder);
}

ASTBuilder::ASTBuilder():
    m_sharedASTBuilder(nullptr)
{
}

RefPtr<PtrType> ASTBuilder::getPtrType( RefPtr<Type>    valueType)
{
    return getPtrType(valueType, "PtrType").dynamicCast<PtrType>();
}

// Construct the type `Out<valueType>`
RefPtr<OutType> ASTBuilder::getOutType(RefPtr<Type> valueType)
{
    return getPtrType(valueType, "OutType").dynamicCast<OutType>();
}

RefPtr<InOutType> ASTBuilder::getInOutType(RefPtr<Type> valueType)
{
    return getPtrType(valueType, "InOutType").dynamicCast<InOutType>();
}

RefPtr<RefType> ASTBuilder::getRefType(RefPtr<Type> valueType)
{
    return getPtrType(valueType, "RefType").dynamicCast<RefType>();
}

RefPtr<PtrTypeBase> ASTBuilder::getPtrType(RefPtr<Type> valueType, char const* ptrTypeName)
{
    auto genericDecl = m_sharedASTBuilder->findMagicDecl(ptrTypeName).dynamicCast<GenericDecl>();
    return getPtrType(valueType, genericDecl);
}

RefPtr<PtrTypeBase> ASTBuilder::getPtrType(RefPtr<Type> valueType, GenericDecl* genericDecl)
{
    auto typeDecl = genericDecl->inner;

    auto substitutions = create<GenericSubstitution>();
    substitutions->genericDecl = genericDecl;
    substitutions->args.add(valueType);

    auto declRef = DeclRef<Decl>(typeDecl.Ptr(), substitutions);
    auto rsType = DeclRefType::create(this, declRef);
    return as<PtrTypeBase>(rsType);
}

RefPtr<ArrayExpressionType> ASTBuilder::getArrayType(Type* elementType, IntVal* elementCount)
{
    RefPtr<ArrayExpressionType> arrayType = create<ArrayExpressionType>();
    arrayType->baseType = elementType;
    arrayType->arrayLength = elementCount;
    return arrayType;
}

RefPtr<VectorExpressionType> ASTBuilder::getVectorType(
    RefPtr<Type>    elementType,
    RefPtr<IntVal>  elementCount)
{
    auto vectorGenericDecl = m_sharedASTBuilder->findMagicDecl("Vector").as<GenericDecl>();
        
    auto vectorTypeDecl = vectorGenericDecl->inner;

    auto substitutions = new GenericSubstitution();
    substitutions->genericDecl = vectorGenericDecl.Ptr();
    substitutions->args.add(elementType);
    substitutions->args.add(elementCount);

    auto declRef = DeclRef<Decl>(vectorTypeDecl.Ptr(), substitutions);

    return DeclRefType::create(this, declRef).as<VectorExpressionType>();
}

RefPtr<TypeType> ASTBuilder::getTypeType(Type* type)
{
    return create<TypeType>(type);
}


} // namespace Slang
