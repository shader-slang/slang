// type-defs.h

// Syntax class definitions for types.

// The type of a reference to an overloaded name
SYNTAX_CLASS(OverloadGroupType, Type)
RAW(
public:
    virtual String ToString() override;

protected:
    virtual bool EqualsImpl(Type * type) override;
    virtual Type* CreateCanonicalType() override;
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
    virtual Type* CreateCanonicalType() override;
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
    virtual Type* CreateCanonicalType() override;
    virtual int GetHashCode() override;
)
END_SYNTAX_CLASS()

// The type of a reference to a basic block
// in our IR
SYNTAX_CLASS(IRBasicBlockType, Type)
RAW(
public:
    virtual String ToString() override;

protected:
    virtual bool EqualsImpl(Type * type) override;
    virtual Type* CreateCanonicalType() override;
    virtual int GetHashCode() override;
)
END_SYNTAX_CLASS()

// A type that takes the form of a reference to some declaration
SYNTAX_CLASS(DeclRefType, Type)
    DECL_FIELD(DeclRef<Decl>, declRef)

RAW(
    virtual String ToString() override;
    virtual RefPtr<Val> SubstituteImpl(Substitutions* subst, int* ioDiff) override;

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
    virtual Type* CreateCanonicalType() override;
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
    virtual Slang::String ToString() override;
protected:
    virtual BasicExpressionType* GetScalarType() override;
    virtual bool EqualsImpl(Type * type) override;
    virtual Type* CreateCanonicalType() override;
)
END_SYNTAX_CLASS()

// Base type for things we think of as "resources"
ABSTRACT_SYNTAX_CLASS(ResourceTypeBase, DeclRefType)
RAW(
    enum
    {
        // Mask for the overall "shape" of the texture
        ShapeMask		= SLANG_RESOURCE_BASE_SHAPE_MASK,

        // Flag for whether the shape has "array-ness"
        ArrayFlag		= SLANG_TEXTURE_ARRAY_FLAG,

        // Whether or not the texture stores multiple samples per pixel
        MultisampleFlag	= SLANG_TEXTURE_MULTISAMPLE_FLAG,

        // Whether or not this is a shadow texture
        //
        // TODO(tfoley): is this even meaningful/used?
        // ShadowFlag		= 0x80, 
    };

    enum Shape : uint8_t
    {
        Shape1D			= SLANG_TEXTURE_1D,
        Shape2D			= SLANG_TEXTURE_2D,
        Shape3D			= SLANG_TEXTURE_3D,
        ShapeCube		= SLANG_TEXTURE_CUBE,
        ShapeBuffer     = SLANG_TEXTURE_BUFFER,

        Shape1DArray	= Shape1D | ArrayFlag,
        Shape2DArray	= Shape2D | ArrayFlag,
        // No Shape3DArray
        ShapeCubeArray	= ShapeCube | ArrayFlag,
    };

    Shape GetBaseShape() const { return Shape(flavor & ShapeMask); }
    bool isArray() const { return (flavor & ArrayFlag) != 0; }
    bool isMultisample() const { return (flavor & MultisampleFlag) != 0; }
//            bool isShadow() const { return (flavor & ShadowFlag) != 0; }

    SlangResourceShape getShape() const { return flavor & 0xFF; }
    SlangResourceAccess getAccess() const { return (flavor >> 8) & 0xFF; }

    // Bits representing the kind of resource we are looking at
    // (e.g., `Texture2DMS` vs. `TextureCubeArray`)
    typedef uint16_t Flavor;

    static Flavor makeFlavor(SlangResourceShape shape, SlangResourceAccess access)
    {
        return Flavor(shape | (access << 8));
    }
)
    FIELD(Flavor, flavor)
END_SYNTAX_CLASS()

// Resources that contain "elements" that can be fetched
ABSTRACT_SYNTAX_CLASS(ResourceType, ResourceTypeBase)
    // The type that results from fetching an element from this resource
    SYNTAX_FIELD(RefPtr<Type>, elementType)
END_SYNTAX_CLASS()

ABSTRACT_SYNTAX_CLASS(TextureTypeBase, ResourceType)
RAW(
    TextureTypeBase()
    {}
    TextureTypeBase(
        Flavor flavor,
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
        Flavor flavor,
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
        Flavor flavor,
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
        Flavor flavor,
        RefPtr<Type> elementType)
        : TextureTypeBase(flavor, elementType)
    {}
)
END_SYNTAX_CLASS()

SYNTAX_CLASS(SamplerStateType, DeclRefType)

    // What flavor of sampler state is this
    RAW(enum class Flavor : uint8_t
    {
        SamplerState,
        SamplerComparisonState,
    };

    )
    FIELD(Flavor, flavor)
END_SYNTAX_CLASS()

// Other cases of generic types known to the compiler
SYNTAX_CLASS(BuiltinGenericType, DeclRefType)
    SYNTAX_FIELD(RefPtr<Type>, elementType)

    RAW(Type* getElementType() { return elementType; })
END_SYNTAX_CLASS()

// Types that behave like pointers, in that they can be
// dereferenced (implicitly) to access members defined
// in the element type.
SIMPLE_SYNTAX_CLASS(PointerLikeType, BuiltinGenericType)

// HLSL buffer-type resources

SIMPLE_SYNTAX_CLASS(HLSLStructuredBufferType, BuiltinGenericType)
SIMPLE_SYNTAX_CLASS(HLSLRWStructuredBufferType, BuiltinGenericType)

SIMPLE_SYNTAX_CLASS(UntypedBufferResourceType, DeclRefType)
SIMPLE_SYNTAX_CLASS(HLSLByteAddressBufferType, UntypedBufferResourceType)
SIMPLE_SYNTAX_CLASS(HLSLRWByteAddressBufferType, UntypedBufferResourceType)

SIMPLE_SYNTAX_CLASS(HLSLAppendStructuredBufferType, BuiltinGenericType)
SIMPLE_SYNTAX_CLASS(HLSLConsumeStructuredBufferType, BuiltinGenericType)

SYNTAX_CLASS(HLSLPatchType, DeclRefType)
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
SIMPLE_SYNTAX_CLASS(GLSLInputAttachmentType, DeclRefType)

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

SYNTAX_CLASS(ArrayExpressionType, Type)
    SYNTAX_FIELD(RefPtr<Type>, baseType)
    SYNTAX_FIELD(RefPtr<IntVal>, ArrayLength)

RAW(
    virtual Slang::String ToString() override;

protected:
    virtual bool EqualsImpl(Type * type) override;
    virtual Type* CreateCanonicalType() override;
    virtual int GetHashCode() override;
    )
END_SYNTAX_CLASS()

// The effective type of a variable declared with `groupshared` storage qualifier.
SYNTAX_CLASS(GroupSharedType, Type)
    SYNTAX_FIELD(RefPtr<Type>, valueType);

RAW(
    virtual ~GroupSharedType()
    {
    }

    virtual Slang::String ToString() override;

protected:
    virtual bool EqualsImpl(Type * type) override;
    virtual Type* CreateCanonicalType() override;
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
    virtual Type* CreateCanonicalType() override;
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

// Base class for types that map down to
// simple pointers as part of code generation.
SYNTAX_CLASS(PtrTypeBase, DeclRefType)
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

// A type alias of some kind (e.g., via `typedef`)
SYNTAX_CLASS(NamedExpressionType, Type)
    DECL_FIELD(DeclRef<TypeDefDecl>, declRef)

RAW(
    NamedExpressionType()
    {}
    NamedExpressionType(
        DeclRef<TypeDefDecl> declRef)
        : declRef(declRef)
    {}


    virtual String ToString() override;

protected:
    virtual bool EqualsImpl(Type * type) override;
    virtual Type* CreateCanonicalType() override;
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
    virtual RefPtr<Val> SubstituteImpl(Substitutions* subst, int* ioDiff) override;
    virtual bool EqualsImpl(Type * type) override;
    virtual Type* CreateCanonicalType() override;
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
    virtual Type* CreateCanonicalType() override;
)
END_SYNTAX_CLASS()
