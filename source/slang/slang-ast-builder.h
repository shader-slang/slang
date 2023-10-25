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
        /// Get the `IDifferentiable` type
    Type* getDiffInterfaceType();

    Type* getErrorType();
    Type* getBottomType();
    Type* getInitializerListType();
    Type* getOverloadedType();

    const ReflectClassInfo* findClassInfo(Name* name);
    SyntaxClass<NodeBase> findSyntaxClass(Name* name);

    const ReflectClassInfo* findClassInfo(const UnownedStringSlice& slice);
    SyntaxClass<NodeBase> findSyntaxClass(const UnownedStringSlice& slice);

        // Look up a magic declaration by its name
    Decl* findMagicDecl(String const& name);

    Decl* tryFindMagicDecl(String const& name);

    Decl* findBuiltinRequirementDecl(BuiltinRequirementKind kind)
    {
        return m_builtinRequirementDecls.getValue(kind);
    }

        /// A name pool that can be used for lookup for findClassInfo etc. It is the same pool as the Session.
    NamePool* getNamePool() { return m_namePool; }

        /// Must be called before used
    void init(Session* session);

    SharedASTBuilder();

    ~SharedASTBuilder();

    ASTBuilder* getInnerASTBuilder() { return m_astBuilder; }

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
    Type* m_diffInterfaceType = nullptr;
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

struct ValKey
{
    Val* val;
    HashCode hashCode;
    ValKey() = default;
    ValKey(Val* v)
    {
        val = v;
        Hasher hasher;
        hasher.hashValue(v->astNodeType);
        for (auto& operand : v->m_operands)
            hasher.hashValue(operand.values.intOperand);
        hashCode = hasher.getResult();
    }
    bool operator==(ValKey other) const
    {
        if (val == other.val) return true;
        if (hashCode != other.hashCode) return false;
        if (val->astNodeType != other.val->astNodeType)
            return false;
        if (val->m_operands.getCount() != other.val->m_operands.getCount())
            return false;
        for (Index i = 0; i < val->m_operands.getCount(); i++)
            if (val->m_operands[i].values.intOperand != other.val->m_operands[i].values.intOperand)
                return false;
        return true;
    }
    bool operator==(const ValNodeDesc& desc) const
    {
        if (hashCode != desc.getHashCode()) return false;
        if (val->astNodeType != desc.type)
            return false;
        if (val->m_operands.getCount() != desc.operands.getCount())
            return false;
        for (Index i = 0; i < val->m_operands.getCount(); i++)
            if (val->m_operands[i].values.intOperand != desc.operands[i].values.intOperand)
                return false;
        return true;
    }
    HashCode getHashCode() const { return hashCode; }
};

// Add a specialization which can hash both ValKey and ValNodeDesc
template<>
struct Hash<ValKey>
{
    using is_transparent = void;
    auto operator()(const ValKey& k) const
    {
        return k.getHashCode();
    }
    auto operator()(const ValNodeDesc& k) const
    {
        return Hash<ValNodeDesc>{}(k);
    }
};

// A functor which can compare ValKey for equality with ValNodeDesc
struct ValKeyEqual
{
    using is_transparent = void;
    bool operator()(const Slang::ValKey& a, const Slang::ValKey& b) const
    {
        return a == b;
    }
    bool operator()(const Slang::ValNodeDesc& a, const Slang::ValKey& b) const
    {
        return b == a;
    }
};

class ASTBuilder : public RefObject
{
    friend class SharedASTBuilder;

public:

    Val* _getOrCreateImpl(ValNodeDesc&& desc)
    {
        if (auto found = m_cachedNodes.tryGetValue(desc))
            return *found;

        auto node = as<Val>(createByNodeType(desc.type));
        SLANG_ASSERT(node);
        for (auto& operand : desc.operands)
            node->m_operands.add(operand);
        auto result = node;
        m_cachedNodes.add(ValKey(node), _Move(node));
        return result;
    }

    /// A cache for AST nodes that are entirely defined by their node type, with
    /// no need for additional state.
    Dictionary<ValKey, Val*, Hash<ValKey>, ValKeyEqual> m_cachedNodes;

    Dictionary<GenericDecl*, List<Val*>> m_cachedGenericDefaultArgs;

    /// Create AST types
    template <typename T>
    T* createImpl()
    {
        auto alloced = m_arena.allocate(sizeof(T));
        memset(alloced, 0, sizeof(T));
        auto result = _initAndAdd(new (alloced) T);
        return result;
    }

    template<typename T, typename... TArgs>
    T* createImpl(TArgs&&... args)
    {
        auto alloced = m_arena.allocate(sizeof(T));
        memset(alloced, 0, sizeof(T));
        auto result = _initAndAdd(new (alloced) T(std::forward<TArgs>(args)...));
        return result;
    }

    template <typename T>
    T* create()
    {
        static_assert(!IsBaseOf<Val, T>::Value, "ASTBuilder::create cannot be used to create a Val, use getOrCreate instead.");
        return createImpl<T>();
    }

    template<typename T, typename... TArgs>
    T* create(TArgs&&... args)
    {
        static_assert(!IsBaseOf<Val, T>::Value, "ASTBuilder::create cannot be used to create a Val, use getOrCreate instead.");
        return createImpl<T>(args...);
    }

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

    Index getEpoch();

    void incrementEpoch();

    MemoryArena& getArena() { return m_arena; }

    template<typename T, typename ... TArgs>
    SLANG_FORCE_INLINE T* getOrCreate(TArgs ... args)
    {
        SLANG_COMPILE_TIME_ASSERT(IsValidType<T>::Value);
        ValNodeDesc desc;
        desc.type = T::kType;
        addOrAppendToNodeList(desc.operands, args...);
        desc.init();
        auto result = (T*)_getOrCreateImpl(_Move(desc));
        return result;
    }

    template<typename T>
    SLANG_FORCE_INLINE T* getOrCreate()
    {
        SLANG_COMPILE_TIME_ASSERT(IsValidType<T>::Value);

        ValNodeDesc desc;
        desc.type = T::kType;
        desc.init();
        auto result = (T*)_getOrCreateImpl(_Move(desc));
        return result;
    }

    InterfaceDecl* createInterfaceDecl(SourceLoc loc)
    {
        auto interfaceDecl = create<InterfaceDecl>();
        // Always include a `This` member and a `This:IThisInterface` member.
        auto thisDecl = create<ThisTypeDecl>();
        thisDecl->nameAndLoc.name = m_sharedASTBuilder->getNamePool()->getName(UnownedStringSlice("This", 4));
        thisDecl->nameAndLoc.loc = loc;
        interfaceDecl->addMember(thisDecl);
        auto thisConstraint = create<ThisTypeConstraintDecl>();
        thisConstraint->loc = loc;
        thisDecl->addMember(thisConstraint);
        return interfaceDecl;
    }

    template<typename T>
    DeclRef<T> getDirectDeclRef(T* decl, typename std::enable_if_t<std::is_base_of_v<Decl, T>>* = nullptr)
    {
        return DeclRef<T>(decl);
    }

    template<typename T>
    DeclRef<T> getMemberDeclRef(DeclRef<Decl> parent, T* memberDecl)
    {
        if (!parent)
            return getDirectDeclRef(memberDecl);
        // A Generic value/type ParamDecl is always referred to directly.
        if (as<GenericTypeParamDecl>(memberDecl) || as<GenericValueParamDecl>(memberDecl))
            return getDirectDeclRef(memberDecl);
        if (as<ThisTypeDecl>(memberDecl) && !as<InterfaceDecl>(memberDecl->parentDecl))
            return as<T>(parent);

        if (auto parentMemberDeclRef = as<MemberDeclRef>(parent.declRefBase))
        {
            return DeclRef<T>(getMemberDeclRef(parentMemberDeclRef->getParent(), memberDecl));
        }
        else if (auto lookupDeclRef = as<LookupDeclRef>(parent.declRefBase))
        {
            // Handle some specicial case rules due to the way some of our builtin decls are
            // represented.
            // - Member(Lookup(w, This), x) ==> Lookup(w, X)
            //   Lookup of x from This is a lookup from w directly.
            // - Member(Lookup(w, someExtension), x) ==> Lookup(w, X)
            //   Lookup of a decl defined in an extension is to lookup directly.
            // - Member(Lookup(w, AssociatedType), TypeConstraintDecl) ==> Lookup(w, TypeConstraintDecl)
            //   Type constraint of an associated type is defined directly in w.

            auto parentDeclKind = lookupDeclRef->getDecl()->astNodeType;
            switch (parentDeclKind)
            {
            case ASTNodeType::ThisTypeDecl:
            case ASTNodeType::ExtensionDecl:
            case ASTNodeType::AssocTypeDecl:
                return getLookupDeclRef(lookupDeclRef->getLookupSource(), lookupDeclRef->getWitness(), memberDecl);
            default:
                break;
            }
        }
        else if (auto directDeclRef = as<DirectDeclRef>(parent.declRefBase))
        {
            return makeDeclRef(memberDecl);
        }

#if _DEBUG
        // Verify that member is indeed a member of parent.
        auto parentDecl = parent.getDecl();
        while (as<ThisTypeDecl>(parentDecl))
            parentDecl = parentDecl->parentDecl;
        bool foundParent = false;
        for (Decl* dd = memberDecl; dd; dd = dd->parentDecl)
        {
            if (dd == parentDecl)
            {
                foundParent = true;
                break;
            }
        }
        SLANG_ASSERT(foundParent);
#endif

        return DeclRef<T>(getOrCreate<MemberDeclRef>(memberDecl, parent.declRefBase));
    }

    ConstantIntVal* getIntVal(Type* type, IntegerLiteralValue value)
    {
        return getOrCreate<ConstantIntVal>(type, value);
    }

    TypeCastIntVal* getTypeCastIntVal(Type* type, Val* base)
    {
        // Unwrap any existing type casts.
        while (auto baseTypeCast = as<TypeCastIntVal>(base))
            base = baseTypeCast->getBase();
            
        return getOrCreate<TypeCastIntVal>(type, base);
    }

    DeclRef<Decl> getGenericAppDeclRef(DeclRef<GenericDecl> genericDeclRef, ConstArrayView<Val*> args, Decl* innerDecl = nullptr)
    {
        if (!innerDecl)
            innerDecl = genericDeclRef.getDecl()->inner;

        return getOrCreate<GenericAppDeclRef>(innerDecl, genericDeclRef, args);
    }

    DeclRef<Decl> getGenericAppDeclRef(DeclRef<GenericDecl> genericDeclRef, Val::OperandView<Val> args, Decl* innerDecl = nullptr)
    {
        if (!innerDecl)
            innerDecl = genericDeclRef.getDecl()->inner;

        return getOrCreate<GenericAppDeclRef>(innerDecl, genericDeclRef, args);
    }

    LookupDeclRef* getLookupDeclRef(Type* base, SubtypeWitness* subtypeWitness, Decl* declToLookup)
    {
        auto result = getOrCreate<LookupDeclRef>(declToLookup, base, subtypeWitness);
        return result;
    }

    LookupDeclRef* getLookupDeclRef(SubtypeWitness* subtypeWitness, Decl* declToLookup)
    {
        return getLookupDeclRef(subtypeWitness->getSub(), subtypeWitness, declToLookup);
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
    Type* getSpecializedBuiltinType(ArrayView<Val*> genericArgs, const char* magicTypeName);

    Type* getInitializerListType() { return m_sharedASTBuilder->getInitializerListType(); }
    Type* getOverloadedType() { return m_sharedASTBuilder->getOverloadedType(); }
    Type* getErrorType() { return m_sharedASTBuilder->getErrorType(); }
    Type* getBottomType() { return m_sharedASTBuilder->getBottomType(); }
    Type* getStringType() { return m_sharedASTBuilder->getStringType(); }
    Type* getNullPtrType() { return m_sharedASTBuilder->getNullPtrType(); }
    Type* getNoneType() { return m_sharedASTBuilder->getNoneType(); }
    Type* getEnumTypeType() { return m_sharedASTBuilder->getEnumTypeType(); }
    Type* getDiffInterfaceType() { return m_sharedASTBuilder->getDiffInterfaceType(); }
        // Construct the type `Ptr<valueType>`, where `Ptr`
        // is looked up as a builtin type.
    PtrType* getPtrType(Type* valueType);

        // Construct the type `Out<valueType>`
    OutType* getOutType(Type* valueType);

        // Construct the type `InOut<valueType>`
    InOutType* getInOutType(Type* valueType);

        // Construct the type `Ref<valueType>`
    RefType* getRefType(Type* valueType);

        // Construct the type `ConstRef<valueType>`
    ConstRefType* getConstRefType(Type* valueType);

        // Construct the type `Optional<valueType>`
    OptionalType* getOptionalType(Type* valueType);

        // Construct a pointer type like `Ptr<valueType>`, but where
        // the actual type name for the pointer type is given by `ptrTypeName`
    PtrTypeBase* getPtrType(Type* valueType, char const* ptrTypeName);

    ArrayExpressionType* getArrayType(Type* elementType, IntVal* elementCount);

    VectorExpressionType* getVectorType(Type* elementType, IntVal* elementCount);

    MatrixExpressionType* getMatrixType(Type* elementType, IntVal* rowCount, IntVal* colCount, IntVal* layout);

    ConstantBufferType* getConstantBufferType(Type* elementType);

    ParameterBlockType* getParameterBlockType(Type* elementType);

    HLSLStructuredBufferType* getStructuredBufferType(Type* elementType);

    HLSLRWStructuredBufferType* getRWStructuredBufferType(Type* elementType);

    SamplerStateType* getSamplerStateType();

    DifferentialPairType* getDifferentialPairType(
        Type* valueType,
        Witness* primalIsDifferentialWitness);

    DeclRef<InterfaceDecl> getDifferentiableInterfaceDecl();
    Type* getDifferentiableInterfaceType();

    bool isDifferentiableInterfaceAvailable();

    MeshOutputType* getMeshOutputTypeFromModifier(
        HLSLMeshShaderOutputModifier* modifier,
        Type* elementType,
        IntVal* maxElementCount);

    DeclRef<Decl> getBuiltinDeclRef(const char* builtinMagicTypeName, Val* genericArg);
    DeclRef<Decl> getBuiltinDeclRef(const char* builtinMagicTypeName, ArrayView<Val*> genericArgs);

    Type* getAndType(Type* left, Type* right);

    Type* getModifiedType(Type* base, Count modifierCount, Val* const* modifiers);
    Type* getModifiedType(Type* base, List<Val*> const& modifiers)
    {
        return getModifiedType(base, modifiers.getCount(), modifiers.getBuffer());
    }
    Val* getUNormModifierVal();
    Val* getSNormModifierVal();
    Val* getNoDiffModifierVal();

    TupleType* getTupleType(List<Type*>& types);

    FuncType* getFuncType(ArrayView<Type*> parameters, Type* result, Type* errorType = nullptr);

    TypeType* getTypeType(Type* type);

        /// Produce a witness that `T : T` for any type `T`
    TypeEqualityWitness* getTypeEqualityWitness(
        Type* type);

    DeclaredSubtypeWitness* getDeclaredSubtypeWitness(
        Type*                   subType,
        Type*                   superType,
        DeclRef<Decl> const&    declRef);

        /// Produce a witness that `A <: C` given witnesses that `A <: B` and `B <: C`
    SubtypeWitness* getTransitiveSubtypeWitness(
        SubtypeWitness*    aIsSubtypeOfBWitness,
        SubtypeWitness*    bIsSubtypeOfCWitness);

        /// Produce a witness that `T <: L` or `T <: R` given `T <: L&R`
    SubtypeWitness* getExtractFromConjunctionSubtypeWitness(
        Type*           subType,
        Type*           superType,
        SubtypeWitness* subIsSubtypeOfConjunction,
        int             indexOfSuperTypeInConjunction);

        /// Produce a witnes that `S <: L&R` given witnesses that `S <: L` and `S <: R`
    SubtypeWitness* getConjunctionSubtypeWitness(
        Type*           sub,
        Type*           lAndR,
        SubtypeWitness* subIsLWitness,
        SubtypeWitness* subIsRWitness);

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

    Index getId() { return m_id; }

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
        if (node->getClassInfo().isSubClassOf(*ASTClassInfo::getInfo(Val::kType)))
        {
            auto val = (Val*)(node);
            val->m_resolvedValEpoch = getEpoch();
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

// Retrieves the ASTBuilder for the current compilation session.
ASTBuilder* getCurrentASTBuilder();

// Sets the ASTBuilder for the current compilation session.
void setCurrentASTBuilder(ASTBuilder* astBuilder);

struct SetASTBuilderContextRAII
{
    ASTBuilder* previousASTBuilder = nullptr;
    SetASTBuilderContextRAII(ASTBuilder* astBuilder)
    {
        previousASTBuilder = getCurrentASTBuilder();
        setCurrentASTBuilder(astBuilder);
    }
    ~SetASTBuilderContextRAII()
    {
        setCurrentASTBuilder(previousASTBuilder);
    }
};

#define SLANG_AST_BUILDER_RAII(astBuilder) SetASTBuilderContextRAII _setASTBuilderContextRAII(astBuilder)

} // namespace Slang

#endif
