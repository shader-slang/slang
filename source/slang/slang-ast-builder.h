// slang-ast-dump.h
#ifndef SLANG_AST_BUILDER_H
#define SLANG_AST_BUILDER_H

#include "slang-ast-support-types.h"
#include "slang-ast-all.h"

#include "../core/slang-type-traits.h"

namespace Slang
{

class SharedASTBuilder : public RefObject
{
    friend class ASTBuilder;
public:
    
    void registerBuiltinDecl(RefPtr<Decl> decl, RefPtr<BuiltinTypeModifier> modifier);
    void registerMagicDecl(RefPtr<Decl> decl, RefPtr<MagicTypeModifier> modifier);

        /// Get the string type
    Type* getStringType();
        /// Get the enum type type
    Type* getEnumTypeType();

    const ReflectClassInfo* findClassInfo(Name* name);
    SyntaxClass<RefObject> findSyntaxClass(Name* name);

    const ReflectClassInfo* findClassInfo(const UnownedStringSlice& slice);
    SyntaxClass<RefObject> findSyntaxClass(const UnownedStringSlice& slice);

        // Look up a magic declaration by its name
    RefPtr<Decl> findMagicDecl(String const& name);

        /// A name pool that can be used for lookup for findClassInfo etc. It is the same pool as the Session.
    NamePool* getNamePool() { return m_namePool; }

        /// Must be called before used
    void init(Session* session);

    SharedASTBuilder();

    ~SharedASTBuilder();

protected:
    // State shared between ASTBuilders

    RefPtr<Type> m_errorType;
    RefPtr<Type> m_initializerListType;
    RefPtr<Type> m_overloadedType;

    // The following types are created lazily, such that part of their definition
    // can be in the standard library
    // 
    // Note(tfoley): These logically belong to `Type`,
    // but order-of-declaration stuff makes that tricky
    //
    // TODO(tfoley): These should really belong to the compilation context!
    //
    RefPtr<Type> m_stringType;
    RefPtr<Type> m_enumTypeType;

    RefPtr<Type> m_builtinTypes[Index(BaseType::CountOf)];

    Dictionary<String, Decl*> m_magicDecls;

    Dictionary<UnownedStringSlice, const ReflectClassInfo*> m_sliceToTypeMap;
    Dictionary<Name*, const ReflectClassInfo*> m_nameToTypeMap;
    
    NamePool* m_namePool = nullptr;

    // This is a private builder used for these shared types
    ASTBuilder* m_astBuilder = nullptr;
    Session* m_session = nullptr;
};

class ASTBuilder : public RefObject
{
    friend class SharedASTBuilder;
public:

    // For compile time check to see if thing being constructed is an AST type
    template <typename T>
    struct IsValidType
    {
        enum
        {
            Value = IsBaseOf<NodeBase, T>::Value
        };
    };

        /// Create AST type. 
    template <typename T>
    T* create() { SLANG_COMPILE_TIME_ASSERT(IsValidType<T>::Value);  T* node = new T; node->init(T::kType, this); return node; }

    template<typename T, typename P0>
    T* create(const P0& p0) { SLANG_COMPILE_TIME_ASSERT(IsValidType<T>::Value); T* node = new T(p0); node->init(T::kType, this); return node;}

    template<typename T, typename P0, typename P1>
    T* create(const P0& p0, const P1& p1) { SLANG_COMPILE_TIME_ASSERT(IsValidType<T>::Value); T* node = new T(p0, p1); node->init(T::kType, this); return node; }

        /// Get the built in types
    SLANG_FORCE_INLINE Type* getBoolType() { return m_sharedASTBuilder->m_builtinTypes[Index(BaseType::Bool)]; }
    SLANG_FORCE_INLINE Type* getHalfType() { return m_sharedASTBuilder->m_builtinTypes[Index(BaseType::Half)]; }
    SLANG_FORCE_INLINE Type* getFloatType() { return m_sharedASTBuilder->m_builtinTypes[Index(BaseType::Float)]; }
    SLANG_FORCE_INLINE Type* getDoubleType() { return m_sharedASTBuilder->m_builtinTypes[Index(BaseType::Double)]; }
    SLANG_FORCE_INLINE Type* getIntType() { return m_sharedASTBuilder->m_builtinTypes[Index(BaseType::Int)]; }
    SLANG_FORCE_INLINE Type* getInt64Type() { return m_sharedASTBuilder->m_builtinTypes[Index(BaseType::Int64)]; }
    SLANG_FORCE_INLINE Type* getUIntType() { return m_sharedASTBuilder->m_builtinTypes[Index(BaseType::UInt)]; }
    SLANG_FORCE_INLINE Type* getUInt64Type() { return m_sharedASTBuilder->m_builtinTypes[Index(BaseType::UInt64)]; }
    SLANG_FORCE_INLINE Type* getVoidType() { return m_sharedASTBuilder->m_builtinTypes[Index(BaseType::Void)]; }

        /// Get a builtin type by the BaseType
    SLANG_FORCE_INLINE Type* getBuiltinType(BaseType flavor) { return m_sharedASTBuilder->m_builtinTypes[Index(flavor)]; }

    Type* getInitializerListType() { return m_sharedASTBuilder->m_initializerListType; }
    Type* getOverloadedType() { return m_sharedASTBuilder->m_overloadedType; }
    Type* getErrorType() { return m_sharedASTBuilder->m_errorType; }
    Type* getStringType() { return m_sharedASTBuilder->getStringType(); }
    Type* getEnumTypeType() { return m_sharedASTBuilder->getEnumTypeType(); }

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

        /// Helpers to get type info from the SharedASTBuilder
    const ReflectClassInfo* findClassInfo(const UnownedStringSlice& slice) { return m_sharedASTBuilder->findClassInfo(slice); }
    SyntaxClass<RefObject> findSyntaxClass(const UnownedStringSlice& slice) { return m_sharedASTBuilder->findSyntaxClass(slice); }

    const ReflectClassInfo* findClassInfo(Name* name) { return m_sharedASTBuilder->findClassInfo(name); }
    SyntaxClass<RefObject> findSyntaxClass(Name* name) { return m_sharedASTBuilder->findSyntaxClass(name); }

        /// Get the shared AST builder
    SharedASTBuilder* getSharedASTBuilder() { return m_sharedASTBuilder; }

        /// Get the global session
    Session* getGlobalSession() { return m_sharedASTBuilder->m_session; }

        /// Ctor
    ASTBuilder(SharedASTBuilder* sharedASTBuilder);

protected:
    // Special default Ctor that can only be used by SharedASTBuilder
    ASTBuilder();

    SharedASTBuilder* m_sharedASTBuilder;
};

} // namespace Slang

#endif
