// slang-ast-dump.h
#ifndef SLANG_AST_BUILDER_H
#define SLANG_AST_BUILDER_H

#include <type_traits>

#include "slang-ast-support-types.h"
#include "slang-ast-all.h"

#include "../core/slang-type-traits.h"
#include "../core/slang-memory-arena.h"

namespace Slang
{

class SharedASTBuilder : public RefObject
{
    friend class ASTBuilder;
public:
    
    void registerBuiltinDecl(Decl* decl, BuiltinTypeModifier* modifier);
    void registerMagicDecl(Decl* decl, MagicTypeModifier* modifier);

        /// Get the string type
    Type* getStringType();
        /// Get the enum type type
    Type* getEnumTypeType();
        /// Get the __Dynamic type
    Type* getDynamicType();

    const ReflectClassInfo* findClassInfo(Name* name);
    SyntaxClass<NodeBase> findSyntaxClass(Name* name);

    const ReflectClassInfo* findClassInfo(const UnownedStringSlice& slice);
    SyntaxClass<NodeBase> findSyntaxClass(const UnownedStringSlice& slice);

        // Look up a magic declaration by its name
    Decl* findMagicDecl(String const& name);

        /// A name pool that can be used for lookup for findClassInfo etc. It is the same pool as the Session.
    NamePool* getNamePool() { return m_namePool; }

        /// Must be called before used
    void init(Session* session);

    SharedASTBuilder();

    ~SharedASTBuilder();

protected:
    // State shared between ASTBuilders

    Type* m_errorType = nullptr;
    Type* m_initializerListType = nullptr;
    Type* m_overloadedType = nullptr;

    // The following types are created lazily, such that part of their definition
    // can be in the standard library
    // 
    // Note(tfoley): These logically belong to `Type`,
    // but order-of-declaration stuff makes that tricky
    //
    // TODO(tfoley): These should really belong to the compilation context!
    //
    Type* m_stringType = nullptr;
    Type* m_enumTypeType = nullptr;
    Type* m_dynamicType = nullptr;

    Type* m_builtinTypes[Index(BaseType::CountOf)];

    Dictionary<String, Decl*> m_magicDecls;

    Dictionary<UnownedStringSlice, const ReflectClassInfo*> m_sliceToTypeMap;
    Dictionary<Name*, const ReflectClassInfo*> m_nameToTypeMap;
    
    NamePool* m_namePool = nullptr;

    // This is a private builder used for these shared types
    ASTBuilder* m_astBuilder = nullptr;
    Session* m_session = nullptr;

    Index m_id = 1;
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


        /// Create AST types 
    template <typename T>
    T* create() { return _initAndAdd(new (m_arena.allocate(sizeof(T))) T); }
    template<typename T, typename P0>
    T* create(const P0& p0) { return _initAndAdd(new (m_arena.allocate(sizeof(T))) T(p0)); }
    template<typename T, typename P0, typename P1>
    T* create(const P0& p0, const P1& p1) { return _initAndAdd(new (m_arena.allocate(sizeof(T))) T(p0, p1));}

    NodeBase* createByNodeType(ASTNodeType nodeType);

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
    PtrType* getPtrType(Type* valueType);

        // Construct the type `Out<valueType>`
    OutType* getOutType(Type* valueType);

        // Construct the type `InOut<valueType>`
    InOutType* getInOutType(Type* valueType);

        // Construct the type `Ref<valueType>`
    RefType* getRefType(Type* valueType);

        // Construct a pointer type like `Ptr<valueType>`, but where
        // the actual type name for the pointer type is given by `ptrTypeName`
    PtrTypeBase* getPtrType(Type* valueType, char const* ptrTypeName);

        // Construct a pointer type like `Ptr<valueType>`, but where
        // the generic declaration for the pointer type is `genericDecl`
    PtrTypeBase* getPtrType(Type* valueType, GenericDecl* genericDecl);

    ArrayExpressionType* getArrayType(Type* elementType, IntVal* elementCount);

    VectorExpressionType* getVectorType(Type* elementType, IntVal* elementCount);

    Type* getAndType(Type* left, Type* right);

    TypeType* getTypeType(Type* type);

        /// Helpers to get type info from the SharedASTBuilder
    const ReflectClassInfo* findClassInfo(const UnownedStringSlice& slice) { return m_sharedASTBuilder->findClassInfo(slice); }
    SyntaxClass<NodeBase> findSyntaxClass(const UnownedStringSlice& slice) { return m_sharedASTBuilder->findSyntaxClass(slice); }

    const ReflectClassInfo* findClassInfo(Name* name) { return m_sharedASTBuilder->findClassInfo(name); }
    SyntaxClass<NodeBase> findSyntaxClass(Name* name) { return m_sharedASTBuilder->findSyntaxClass(name); }

    MemoryArena& getMemoryArena() { return m_arena; }

        /// Get the shared AST builder
    SharedASTBuilder* getSharedASTBuilder() { return m_sharedASTBuilder; }

        /// Get the global session
    Session* getGlobalSession() { return m_sharedASTBuilder->m_session; }

        /// Ctor
    ASTBuilder(SharedASTBuilder* sharedASTBuilder, const String& name);

        /// Dtor
    ~ASTBuilder();

protected:
    // Special default Ctor that can only be used by SharedASTBuilder
    ASTBuilder();

    template <typename T>
    SLANG_FORCE_INLINE T* _initAndAdd(T* node)
    {
        SLANG_COMPILE_TIME_ASSERT(IsValidType<T>::Value);

        node->init(T::kType, this);
        // Only add it if it has a dtor that does some work
        if (!std::is_trivially_destructible<T>::value)
        {
            // Keep such that dtor can be run on ASTBuilder being dtored
            m_dtorNodes.add(node);
        }
        return node;
    }

    String m_name;
    Index m_id;

        /// List of all nodes that require being dtored when ASTBuilder is dtored
    List<NodeBase*> m_dtorNodes;

    SharedASTBuilder* m_sharedASTBuilder;

    MemoryArena m_arena;
};

} // namespace Slang

#endif
