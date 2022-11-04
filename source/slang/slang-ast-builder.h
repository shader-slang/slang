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
    void registerBuiltinRequirementDecl(Decl* decl, BuiltinRequirementModifier* modifier);
    void registerMagicDecl(Decl* decl, MagicTypeModifier* modifier);

        /// Get the string type
    Type* getStringType();

        /// Get the native string type
    Type* getNativeStringType();

        /// Get the enum type type
    Type* getEnumTypeType();
        /// Get the __Dynamic type
    Type* getDynamicType();
        /// Get the NullPtr type
    Type* getNullPtrType();
        /// Get the NullPtr type
    Type* getNoneType();
        /// Get the DifferentialBottom type.
    Type* getDifferentialBottomType();

    const ReflectClassInfo* findClassInfo(Name* name);
    SyntaxClass<NodeBase> findSyntaxClass(Name* name);

    const ReflectClassInfo* findClassInfo(const UnownedStringSlice& slice);
    SyntaxClass<NodeBase> findSyntaxClass(const UnownedStringSlice& slice);

        // Look up a magic declaration by its name
    Decl* findMagicDecl(String const& name);

    Decl* tryFindMagicDecl(String const& name);

    Decl* findBuiltinRequirementDecl(BuiltinRequirementKind kind)
    {
        return m_builtinRequirementDecls[kind].GetValue();
    }

        /// A name pool that can be used for lookup for findClassInfo etc. It is the same pool as the Session.
    NamePool* getNamePool() { return m_namePool; }

        /// Must be called before used
    void init(Session* session);

    SharedASTBuilder();

    ~SharedASTBuilder();

protected:
    // State shared between ASTBuilders

    Type* m_errorType = nullptr;
    Type* m_bottomType = nullptr;
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
    Type* m_nativeStringType = nullptr;
    Type* m_enumTypeType = nullptr;
    Type* m_dynamicType = nullptr;
    Type* m_nullPtrType = nullptr;
    Type* m_noneType = nullptr;
    Type* m_diffBottomType = nullptr;
    Type* m_builtinTypes[Index(BaseType::CountOf)];

    Dictionary<String, Decl*> m_magicDecls;
    Dictionary<BuiltinRequirementKind, Decl*> m_builtinRequirementDecls;

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
    // Node cache:
    struct NodeOperand
    {
        union
        {
            NodeBase* nodeOperand;
            int64_t intOperand;
        } values;
        NodeOperand() { values.nodeOperand = nullptr; }
        NodeOperand(NodeBase* node) { values.nodeOperand = node; }
        template<typename EnumType>
        NodeOperand(EnumType intVal)
        {
            static_assert(sizeof(EnumType) <= sizeof(values), "size of operand must be less than pointer size.");
            values.intOperand = 0;
            memcpy(&values, &intVal, sizeof(intVal));
        }
    };
    struct NodeDesc
    {
        ASTNodeType             type;
        ShortList<NodeOperand, 4> operands;

        bool operator==(NodeDesc const& that) const;
        HashCode getHashCode() const;
    };

    template<typename NodeCreateFunc>
    NodeBase* _getOrCreateImpl(NodeDesc const& desc, NodeCreateFunc createFunc)
    {
        if (auto found = m_cachedNodes.TryGetValue(desc))
            return *found;

        auto node = createFunc();
        m_cachedNodes.Add(desc, node);
        return node;
    }

    /// A cache for AST nodes that are entirely defined by their node type, with
    /// no need for additional state.
    Dictionary<NodeDesc, NodeBase*> m_cachedNodes;

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
    T* create()
    {
        auto alloced = m_arena.allocate(sizeof(T));
        memset(alloced, 0, sizeof(T));
        return _initAndAdd(new (alloced) T);
    }

    template<typename T, typename... TArgs>
    T* create(TArgs... args)
    {
        auto alloced = m_arena.allocate(sizeof(T));
        memset(alloced, 0, sizeof(T));
        return _initAndAdd(new (alloced) T(args...));
    }

    template<typename T, typename ... TArgs>
    SLANG_FORCE_INLINE T* getOrCreate(TArgs ... args)
    {
        SLANG_COMPILE_TIME_ASSERT(IsValidType<T>::Value);
        NodeDesc desc;
        desc.type = T::kType;
        addToList(desc.operands, args...);
        return (T*)_getOrCreateImpl(desc, [&]()
            {
                return create<T>(args...);
            });
    }

    template<typename T>
    SLANG_FORCE_INLINE T* getOrCreate()
    {
        SLANG_COMPILE_TIME_ASSERT(IsValidType<T>::Value);

        NodeDesc desc;
        desc.type = T::kType;
        return (T*)_getOrCreateImpl(desc, [this]() { return create<T>(); });
    }

    template<typename T, typename ... TArgs>
    SLANG_FORCE_INLINE T* getOrCreateWithDefaultCtor(TArgs ... args)
    {
        SLANG_COMPILE_TIME_ASSERT(IsValidType<T>::Value);
        NodeDesc desc;
        desc.type = T::kType;
        addToList(desc.operands, args...);
        return (T*)_getOrCreateImpl(desc, [&]()
            {
                return create<T>();
            });
    }

    template<typename T>
    SLANG_FORCE_INLINE T* getOrCreateWithDefaultCtor(ConstArrayView<NodeOperand> operands)
    {
        SLANG_COMPILE_TIME_ASSERT(IsValidType<T>::Value);
        NodeDesc desc;
        desc.type = T::kType;
        desc.operands.addRange(operands);
        return (T*)_getOrCreateImpl(desc, [&]()
            {
                return create<T>();
            });
    }

    DeclRefType* getOrCreateDeclRefType(Decl* decl, Substitutions* outer)
    {
        NodeDesc desc;
        desc.type = DeclRefType::kType;
        desc.operands.add(decl);
        if (outer)
        {
            desc.operands.add(outer);
        }
        auto result = (DeclRefType*)_getOrCreateImpl(desc, [&]() {return create<DeclRefType>(decl, outer); });
        return result;
    }

    GenericSubstitution* getOrCreateGenericSubstitution(GenericDecl* decl, const List<Val*>& args, Substitutions* outer)
    {
        NodeDesc desc;
        desc.type = GenericSubstitution::kType;
        desc.operands.add(decl);
        for (auto arg : args)
            desc.operands.add(arg);
        if (outer)
        {
            desc.operands.add(outer);
        }
        auto result = (GenericSubstitution*)_getOrCreateImpl(desc, [this]() {return create<GenericSubstitution>(); });
        if (result->args.getCount() != args.getCount())
        {
            SLANG_RELEASE_ASSERT(result->args.getCount() == 0);
            result->args.addRange(args);
            result->genericDecl = decl;
            result->outer = outer;
        }
        return result;
    }

    ThisTypeSubstitution* getOrCreateThisTypeSubstitution(InterfaceDecl* interfaceDecl, SubtypeWitness* subtypeWitness, Substitutions* outer)
    {
        NodeDesc desc;
        desc.type = ThisTypeSubstitution::kType;
        desc.operands.add(interfaceDecl);
        desc.operands.add(subtypeWitness);
        if (outer)
        {
            desc.operands.add(outer);
        }
        auto result = (ThisTypeSubstitution*)_getOrCreateImpl(desc, [this]() {return create<ThisTypeSubstitution>(); });
        result->interfaceDecl = interfaceDecl;
        result->witness = subtypeWitness;
        result->outer = outer;
        return result;
    }

    NodeBase* createByNodeType(ASTNodeType nodeType);

        /// Get the built in types
    SLANG_FORCE_INLINE Type* getBoolType() { return m_sharedASTBuilder->m_builtinTypes[Index(BaseType::Bool)]; }
    SLANG_FORCE_INLINE Type* getHalfType() { return m_sharedASTBuilder->m_builtinTypes[Index(BaseType::Half)]; }
    SLANG_FORCE_INLINE Type* getFloatType() { return m_sharedASTBuilder->m_builtinTypes[Index(BaseType::Float)]; }
    SLANG_FORCE_INLINE Type* getDoubleType() { return m_sharedASTBuilder->m_builtinTypes[Index(BaseType::Double)]; }
    SLANG_FORCE_INLINE Type* getIntType() { return m_sharedASTBuilder->m_builtinTypes[Index(BaseType::Int)]; }
    SLANG_FORCE_INLINE Type* getInt64Type() { return m_sharedASTBuilder->m_builtinTypes[Index(BaseType::Int64)]; }
    SLANG_FORCE_INLINE Type* getIntPtrType() { return m_sharedASTBuilder->m_builtinTypes[Index(BaseType::IntPtr)]; }
    SLANG_FORCE_INLINE Type* getUIntType() { return m_sharedASTBuilder->m_builtinTypes[Index(BaseType::UInt)]; }
    SLANG_FORCE_INLINE Type* getUInt64Type() { return m_sharedASTBuilder->m_builtinTypes[Index(BaseType::UInt64)]; }
    SLANG_FORCE_INLINE Type* getUIntPtrType() { return m_sharedASTBuilder->m_builtinTypes[Index(BaseType::UIntPtr)]; }
    SLANG_FORCE_INLINE Type* getVoidType() { return m_sharedASTBuilder->m_builtinTypes[Index(BaseType::Void)]; }

        /// Get a builtin type by the BaseType
    SLANG_FORCE_INLINE Type* getBuiltinType(BaseType flavor) { return m_sharedASTBuilder->m_builtinTypes[Index(flavor)]; }

    Type* getSpecializedBuiltinType(Type* typeParam, const char* magicTypeName);

    Type* getInitializerListType() { return m_sharedASTBuilder->m_initializerListType; }
    Type* getOverloadedType() { return m_sharedASTBuilder->m_overloadedType; }
    Type* getErrorType() { return m_sharedASTBuilder->m_errorType; }
    Type* getBottomType() { return m_sharedASTBuilder->m_bottomType; }
    Type* getDifferentialBottomType() { return m_sharedASTBuilder->getDifferentialBottomType(); }
    Type* getStringType() { return m_sharedASTBuilder->getStringType(); }
    Type* getNullPtrType() { return m_sharedASTBuilder->getNullPtrType(); }
    Type* getNoneType() { return m_sharedASTBuilder->getNoneType(); }
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

        // Construct the type `Optional<valueType>`
    OptionalType* getOptionalType(Type* valueType);

        // Construct a pointer type like `Ptr<valueType>`, but where
        // the actual type name for the pointer type is given by `ptrTypeName`
    PtrTypeBase* getPtrType(Type* valueType, char const* ptrTypeName);

    ArrayExpressionType* getArrayType(Type* elementType, IntVal* elementCount);

    VectorExpressionType* getVectorType(Type* elementType, IntVal* elementCount);

    DifferentialPairType* getDifferentialPairType(
        Type* valueType,
        Witness* primalIsDifferentialWitness);

    DeclRef<InterfaceDecl> getDifferentiableInterface();
    Decl* getDifferentiableAssociatedTypeRequirement();

    bool isDifferentiableInterfaceAvailable();

    DeclRef<Decl> getBuiltinDeclRef(const char* builtinMagicTypeName, Val* genericArg);

    Type* getAndType(Type* left, Type* right);

    Type* getModifiedType(Type* base, Count modifierCount, Val* const* modifiers);
    Type* getModifiedType(Type* base, List<Val*> const& modifiers)
    {
        return getModifiedType(base, modifiers.getCount(), modifiers.getBuffer());
    }
    Val* getUNormModifierVal();
    Val* getSNormModifierVal();

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

    Dictionary<Decl*, GenericSubstitution*> m_genericDefaultSubst;

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
