#pragma once

namespace Slang
{

/*
This pass implements a intra-function optimization that defers the loading of buffer
elements to the end of the access chain to avoid loading unnecessary data. For example, if we see:
    val = StructuredBufferLoad(s, i)
    val2 = GetElement(val, j)
    val3 = FieldExtract(val2, field_key_0)
    call(foo, val3)
We should rewrite the code into:
    ptr = RWStructuredBufferGetElementPtr(s, i)
    ptr2 = ElementAddress(ptr, j)
    ptr3 = FieldAddress(ptr2, field_key_0)
    val3 = Load(ptr3)
    call(foo, val3)
*/

struct IRModule;
struct IRType;
struct CodeGenContext;
struct IRInst;
class TargetRequest;

void deferBufferLoad(IRModule* module, CodeGenContext* context);

// Returns true if the type is suitable for defer-load optimization.
// Generally, we want to defer loading large structs or composites that contain arrays.
bool isTypePreferrableToDeferLoad(CodeGenContext* context, IRType* type);

// Returns true if memory loaded by `loadInst` may be modified before `userInst` after it is
// loaded.
bool isMemoryLocationUnmodifiedBetweenLoadAndUser(
    TargetRequest* target,
    IRInst* loadInst,
    IRInst* userInst);

} // namespace Slang
