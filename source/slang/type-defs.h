// type-defs.h

// Syntax class definitions for types.

// The type of a reference to an overloaded name
SYNTAX_CLASS(OverloadGroupType, Type)
RAW(
public:
    virtual String ToString() override;

protected:
    virtual bool EqualsImpl(Type * type) override;
    virtual RefPtr<Type> CreateCanonicalType() override;
    virtual int GetHashCode() override;
)
END_SYNTAX_CLASS()

// The type of an initializer-list expression (before it has
// been coerced to some other type)
SYNTAX_CLASS(InitializerListType, Type)
RAW(
    virtual String ToString() override;

protected:
    virtual bool EqualsImpl(Type * type) override;
    virtual RefPtr<Type> CreateCanonicalType() override;
    virtual int GetHashCode() override;
)
END_SYNTAX_CLASS()

// The type of an expression that was erroneous
SYNTAX_CLASS(ErrorType, Type)
RAW(
public:
    virtual String ToString() override;

protected:
    virtual bool EqualsImpl(Type * type) override;
    virtual RefPtr<Val> SubstituteImpl(SubstitutionSet subst, int* ioDiff) override;
    virtual RefPtr<Type> CreateCanonicalType() override;
    virtual int GetHashCode() override;
)
END_SYNTAX_CLASS()

// A type that takes the form of a reference to some declaration
SYNTAX_CLASS(DeclRefType, Type)
    DECL_FIELD(DeclRef<Decl>, declRef)

RAW(
    virtual String ToString() override;
    virtual RefPtr<Val> SubstituteImpl(SubstitutionSet subst, int* ioDiff) override;

    static DeclRefType* Create(
        Session*        session,
        DeclRef<Decl>   declRef);

    DeclRefType()
    {}
    DeclRefType(
        DeclRef<Decl> declRef)
        : declRef(declRef)
    {}
protected:
    virtual int GetHashCode() override;
    virtual bool EqualsImpl(Type * type) override;
    virtual RefPtr<Type> CreateCanonicalType() override;
)
END_SYNTAX_CLASS()

// Base class for types that can be used in arithmetic expressions
ABSTRACT_SYNTAX_CLASS(ArithmeticExpressionType, DeclRefType)
RAW(
public:
    virtual BasicExpressionType* GetScalarType() = 0;
)
END_SYNTAX_CLASS()

SYNTAX_CLASS(BasicExpressionType, ArithmeticExpressionType)

    FIELD(BaseType, baseType)

RAW(
    BasicExpressionType() {}
    BasicExpressionType(
        Slang::BaseType baseType)
        : baseType(baseType)
    {}
protected:
    virtual BasicExpressionType* GetScalarType() override;
    virtual bool EqualsImpl(Type * type) override;
    virtual RefPtr<Type> CreateCanonicalType() override;
)
END_SYNTAX_CLASS()

// Base type for things that are built in to the compiler,
// and will usually have special behavior or a custom
// mapping to the IR level.
ABSTRACT_SYNTAX_CLASS(BuiltinType, DeclRefType)
END_SYNTAX_CLASS()

// Resources that contain "elements" that can be fetched
ABSTRACT_SYNTAX_CLASS(ResourceType, BuiltinType)
    // The type that results from fetching an element from this resource
    SYNTAX_FIELD(RefPtr<Type>, elementType)

    // Shape and access level information for this resource type
    FIELD(TextureFlavor, flavor)

    RAW(
        TextureFlavor::Shape GetBaseShape()
        {
            return flavor.GetBaseShape();
        }
        bool isMultisample() { return flavor.isMultisample(); }
        bool isArray() { return flavor.isArray(); }
        SlangResourceShape getShape() const { return flavor.getShape(); }
        SlangResourceAccess getAccess() { return flavor.getAccess(); }

    )
END_SYNTAX_CLASS()

ABSTRACT_SYNTAX_CLASS(TextureTypeBase, ResourceType)
RAW(
    TextureTypeBase()
    {}
    TextureTypeBase(
        TextureFlavor flavor,
        RefPtr<Type> elementType)
    {
        this->elementType = elementType;
        this->flavor = flavor;
    }
)
END_SYNTAX_CLASS()

SYNTAX_CLASS(TextureType, TextureTypeBase)
RAW(
    TextureType()
    {}
    TextureType(
        TextureFlavor flavor,
        RefPtr<Type> elementType)
        : TextureTypeBase(flavor, elementType)
    {}
)
END_SYNTAX_CLASS()

// This is a base type for texture/sampler pairs,
// as they exist in, e.g., GLSL
SYNTAX_CLASS(TextureSamplerType, TextureTypeBase)
RAW(
    TextureSamplerType()
    {}
    TextureSamplerType(
        TextureFlavor flavor,
        RefPtr<Type> elementType)
        : TextureTypeBase(flavor, elementType)
    {}
)
END_SYNTAX_CLASS()

// This is a base type for `image*` types, as they exist in GLSL
SYNTAX_CLASS(GLSLImageType, TextureTypeBase)
RAW(
    GLSLImageType()
    {}
    GLSLImageType(
        TextureFlavor flavor,
        RefPtr<Type> elementType)
        : TextureTypeBase(flavor, elementType)
    {}
)
END_SYNTAX_CLASS()

SYNTAX_CLASS(SamplerStateType, BuiltinType)
    // What flavor of sampler state is this
    FIELD(SamplerStateFlavor, flavor)
END_SYNTAX_CLASS()

// Other cases of generic types known to the compiler
SYNTAX_CLASS(BuiltinGenericType, BuiltinType)
    SYNTAX_FIELD(RefPtr<Type>, elementType)

    RAW(Type* getElementType() { return elementType; })
END_SYNTAX_CLASS()

// Types that behave like pointers, in that they can be
// dereferenced (implicitly) to access members defined
// in the element type.
SIMPLE_SYNTAX_CLASS(PointerLikeType, BuiltinGenericType)

// HLSL buffer-type resources

SIMPLE_SYNTAX_CLASS(HLSLStructuredBufferTypeBase, BuiltinGenericType)
SIMPLE_SYNTAX_CLASS(HLSLStructuredBufferType, HLSLStructuredBufferTypeBase)
SIMPLE_SYNTAX_CLASS(HLSLRWStructuredBufferType, HLSLStructuredBufferTypeBase)
SIMPLE_SYNTAX_CLASS(HLSLRasterizerOrderedStructuredBufferType, HLSLStructuredBufferTypeBase)

SIMPLE_SYNTAX_CLASS(UntypedBufferResourceType, BuiltinType)
SIMPLE_SYNTAX_CLASS(HLSLByteAddressBufferType, UntypedBufferResourceType)
SIMPLE_SYNTAX_CLASS(HLSLRWByteAddressBufferType, UntypedBufferResourceType)
SIMPLE_SYNTAX_CLASS(HLSLRasterizerOrderedByteAddressBufferType, UntypedBufferResourceType)
SIMPLE_SYNTAX_CLASS(RaytracingAccelerationStructureType, UntypedBufferResourceType)

SIMPLE_SYNTAX_CLASS(HLSLAppendStructuredBufferType, HLSLStructuredBufferTypeBase)
SIMPLE_SYNTAX_CLASS(HLSLConsumeStructuredBufferType, HLSLStructuredBufferTypeBase)

SYNTAX_CLASS(HLSLPatchType, BuiltinType)
RAW(
    Type* getElementType();
    IntVal*         getElementCount();
)
END_SYNTAX_CLASS()

SIMPLE_SYNTAX_CLASS(HLSLInputPatchType, HLSLPatchType)
SIMPLE_SYNTAX_CLASS(HLSLOutputPatchType, HLSLPatchType)

// HLSL geometry shader output stream types

SIMPLE_SYNTAX_CLASS(HLSLStreamOutputType, BuiltinGenericType)
SIMPLE_SYNTAX_CLASS(HLSLPointStreamType, HLSLStreamOutputType)
SIMPLE_SYNTAX_CLASS(HLSLLineStreamType, HLSLStreamOutputType)
SIMPLE_SYNTAX_CLASS(HLSLTriangleStreamType, HLSLStreamOutputType)

//
SIMPLE_SYNTAX_CLASS(GLSLInputAttachmentType, BuiltinType)

// Base class for types used when desugaring parameter block
// declarations, includeing HLSL `cbuffer` or GLSL `uniform` blocks.
SIMPLE_SYNTAX_CLASS(ParameterGroupType, PointerLikeType)

SIMPLE_SYNTAX_CLASS(UniformParameterGroupType, ParameterGroupType)
SIMPLE_SYNTAX_CLASS(VaryingParameterGroupType, ParameterGroupType)

// type for HLSL `cbuffer` declarations, and `ConstantBuffer<T>`
// ALso used for GLSL `uniform` blocks.
SIMPLE_SYNTAX_CLASS(ConstantBufferType, UniformParameterGroupType)

// type for HLSL `tbuffer` declarations, and `TextureBuffer<T>`
SIMPLE_SYNTAX_CLASS(TextureBufferType, UniformParameterGroupType)

// type for GLSL `in` and `out` blocks
SIMPLE_SYNTAX_CLASS(GLSLInputParameterGroupType, VaryingParameterGroupType)
SIMPLE_SYNTAX_CLASS(GLSLOutputParameterGroupType, VaryingParameterGroupType)

// type for GLLSL `buffer` blocks
SIMPLE_SYNTAX_CLASS(GLSLShaderStorageBufferType, UniformParameterGroupType)

// type for Slang `ParameterBlock<T>` type
SIMPLE_SYNTAX_CLASS(ParameterBlockType, UniformParameterGroupType)

SYNTAX_CLASS(ArrayExpressionType, Type)
    SYNTAX_FIELD(RefPtr<Type>, baseType)
    SYNTAX_FIELD(RefPtr<IntVal>, ArrayLength)

RAW(
    virtual Slang::String ToString() override;

protected:
    virtual bool EqualsImpl(Type * type) override;
    virtual RefPtr<Type> CreateCanonicalType() override;
    virtual RefPtr<Val> SubstituteImpl(SubstitutionSet subst, int* ioDiff) override;
    virtual int GetHashCode() override;
    )
END_SYNTAX_CLASS()

// The "type" of an expression that resolves to a type.
// For example, in the expression `float(2)` the sub-expression,
// `float` would have the type `TypeType(float)`.
SYNTAX_CLASS(TypeType, Type)
    // The type that this is the type of...
    SYNTAX_FIELD(RefPtr<Type>, type)

RAW(
public:
    TypeType()
    {}
    TypeType(RefPtr<Type> type)
        : type(type)
    {}

    virtual String ToString() override;

protected:
    virtual bool EqualsImpl(Type * type) override;
    virtual RefPtr<Type> CreateCanonicalType() override;
    virtual int GetHashCode() override;
)
END_SYNTAX_CLASS()

// A vector type, e.g., `vector<T,N>`
SYNTAX_CLASS(VectorExpressionType, ArithmeticExpressionType)

    // The type of vector elements.
    // As an invariant, this should be a basic type or an alias.
    SYNTAX_FIELD(RefPtr<Type>, elementType)

    // The number of elements
    SYNTAX_FIELD(RefPtr<IntVal>, elementCount)

RAW(
    virtual String ToString() override;

protected:
    virtual BasicExpressionType* GetScalarType() override;
)
END_SYNTAX_CLASS()

// A matrix type, e.g., `matrix<T,R,C>`
SYNTAX_CLASS(MatrixExpressionType, ArithmeticExpressionType)
RAW(

    Type* getElementType();
    IntVal*         getRowCount();
    IntVal*         getColumnCount();


    virtual String ToString() override;

protected:
    virtual BasicExpressionType* GetScalarType() override;
)
END_SYNTAX_CLASS()

// The built-in `String` type
SIMPLE_SYNTAX_CLASS(StringType, BuiltinType)

// Type built-in `__EnumType` type
SYNTAX_CLASS(EnumTypeType, BuiltinType)

// TODO: provide accessors for the declaration, the "tag" type, etc.

END_SYNTAX_CLASS()

// Base class for types that map down to
// simple pointers as part of code generation.
SYNTAX_CLASS(PtrTypeBase, BuiltinType)
RAW(
    // Get the type of the pointed-to value.
    Type*   getValueType();
)
END_SYNTAX_CLASS()

// A true (user-visible) pointer type, e.g., `T*`
SYNTAX_CLASS(PtrType, PtrTypeBase)
END_SYNTAX_CLASS()

// A type that represents the behind-the-scenes
// logical pointer that is passed for an `out`
// or `in out` parameter
SYNTAX_CLASS(OutTypeBase, PtrTypeBase)
END_SYNTAX_CLASS()

// The type for an `out` parameter, e.g., `out T`
SYNTAX_CLASS(OutType, OutTypeBase)
END_SYNTAX_CLASS()

// The type for an `in out` parameter, e.g., `in out T`
SYNTAX_CLASS(InOutType, OutTypeBase)
END_SYNTAX_CLASS()

// The type for an `ref` parameter, e.g., `ref T`
SYNTAX_CLASS(RefType, PtrTypeBase)
END_SYNTAX_CLASS()

// A type alias of some kind (e.g., via `typedef`)
SYNTAX_CLASS(NamedExpressionType, Type)
DECL_FIELD(DeclRef<TypeDefDecl>, declRef)

RAW(
    RefPtr<Type> innerType;
    NamedExpressionType()
    {}
    NamedExpressionType(
        DeclRef<TypeDefDecl> declRef)
        : declRef(declRef)
    {}


    virtual String ToString() override;

protected:
    virtual bool EqualsImpl(Type * type) override;
    virtual RefPtr<Type> CreateCanonicalType() override;
    virtual int GetHashCode() override;
)
END_SYNTAX_CLASS()

// A function type is defined by its parameter types
// and its result type.
SYNTAX_CLASS(FuncType, Type)

    // TODO: We may want to preserve parameter names
    // in the list here, just so that we can print
    // out friendly names when printing a function
    // type, even if they don't affect the actual
    // semantic type underneath.

    FIELD(List<RefPtr<Type>>, paramTypes)
    FIELD(RefPtr<Type>, resultType)
RAW(
    FuncType()
    {}

    UInt getParamCount() { return paramTypes.Count(); }
    Type* getParamType(UInt index) { return paramTypes[index]; }
    Type* getResultType() { return resultType; }

    virtual String ToString() override;
protected:
    virtual RefPtr<Val> SubstituteImpl(SubstitutionSet subst, int* ioDiff) override;
    virtual bool EqualsImpl(Type * type) override;
    virtual RefPtr<Type> CreateCanonicalType() override;
    virtual int GetHashCode() override;
)
END_SYNTAX_CLASS()

// The "type" of an expression that names a generic declaration.
SYNTAX_CLASS(GenericDeclRefType, Type)

    DECL_FIELD(DeclRef<GenericDecl>, declRef)

    RAW(
    GenericDeclRefType()
    {}
    GenericDeclRefType(
        DeclRef<GenericDecl> declRef)
        : declRef(declRef)
    {}


    DeclRef<GenericDecl> const& GetDeclRef() const { return declRef; }

    virtual String ToString() override;

protected:
    virtual bool EqualsImpl(Type * type) override;
    virtual int GetHashCode() override;
    virtual RefPtr<Type> CreateCanonicalType() override;
)
END_SYNTAX_CLASS()

// The concrete type for a value wrapped in an existential, accessible
// when the existential is "opened" in some context.
SYNTAX_CLASS(ExtractExistentialType, Type)
RAW(
    DeclRef<VarDeclBase> declRef;

    virtual String ToString() override;
    virtual bool EqualsImpl(Type * type) override;
    virtual int GetHashCode() override;
    virtual RefPtr<Type> CreateCanonicalType() override;
    virtual RefPtr<Val> SubstituteImpl(SubstitutionSet subst, int* ioDiff) override;
)
END_SYNTAX_CLASS()
