// A call passing an uninitialized texture triggers call specialization in
// `eliminateUndefinedOpaqueHandles`, which clones the callee while resource-selection
// sinking has split the callee's entry block into dispatch arms plus a merge block.
// The merge block must not be appended after the blocks that use values it defines,
// or the cloner wires those uses to the original callee's instructions and the
// deallocated original leaves dangling operands behind (shader-slang/slang#14646).

//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry main -stage compute -validate-ir
//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry main -stage compute -validate-ir -O0

Texture2D gA;
Texture2D gB;
RWStructuredBuffer<float4> gOut;
cbuffer C { uint gMode; }

// The runtime texture select sinks into a branch, and `w` is defined in the merge
// block but consumed by a later block.
void f(Texture2D unused)
{
    Texture2D t = gMode != 0 ? gA : gB;
    gOut[0] = t.Load(0);
    uint w = gMode * 3;
    if (gMode > 4)
        gOut[1] = w;
}

// The undefined texture argument forces the callee to be cloned by the caller.
// CHECK: %main = OpFunction
// The sunk select keeps its conditional branch in the specialized callee.
// CHECK: OpSelectionMerge
// The merge-block value must survive the clone with a live operand.
// CHECK: OpIMul
// CHECK: OpFunctionEnd
[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    Texture2D undef;
    f(undef);
}
