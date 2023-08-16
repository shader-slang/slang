// slang-ir-layout.h
#pragma once

// This file provides utilities for computing and caching the *natural*
// layout of types in the IR.
//
// The natural layout is the layout a target uses for a type when it is
// stored in unconstrainted general-purpose memory (to the extent that
// the target supports unconstrained general-purpose memory).
//
// For targets like the CPU and CUDA which support a simple flat address
// space, the natural layout is the only layout used for any type.
//
// For targets like D3D DXBC/DXIL and Vulkan SPIR-V, the natural layout
// matches how a type is stored in a "structured buffer" or "shader
// storage buffer."
//

#include "slang-ir.h"

namespace Slang
{
class TargetRequest;

    /// Align `value` to the next multiple of `alignment`, which must be a power of two.
inline IRIntegerValue align(IRIntegerValue value, int alignment)
{
    return (value + alignment-1) & ~IRIntegerValue(alignment-1);
}


    /// The size and alignment of an IR type.
struct IRSizeAndAlignment
{
    IRSizeAndAlignment()
    {}

    IRSizeAndAlignment(IRIntegerValue size, int alignment)
        : size(size)
        , alignment(alignment)
    {}

    IRIntegerValue  size = 0;

    int             alignment = 1;

    inline IRIntegerValue getStride()
    {
        return align(size, alignment);
    }
};

struct IRTypeLayoutRules
{
public:
    IRTypeLayoutRuleName ruleName;
    virtual IRSizeAndAlignment alignCompositeElement(IRSizeAndAlignment elementSize) = 0;
    virtual IRSizeAndAlignment getVectorSizeAndAlignment(IRSizeAndAlignment element, IRIntegerValue count) = 0;
    static IRTypeLayoutRules* getStd430();
    static IRTypeLayoutRules* getStd140();
    static IRTypeLayoutRules* getNatural();
    static IRTypeLayoutRules* get(IRTypeLayoutRuleName name);
};

Result getOffset(IRTypeLayoutRules* rules, IRStructField* field, IRIntegerValue* outOffset);

Result getSizeAndAlignment(IRTypeLayoutRules* rules, IRType* type, IRSizeAndAlignment* outSizeAndAlignment);

    /// Compute (if necessary) and return the natural size and alignment of `type`.
    ///
    /// This operation may fail if `type` is not one that can be stored in
    /// general-purpose memory for the current target. In that case the
    /// type is considered to have no natural layout.
    ///
Result getNaturalSizeAndAlignment(IRType* type, IRSizeAndAlignment* outSizeAndAlignment);

    /// Compute (if necessary) and return the natural offset of `field`
    ///
    /// This operation can fail if the parent type of `field` is not one
    /// that can be stored in general-purpose memory. In that case, the
    /// field is considered to have no natural offset.
    ///
Result getNaturalOffset(IRStructField* field, IRIntegerValue* outOffset);

/// Compute (if necessary) and return the std430 size and alignment of `type`.
///
/// This operation may fail if `type` is not one that can be stored in
/// general-purpose memory for the current target. In that case the
/// type is considered to have no std430 layout.
///
Result getStd430SizeAndAlignment(IRType* type, IRSizeAndAlignment* outSizeAndAlignment);

/// Compute (if necessary) and return the std430 offset of `field`
///
/// This operation can fail if the parent type of `field` is not one
/// that can be stored in general-purpose memory. In that case, the
/// field is considered to have no std430 offset.
///
Result getStd430Offset(IRStructField* field, IRIntegerValue* outOffset);

}

