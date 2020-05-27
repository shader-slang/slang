// slang-ast-builder.cpp
#include "slang-ast-builder.h"
#include <assert.h>

#include "slang-compiler.h"

namespace Slang {

ASTBuilder::ASTBuilder():
    m_session(nullptr)
{
    memset(m_builtinTypes, 0, sizeof(m_builtinTypes));
}

void ASTBuilder::init(Session* session)
{
    SLANG_ASSERT(session);

    for (Index i = 0; i < Index(BaseType::CountOf); ++i)
    {
        m_builtinTypes[i] = session->getBuiltinType(BaseType(i));
    }

    m_errorType = session->getErrorType();
    m_initializerListType = session->getInitializerListType();
    m_overloadedType = session->getOverloadedType();

    //m_constExprRate = session->g
    //m_irBasicBlockType = session->get

    m_stringType = session->getStringType();
    m_enumTypeType = session->getEnumTypeType();
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
    auto genericDecl = findMagicDecl(m_session, ptrTypeName).dynamicCast<GenericDecl>();
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
    auto vectorGenericDecl = findMagicDecl(m_session, "Vector").as<GenericDecl>();
        
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
