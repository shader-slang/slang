// slang-ast-type.h

#pragma once

#include "slang-ast-base.h"

namespace Slang {

// Syntax class definitions for types.

// The type of a reference to an overloaded name
class OverloadGroupType : public Type 
{
    SLANG_AST_CLASS(OverloadGroupType)

    // Overrides should be public so base classes can access
    void _toTextOverride(StringBuilder& out);
    Type* _createCanonicalTypeOverride();
    bool _equalsImplOverride(Type* type);
    HashCode _getHashCodeOverride();
};

// The type of an initializer-list expression (before it has
// been coerced to some other type)
class InitializerListType : public Type 
{
    SLANG_AST_CLASS(InitializerListType)

    
    // Overrides should be public so base classes can access
    void _toTextOverride(StringBuilder& out);
    Type* _createCanonicalTypeOverride();
    bool _equalsImplOverride(Type* type);
    HashCode _getHashCodeOverride();
};

// The type of an expression that was erroneous
class ErrorType : public Type 
{
    SLANG_AST_CLASS(ErrorType)

    // Overrides should be public so base classes can access
    void _toTextOverride(StringBuilder& out);
    Type* _createCanonicalTypeOverride();
    bool _equalsImplOverride(Type* type);
    HashCode _getHashCodeOverride();
    Val* _substituteImplOverride(ASTBuilder* astBuilder, SubstitutionSet subst, int* ioDiff);
};

// The bottom/empty type that has no values.
class BottomType : public Type
{
    SLANG_AST_CLASS(BottomType)

    // Overrides should be public so base classes can access
    void _toTextOverride(StringBuilder& out);
    Type* _createCanonicalTypeOverride();
    bool _equalsImplOverride(Type* type);
    HashCode _getHashCodeOverride();
    Val* _substituteImplOverride(ASTBuilder* astBuilder, SubstitutionSet subst, int* ioDiff);
};

// A type that takes the form of a reference to some declaration
class DeclRefType : public Type 
{
    SLANG_AST_CLASS(DeclRefType)

    DeclRef<Decl> declRef;

    
    static DeclRefType* create(ASTBuilder* astBuilder, DeclRef<Decl> declRef);

    // Overrides should be public so base classes can access
    void _toTextOverride(StringBuilder& out);
    Type* _createCanonicalTypeOverride();
    bool _equalsImplOverride(Type* type);
    HashCode _getHashCodeOverride();
    Val* _substituteImplOverride(ASTBuilder* astBuilder, SubstitutionSet subst, int* ioDiff);

protected:
    DeclRefType( DeclRef<Decl> declRef)
        : declRef(declRef)
    {}
};

// Base class for types that can be used in arithmetic expressions
class ArithmeticExpressionType : public DeclRefType 
{
    SLANG_ABSTRACT_AST_CLASS(ArithmeticExpressionType)

    BasicExpressionType* getScalarType();

    // Overrides should be public so base classes can access
    BasicExpressionType* _getScalarTypeOverride();
};

class BasicExpressionType : public ArithmeticExpressionType 
{
    SLANG_AST_CLASS(BasicExpressionType)

    BaseType baseType;

    // Overrides should be public so base classes can access
    Type* _createCanonicalTypeOverride();
    bool _equalsImplOverride(Type* type);
    BasicExpressionType* _getScalarTypeOverride();

protected:
    BasicExpressionType(
        Slang::BaseType baseType)
        : baseType(baseType)
    {}
};

// Base type for things that are built in to the compiler,
// and will usually have special behavior or a custom
// mapping to the IR level.
class BuiltinType : public DeclRefType 
{
    SLANG_ABSTRACT_AST_CLASS(BuiltinType)
};

class FeedbackType : public BuiltinType
{
    SLANG_AST_CLASS(FeedbackType)

    enum class Kind : uint8_t
    {
        MinMip,                 /// SAMPLER_FEEDBACK_MIN_MIP
        MipRegionUsed,          /// SAMPLER_FEEDBACK_MIP_REGION_USED
    };

    Kind kind;
};

// Resources that contain "elements" that can be fetched
class ResourceType : public BuiltinType 
{
    SLANG_ABSTRACT_AST_CLASS(ResourceType)

    // The type that results from fetching an element from this resource
    Type* elementType = nullptr;

    // Shape and access level information for this resource type
    TextureFlavor flavor;

    TextureFlavor::Shape getBaseShape()
    {
        return flavor.getBaseShape();
    }
    bool isMultisample() { return flavor.isMultisample(); }
    bool isArray() { return flavor.isArray(); }
    SlangResourceShape getShape() const { return flavor.getShape(); }
    SlangResourceAccess getAccess() { return flavor.getAccess(); }
};

class TextureTypeBase : public ResourceType 
{
    SLANG_ABSTRACT_AST_CLASS(TextureTypeBase)

protected:
    TextureTypeBase(TextureFlavor inFlavor, Type* inElementType)
    {
        elementType = inElementType;
        flavor = inFlavor;
    }
};



class TextureType : public TextureTypeBase 
{
    SLANG_AST_CLASS(TextureType)

protected:
    TextureType(TextureFlavor flavor, Type* elementType)
        : TextureTypeBase(flavor, elementType)
    {}
};


// This is a base type for texture/sampler pairs,
// as they exist in, e.g., GLSL
class TextureSamplerType : public TextureTypeBase 
{
    SLANG_AST_CLASS(TextureSamplerType)

protected:
    TextureSamplerType(TextureFlavor flavor, Type* elementType)
        : TextureTypeBase(flavor, elementType)
    {}
};

// This is a base type for `image*` types, as they exist in GLSL
class GLSLImageType : public TextureTypeBase 
{
    SLANG_AST_CLASS(GLSLImageType)

protected:
    GLSLImageType(
        TextureFlavor flavor,
        Type* elementType)
        : TextureTypeBase(flavor, elementType)
    {}
};

class SamplerStateType : public BuiltinType 
{
    SLANG_AST_CLASS(SamplerStateType)

    // What flavor of sampler state is this
    SamplerStateFlavor flavor;
};

// Other cases of generic types known to the compiler
class BuiltinGenericType : public BuiltinType 
{
    SLANG_AST_CLASS(BuiltinGenericType)

    Type* elementType = nullptr;

    Type* getElementType() { return elementType; }
};

// Types that behave like pointers, in that they can be
// dereferenced (implicitly) to access members defined
// in the element type.
class PointerLikeType : public BuiltinGenericType 
{
    SLANG_AST_CLASS(PointerLikeType)
};


// HLSL buffer-type resources

class HLSLStructuredBufferTypeBase : public BuiltinGenericType 
{
    SLANG_AST_CLASS(HLSLStructuredBufferTypeBase)
};

class HLSLStructuredBufferType : public HLSLStructuredBufferTypeBase 
{
    SLANG_AST_CLASS(HLSLStructuredBufferType)
};

class HLSLRWStructuredBufferType : public HLSLStructuredBufferTypeBase 
{
    SLANG_AST_CLASS(HLSLRWStructuredBufferType)
};

class HLSLRasterizerOrderedStructuredBufferType : public HLSLStructuredBufferTypeBase 
{
    SLANG_AST_CLASS(HLSLRasterizerOrderedStructuredBufferType)
};


class UntypedBufferResourceType : public BuiltinType 
{
    SLANG_AST_CLASS(UntypedBufferResourceType)
};

class HLSLByteAddressBufferType : public UntypedBufferResourceType 
{
    SLANG_AST_CLASS(HLSLByteAddressBufferType)
};

class HLSLRWByteAddressBufferType : public UntypedBufferResourceType 
{
    SLANG_AST_CLASS(HLSLRWByteAddressBufferType)
};

class HLSLRasterizerOrderedByteAddressBufferType : public UntypedBufferResourceType 
{
    SLANG_AST_CLASS(HLSLRasterizerOrderedByteAddressBufferType)
};

class RaytracingAccelerationStructureType : public UntypedBufferResourceType 
{
    SLANG_AST_CLASS(RaytracingAccelerationStructureType)
};


class HLSLAppendStructuredBufferType : public HLSLStructuredBufferTypeBase 
{
    SLANG_AST_CLASS(HLSLAppendStructuredBufferType)
};

class HLSLConsumeStructuredBufferType : public HLSLStructuredBufferTypeBase 
{
    SLANG_AST_CLASS(HLSLConsumeStructuredBufferType)
};


class HLSLPatchType : public BuiltinType 
{
    SLANG_AST_CLASS(HLSLPatchType)

    Type* getElementType();
    IntVal* getElementCount();
};

class HLSLInputPatchType : public HLSLPatchType 
{
    SLANG_AST_CLASS(HLSLInputPatchType)
};

class HLSLOutputPatchType : public HLSLPatchType 
{
    SLANG_AST_CLASS(HLSLOutputPatchType)
};


// HLSL geometry shader output stream types

class HLSLStreamOutputType : public BuiltinGenericType 
{
    SLANG_AST_CLASS(HLSLStreamOutputType)
};

class HLSLPointStreamType : public HLSLStreamOutputType 
{
    SLANG_AST_CLASS(HLSLPointStreamType)
};

class HLSLLineStreamType : public HLSLStreamOutputType 
{
    SLANG_AST_CLASS(HLSLLineStreamType)
};

class HLSLTriangleStreamType : public HLSLStreamOutputType 
{
    SLANG_AST_CLASS(HLSLTriangleStreamType)
};


//
class GLSLInputAttachmentType : public BuiltinType 
{
    SLANG_AST_CLASS(GLSLInputAttachmentType)
};


// Base class for types used when desugaring parameter block
// declarations, includeing HLSL `cbuffer` or GLSL `uniform` blocks.
class ParameterGroupType : public PointerLikeType 
{
    SLANG_AST_CLASS(ParameterGroupType)
};

class UniformParameterGroupType : public ParameterGroupType 
{
    SLANG_AST_CLASS(UniformParameterGroupType)
};

class VaryingParameterGroupType : public ParameterGroupType 
{
    SLANG_AST_CLASS(VaryingParameterGroupType)
};


// type for HLSL `cbuffer` declarations, and `ConstantBuffer<T>`
// ALso used for GLSL `uniform` blocks.
class ConstantBufferType : public UniformParameterGroupType 
{
    SLANG_AST_CLASS(ConstantBufferType)
};


// type for HLSL `tbuffer` declarations, and `TextureBuffer<T>`
class TextureBufferType : public UniformParameterGroupType 
{
    SLANG_AST_CLASS(TextureBufferType)
};


// type for GLSL `in` and `out` blocks
class GLSLInputParameterGroupType : public VaryingParameterGroupType 
{
    SLANG_AST_CLASS(GLSLInputParameterGroupType)
};

class GLSLOutputParameterGroupType : public VaryingParameterGroupType 
{
    SLANG_AST_CLASS(GLSLOutputParameterGroupType)
};


// type for GLLSL `buffer` blocks
class GLSLShaderStorageBufferType : public UniformParameterGroupType 
{
    SLANG_AST_CLASS(GLSLShaderStorageBufferType)
};


// type for Slang `ParameterBlock<T>` type
class ParameterBlockType : public UniformParameterGroupType 
{
    SLANG_AST_CLASS(ParameterBlockType)
};

class ArrayExpressionType : public Type 
{
    SLANG_AST_CLASS(ArrayExpressionType)

    Type* baseType = nullptr;
    IntVal* arrayLength = nullptr;

    // Overrides should be public so base classes can access
    void _toTextOverride(StringBuilder& out);
    Type* _createCanonicalTypeOverride();
    bool _equalsImplOverride(Type* type);
    Val* _substituteImplOverride(ASTBuilder* astBuilder, SubstitutionSet subst, int* ioDiff);
    HashCode _getHashCodeOverride();
};

// The "type" of an expression that resolves to a type.
// For example, in the expression `float(2)` the sub-expression,
// `float` would have the type `TypeType(float)`.
class TypeType : public Type 
{
    SLANG_AST_CLASS(TypeType)

    // The type that this is the type of...
    Type* type = nullptr;

    // Overrides should be public so base classes can access
    void _toTextOverride(StringBuilder& out);
    Type* _createCanonicalTypeOverride();
    bool _equalsImplOverride(Type* type);
    HashCode _getHashCodeOverride();

protected:
    TypeType(Type* type)
        : type(type)
    {}

    
};

// A differential pair type, e.g., `__DifferentialPair<T>`
class DifferentialPairType : public ArithmeticExpressionType 
{
    SLANG_AST_CLASS(DifferentialPairType)

    // The type of vector elements.
    // As an invariant, this should be a basic type or an alias.
    Type* baseType = nullptr;
};

class DifferentiableType : public BuiltinType
{
    SLANG_AST_CLASS(DifferentiableType)
};

// A vector type, e.g., `vector<T,N>`
class VectorExpressionType : public ArithmeticExpressionType 
{
    SLANG_AST_CLASS(VectorExpressionType)

    // The type of vector elements.
    // As an invariant, this should be a basic type or an alias.
    Type* elementType = nullptr;

    // The number of elements
    IntVal* elementCount = nullptr;

    // Overrides should be public so base classes can access
    void _toTextOverride(StringBuilder& out);
    BasicExpressionType* _getScalarTypeOverride();
};

// A matrix type, e.g., `matrix<T,R,C>`
class MatrixExpressionType : public ArithmeticExpressionType 
{
    SLANG_AST_CLASS(MatrixExpressionType)

    Type*           getElementType();
    IntVal*         getRowCount();
    IntVal*         getColumnCount();

    Type* getRowType();

    // Overrides should be public so base classes can access
    void _toTextOverride(StringBuilder& out);
    BasicExpressionType* _getScalarTypeOverride();

private:
    Type* rowType = nullptr;
};

// Base class for built in string types
class StringTypeBase : public BuiltinType
{
    SLANG_AST_CLASS(StringTypeBase)
};

// The regular built-in `String` type
class StringType : public StringTypeBase
{
    SLANG_AST_CLASS(StringType)
};

// The string type native to the target
class NativeStringType : public StringTypeBase
{
    SLANG_AST_CLASS(NativeStringType)
};

// The built-in `__Dynamic` type
class DynamicType : public BuiltinType
{
    SLANG_AST_CLASS(DynamicType)
};

// Type built-in `__EnumType` type
class EnumTypeType : public BuiltinType 
{
    SLANG_AST_CLASS(EnumTypeType)
    // TODO: provide accessors for the declaration, the "tag" type, etc.
};

// Base class for types that map down to
// simple pointers as part of code generation.
class PtrTypeBase : public BuiltinType 
{
    SLANG_AST_CLASS(PtrTypeBase)

    // Get the type of the pointed-to value.
    Type* getValueType();
};

class NoneType : public BuiltinType
{
    SLANG_AST_CLASS(NoneType)
};

class NullPtrType : public BuiltinType
{
    SLANG_AST_CLASS(NullPtrType)
};

// A true (user-visible) pointer type, e.g., `T*`
class PtrType : public PtrTypeBase 
{
    SLANG_AST_CLASS(PtrType)
};

/// A pointer-like type used to represent a parameter "direction"
class ParamDirectionType : public PtrTypeBase
{
    SLANG_AST_CLASS(ParamDirectionType)
};

// A type that represents the behind-the-scenes
// logical pointer that is passed for an `out`
// or `in out` parameter
class OutTypeBase : public ParamDirectionType
{
    SLANG_AST_CLASS(OutTypeBase)
};

// The type for an `out` parameter, e.g., `out T`
class OutType : public OutTypeBase 
{
    SLANG_AST_CLASS(OutType)
};

// The type for an `in out` parameter, e.g., `in out T`
class InOutType : public OutTypeBase 
{
    SLANG_AST_CLASS(InOutType)
};

// The type for an `ref` parameter, e.g., `ref T`
class RefType : public ParamDirectionType
{
    SLANG_AST_CLASS(RefType)
};

class OptionalType : public BuiltinType
{
    SLANG_AST_CLASS(OptionalType)
    Type* getValueType();
};

// A raw-pointer reference to an managed value.
class NativeRefType : public BuiltinType
{
    SLANG_AST_CLASS(NativeRefType)
    Type* getValueType();
};

// A type alias of some kind (e.g., via `typedef`)
class NamedExpressionType : public Type 
{
    SLANG_AST_CLASS(NamedExpressionType)

    DeclRef<TypeDefDecl> declRef;
    Type* innerType = nullptr;

    // Overrides should be public so base classes can access
    void _toTextOverride(StringBuilder& out);
    Type* _createCanonicalTypeOverride();
    bool _equalsImplOverride(Type* type);
    HashCode _getHashCodeOverride();

protected:
    NamedExpressionType(
        DeclRef<TypeDefDecl> declRef)
        : declRef(declRef)
    {}


};

// A function type is defined by its parameter types
// and its result type.
class FuncType : public Type 
{
    SLANG_AST_CLASS(FuncType)

    // TODO: We may want to preserve parameter names
    // in the list here, just so that we can print
    // out friendly names when printing a function
    // type, even if they don't affect the actual
    // semantic type underneath.

    List<Type*> paramTypes;
    Type* resultType = nullptr;
    Type* errorType = nullptr;

    UInt getParamCount() { return paramTypes.getCount(); }
    Type* getParamType(UInt index) { return paramTypes[index]; }
    Type* getResultType() { return resultType; }
    Type* getErrorType() { return errorType; }

    ParameterDirection getParamDirection(Index index);

    // Overrides should be public so base classes can access
    void _toTextOverride(StringBuilder& out);
    Type* _createCanonicalTypeOverride();
    Val* _substituteImplOverride(ASTBuilder* astBuilder, SubstitutionSet subst, int* ioDiff);
    bool _equalsImplOverride(Type* type);
    HashCode _getHashCodeOverride();
};

// The "type" of an expression that names a generic declaration.
class GenericDeclRefType : public Type 
{
    SLANG_AST_CLASS(GenericDeclRefType)

    DeclRef<GenericDecl> declRef;

    DeclRef<GenericDecl> const& getDeclRef() const { return declRef; }

    // Overrides should be public so base classes can access
    void _toTextOverride(StringBuilder& out);
    bool _equalsImplOverride(Type* type);
    HashCode _getHashCodeOverride();
    Type* _createCanonicalTypeOverride();

protected:
    GenericDeclRefType(
        DeclRef<GenericDecl> declRef)
        : declRef(declRef)
    {}
};

// The "type" of a reference to a module or namespace
class NamespaceType : public Type 
{
    SLANG_AST_CLASS(NamespaceType)

    DeclRef<NamespaceDeclBase> declRef;

    DeclRef<NamespaceDeclBase> const& getDeclRef() const { return declRef; }

    // Overrides should be public so base classes can access
    void _toTextOverride(StringBuilder& out);
    bool _equalsImplOverride(Type* type);
    HashCode _getHashCodeOverride();
    Type* _createCanonicalTypeOverride();    
};

// The concrete type for a value wrapped in an existential, accessible
// when the existential is "opened" in some context.
class ExtractExistentialType : public Type 
{
    SLANG_AST_CLASS(ExtractExistentialType)

    DeclRef<VarDeclBase> declRef;

    // A reference to the original interface this type is known
    // to be a subtype of.
    //
    Type* originalInterfaceType;
    DeclRef<InterfaceDecl> originalInterfaceDeclRef;

// Following fields will not be reflected (and thus won't be serialized, etc.)
SLANG_UNREFLECTED

    // A cached decl-ref to the original interface above, with
    // a this-type substitution that refers to the type extracted here.
    //
    // This field is optional and can be filled in on-demand. It does *not*
    // represent part of the logical value of this `Type`, and should not
    // be serialized, included in hashes, etc.
    //
    DeclRef<InterfaceDecl> cachedSpecializedInterfaceDeclRef;

    // A cached pointer to a witness that shows how this type is a subtype
    // of `originalInterfaceType`.
    //
    SubtypeWitness* cachedSubtypeWitness = nullptr;

    // Overrides should be public so base classes can access
    void _toTextOverride(StringBuilder& out);
    bool _equalsImplOverride(Type* type);
    HashCode _getHashCodeOverride();
    Type* _createCanonicalTypeOverride();
    Val* _substituteImplOverride(ASTBuilder* astBuilder, SubstitutionSet subst, int* ioDiff);

        /// Get a witness that shows how this type is a subtype of `originalInterfaceType`.
        ///
        /// This operation may create the witness on demand and cache it.
        ///
    SubtypeWitness* getSubtypeWitness();

        /// Get a interface decl-ref for the original interface specialized to this type
        /// (using a type-type substitution).
        ///
        /// This operation may create the decl-ref on demand and cache it.
        ///
    DeclRef<InterfaceDecl> getSpecializedInterfaceDeclRef();
};

    /// A tagged union of zero or more other types.
class TaggedUnionType : public Type 
{
    SLANG_AST_CLASS(TaggedUnionType)

        /// The distinct "cases" the tagged union can store.
        ///
        /// For each type in this array, the array index is the
        /// tag value for that case.
        ///
    List<Type*> caseTypes;

    // Overrides should be public so base classes can access
    void _toTextOverride(StringBuilder& out);
    bool _equalsImplOverride(Type* type);
    HashCode _getHashCodeOverride();
    Type* _createCanonicalTypeOverride();
    Val* _substituteImplOverride(ASTBuilder* astBuilder, SubstitutionSet subst, int* ioDiff);
};

class ExistentialSpecializedType : public Type 
{
    SLANG_AST_CLASS(ExistentialSpecializedType)

    Type* baseType = nullptr;
    ExpandedSpecializationArgs args;

    // Overrides should be public so base classes can access
    void _toTextOverride(StringBuilder& out);
    bool _equalsImplOverride(Type* type);
    HashCode _getHashCodeOverride();
    Type* _createCanonicalTypeOverride();
    Val* _substituteImplOverride(ASTBuilder* astBuilder, SubstitutionSet subst, int* ioDiff);
};

    /// The type of `this` within a polymorphic declaration
class ThisType : public Type 
{
    SLANG_AST_CLASS(ThisType)

    DeclRef<InterfaceDecl> interfaceDeclRef;

    // Overrides should be public so base classes can access
    void _toTextOverride(StringBuilder& out);
    bool _equalsImplOverride(Type* type);
    HashCode _getHashCodeOverride();
    Type* _createCanonicalTypeOverride();
    Val* _substituteImplOverride(ASTBuilder* astBuilder, SubstitutionSet subst, int* ioDiff);
};

    /// The type of `A & B` where `A` and `B` are types
    ///
    /// A value `v` is of type `A & B` if it is both of type `A` and of type `B`.
class AndType : public Type
{
    SLANG_AST_CLASS(AndType)

    Type* left;
    Type* right;

    // Overrides should be public so base classes can access
    void _toTextOverride(StringBuilder& out);
    bool _equalsImplOverride(Type* type);
    HashCode _getHashCodeOverride();
    Type* _createCanonicalTypeOverride();
    Val* _substituteImplOverride(ASTBuilder* astBuilder, SubstitutionSet subst, int* ioDiff);
};

class ModifiedType : public Type
{
    SLANG_AST_CLASS(ModifiedType)

    Type* base;
    List<Val*> modifiers;

    // Overrides should be public so base classes can access
    void _toTextOverride(StringBuilder& out);
    bool _equalsImplOverride(Type* type);
    HashCode _getHashCodeOverride();
    Type* _createCanonicalTypeOverride();
    Val* _substituteImplOverride(ASTBuilder* astBuilder, SubstitutionSet subst, int* ioDiff);
};

} // namespace Slang
