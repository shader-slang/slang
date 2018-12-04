#ifndef SLANG_TYPE_LAYOUT_H
#define SLANG_TYPE_LAYOUT_H

#include "../core/basic.h"
#include "compiler.h"
#include "profile.h"
#include "syntax.h"

#include "../../slang.h"

namespace Slang {

typedef intptr_t Int;
typedef uintptr_t UInt;

// Forward declarations

enum class BaseType;
class Type;

//

enum class LayoutRule
{
    Std140,
    Std430,
    HLSLConstantBuffer,
    HLSLStructuredBuffer,
};

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
// a single reosurce kind.
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

// A reified reprsentation of a particular laid-out type
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
        resourceInfos.Add(info);
        return &resourceInfos.Last();
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

        /// "Unwrap" any layers of array-ness from this type layout.
        ///
        /// If this is an `ArrayTypeLayout`, returns the result of unwrapping the elemnt type layout.
        /// Otherwise, returns this type layout.
        ///
    RefPtr<TypeLayout> unwrapArray();
};

typedef unsigned int VarLayoutFlags;
enum VarLayoutFlag : VarLayoutFlags
{
    IsRedeclaration = 1 << 0, ///< This is a redeclaration of some shader parameter
    HasSemantic = 1 << 1
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

    // General cse semantic name and index
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

        resourceInfos.Add(info);
        return &resourceInfos.Last();
    }

    ResourceInfo* findOrAddResourceInfo(LayoutResourceKind kind)
    {
        auto existing = FindResourceInfo(kind);
        if(existing) return existing;

        return AddResourceInfo(kind);
    }
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

// Specific case of type layout for an array
class ArrayTypeLayout : public TypeLayout
{
public:
    // The original type layout for the array elements,
    // which doesn't include any adjustments based on
    // resource type splitting.
    RefPtr<TypeLayout> originalElementTypeLayout;

    // The *adjusted* layout used for the element type
    RefPtr<TypeLayout>  elementTypeLayout;

    // the stride between elements when used in
    // a uniform buffer
    size_t              uniformStride;
};

// type layout for a variable with stream-output type
class StreamOutputTypeLayout : public TypeLayout
{
public:
    RefPtr<TypeLayout> elementTypeLayout;
};


class MatrixTypeLayout : public TypeLayout
{
public:
    MatrixLayoutMode    mode;
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
    Dictionary<Decl*, RefPtr<VarLayout>> mapVarToLayout;
};

class GenericParamTypeLayout : public TypeLayout
{
public:
    RefPtr<GlobalGenericParamDecl> getGlobalGenericParamDecl();
    int paramIndex = 0;
};

// Layout information for a single shader entry point
// within a program
//
// Treated as a subclass of `StructTypeLayout` becase
// it needs to include computed layout information
// for the parameters of the entry point.
//
// TODO: where to store layout info for the return
// type of the function?
class EntryPointLayout : public StructTypeLayout
{
public:
    // The corresponding function declaration
    RefPtr<FuncDecl> entryPoint;

    // The shader profile that was used to compile the entry point
    Profile profile;

    // Layout for any results of the entry point
    RefPtr<VarLayout> resultLayout;

    enum Flag : unsigned
    {
        usesAnySampleRateInput = 0x1,
    };
    unsigned flags = 0;
};

class GenericParamLayout : public Layout
{
public:
    RefPtr<GlobalGenericParamDecl> decl;
    int index;
};

// Layout information for the global scope of a program
class ProgramLayout : public Layout
{
public:
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

    // We catalog the requested entry points here,
    // and any entry-point-specific parameter data
    // will (eventually) belong there...
    List<RefPtr<EntryPointLayout>> entryPoints;

    List<RefPtr<GenericParamLayout>> globalGenericParams;
    Dictionary<String, GenericParamLayout*> globalGenericParamsMap;

    TargetRequest* targetRequest = nullptr;
};

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
    virtual SimpleLayoutInfo GetVectorLayout(SimpleLayoutInfo elementInfo, size_t elementCount) = 0;
    virtual SimpleLayoutInfo GetMatrixLayout(SimpleLayoutInfo elementInfo, size_t rowCount, size_t columnCount) = 0;

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

    SimpleLayoutInfo GetVectorLayout(SimpleLayoutInfo elementInfo, size_t elementCount)
    {
        return simpleRules->GetVectorLayout(elementInfo, elementCount);
    }

    SimpleLayoutInfo GetMatrixLayout(SimpleLayoutInfo elementInfo, size_t rowCount, size_t columnCount)
    {
        return simpleRules->GetMatrixLayout(elementInfo, rowCount, columnCount);
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
};

struct TypeLayoutContext
{
    // The layout rules to use (e.g., we compute
    // layout differently in a `cbuffer` vs. the
    // parameter list of a fragment shader).
    LayoutRulesImpl*    rules;

    // The target request that is triggering layout
    TargetRequest*      targetReq;

    // Whether to lay out matrices column-major
    // or row-major.
    MatrixLayoutMode    matrixLayoutMode;

    LayoutRulesImpl* getRules() { return rules; }
    LayoutRulesFamilyImpl* getRulesFamily() { return rules->getLayoutRulesFamily(); }

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
};


// Get an appropriate set of layout rules (packaged up
// as a `TypeLayoutContext`) to perform type layout
// for the given target.
TypeLayoutContext getInitialLayoutContextForTarget(
    TargetRequest* targetReq);

// Get the "simple" layout for a type accordinging to a given set of layout
// rules. Note that a "simple" layout can only consume one `LayoutResourceKind`,
// and so this operation may not correctly capture the full resource usage
// of a type.
SimpleLayoutInfo GetLayout(
    TypeLayoutContext const&    context,
    Type*                       type);

// Create a full type-layout object for a type,
// according to the layout rules in `context`.
RefPtr<TypeLayout> CreateTypeLayout(
    TypeLayoutContext const&    context,
    Type*                       type);

// Create a full type layout for a type, while applying the given "simple"
// layout information as an offset to any `VarLayout`s created along
// the way.
RefPtr<TypeLayout> CreateTypeLayout(
    TypeLayoutContext const&    context,
    Type*                       type,
    SimpleLayoutInfo            offset);

//

// Create a type layout for a parameter block type.
RefPtr<ParameterGroupTypeLayout>
createParameterGroupTypeLayout(
    TypeLayoutContext const&    context,
    RefPtr<ParameterGroupType>  parameterGroupType);

RefPtr<ParameterGroupTypeLayout>
createParameterGroupTypeLayout(
    TypeLayoutContext const&    context,
    RefPtr<ParameterGroupType>  parameterGroupType,
    RefPtr<Type>                elementType,
    LayoutRulesImpl*            elementTypeRules);

RefPtr<ParameterGroupTypeLayout>
createParameterGroupTypeLayout(
    TypeLayoutContext const&    context,
    RefPtr<ParameterGroupType>  parameterGroupType,
    SimpleLayoutInfo            parameterGroupInfo,
    RefPtr<TypeLayout>          elementTypeLayout);

RefPtr<ParameterGroupTypeLayout>
createParameterGroupTypeLayout(
    TypeLayoutContext const&    context,
    RefPtr<ParameterGroupType>  parameterGroupType,
    LayoutRulesImpl*            parameterGroupRules,
    SimpleLayoutInfo            parameterGroupInfo,
    RefPtr<TypeLayout>          elementTypeLayout);

// Create a type layout for a structured buffer type.
RefPtr<StructuredBufferTypeLayout>
createStructuredBufferTypeLayout(
    TypeLayoutContext const&    context,
    ShaderParameterKind         kind,
    RefPtr<Type>                structuredBufferType,
    RefPtr<Type>                elementType);

int findGenericParam(List<RefPtr<GenericParamLayout>> & genericParameters, GlobalGenericParamDecl * decl);
//

}

#endif