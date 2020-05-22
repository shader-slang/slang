// slang-ast-type.h

#pragma once

#include "slang-ast-base.h"

namespace Slang {

// Syntax class definitions for types.

// The type of a reference to an overloaded name
class OverloadGroupType : public Type 
{
    SLANG_CLASS(OverloadGroupType)

public:
    virtual String toString() override;

protected:
    virtual bool equalsImpl(Type * type) override;
    virtual RefPtr<Type> createCanonicalType() override;
    virtual HashCode getHashCode() override;
};

// The type of an initializer-list expression (before it has
// been coerced to some other type)
class InitializerListType : public Type 
{
    SLANG_CLASS(InitializerListType)

    virtual String toString() override;

protected:
    virtual bool equalsImpl(Type * type) override;
    virtual RefPtr<Type> createCanonicalType() override;
    virtual HashCode getHashCode() override;
};

// The type of an expression that was erroneous
class ErrorType : public Type 
{
    SLANG_CLASS(ErrorType)

public:
    virtual String toString() override;

protected:
    virtual bool equalsImpl(Type * type) override;
    virtual RefPtr<Val> substituteImpl(SubstitutionSet subst, int* ioDiff) override;
    virtual RefPtr<Type> createCanonicalType() override;
    virtual HashCode getHashCode() override;
};

// A type that takes the form of a reference to some declaration
class DeclRefType : public Type 
{
    SLANG_CLASS(DeclRefType)

    DeclRef<Decl> declRef;

    virtual String toString() override;
    virtual RefPtr<Val> substituteImpl(SubstitutionSet subst, int* ioDiff) override;

    static RefPtr<DeclRefType> Create(
        Session*        session,
        DeclRef<Decl>   declRef);

    DeclRefType()
    {}
    DeclRefType(
        DeclRef<Decl> declRef)
        : declRef(declRef)
    {}
protected:
    virtual HashCode getHashCode() override;
    virtual bool equalsImpl(Type * type) override;
    virtual RefPtr<Type> createCanonicalType() override;
};

// Base class for types that can be used in arithmetic expressions
class ArithmeticExpressionType : public DeclRefType 
{
    SLANG_ABSTRACT_CLASS(ArithmeticExpressionType)

public:
    virtual BasicExpressionType* GetScalarType() = 0;
};

class BasicExpressionType : public ArithmeticExpressionType 
{
    SLANG_CLASS(BasicExpressionType)


    BaseType baseType;

    BasicExpressionType() {}
    BasicExpressionType(
        Slang::BaseType baseType)
        : baseType(baseType)
    {}
protected:
    virtual BasicExpressionType* GetScalarType() override;
    virtual bool equalsImpl(Type * type) override;
    virtual RefPtr<Type> createCanonicalType() override;

};

// Base type for things that are built in to the compiler,
// and will usually have special behavior or a custom
// mapping to the IR level.
class BuiltinType : public DeclRefType 
{
    SLANG_ABSTRACT_CLASS(BuiltinType)

};

// Resources that contain "elements" that can be fetched
class ResourceType : public BuiltinType 
{
    SLANG_ABSTRACT_CLASS(ResourceType)

    // The type that results from fetching an element from this resource
    RefPtr<Type> elementType;

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
    SLANG_ABSTRACT_CLASS(TextureTypeBase)

    TextureTypeBase()
    {}
    TextureTypeBase(
        TextureFlavor flavor,
        RefPtr<Type> elementType)
    {
        this->elementType = elementType;
        this->flavor = flavor;
    }
};

class TextureType : public TextureTypeBase 
{
    SLANG_CLASS(TextureType)

    TextureType()
    {}
    TextureType(
        TextureFlavor flavor,
        RefPtr<Type> elementType)
        : TextureTypeBase(flavor, elementType)
    {}
};

// This is a base type for texture/sampler pairs,
// as they exist in, e.g., GLSL
class TextureSamplerType : public TextureTypeBase 
{
    SLANG_CLASS(TextureSamplerType)

    TextureSamplerType()
    {}
    TextureSamplerType(
        TextureFlavor flavor,
        RefPtr<Type> elementType)
        : TextureTypeBase(flavor, elementType)
    {}
};

// This is a base type for `image*` types, as they exist in GLSL
class GLSLImageType : public TextureTypeBase 
{
    SLANG_CLASS(GLSLImageType)

    GLSLImageType()
    {}
    GLSLImageType(
        TextureFlavor flavor,
        RefPtr<Type> elementType)
        : TextureTypeBase(flavor, elementType)
    {}
};

class SamplerStateType : public BuiltinType 
{
    SLANG_CLASS(SamplerStateType)

    // What flavor of sampler state is this
    SamplerStateFlavor flavor;
};

// Other cases of generic types known to the compiler
class BuiltinGenericType : public BuiltinType 
{
    SLANG_CLASS(BuiltinGenericType)

    RefPtr<Type> elementType;

    Type* getElementType() { return elementType; }
};

// Types that behave like pointers, in that they can be
// dereferenced (implicitly) to access members defined
// in the element type.
class PointerLikeType : public BuiltinGenericType 
{
    SLANG_CLASS(PointerLikeType)
};


// HLSL buffer-type resources

class HLSLStructuredBufferTypeBase : public BuiltinGenericType 
{
    SLANG_CLASS(HLSLStructuredBufferTypeBase)
};

class HLSLStructuredBufferType : public HLSLStructuredBufferTypeBase 
{
    SLANG_CLASS(HLSLStructuredBufferType)
};

class HLSLRWStructuredBufferType : public HLSLStructuredBufferTypeBase 
{
    SLANG_CLASS(HLSLRWStructuredBufferType)
};

class HLSLRasterizerOrderedStructuredBufferType : public HLSLStructuredBufferTypeBase 
{
    SLANG_CLASS(HLSLRasterizerOrderedStructuredBufferType)
};


class UntypedBufferResourceType : public BuiltinType 
{
    SLANG_CLASS(UntypedBufferResourceType)
};

class HLSLByteAddressBufferType : public UntypedBufferResourceType 
{
    SLANG_CLASS(HLSLByteAddressBufferType)
};

class HLSLRWByteAddressBufferType : public UntypedBufferResourceType 
{
    SLANG_CLASS(HLSLRWByteAddressBufferType)
};

class HLSLRasterizerOrderedByteAddressBufferType : public UntypedBufferResourceType 
{
    SLANG_CLASS(HLSLRasterizerOrderedByteAddressBufferType)
};

class RaytracingAccelerationStructureType : public UntypedBufferResourceType 
{
    SLANG_CLASS(RaytracingAccelerationStructureType)
};


class HLSLAppendStructuredBufferType : public HLSLStructuredBufferTypeBase 
{
    SLANG_CLASS(HLSLAppendStructuredBufferType)
};

class HLSLConsumeStructuredBufferType : public HLSLStructuredBufferTypeBase 
{
    SLANG_CLASS(HLSLConsumeStructuredBufferType)
};


class HLSLPatchType : public BuiltinType 
{
    SLANG_CLASS(HLSLPatchType)

    Type* getElementType();
    IntVal* getElementCount();
};

class HLSLInputPatchType : public HLSLPatchType 
{
    SLANG_CLASS(HLSLInputPatchType)
};

class HLSLOutputPatchType : public HLSLPatchType 
{
    SLANG_CLASS(HLSLOutputPatchType)
};


// HLSL geometry shader output stream types

class HLSLStreamOutputType : public BuiltinGenericType 
{
    SLANG_CLASS(HLSLStreamOutputType)
};

class HLSLPointStreamType : public HLSLStreamOutputType 
{
    SLANG_CLASS(HLSLPointStreamType)
};

class HLSLLineStreamType : public HLSLStreamOutputType 
{
    SLANG_CLASS(HLSLLineStreamType)
};

class HLSLTriangleStreamType : public HLSLStreamOutputType 
{
    SLANG_CLASS(HLSLTriangleStreamType)
};


//
class GLSLInputAttachmentType : public BuiltinType 
{
    SLANG_CLASS(GLSLInputAttachmentType)
};


// Base class for types used when desugaring parameter block
// declarations, includeing HLSL `cbuffer` or GLSL `uniform` blocks.
class ParameterGroupType : public PointerLikeType 
{
    SLANG_CLASS(ParameterGroupType)
};

class UniformParameterGroupType : public ParameterGroupType 
{
    SLANG_CLASS(UniformParameterGroupType)
};

class VaryingParameterGroupType : public ParameterGroupType 
{
    SLANG_CLASS(VaryingParameterGroupType)
};


// type for HLSL `cbuffer` declarations, and `ConstantBuffer<T>`
// ALso used for GLSL `uniform` blocks.
class ConstantBufferType : public UniformParameterGroupType 
{
    SLANG_CLASS(ConstantBufferType)
};


// type for HLSL `tbuffer` declarations, and `TextureBuffer<T>`
class TextureBufferType : public UniformParameterGroupType 
{
    SLANG_CLASS(TextureBufferType)
};


// type for GLSL `in` and `out` blocks
class GLSLInputParameterGroupType : public VaryingParameterGroupType 
{
    SLANG_CLASS(GLSLInputParameterGroupType)
};

class GLSLOutputParameterGroupType : public VaryingParameterGroupType 
{
    SLANG_CLASS(GLSLOutputParameterGroupType)
};


// type for GLLSL `buffer` blocks
class GLSLShaderStorageBufferType : public UniformParameterGroupType 
{
    SLANG_CLASS(GLSLShaderStorageBufferType)
};


// type for Slang `ParameterBlock<T>` type
class ParameterBlockType : public UniformParameterGroupType 
{
    SLANG_CLASS(ParameterBlockType)
};

class ArrayExpressionType : public Type 
{
    SLANG_CLASS(ArrayExpressionType)

    RefPtr<Type> baseType;
    RefPtr<IntVal> arrayLength;

    virtual String toString() override;

protected:
    virtual bool equalsImpl(Type * type) override;
    virtual RefPtr<Type> createCanonicalType() override;
    virtual RefPtr<Val> substituteImpl(SubstitutionSet subst, int* ioDiff) override;
    virtual HashCode getHashCode() override;
};

// The "type" of an expression that resolves to a type.
// For example, in the expression `float(2)` the sub-expression,
// `float` would have the type `TypeType(float)`.
class TypeType : public Type 
{
    SLANG_CLASS(TypeType)

    // The type that this is the type of...
    RefPtr<Type> type;

public:
    TypeType()
    {}
    TypeType(RefPtr<Type> type)
        : type(type)
    {}

    virtual String toString() override;

protected:
    virtual bool equalsImpl(Type * type) override;
    virtual RefPtr<Type> createCanonicalType() override;
    virtual HashCode getHashCode() override;
};

// A vector type, e.g., `vector<T,N>`
class VectorExpressionType : public ArithmeticExpressionType 
{
    SLANG_CLASS(VectorExpressionType)

    // The type of vector elements.
    // As an invariant, this should be a basic type or an alias.
    RefPtr<Type> elementType;

    // The number of elements
    RefPtr<IntVal> elementCount;

    virtual String toString() override;

protected:
    virtual BasicExpressionType* GetScalarType() override;
};

// A matrix type, e.g., `matrix<T,R,C>`
class MatrixExpressionType : public ArithmeticExpressionType 
{
    SLANG_CLASS(MatrixExpressionType)

    Type*           getElementType();
    IntVal*         getRowCount();
    IntVal*         getColumnCount();

    RefPtr<Type> getRowType();

    virtual String toString() override;

protected:
    virtual BasicExpressionType* GetScalarType() override;

private:
    RefPtr<Type> rowType;
};

// The built-in `String` type
class StringType : public BuiltinType 
{
    SLANG_CLASS(StringType)
};


// Type built-in `__EnumType` type
class EnumTypeType : public BuiltinType 
{
    SLANG_CLASS(EnumTypeType)
    // TODO: provide accessors for the declaration, the "tag" type, etc.
};

// Base class for types that map down to
// simple pointers as part of code generation.
class PtrTypeBase : public BuiltinType 
{
    SLANG_CLASS(PtrTypeBase)

    // Get the type of the pointed-to value.
    Type*   getValueType();
};

// A true (user-visible) pointer type, e.g., `T*`
class PtrType : public PtrTypeBase 
{
    SLANG_CLASS(PtrType)
};

// A type that represents the behind-the-scenes
// logical pointer that is passed for an `out`
// or `in out` parameter
class OutTypeBase : public PtrTypeBase 
{
    SLANG_CLASS(OutTypeBase)
};

// The type for an `out` parameter, e.g., `out T`
class OutType : public OutTypeBase 
{
    SLANG_CLASS(OutType)
};

// The type for an `in out` parameter, e.g., `in out T`
class InOutType : public OutTypeBase 
{
    SLANG_CLASS(InOutType)
};

// The type for an `ref` parameter, e.g., `ref T`
class RefType : public PtrTypeBase 
{
    SLANG_CLASS(RefType)
};

// A type alias of some kind (e.g., via `typedef`)
class NamedExpressionType : public Type 
{
    SLANG_CLASS(NamedExpressionType)

    DeclRef<TypeDefDecl> declRef;

    RefPtr<Type> innerType;
    NamedExpressionType()
    {}
    NamedExpressionType(
        DeclRef<TypeDefDecl> declRef)
        : declRef(declRef)
    {}

    virtual String toString() override;

protected:
    virtual bool equalsImpl(Type * type) override;
    virtual RefPtr<Type> createCanonicalType() override;
    virtual HashCode getHashCode() override;
};

// A function type is defined by its parameter types
// and its result type.
class FuncType : public Type 
{
    SLANG_CLASS(FuncType)

    // TODO: We may want to preserve parameter names
    // in the list here, just so that we can print
    // out friendly names when printing a function
    // type, even if they don't affect the actual
    // semantic type underneath.

    List<RefPtr<Type>> paramTypes;
    RefPtr<Type> resultType;

    FuncType()
    {}

    UInt getParamCount() { return paramTypes.getCount(); }
    Type* getParamType(UInt index) { return paramTypes[index]; }
    Type* getResultType() { return resultType; }

    virtual String toString() override;
protected:
    virtual RefPtr<Val> substituteImpl(SubstitutionSet subst, int* ioDiff) override;
    virtual bool equalsImpl(Type * type) override;
    virtual RefPtr<Type> createCanonicalType() override;
    virtual HashCode getHashCode() override;
};

// The "type" of an expression that names a generic declaration.
class GenericDeclRefType : public Type 
{
    SLANG_CLASS(GenericDeclRefType)

    DeclRef<GenericDecl> declRef;

    GenericDeclRefType()
    {}
    GenericDeclRefType(
        DeclRef<GenericDecl> declRef)
        : declRef(declRef)
    {}

    DeclRef<GenericDecl> const& getDeclRef() const { return declRef; }

    virtual String toString() override;

protected:
    virtual bool equalsImpl(Type * type) override;
    virtual HashCode getHashCode() override;
    virtual RefPtr<Type> createCanonicalType() override;
};

// The "type" of a reference to a module or namespace
class NamespaceType : public Type 
{
    SLANG_CLASS(NamespaceType)

    DeclRef<NamespaceDeclBase> declRef;

    NamespaceType()
    {}

    DeclRef<NamespaceDeclBase> const& getDeclRef() const { return declRef; }

    virtual String toString() override;

protected:
    virtual bool equalsImpl(Type * type) override;
    virtual HashCode getHashCode() override;
    virtual RefPtr<Type> createCanonicalType() override;    
};

// The concrete type for a value wrapped in an existential, accessible
// when the existential is "opened" in some context.
class ExtractExistentialType : public Type 
{
    SLANG_CLASS(ExtractExistentialType)

    DeclRef<VarDeclBase> declRef;

    virtual String toString() override;
    virtual bool equalsImpl(Type * type) override;
    virtual HashCode getHashCode() override;
    virtual RefPtr<Type> createCanonicalType() override;
    virtual RefPtr<Val> substituteImpl(SubstitutionSet subst, int* ioDiff) override;
};

    /// A tagged union of zero or more other types.
class TaggedUnionType : public Type 
{
    SLANG_CLASS(TaggedUnionType)

        /// The distinct "cases" the tagged union can store.
        ///
        /// For each type in this array, the array index is the
        /// tag value for that case.
        ///
    List<RefPtr<Type>> caseTypes;

    virtual String toString() override;
    virtual bool equalsImpl(Type * type) override;
    virtual HashCode getHashCode() override;
    virtual RefPtr<Type> createCanonicalType() override;
    virtual RefPtr<Val> substituteImpl(SubstitutionSet subst, int* ioDiff) override;
};

class ExistentialSpecializedType : public Type 
{
    SLANG_CLASS(ExistentialSpecializedType)

    RefPtr<Type> baseType;
    ExpandedSpecializationArgs args;

    virtual String toString() override;
    virtual bool equalsImpl(Type * type) override;
    virtual HashCode getHashCode() override;
    virtual RefPtr<Type> createCanonicalType() override;
    virtual RefPtr<Val> substituteImpl(SubstitutionSet subst, int* ioDiff) override;
};

    /// The type of `this` within a polymorphic declaration
class ThisType : public Type 
{
    SLANG_CLASS(ThisType)

    DeclRef<InterfaceDecl> interfaceDeclRef;

    virtual String toString() override;
    virtual bool equalsImpl(Type * type) override;
    virtual HashCode getHashCode() override;
    virtual RefPtr<Type> createCanonicalType() override;
    virtual RefPtr<Val> substituteImpl(SubstitutionSet subst, int* ioDiff) override;

};

} // namespace Slang
