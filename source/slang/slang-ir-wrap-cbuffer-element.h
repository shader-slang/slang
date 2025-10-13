#ifndef SLANG_IR_WRAP_CBUFFER_ELEMENT_H
#define SLANG_IR_WRAP_CBUFFER_ELEMENT_H

namespace Slang
{
struct IRModule;
struct IRParameterGroupType;

class WrapCBufferElementPolicy
{
public:
    virtual bool shouldWrapBufferElementInStruct(IRParameterGroupType* cbufferType) = 0;
};

// Wrap the element type of a ConstantBuffer/ParameterBlock in a struct if the element type is not
// something that allowed directly as a buffer element type by the target.
void wrapCBufferElements(IRModule* module, WrapCBufferElementPolicy* policy);

void wrapCBufferElementsForMetal(IRModule* module);

} // namespace Slang

#endif
