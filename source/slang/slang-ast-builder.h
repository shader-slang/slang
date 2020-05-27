// slang-ast-dump.h
#ifndef SLANG_AST_BUILDER_H
#define SLANG_AST_BUILDER_H

#include "slang-ast-support-types.h"
#include "slang-ast-all.h"

namespace Slang
{

class ASTBuilder
{
public:
    template <typename T>
    T* create() { T* node = new T; node->setASTBuilder(this); return node; }

    template<typename T, typename P0>
    T* create(const P0& p0) { T* node = new T(p0); node->setASTBuilder(this); return node;}

    template<typename T, typename P0, typename P1>
    T* create(const P0& p0, const P1& p1) { T* node = new T(p0, p1); node->setASTBuilder(this); return node; }

    SLANG_FORCE_INLINE Type* getBoolType() { return m_builtinTypes[Index(BaseType::Bool)]; }
    SLANG_FORCE_INLINE Type* getHalfType() { return m_builtinTypes[Index(BaseType::Half)]; }
    SLANG_FORCE_INLINE Type* getFloatType() { return m_builtinTypes[Index(BaseType::Float)]; }
    SLANG_FORCE_INLINE Type* getDoubleType() { return m_builtinTypes[Index(BaseType::Double)]; }
    SLANG_FORCE_INLINE Type* getIntType() { return m_builtinTypes[Index(BaseType::Int)]; }
    SLANG_FORCE_INLINE Type* getInt64Type() { return m_builtinTypes[Index(BaseType::Int64)]; }
    SLANG_FORCE_INLINE Type* getUIntType() { return m_builtinTypes[Index(BaseType::UInt)]; }
    SLANG_FORCE_INLINE Type* getUInt64Type() { return m_builtinTypes[Index(BaseType::UInt64)]; }
    SLANG_FORCE_INLINE Type* getVoidType() { return m_builtinTypes[Index(BaseType::Void)]; }
    SLANG_FORCE_INLINE Type* getBuiltinType(BaseType flavor) { return m_builtinTypes[Index(flavor)]; }

    Type* getInitializerListType() { return m_initializerListType; }
    Type* getOverloadedType() { return m_overloadedType; }
    Type* getErrorType() { return m_errorType; }
    Type* getStringType() { return m_stringType; }
    Type* getEnumTypeType() { return m_enumTypeType; }

        // Construct the type `Ptr<valueType>`, where `Ptr`
        // is looked up as a builtin type.
    RefPtr<PtrType> getPtrType(RefPtr<Type> valueType);

        // Construct the type `Out<valueType>`
    RefPtr<OutType> getOutType(RefPtr<Type> valueType);

        // Construct the type `InOut<valueType>`
    RefPtr<InOutType> getInOutType(RefPtr<Type> valueType);

        // Construct the type `Ref<valueType>`
    RefPtr<RefType> getRefType(RefPtr<Type> valueType);

        // Construct a pointer type like `Ptr<valueType>`, but where
        // the actual type name for the pointer type is given by `ptrTypeName`
    RefPtr<PtrTypeBase> getPtrType(RefPtr<Type> valueType, char const* ptrTypeName);

        // Construct a pointer type like `Ptr<valueType>`, but where
        // the generic declaration for the pointer type is `genericDecl`
    RefPtr<PtrTypeBase> getPtrType(RefPtr<Type> valueType, GenericDecl* genericDecl);

    RefPtr<ArrayExpressionType> getArrayType(Type* elementType, IntVal* elementCount);

    RefPtr<VectorExpressionType> getVectorType(RefPtr<Type> elementType, RefPtr<IntVal> elementCount);

    RefPtr<TypeType> getTypeType(Type* type);

    Session* getGlobalSession() { return m_session; }

        /// Ctor
    ASTBuilder();

        /// Must be called before can be used
    void init(Session* session);

protected:
    Type* m_errorType;
    Type* m_initializerListType;
    Type* m_overloadedType;
    Type* m_constExprRate;
    Type* m_irBasicBlockType;

    Type* m_stringType;
    Type* m_enumTypeType;

    Type* m_builtinTypes[Index(BaseType::CountOf)];

    Session* m_session;
};

} // namespace Slang

#endif
