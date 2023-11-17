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
};

// The type of an initializer-list expression (before it has
// been coerced to some other type)
class InitializerListType : public Type 
{
    SLANG_AST_CLASS(InitializerListType)

    // Overrides should be public so base classes can access
    void _toTextOverride(StringBuilder& out);
    Type* _createCanonicalTypeOverride();
};

// The type of an expression that was erroneous
class ErrorType : public Type 
{
    SLANG_AST_CLASS(ErrorType)

    // Overrides should be public so base classes can access
    void _toTextOverride(StringBuilder& out);
    Type* _createCanonicalTypeOverride();
    Val* _substituteImplOverride(ASTBuilder* astBuilder, SubstitutionSet subst, int* ioDiff);
};

// The bottom/empty type that has no values.
class BottomType : public Type
{
    SLANG_AST_CLASS(BottomType)

    // Overrides should be public so base classes can access
    void _toTextOverride(StringBuilder& out);
    Val* _substituteImplOverride(ASTBuilder* astBuilder, SubstitutionSet subst, int* ioDiff);
};

// A type that takes the form of a reference to some declaration
class DeclRefType : public Type
{
    SLANG_AST_CLASS(DeclRefType)

    static Type* create(ASTBuilder* astBuilder, DeclRef<Decl> declRef);

    DeclRef<Decl> getDeclRef() const { return DeclRef<Decl>(as<DeclRefBase>(getOperand(0))); }
    DeclRefBase* getDeclRefBase() const { return as<DeclRefBase>(getOperand(0)); }

    // Overrides should be public so base classes can access
    void _toTextOverride(StringBuilder& out);
    Type* _createCanonicalTypeOverride();
    Val* _substituteImplOverride(ASTBuilder* astBuilder, SubstitutionSet subst, int* ioDiff);

    DeclRefType(DeclRefBase* declRefBase)
    {
        setOperands(declRefBase);
    }
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

    BaseType getBaseType() const;

    // Overrides should be public so base classes can access
    BasicExpressionType* _getScalarTypeOverride();

    BasicExpressionType(DeclRefBase* inDeclRef)
    {
        setOperands(inDeclRef);
    }
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

    Kind getKind() const;
};

class TextureShapeType : public BuiltinType
{
    SLANG_ABSTRACT_AST_CLASS(TextureShapeType)
};

class TextureShape1DType : public TextureShapeType
{
    SLANG_AST_CLASS(TextureShape1DType)
};
class TextureShape2DType : public TextureShapeType
{
    SLANG_AST_CLASS(TextureShape2DType)
};
class TextureShape3DType : public TextureShapeType
{
    SLANG_AST_CLASS(TextureShape3DType)
};
class TextureShapeCubeType : public TextureShapeType
{
    SLANG_AST_CLASS(TextureShapeCubeType)
};
class TextureShapeBufferType : public TextureShapeType
{
    SLANG_AST_CLASS(TextureShapeBufferType)
};

// Resources that contain "elements" that can be fetched
class ResourceType : public BuiltinType 
{
    SLANG_ABSTRACT_AST_CLASS(ResourceType)

    bool isMultisample();
    bool isArray();
    bool isShadow();
    bool isFeedback();
    bool isCombined();
    SlangResourceShape getBaseShape();
    SlangResourceShape getShape();
    SlangResourceAccess getAccess();
    Type* getElementType();
    void _toTextOverride(StringBuilder& out);
};

class TextureTypeBase : public ResourceType 
{
    SLANG_ABSTRACT_AST_CLASS(TextureTypeBase)

    Val* getSampleCount();
};

class TextureType : public TextureTypeBase 
{
    SLANG_AST_CLASS(TextureType)
};

// This is a base type for `image*` types, as they exist in GLSL
class GLSLImageType : public TextureTypeBase 
{
    SLANG_AST_CLASS(GLSLImageType)
};

class SamplerStateType : public BuiltinType 
{
    SLANG_AST_CLASS(SamplerStateType)

    // Returns flavor of sampler state of this type.
    SamplerStateFlavor getFlavor() const;
};

// Other cases of generic types known to the compiler
class BuiltinGenericType : public BuiltinType 
{
    SLANG_AST_CLASS(BuiltinGenericType)

    Type* getElementType() const;
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

// mesh shader output types

class MeshOutputType : public BuiltinGenericType
{
    SLANG_AST_CLASS(MeshOutputType)

    Type* getElementType();

    IntVal* getMaxElementCount();
};

class VerticesType : public MeshOutputType
{
    SLANG_AST_CLASS(VerticesType)
};

class IndicesType : public MeshOutputType
{
    SLANG_AST_CLASS(IndicesType)
};

class PrimitivesType : public MeshOutputType
{
    SLANG_AST_CLASS(PrimitivesType)
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

class ArrayExpressionType : public DeclRefType 
{
    SLANG_AST_CLASS(ArrayExpressionType)

    bool isUnsized();
    void _toTextOverride(StringBuilder& out);
    Type* getElementType();
    IntVal* getElementCount();
};

// The "type" of an expression that resolves to a type.
// For example, in the expression `float(2)` the sub-expression,
// `float` would have the type `TypeType(float)`.
class TypeType : public Type 
{
    SLANG_AST_CLASS(TypeType)

    // Overrides should be public so base classes can access
    void _toTextOverride(StringBuilder& out);
    Type* _createCanonicalTypeOverride();

    Type* getType() { return as<Type>(getOperand(0)); }

    TypeType(Type* type)
    {
        setOperands(type);
    }
};

// A differential pair type, e.g., `__DifferentialPair<T>`
class DifferentialPairType : public ArithmeticExpressionType 
{
    SLANG_AST_CLASS(DifferentialPairType)
    Type* getPrimalType();
};

class DifferentiableType : public BuiltinType
{
    SLANG_AST_CLASS(DifferentiableType)
};

// A vector type, e.g., `vector<T,N>`
class VectorExpressionType : public ArithmeticExpressionType 
{
    SLANG_AST_CLASS(VectorExpressionType)

    // Overrides should be public so base classes can access
    void _toTextOverride(StringBuilder& out);
    BasicExpressionType* _getScalarTypeOverride();

    Type* getElementType();
    IntVal* getElementCount();
};

// A matrix type, e.g., `matrix<T,R,C,L>`
class MatrixExpressionType : public ArithmeticExpressionType 
{
    SLANG_AST_CLASS(MatrixExpressionType)

    Type*           getElementType();
    IntVal*         getRowCount();
    IntVal*         getColumnCount();
    IntVal*         getLayout();

    Type* getRowType();

    // Overrides should be public so base classes can access
    void _toTextOverride(StringBuilder& out);
    BasicExpressionType* _getScalarTypeOverride();

private:
    SLANG_UNREFLECTED Type* rowType = nullptr;
};

class TensorViewType : public BuiltinType
{
    SLANG_AST_CLASS(TensorViewType)

    Type* getElementType();
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

// A GPU pointer type that for general readonly memory access.
class ConstBufferPointerType : public PtrTypeBase
{
    SLANG_AST_CLASS(ConstBufferPointerType)
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

class RefTypeBase : public ParamDirectionType
{
    SLANG_AST_CLASS(RefTypeBase)
};

// The type for an `ref` parameter, e.g., `ref T`
class RefType : public RefTypeBase
{
    SLANG_AST_CLASS(RefType)
};

// The type for an `constref` parameter, e.g., `constref T`
class ConstRefType : public RefTypeBase
{
    SLANG_AST_CLASS(ConstRefType)
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

    DeclRef<TypeDefDecl> getDeclRef() { return as<DeclRefBase>(getOperand(0)); }

    // Overrides should be public so base classes can access
    void _toTextOverride(StringBuilder& out);
    Type* _createCanonicalTypeOverride();

    NamedExpressionType(DeclRef<TypeDefDecl> inDeclRef)
    {
        setOperands(inDeclRef);
    }
};

// A function type is defined by its parameter types
// and its result type.
class FuncType : public Type 
{
    SLANG_AST_CLASS(FuncType)

    // Construct a unary function
    FuncType(Type* paramType, Type* resultType, Type* errorType)
    {
        setOperands(paramType, resultType, errorType);
    }

    FuncType(ArrayView<Type*> parameters, Type* result, Type* error)
    {
        for (auto paramType : parameters)
            m_operands.add(ValNodeOperand(paramType));
        m_operands.add(ValNodeOperand(result));
        m_operands.add(ValNodeOperand(error));
    }

    OperandView<Type> getParamTypes() { return OperandView<Type>(this, 0, getOperandCount() - 2); }

    Index getParamCount() { return m_operands.getCount() - 2; }
    Type* getParamType(Index index) { return as<Type>(getOperand(index)); }
    Type* getResultType() { return as<Type>(getOperand(m_operands.getCount() - 2)); }
    Type* getErrorType() { return as<Type>(getOperand(m_operands.getCount() - 1)); }

    ParameterDirection getParamDirection(Index index);

    // Overrides should be public so base classes can access
    void _toTextOverride(StringBuilder& out);
    Type* _createCanonicalTypeOverride();
    Val* _substituteImplOverride(ASTBuilder* astBuilder, SubstitutionSet subst, int* ioDiff);
};

// A tuple is a product of its member types
class TupleType : public Type
{
    SLANG_AST_CLASS(TupleType)

    // Construct a unary tupletion
    TupleType(ArrayView<Type*> memberTypes)
    {
        for (auto t : memberTypes)
            m_operands.add(ValNodeOperand(t));
    }

    auto getMemberCount() const { return getOperandCount(); }
    Type* getMember(Index i) const { return as<Type>(getOperand(i)); }

    // Overrides should be public so base classes can access
    void _toTextOverride(StringBuilder& out);
    Type* _createCanonicalTypeOverride();
    Val* _substituteImplOverride(ASTBuilder* astBuilder, SubstitutionSet subst, int* ioDiff);
};

// The "type" of an expression that names a generic declaration.
class GenericDeclRefType : public Type 
{
    SLANG_AST_CLASS(GenericDeclRefType)

    DeclRef<GenericDecl> getDeclRef() const { return as<DeclRefBase>(getOperand(0)); }

    // Overrides should be public so base classes can access
    void _toTextOverride(StringBuilder& out);
    Type* _createCanonicalTypeOverride();

    GenericDeclRefType(DeclRef<GenericDecl> declRef)
    {
        setOperands(declRef);
    }
};

// The "type" of a reference to a module or namespace
class NamespaceType : public Type 
{
    SLANG_AST_CLASS(NamespaceType)

    DeclRef<NamespaceDeclBase> getDeclRef() const { return as<DeclRefBase>(getOperand(0)); }

    NamespaceType(DeclRef<NamespaceDeclBase> inDeclRef)
    {
        setOperands(inDeclRef);
    }

    // Overrides should be public so base classes can access
    void _toTextOverride(StringBuilder& out);
    Type* _createCanonicalTypeOverride();
};

// The concrete type for a value wrapped in an existential, accessible
// when the existential is "opened" in some context.
class ExtractExistentialType : public Type 
{
    SLANG_AST_CLASS(ExtractExistentialType)

    DeclRef<VarDeclBase> getDeclRef() const { return as<DeclRefBase>(getOperand(0)); }

    // A reference to the original interface this type is known
    // to be a subtype of.
    //
    Type* getOriginalInterfaceType() { return as<Type>(getOperand(1)); }
    DeclRef<InterfaceDecl> getOriginalInterfaceDeclRef() { return as<DeclRefBase>(getOperand(2)); }

    ExtractExistentialType(
        DeclRef<VarDeclBase> inDeclRef,
        Type* inOriginalInterfaceType,
        DeclRef<InterfaceDecl> inOriginalInterfaceDeclRef)
    {
        setOperands(inDeclRef, inOriginalInterfaceType, inOriginalInterfaceDeclRef);
    }

// Following fields will not be reflected (and thus won't be serialized, etc.)
SLANG_UNREFLECTED

    // A cached decl-ref to the original interface's ThisType Decl, with
    // a witness that refers to the type extracted here.
    //
    // This field is optional and can be filled in on-demand. It does *not*
    // represent part of the logical value of this `Type`, and should not
    // be serialized, included in hashes, etc.
    //
    DeclRef<ThisTypeDecl> cachedThisTypeDeclRef;

    // A cached pointer to a witness that shows how this type is a subtype
    // of `originalInterfaceType`.
    //
    SubtypeWitness* cachedSubtypeWitness = nullptr;

    // Overrides should be public so base classes can access
    void _toTextOverride(StringBuilder& out);
    Type* _createCanonicalTypeOverride();
    Val* _substituteImplOverride(ASTBuilder* astBuilder, SubstitutionSet subst, int* ioDiff);

        /// Get a witness that shows how this type is a subtype of `originalInterfaceType`.
        ///
        /// This operation may create the witness on demand and cache it.
        ///
    SubtypeWitness* getSubtypeWitness();

        /// Get a decl-ref to the interface's ThisType decl, which represents a substitutable type
        /// from which lookup can be performed.
        ///
        /// This operation may create the decl-ref on demand and cache it.
        ///
    DeclRef<ThisTypeDecl> getThisTypeDeclRef();
};

class ExistentialSpecializedType : public Type 
{
    SLANG_AST_CLASS(ExistentialSpecializedType)

    Type* getBaseType() { return as<Type>(getOperand(0)); }
    ExpandedSpecializationArg getArg(Index i)
    {
        ExpandedSpecializationArg arg;
        arg.val = getOperand(i * 2 + 1);
        arg.witness = getOperand(i * 2 + 2);
        return arg;
    }
    Index getArgCount() { return (getOperandCount() - 1) / 2; }

    ExistentialSpecializedType(
        Type* inBaseType,
        ExpandedSpecializationArgs const& inArgs)
    {
        m_operands.add(ValNodeOperand(inBaseType));
        for (auto arg : inArgs)
        {
            m_operands.add(ValNodeOperand(arg.val));
            m_operands.add(ValNodeOperand(arg.witness));
        }
    }

    // Overrides should be public so base classes can access
    void _toTextOverride(StringBuilder& out);
    Type* _createCanonicalTypeOverride();
    Val* _substituteImplOverride(ASTBuilder* astBuilder, SubstitutionSet subst, int* ioDiff);
};

    /// The type of `this` within a polymorphic declaration
class ThisType : public DeclRefType
{
    SLANG_AST_CLASS(ThisType)

    ThisType(DeclRefBase* declRef) : DeclRefType(declRef) {}

    DeclRef<InterfaceDecl> getInterfaceDeclRef();
};

    /// The type of `A & B` where `A` and `B` are types
    ///
    /// A value `v` is of type `A & B` if it is both of type `A` and of type `B`.
class AndType : public Type
{
    SLANG_AST_CLASS(AndType)

    Type* getLeft() { return as<Type>(getOperand(0)); }
    Type* getRight() { return as<Type>(getOperand(1)); }
    
    AndType(Type* leftType, Type* rightType)
    {
        setOperands(leftType, rightType);
    }

    // Overrides should be public so base classes can access
    void _toTextOverride(StringBuilder& out);
    Type* _createCanonicalTypeOverride();
    Val* _substituteImplOverride(ASTBuilder* astBuilder, SubstitutionSet subst, int* ioDiff);
};

class ModifiedType : public Type
{
    SLANG_AST_CLASS(ModifiedType)

    Type* getBase()
    {
        return as<Type>(getOperand(0));
    }

    Index getModifierCount() { return getOperandCount() - 1; }
    Val* getModifier(Index index) { return getOperand(index + 1); }

    ModifiedType(Type* inBase, ArrayView<Val*> inModifiers)
    {
        m_operands.add(ValNodeOperand(inBase));
        for (auto modifier : inModifiers)
            m_operands.add(ValNodeOperand(modifier));
    }

    template<typename T>
    T* findModifier()
    {
        for (Index i = 1; i < getOperandCount(); i++)
            if (auto rs = as<T>(getOperand(i)))
                return rs;
        return nullptr;
    }

    // Overrides should be public so base classes can access
    void _toTextOverride(StringBuilder& out);
    Type* _createCanonicalTypeOverride();
    Val* _substituteImplOverride(ASTBuilder* astBuilder, SubstitutionSet subst, int* ioDiff);
};

Type* removeParamDirType(Type* type);
bool isNonCopyableType(Type* type);

} // namespace Slang
