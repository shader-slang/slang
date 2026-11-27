//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -emit-spirv-directly -entry main -stage compute -O0
//TEST:SIMPLE(filecheck=CHECK1): -target spirv-asm -emit-spirv-directly -entry main -stage compute -O1
//TEST:SIMPLE(filecheck=CHECK1): -target spirv-asm -emit-spirv-directly -entry main -stage compute -O2
//TEST:SIMPLE(filecheck=CHECK1): -target spirv-asm -emit-spirv-directly -entry main -stage compute -O3
//TEST:SIMPLE(filecheck=CHECKM): -target spirv-asm -emit-spirv-directly -entry main1 -entry main2 -O0
//TEST:SIMPLE(filecheck=CHECKV): -target spirv-asm -emit-spirv-directly -entry mainVec -O0

// A `static const` array read with a dynamic index must be materialized once per
// module, not copied into a fresh function-local array at every read site
// (shader-slang/slang#14739). Inlining multiplies read sites, so per-read-site
// copies scale with the number of inlined calls.
//
// At `-O0` the table is one Private global, stored into exactly once before
// the first read, and every read is an access chain into it. At higher
// optimization levels the downstream `spirv-opt` pipeline inlines the
// reads and then demotes that global (`private-to-local`) to one
// function-scope copy in the entry point -- the same end state a `static`
// non-const array reaches -- so the array type still has exactly one variable
// instead of one per read site.
//
// The `mainVec` leg covers a mixed-operand module-scope vector constructor
// (`float4(1.0, float2(2.0, 3.0), 4.0)`), whose general-form `MakeVector` the
// SPIR-V legalizer normalizes in the same pass: the stored composite must have
// one constituent per component, proving the initializer store stayed in the
// use-def chain across that rewrite.
//
// The `main1`/`main2` leg compiles two entry points that read the table
// through a shared helper, pinning that the single Private global receives
// exactly one store per entry point.
//
// The precompiled-library companion test
// (tests/library/precompiled-spirv-static-table.slang) covers the
// entry-point-less shape, where the shared global has no store site and the
// read keeps its self-contained per-read-site copy.

static const float kTable[8] = {0.0, 0.125, 0.25, 0.375, 0.5, 0.625, 0.75, 0.875};

float lookup(float x)
{
    int i = clamp(int(x), 0, 7);
    return kTable[i] + kTable[min(i + 1, 7)];
}

RWStructuredBuffer<float> gOut;

[shader("compute")]
[numthreads(8, 1, 1)]
void main(uint3 id : SV_DispatchThreadID)
{
    float acc = gOut[id.x];
    acc += lookup(acc + float(id.x));
    acc += lookup(acc * 2.0 + float(id.x));
    acc += lookup(acc * 3.0 + float(id.x));
    acc += lookup(acc * 4.0 + float(id.x));
    gOut[id.x] = acc;
}

[shader("compute")]
[numthreads(8, 1, 1)]
void main1(uint3 id : SV_DispatchThreadID)
{
    gOut[id.x] = lookup(float(id.x));
}

[shader("compute")]
[numthreads(8, 1, 1)]
void main2(uint3 id : SV_DispatchThreadID)
{
    gOut[id.x] = lookup(float(id.x)) * 2.0;
}

static const float4 kVec = float4(1.0, float2(2.0, 3.0), 4.0);

[shader("compute")]
[numthreads(1, 1, 1)]
void mainVec(uint3 id : SV_DispatchThreadID)
{
    gOut[0] = kVec[id.x % 4];
}

// At `-O0` the array is one Private global, stored into exactly once before
// the first read, with no function-scope copy anywhere in the module. The
// interstitial CHECK-NOTs cover the global section and the entry-point tops,
// where SPIR-V places every remaining variable declaration.
// CHECK: %[[TABLE:[a-zA-Z0-9_]+]] = OpVariable %_ptr_Private__arr_float_int_8 Private
// CHECK-NOT: OpVariable %_ptr_Private__arr_float_int_8 Private
// CHECK-NOT: OpVariable %_ptr_Function__arr_float_int_8 Function
// CHECK: OpStore %[[TABLE]]
// CHECK-NOT: OpStore %[[TABLE]]
// CHECK: OpAccessChain %_ptr_Private_float %[[TABLE]]
// CHECK-NOT: OpStore %[[TABLE]]
// CHECK-NOT: OpVariable %_ptr_Function__arr_float_int_8 Function

// After the `-O1`..`-O3` downstream inlining and `private-to-local`
// demotion, the array has exactly one variable, stored into once, and all
// reads share it. (The `-O2` and `-O3` legs reuse these patterns: the
// downstream spirv-opt presets differ between optimization levels, but every
// level above `-O0` runs inlining and `private-to-local`, and all three legs
// produce the same demoted single-copy shape.)
// CHECK1: %[[T1:[a-zA-Z0-9_]+]] = OpVariable %_ptr_Function__arr_float_int_8 Function
// CHECK1-NOT: OpVariable %_ptr_Function__arr_float_int_8 Function
// CHECK1: OpStore %[[T1]]
// CHECK1-NOT: OpStore %[[T1]]
// CHECK1: OpAccessChain %_ptr_Function_float %[[T1]]
// CHECK1-NOT: OpStore %[[T1]]
// CHECK1-NOT: OpVariable %_ptr_Function__arr_float_int_8 Function

// Two entry points share one Private global; each entry point's body stores
// the table into it exactly once.
// CHECKM: %[[TM:[a-zA-Z0-9_]+]] = OpVariable %_ptr_Private__arr_float_int_8 Private
// CHECKM-NOT: OpVariable %_ptr_Private__arr_float_int_8 Private
// CHECKM-NOT: OpVariable %_ptr_Function__arr_float_int_8 Function
// CHECKM: OpStore %[[TM]]
// CHECKM: OpAccessChain %_ptr_Private_float %[[TM]]
// CHECKM-NOT: OpVariable %_ptr_Function__arr_float_int_8 Function
// CHECKM: OpStore %[[TM]]
// CHECKM-NOT: OpStore %[[TM]]

// The normalized mixed-operand vector constructor keeps one constituent per
// component when it reaches the Private global's initializer store.
// CHECKV: %[[VEC:[a-zA-Z0-9_]+]] = OpVariable %_ptr_Private_v4float Private
// CHECKV: {{%[a-zA-Z0-9_]+}} = OpCompositeConstruct %v4float %float_1 %float_2 %float_3 %float_4
// CHECKV: OpStore %[[VEC]]
// CHECKV-NOT: OpStore %[[VEC]]
// CHECKV: OpAccessChain %_ptr_Private_float %[[VEC]]
