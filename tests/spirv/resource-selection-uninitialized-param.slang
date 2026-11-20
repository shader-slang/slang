// A call passing an uninitialized texture triggers call specialization in
// `eliminateUndefinedOpaqueHandles`, which clones the callee after resource selection has
// split the callee's entry block into dispatch arms plus a merge block. The merge block
// must not be appended after the blocks that use values it defines, or the cloner wires
// those uses to the original callee's instructions and the deallocated original leaves
// dangling operands behind (shader-slang/slang#14646).
//
// The regression signal is the compile itself: without the fix the compiler crashes
// (SIGSEGV in Release; "def must be ancestor of use" from -validate-ir in Debug). A fixed
// compile still emits warning E41016 for the uninitialized argument, which is expected.
// The CHECK lines pin the emitted structure so the trigger shape cannot silently
// disappear: the -O0 runs keep the specialized callee visible as a separate function,
// while the default runs inline it into the entry point, so the two levels get their
// own labels.

//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry main -stage compute -validate-ir
//TEST:SIMPLE(filecheck=CHECK_UNOPT):-target spirv -entry main -stage compute -validate-ir -O0

// The switch variants drive the same pass through its multi-arm dispatch (three texture
// alternatives behind two conditions, so no single two-way selector exists), and the
// merge value is consumed by two different later blocks.
//TEST:SIMPLE(filecheck=CHECK_SWITCH):-target spirv -entry mainSwitch -stage compute -validate-ir
//TEST:SIMPLE(filecheck=CHECK_SWITCH_UNOPT):-target spirv -entry mainSwitch -stage compute -validate-ir -O0

Texture2D gA;
Texture2D gB;
Texture2D gC;
RWStructuredBuffer<float4> gOut;
cbuffer C { uint gMode; }

// The runtime texture select sinks into a two-arm branch, and `w` is defined in the merge
// block but consumed by a later block.
void f(Texture2D unused)
{
    Texture2D t = gMode != 0 ? gA : gB;
    gOut[0] = t.Load(0);
    uint w = gMode * 3;
    if (gMode > 4)
        gOut[1] = w;
}

// CHECK: %main = OpFunction
// The sunk select leaves two texture fetches merged by a phi inside the entry point.
// CHECK: OpImageFetch
// CHECK: OpImageFetch
// CHECK: OpPhi %v4float
// CHECK: %{{[^ ]+}} = OpIMul %uint %{{[^ ]+}} %uint_3
// CHECK: OpSelectionMerge
// CHECK: OpFunctionEnd

// CHECK_UNOPT: %main = OpFunction
// The undefined handle forces the callee to be cloned, so the call targets a specialized
// function that must still contain the sunk dispatch and a live `w`.
// CHECK_UNOPT: OpFunctionCall %void %[[F:[^ ]+]]
// CHECK_UNOPT: %[[F]] = OpFunction
// CHECK_UNOPT: OpImageFetch
// CHECK_UNOPT: OpImageFetch
// CHECK_UNOPT: %[[W:[^ ]+]] = OpIMul %uint %{{[^ ]+}} %uint_3
// CHECK_UNOPT: OpCompositeConstruct %v4uint %[[W]] %[[W]] %[[W]] %[[W]]
// CHECK_UNOPT: OpFunctionEnd

// A three-way select gives the dispatch three alternatives and no single condition.
void g(Texture2D unused)
{
    Texture2D t = gMode == 0 ? gA : (gMode == 1 ? gB : gC);
    gOut[0] = t.Load(0);
    uint w = gMode * 3;
    if (gMode > 4)
        gOut[1] = w;
    else
        gOut[2] = w * 2;
}

// CHECK_SWITCH: %mainSwitch = OpFunction
// CHECK_SWITCH: OpSwitch %{{[^ ]+}} %{{[^ ]+}} 0 %{{[^ ]+}} 1 %{{[^ ]+}}
// CHECK_SWITCH: OpImageFetch
// CHECK_SWITCH: OpImageFetch
// CHECK_SWITCH: OpImageFetch
// CHECK_SWITCH: OpIMul %uint %{{[^ ]+}} %uint_3
// CHECK_SWITCH: OpFunctionEnd

// CHECK_SWITCH_UNOPT: %mainSwitch = OpFunction
// The clone of the multi-arm dispatch must keep every arm reachable and both later-block
// uses of `w` wired to the merge-block definition.
// CHECK_SWITCH_UNOPT: OpFunctionCall %void %[[G:[^ ]+]]
// CHECK_SWITCH_UNOPT: %[[G]] = OpFunction
// CHECK_SWITCH_UNOPT: OpSwitch %{{[^ ]+}} %{{[^ ]+}} 0 %{{[^ ]+}} 1 %{{[^ ]+}}
// CHECK_SWITCH_UNOPT: OpImageFetch
// CHECK_SWITCH_UNOPT: OpImageFetch
// CHECK_SWITCH_UNOPT: OpImageFetch
// CHECK_SWITCH_UNOPT: %[[W2:[^ ]+]] = OpIMul %uint %{{[^ ]+}} %uint_3
// CHECK_SWITCH_UNOPT: OpCompositeConstruct %v4uint %[[W2]] %[[W2]] %[[W2]] %[[W2]]
// CHECK_SWITCH_UNOPT: OpIMul %uint %[[W2]] %uint_2
// CHECK_SWITCH_UNOPT: OpFunctionEnd

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    Texture2D undef;
    f(undef);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void mainSwitch()
{
    Texture2D undef;
    g(undef);
}
