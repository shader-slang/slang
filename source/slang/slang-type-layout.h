#ifndef SLANG_TYPE_LAYOUT_H
#define SLANG_TYPE_LAYOUT_H

#include "../core/slang-basic.h"
#include "slang-compiler.h"
#include "slang-profile.h"
#include "slang-syntax.h"

#include "../../slang.h"

namespace Slang {

// Forward declarations

enum class BaseType;
class Type;

struct IRLayout;

//

#if 0
enum class LayoutRulesFamily
{
    HLSL,
    GLSL,
};
#endif

// A "size" that can either be a simple finite size or
// the special case of an infinite/unbounded size.
//
struct LayoutSize
{
    typedef size_t RawValue;

    LayoutSize()
        : raw(0)
    {}

    LayoutSize(RawValue size)
        : raw(size)
    {
        SLANG_ASSERT(size != RawValue(-1));
    }

    static LayoutSize fromRaw(RawValue raw)
    {
        LayoutSize size;
        size.raw = raw;
        return size;
    }

    static LayoutSize infinite()
    {
        LayoutSize result;
        result.raw = RawValue(-1);
        return result;
    }

    bool isInfinite() const { return raw == RawValue(-1); }

    bool isFinite() const { return raw != RawValue(-1); }
    RawValue getFiniteValue() const { SLANG_ASSERT(isFinite()); return raw; }

    bool operator==(LayoutSize that) const
    {
        return raw == that.raw;
    }

    bool operator!=(LayoutSize that) const
    {
        return raw != that.raw;
    }

    void operator+=(LayoutSize right)
    {
        if( isInfinite() ) {}
        else if( right.isInfinite() )
        {
            *this = LayoutSize::infinite();
        }
        else
        {
            *this = LayoutSize(raw + right.raw);
        }
    }

    void operator*=(LayoutSize right)
    {
        // Deal with zero first, so that anything (even the "infinite" value) times zero is zero.
        if( raw == 0 )
        {
            return;
        }

        if( right.raw == 0 )
        {
            raw = 0;
            return;
        }

        // Next we deal with infinite cases, so that infinite times anything non-zero is infinite
        if( isInfinite() )
        {
            return;
        }

        if( right.isInfinite() )
        {
            *this = LayoutSize::infinite();
            return;
        }

        // Finally deal with the case where both sides are finite
        *this = LayoutSize(raw * right.raw);
    }

    void operator-=(RawValue right)
    {
        if( isInfinite() ) {}
        else
        {
            *this = LayoutSize(raw - right);
        }
    }

    void operator/=(RawValue right)
    {
        if( isInfinite() ) {}
        else
        {
            *this = LayoutSize(raw / right);
        }
    }
    RawValue raw;
};

inline LayoutSize operator+(LayoutSize left, LayoutSize right)
{
    LayoutSize result(left);
    result += right;
    return result;
}

inline LayoutSize operator*(LayoutSize left, LayoutSize right)
{
    LayoutSize result(left);
    result *= right;
    return result;
}

inline LayoutSize operator-(LayoutSize left, LayoutSize::RawValue right)
{
    LayoutSize result(left);
    result -= right;
    return result;
}

inline LayoutSize operator/(LayoutSize left, LayoutSize::RawValue right)
{
    LayoutSize result(left);
    result /= right;
    return result;
}

inline LayoutSize maximum(LayoutSize left, LayoutSize right)
{
    if(left.isInfinite() || right.isInfinite())
        return LayoutSize::infinite();

    return LayoutSize(Math::Max(
            left.getFiniteValue(),
            right.getFiniteValue()));
}

inline bool operator>(LayoutSize left, LayoutSize::RawValue right)
{
    return left.isInfinite() || (left.getFiniteValue() > right);
}

inline bool operator<=(LayoutSize left, LayoutSize::RawValue right)
{
    return left.isFinite() && (left.getFiniteValue() <= right);
}

// Layout appropriate to "just memory" scenarios,
// such as laying out the members of a constant buffer.
struct UniformLayoutInfo
{
    LayoutSize  size;
    size_t      alignment;

    UniformLayoutInfo()
        : size(0)
        , alignment(1)
    {}

    UniformLayoutInfo(
        LayoutSize  size,
        size_t      alignment)
        : size(size)
        , alignment(alignment)
    {}
};

// Extended information required for an array of uniform data,
// including the "stride" of the array (the space between
// consecutive elements).
struct UniformArrayLayoutInfo : UniformLayoutInfo
{
    size_t elementStride;

    UniformArrayLayoutInfo()
        : elementStride(0)
    {}

    UniformArrayLayoutInfo(
        LayoutSize  size,
        size_t      alignment,
        size_t      elementStride)
        : UniformLayoutInfo(size, alignment)
        , elementStride(elementStride)
    {}
};

typedef slang::ParameterCategory LayoutResourceKind;

// Layout information for a value that only consumes
// a single resource kind.
struct SimpleLayoutInfo
{
    // What kind of resource should we consume?
    LayoutResourceKind kind;

    // How many resources of that kind?
    LayoutSize size;

    // only useful in the uniform case
    size_t alignment;

    SimpleLayoutInfo()
        : kind(LayoutResourceKind::None)
        , size(0)
        , alignment(1)
    {}

    SimpleLayoutInfo(
        UniformLayoutInfo uniformInfo)
        : kind(LayoutResourceKind::Uniform)
        , size(uniformInfo.size)
        , alignment(uniformInfo.alignment)
    {}

    SimpleLayoutInfo(LayoutResourceKind kind, LayoutSize size, size_t alignment=1)
        : kind(kind)
        , size(size)
        , alignment(alignment)
    {}

    // Convert to layout for uniform data
    UniformLayoutInfo getUniformLayout()
    {
        if(kind == LayoutResourceKind::Uniform)
        {
            return UniformLayoutInfo(size, alignment);
        }
        else
        {
            return UniformLayoutInfo(0, 1);
        }
    }
};

// Only useful in the case of a homogeneous array
struct SimpleArrayLayoutInfo : SimpleLayoutInfo
{
    // This field is only useful in the uniform case
    size_t elementStride;

    // Convert to layout for uniform data
    UniformArrayLayoutInfo getUniformLayout()
    {
        if(kind == LayoutResourceKind::Uniform)
        {
            return UniformArrayLayoutInfo(size, alignment, elementStride);
        }
        else
        {
            return UniformArrayLayoutInfo(0, 1, 0);
        }
    }
};

struct LayoutRulesImpl;

// Base class for things that store layout info
class Layout : public RefObject
{
};

// A reified representation of a particular laid-out type
class TypeLayout : public Layout
{
public:
    // The type that was laid out
    RefPtr<Type>  type;
    Type* getType() { return type.Ptr(); }

    // The layout rules that were used to produce this type
    LayoutRulesImpl*        rules;

    struct ResourceInfo
    {
        // What kind of register was it?
        LayoutResourceKind  kind = LayoutResourceKind::None;

        // How many registers of the above kind did we use?
        LayoutSize          count;
    };

    List<ResourceInfo>      resourceInfos;

    // For uniform data, alignment matters, but not for
    // any other resource category, so we don't waste
    // the space storing it in the above array
    UInt uniformAlignment = 1;


        /// The layout for data that is conceptually owned by this type, but which is pending layout.
        ///
        /// When a type contains interface/existential fields (recursively), the
        /// actual data referenced by these fields needs to get allocated somewhere,
        /// but it cannot go inline at the point where the interface/existential
        /// type appears, or else the layout of a composite object would change
        /// when the concrete type(s) we plug in change.
        ///
        /// We solve this problem by tracking this data that is "pending" layout,
        /// and then "flushing" the pending data at appropriate places during
        /// the layout process.
        ///
    RefPtr<TypeLayout> pendingDataTypeLayout;

    ResourceInfo* FindResourceInfo(LayoutResourceKind kind)
    {
        for(auto& rr : resourceInfos)
        {
            if(rr.kind == kind)
                return &rr;
        }
        return nullptr;
    }

    ResourceInfo* findOrAddResourceInfo(LayoutResourceKind kind)
    {
        auto existing = FindResourceInfo(kind);
        if(existing) return existing;

        ResourceInfo info;
        info.kind = kind;
        info.count = 0;
        resourceInfos.add(info);
        return &resourceInfos.getLast();
    }

    void addResourceUsage(ResourceInfo info)
    {
        if(info.count == 0) return;

        findOrAddResourceInfo(info.kind)->count += info.count;
    }

    void addResourceUsage(LayoutResourceKind kind, LayoutSize count)
    {
        ResourceInfo info;
        info.kind = kind;
        info.count = count;
        addResourceUsage(info);
    }

    void removeResourceUsage(LayoutResourceKind kind);

    void addResourceUsageFrom(TypeLayout* otherTypeLayout);

        /// "Unwrap" any layers of array-ness from this type layout.
        ///
        /// If this is an `ArrayTypeLayout`, returns the result of unwrapping the element type layout.
        /// Otherwise, returns this type layout.
        ///
    RefPtr<TypeLayout> unwrapArray();
};

typedef unsigned int VarLayoutFlags;
enum VarLayoutFlag : VarLayoutFlags
{
    HasSemantic = 1 << 0
};

// A reified layout for a particular variable, field, etc.
class VarLayout : public Layout
{
public:
    // The variable we are laying out
    DeclRef<VarDeclBase>          varDecl;
    VarDeclBase* getVariable() { return varDecl.getDecl(); }

    Name* getName() { return getVariable()->getName(); }

    // The result of laying out the variable's type
    RefPtr<TypeLayout>      typeLayout;
    TypeLayout* getTypeLayout() { return typeLayout.Ptr(); }

    // Additional flags
    VarLayoutFlags flags = 0;

    // System-value semantic (and index) if this is a system value
    String  systemValueSemantic;
    int     systemValueSemanticIndex;

    // General case semantic name and index
    // TODO: this and the system-value field are redundant
    // TODO: the `VarLayout` type is getting bloated; we need to not store this
    // information unless actually required.
    String  semanticName;
    int     semanticIndex;

    // The stage this variable belongs to, in case it is
    // stage-specific.
    // TODO: This is wasteful to be storing on every single
    // variable layout.
    Stage stage = Stage::Unknown;

    // The start register(s) for any resources
    struct ResourceInfo
    {
        // What kind of register was it?
        LayoutResourceKind  kind = LayoutResourceKind::None;

        // What binding space (HLSL) or set (Vulkan) are we placed in?
        UInt                space;

        // What is our starting register in that space?
        //
        // (In the case of uniform data, this is a byte offset)
        UInt                index;
    };
    List<ResourceInfo>      resourceInfos;

    ResourceInfo* FindResourceInfo(LayoutResourceKind kind)
    {
        for(auto& rr : resourceInfos)
        {
            if(rr.kind == kind)
                return &rr;
        }
        return nullptr;
    }

    ResourceInfo* AddResourceInfo(LayoutResourceKind kind)
    {
        ResourceInfo info;
        info.kind = kind;
        info.space = 0;
        info.index = 0;

        resourceInfos.add(info);
        return &resourceInfos.getLast();
    }

    ResourceInfo* findOrAddResourceInfo(LayoutResourceKind kind)
    {
        auto existing = FindResourceInfo(kind);
        if(existing) return existing;

        return AddResourceInfo(kind);
    }

    RefPtr<VarLayout> pendingVarLayout;
};

// type layout for a variable that has a constant-buffer type
class ParameterGroupTypeLayout : public TypeLayout
{
public:
    // The layout of the "container" part itself.
    // E.g., for a constant buffer, this would reflect
    // the resource usage of the container, without
    // the element type factored in. All of the offsets
    // for this variable should be zero, but it is included
    // for completeness.
    RefPtr<VarLayout>  containerVarLayout;

    // A variable layout for the element of the container.
    // The offsets of the variable layout will reflect
    // the offsets that need to applied to get past the
    // container types resource usage, while the actual
    // type layout won't have offsets applied (unlike
    // `offsetElementTypeLayout` below).
    RefPtr<VarLayout>   elementVarLayout;

    // The layout of the element type, with offsets applied
    // so that any fields (if the element type is a `struct`)
    // will be offset by the resource usage of the container.
    RefPtr<TypeLayout>  offsetElementTypeLayout;
};

// type layout for a variable that has a constant-buffer type
class StructuredBufferTypeLayout : public TypeLayout
{
public:
    RefPtr<TypeLayout> elementTypeLayout;
};

    /// Type layout for a logical sequence type
class SequenceTypeLayout : public TypeLayout
{
public:
        /// The layout of the element type.
        ///
        /// This layout may include adjustments to make lookups in elements
        /// of the array Just Work, and may not be the same as the layout
        /// of the element type when used in a non-array context.
        ///
    RefPtr<TypeLayout>  elementTypeLayout;

        /// The stride in bytes between elements.
    size_t              uniformStride = 0;
};

    /// Type layout for an array type
class ArrayTypeLayout : public SequenceTypeLayout
{
public:
        /// The original layout of the element type.
        ///
        /// This layout does not include any adjustments that
        /// were made to the element type in order to make
        /// lookup into array elements Just Work.
        ///
    RefPtr<TypeLayout> originalElementTypeLayout;
};

// type layout for a variable with stream-output type
class StreamOutputTypeLayout : public TypeLayout
{
public:
    RefPtr<TypeLayout> elementTypeLayout;
};

class VectorTypeLayout : public SequenceTypeLayout
{
public:
};


class MatrixTypeLayout : public SequenceTypeLayout
{
public:
        /// Is this matrix laid out as row-major or column-major?
        ///
        /// Note that this does *not* affect the interpretation
        /// of the `elementTypeLayout` field, which always represents
        /// the logical elements of the matrix type, which are its
        /// rows.
        ///
    MatrixLayoutMode            mode;
};

// Specific case of type layout for a struct
class StructTypeLayout : public TypeLayout
{
public:
    // An ordered list of layouts for the known fields
    List<RefPtr<VarLayout>> fields;

    // Map a variable to its layout directly.
    //
    // Note that in the general case, there may be entries
    // in the `fields` array that came from multiple
    // translation units, and in cases where there are
    // multiple declarations of the same parameter, only
    // one will appear in `fields`, while all of
    // them will be reflected in `mapVarToLayout`.
    //
    // TODO: This should map from a declaration to the *index*
    // in the array above, rather than to the actual pointer,
    // so that we 
    Dictionary<VarDeclBase*, RefPtr<VarLayout>> mapVarToLayout;
};

class GenericParamTypeLayout : public TypeLayout
{
public:
    RefPtr<GlobalGenericParamDecl> getGlobalGenericParamDecl();
    Index paramIndex = 0;
};

    /// Layout information for a tagged union type.
class TaggedUnionTypeLayout : public TypeLayout
{
public:
        /// The layouts of each of the case types.
        ///
        /// The order of entries in this array matches
        /// the order of case types on the original
        /// `TaggedUnionType`, and the index of a case
        /// type is also the tag value for that case.
        ///
    List<RefPtr<TypeLayout>> caseTypeLayouts;

        /// The byte offset for the tag field.
        ///
        /// The tag field will always be allocated as
        /// a `uint`, so we don't store a separate layout
        /// for it.
        ///
    LayoutSize tagOffset;
};

    /// Layout information for a type with existential (sub-)field types specialized.
class ExistentialSpecializedTypeLayout : public TypeLayout
{
public:
    RefPtr<TypeLayout> baseTypeLayout;
    RefPtr<VarLayout> pendingDataVarLayout;
};

    /// Layout for a scoped entity like a program, module, or entry point
class ScopeLayout : public Layout
{
public:
    // The layout for the parameters of this entity.
    //
    RefPtr<VarLayout> parametersLayout;
};

StructTypeLayout* getScopeStructLayout(
    ScopeLayout*  programLayout);

// Layout information for a single shader entry point
// within a program
//
// Treated as a subclass of `StructTypeLayout` because
// it needs to include computed layout information
// for the parameters of the entry point.
//
// TODO: where to store layout info for the return
// type of the function?
class EntryPointLayout : public ScopeLayout
{
public:
    // The corresponding function declaration
    DeclRef<FuncDecl> entryPoint;

    DeclRef<FuncDecl> getFuncDeclRef() { return entryPoint; }
    FuncDecl* getFuncDecl() { return entryPoint.getDecl(); }

    // The shader profile that was used to compile the entry point
    Profile profile;

    // The name of the entry point. Always available even if entryPoint is nullptr (for example when it came from a library)
    Name* name = nullptr;

    // Layout for any results of the entry point
    RefPtr<VarLayout> resultLayout;

    enum Flag : unsigned
    {
        usesAnySampleRateInput = 0x1,
    };
    unsigned flags = 0;

//    EntryPointLayout* getAbsoluteLayout(VarLayout* parentLayout);

//    RefPtr<EntryPointLayout> m_absoluteLayout;
};

    /// Reflection/layout information about a specialization parameter
class SpecializationParamLayout : public Layout
{
public:
    Index index;
};

    /// Reflection/layout information about a generic specialization parameter
class GenericSpecializationParamLayout : public SpecializationParamLayout
{
public:
        /// The declaration of the generic parameter.
        ///
        /// Could be any subclass of `Decl` that represents a generic value or type parameter.
    RefPtr<Decl> decl;
};

    /// Reflection/layout information about an existential/interface specialization parameter.
class ExistentialSpecializationParamLayout : public SpecializationParamLayout
{
public:
        /// The type that needs to be specialized.
        ///
        /// Currently, this will be an `interface` type that any concrete
        /// type argument getting plugged in must conform to.
        ///
    RefPtr<Type> type;
};

// Layout information for the global scope of a program
class ProgramLayout : public ScopeLayout
{
public:
    /*
    // We store a layout for the declarations at the global
    // scope. Note that this will *either* be a single
    // `StructTypeLayout` with the fields stored directly,
    // or it will be a single `ParameterGroupTypeLayout`,
    // where the global-scope fields are the members of
    // that constant buffer.
    //
    // The `struct` case will be used if there are no
    // "naked" global-scope uniform variables, and the
    // constant-buffer case will be used if there are
    // (since a constant buffer will have to be allocated
    // to store them).
    //
    RefPtr<VarLayout> globalScopeLayout;
    */

        /// The target and program for which layout was computed
    TargetProgram* targetProgram;

    TargetProgram* getTargetProgram() { return targetProgram; }
    TargetRequest* getTargetReq() { return targetProgram->getTargetReq(); }
    ComponentType* getProgram() { return targetProgram->getProgram(); }

    ProgramLayout():
        hashedStringLiteralPool(StringSlicePool::Style::Empty)
    {
    }

    // We catalog the requested entry points here,
    // and any entry-point-specific parameter data
    // will (eventually) belong there...
    List<RefPtr<EntryPointLayout>> entryPoints;

        /// Reflection information on (unspecialized) specialization parameters.
    List<RefPtr<SpecializationParamLayout>> specializationParams;

        /// Concrete argument values that were provided to specific global generic parameters.
        ///
        /// Not useful for reflection, but valuable for code generation.
        ///
    Dictionary<GlobalGenericParamDecl*, RefPtr<Val>> globalGenericArgs;

        /// Layouts for all tagged union types required by this program
        ///
        /// These are any tagged union types used by the specialization
        /// arguments that have been used to specialize the program.
        ///
    List<RefPtr<TypeLayout>> taggedUnionTypeLayouts;

        /// Holds all of the string literals that have been hashed
    StringSlicePool hashedStringLiteralPool;
};

StructTypeLayout* getGlobalStructLayout(
    ProgramLayout*  programLayout);

struct LayoutRulesFamilyImpl;

// A delineation of shader parameter types into fine-grained
// categories that can then be mapped down to actual resources
// by a given set of rules.
//
// TODO(tfoley): `SlangParameterCategory` and `slang::ParameterCategory`
// are badly named, and need to be revised so they can't be confused
// with this concept.
enum class ShaderParameterKind
{
    ConstantBuffer,
    TextureUniformBuffer,
    ShaderStorageBuffer,

    StructuredBuffer,
    MutableStructuredBuffer,

    RawBuffer,
    MutableRawBuffer,

    Buffer,
    MutableBuffer,

    Texture,
    MutableTexture,

    TextureSampler,
    MutableTextureSampler,

    InputRenderTarget,

    SamplerState,

    Image,
    MutableImage,

    RegisterSpace,
};

struct SimpleLayoutRulesImpl
{
    // Get size and alignment for a single value of base type.
    virtual SimpleLayoutInfo GetScalarLayout(BaseType baseType) = 0;

    // Get size and alignment for an array of elements
    virtual SimpleArrayLayoutInfo GetArrayLayout(SimpleLayoutInfo elementInfo, LayoutSize elementCount) = 0;

    // Get layout for a vector or matrix type
    virtual SimpleLayoutInfo GetVectorLayout(BaseType elementType, SimpleLayoutInfo elementInfo, size_t elementCount) = 0;
    virtual SimpleArrayLayoutInfo GetMatrixLayout(BaseType elementType, SimpleLayoutInfo elementInfo, size_t rowCount, size_t columnCount) = 0;

    // Begin doing layout on a `struct` type
    virtual UniformLayoutInfo BeginStructLayout() = 0;

    // Add a field to a `struct` type, and return the offset for the field
    virtual LayoutSize AddStructField(UniformLayoutInfo* ioStructInfo, UniformLayoutInfo fieldInfo) = 0;

    // End layout for a struct, and finalize its size/alignment.
    virtual void EndStructLayout(UniformLayoutInfo* ioStructInfo) = 0;
};

struct ObjectLayoutRulesImpl
{
    // Compute layout info for an object type
    virtual SimpleLayoutInfo GetObjectLayout(ShaderParameterKind kind) = 0;
};

struct LayoutRulesImpl
{
    LayoutRulesFamilyImpl*  family;
    SimpleLayoutRulesImpl*  simpleRules;
    ObjectLayoutRulesImpl*  objectRules;

    // Forward `SimpleLayoutRulesImpl` interface

    SimpleLayoutInfo GetScalarLayout(BaseType baseType)
    {
        return simpleRules->GetScalarLayout(baseType);
    }

    SimpleArrayLayoutInfo GetArrayLayout(SimpleLayoutInfo elementInfo, LayoutSize elementCount)
    {
        return simpleRules->GetArrayLayout(elementInfo, elementCount);
    }

    SimpleLayoutInfo GetVectorLayout(BaseType elementType, SimpleLayoutInfo elementInfo, size_t elementCount)
    {
        return simpleRules->GetVectorLayout(elementType, elementInfo, elementCount);
    }

    SimpleArrayLayoutInfo GetMatrixLayout(BaseType elementType, SimpleLayoutInfo elementInfo, size_t rowCount, size_t columnCount)
    {
        return simpleRules->GetMatrixLayout(elementType, elementInfo, rowCount, columnCount);
    }

    UniformLayoutInfo BeginStructLayout()
    {
        return simpleRules->BeginStructLayout();
    }

    LayoutSize AddStructField(UniformLayoutInfo* ioStructInfo, UniformLayoutInfo fieldInfo)
    {
        return simpleRules->AddStructField(ioStructInfo, fieldInfo);
    }

    void EndStructLayout(UniformLayoutInfo* ioStructInfo)
    {
        return simpleRules->EndStructLayout(ioStructInfo);
    }

    // Forward `ObjectLayoutRulesImpl` interface

    SimpleLayoutInfo GetObjectLayout(ShaderParameterKind kind)
    {
        return objectRules->GetObjectLayout(kind);
    }

    //

    LayoutRulesFamilyImpl* getLayoutRulesFamily() { return family; }
};

struct LayoutRulesFamilyImpl
{
    virtual LayoutRulesImpl* getConstantBufferRules()       = 0;
    virtual LayoutRulesImpl* getPushConstantBufferRules()   = 0;
    virtual LayoutRulesImpl* getTextureBufferRules()        = 0;
    virtual LayoutRulesImpl* getVaryingInputRules()         = 0;
    virtual LayoutRulesImpl* getVaryingOutputRules()        = 0;
    virtual LayoutRulesImpl* getSpecializationConstantRules()= 0;
    virtual LayoutRulesImpl* getShaderStorageBufferRules()  = 0;
    virtual LayoutRulesImpl* getParameterBlockRules()       = 0;

    virtual LayoutRulesImpl* getRayPayloadParameterRules()  = 0;
    virtual LayoutRulesImpl* getCallablePayloadParameterRules()  = 0;
    virtual LayoutRulesImpl* getHitAttributesParameterRules()= 0;

    virtual LayoutRulesImpl* getShaderRecordConstantBufferRules() = 0;

    virtual LayoutRulesImpl* getStructuredBufferRules() = 0;
};

struct TypeLayoutContext
{
    // The layout rules to use (e.g., we compute
    // layout differently in a `cbuffer` vs. the
    // parameter list of a fragment shader).
    LayoutRulesImpl*    rules;

    // The target request that is triggering layout
    TargetRequest*  targetReq;

    // A parent program layout that will establish the ordering
    // of all global generic type parameters.
    //
    ProgramLayout* programLayout;

    // Whether to lay out matrices column-major
    // or row-major.
    MatrixLayoutMode    matrixLayoutMode;

    // The concrete types (if any) to plug into the currently in-scope
    // specialization params.
    //
    Int                                 specializationArgCount = 0;
    ExpandedSpecializationArg const*    specializationArgs = nullptr;

    LayoutRulesImpl* getRules() { return rules; }
    LayoutRulesFamilyImpl* getRulesFamily() const { return rules->getLayoutRulesFamily(); }

    TypeLayoutContext with(LayoutRulesImpl* inRules) const
    {
        TypeLayoutContext result = *this;
        result.rules = inRules;
        return result;
    }

    TypeLayoutContext with(MatrixLayoutMode inMatrixLayoutMode) const
    {
        TypeLayoutContext result = *this;
        result.matrixLayoutMode = inMatrixLayoutMode;
        return result;
    }

    TypeLayoutContext withSpecializationArgs(
        ExpandedSpecializationArg const*    args,
        Int                                 argCount) const
    {
        TypeLayoutContext result = *this;
        result.specializationArgCount  = argCount;
        result.specializationArgs      = args;
        return result;
    }

    TypeLayoutContext withSpecializationArgsOffsetBy(
        Int offset) const
    {
        TypeLayoutContext result = *this;
        if( specializationArgCount > offset )
        {
            result.specializationArgCount  = specializationArgCount - offset;
            result.specializationArgs      = specializationArgs + offset;
        }
        else
        {
            result.specializationArgCount  = 0;
            result.specializationArgs      = nullptr;
        }
        return result;

    }
};

//

    /// A custom tuple to capture the outputs of type layout
struct TypeLayoutResult
{
        /// The actual heap-allocated layout object with all the details
    RefPtr<TypeLayout>  layout;

        /// A simplified representation of layout information.
        ///
        /// This information is suitable for the case where a type only
        /// consumes a single resource.
        ///
    SimpleLayoutInfo    info;

        /// Default constructor.
    TypeLayoutResult()
    {}

        /// Construct a result from the given layout object and simple layout info.
    TypeLayoutResult(RefPtr<TypeLayout> inLayout, SimpleLayoutInfo const& inInfo)
        : layout(inLayout)
        , info(inInfo)
    {}
};

    /// Helper type for building `struct` type layouts
struct StructTypeLayoutBuilder
{
public:
        /// Begin the layout process for `type`, using `rules`
    void beginLayout(
        Type*               type,
        LayoutRulesImpl*    rules);

        /// Begin the layout process for `type`, using `rules`, if it hasn't already been begun.
        ///
        /// This functions allows for a `StructTypeLayoutBuilder` to be use lazily,
        /// only allocating a type layout object if it is actaully needed.
        ///
    void beginLayoutIfNeeded(
        Type*               type,
        LayoutRulesImpl*    rules);

        /// Add a field to the struct type layout.
        ///
        /// One of the `beginLayout*()` functions must have been called previously.
        ///
    RefPtr<VarLayout> addField(
        DeclRef<VarDeclBase>    field,
        TypeLayoutResult        fieldResult);

        /// Add a field to the struct type layout.
        ///
        /// One of the `beginLayout*()` functions must have been called previously.
        ///
    RefPtr<VarLayout> addField(
        DeclRef<VarDeclBase>    field,
        RefPtr<TypeLayout>      fieldTypeLayout);

        /// Complete layout.
        ///
        /// If layout was begun, ensures that the result of `getTypeLayout()` is usable.
        /// If layout was never begin, does nothing.
        ///
    void endLayout();

        /// Get the type layout.
        ///
        /// This can be called any time after `beginLayout*()`.
        /// In particular, it can be called before `endLayout`.
        ///
    RefPtr<StructTypeLayout> getTypeLayout();

        /// The the type layout result.
        ///
        /// This is primarily useful for implementation code in `_createTypeLayout`.
        ///
    TypeLayoutResult getTypeLayoutResult();

    UniformLayoutInfo* getStructLayoutInfo() { return &m_info; }

private:
        /// The layout rules being used, if layout has begun.
    LayoutRulesImpl* m_rules = nullptr;

        /// The type layout being computed, if layout has begun.
    RefPtr<StructTypeLayout> m_typeLayout;

        /// Uniform offset/alignment statte used when computing offset for uniform fields.
    UniformLayoutInfo m_info;
};

//

// Get an appropriate set of layout rules (packaged up
// as a `TypeLayoutContext`) to perform type layout
// for the given target.
//
// The provided `programLayout` is used to establish
// the ordering of all global generic type paramters.
//
TypeLayoutContext getInitialLayoutContextForTarget(
    TargetRequest*  targetReq,
    ProgramLayout*  programLayout);

    /// Direction(s) of a varying shader parameter
typedef unsigned int EntryPointParameterDirectionMask;
enum
{
    kEntryPointParameterDirection_Input  = 0x1,
    kEntryPointParameterDirection_Output = 0x2,
};


    /// Get layout information for a simple varying parameter type.
    ///
    /// A simple varying parameter is a scalar, vector, or matrix.
    ///
RefPtr<TypeLayout> getSimpleVaryingParameterTypeLayout(
    TypeLayoutContext const&            context,
    Type*                               type,
    EntryPointParameterDirectionMask    directionMask);

// Create a full type-layout object for a type,
// according to the layout rules in `context`.
RefPtr<TypeLayout> createTypeLayout(
    TypeLayoutContext const&    context,
    Type*                       type);

//

    /// Create a layout for a parameter-group type (a `ConstantBuffer` or `ParameterBlock`).
RefPtr<TypeLayout> createParameterGroupTypeLayout(
    TypeLayoutContext const&    context,
    RefPtr<ParameterGroupType>  parameterGroupType);

    /// Create a wrapper constant buffer type layout, if needed.
    ///
    /// When dealing with entry-point `uniform` and global-scope parameters,
    /// we want to create a wrapper constant buffer for all the parameters
    /// if and only if there exist some parameters that use "ordinary" data
    /// (`LayoutResourceKind::Uniform`).
    ///
    /// This function determines whether such a wrapper is needed, based
    /// on the `elementTypeLayout` given, and either creates and returns
    /// the layout for the wrapper, or the unmodified `elementTypeLayout`.
    ///
RefPtr<TypeLayout> createConstantBufferTypeLayoutIfNeeded(
    TypeLayoutContext const&    context,
    RefPtr<TypeLayout>          elementTypeLayout);

// Create a type layout for a structured buffer type.
RefPtr<StructuredBufferTypeLayout>
createStructuredBufferTypeLayout(
    TypeLayoutContext const&    context,
    ShaderParameterKind         kind,
    RefPtr<Type>                structuredBufferType,
    RefPtr<Type>                elementType);

    /// Create a type layout for an unspecialized `globalGenericParamDecl`.
RefPtr<TypeLayout> createTypeLayoutForGlobalGenericTypeParam(
    TypeLayoutContext const&    context,
    Type*                       type,
    GlobalGenericParamDecl*     globalGenericParamDecl);

    /// Find the concrete type (if any) that was plugged in for the global generic type parameter `decl`.
RefPtr<Type> findGlobalGenericSpecializationArg(
    TypeLayoutContext const&    context,
    GlobalGenericParamDecl*     decl);

// Given an existing type layout `oldTypeLayout`, apply offsets
// to any contained fields based on the resource infos in `offsetVarLayout`.
RefPtr<TypeLayout> applyOffsetToTypeLayout(
    RefPtr<TypeLayout>  oldTypeLayout,
    RefPtr<VarLayout>   offsetVarLayout);

struct IRBuilder;
struct IRTypeLayout;
struct IRVarLayout;

IRTypeLayout* applyOffsetToTypeLayout(
    IRBuilder*      irBuilder,
    IRTypeLayout*   oldTypeLayout,
    IRVarLayout*    offsetVarLayout);

    /// Create a layout like `baseLayout`, but offset by `offsetLayout`
IRVarLayout* applyOffsetToVarLayout(
    IRBuilder*      irBuilder,
    IRVarLayout*    baseLayout,
    IRVarLayout*    offsetLayout);

}

#endif
