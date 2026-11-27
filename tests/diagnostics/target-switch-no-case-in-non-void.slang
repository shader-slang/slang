// A `__target_switch` with no case for the current target lowers to `missingReturn`, and a
// value-returning function that falls off its end is undefined behavior at runtime. When such a
// function would still be generated — here because the entry point reaches it through an
// interface-constrained generic, which the frontend capability check does not see through, or
// because `-ignore-capabilities` disabled that check — the compiler reports E41037 instead of
// emitting the empty body. On targets that tolerate a missing return, a case-less switch in a void
// function keeps compiling silently, because an empty body is a well-defined no-op there; the
// targets that reject a missing return keep their pre-existing E41009 for any `missingReturn`, and
// the `slangvm` target keeps its link-time E41011, because its empty capability profile matches no
// case. A helper that only another target's case references is not diagnosed either, because DCE
// removes it before the report.

//DIAGNOSTIC_TEST:SIMPLE(diag=GENERIC):-target cuda -entry main_generic -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=GENERIC):-target cpp -entry main_generic -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=GENERIC):-target metal -entry main_generic -stage compute

//DIAGNOSTIC_TEST:SIMPLE(diag=IGNORE_CAPS):-target cuda -entry main_direct -stage compute -ignore-capabilities
//DIAGNOSTIC_TEST:SIMPLE(diag=IGNORE_CAPS):-target cpp -entry main_direct -stage compute -ignore-capabilities

// Without -ignore-capabilities the frontend capability check still rejects the direct call, so
// nothing changes for it.
//DIAGNOSTIC_TEST:SIMPLE(diag=DIRECT):-target cuda -entry main_direct -stage compute

// A non-void function whose only case is for another target. On targets that reject missing
// returns, E41037 is also reported when `-disable-non-essential-validations` skipped the early
// E41009 check, because the report runs late in the emit pipeline, after that gate.
//DIAGNOSTIC_TEST:SIMPLE(diag=METALONLY):-target cuda -entry main_metal_only -stage compute -ignore-capabilities
//DIAGNOSTIC_TEST:SIMPLE(diag=METALONLY):-target hlsl -entry main_metal_only -stage compute -ignore-capabilities
//DIAGNOSTIC_TEST:SIMPLE(diag=METALONLY):-target spirv -entry main_metal_only -stage compute -ignore-capabilities -disable-non-essential-validations

// With the early check intact, the strict targets keep their pre-existing E41009 for the same
// function — never E41037.
//TEST:SIMPLE(filecheck=PIN_E41009_GLSL,exit-code=2):-target glsl -entry main_metal_only -stage compute -ignore-capabilities
//TEST:SIMPLE(filecheck=PIN_E41009_WGSL,exit-code=2):-target wgsl -entry main_metal_only -stage compute -ignore-capabilities

// The `slangvm` target has an empty capability profile, so the same function is rejected at link
// time with E41011 and the late report never runs for it.
//TEST:SIMPLE(filecheck=PIN_SLANGVM,exit-code=2):-target slangvm -entry main_metal_only -stage compute

// A matching case is the positive control: on metal the same function compiles cleanly.
//TEST:SIMPLE(filecheck=CLEAN):-target metal -entry main_metal_only -stage compute

// A case-less switch in a void function: silent on the missing-return-tolerant targets, E41009 on
// the strict ones with the early check intact, and silent on GLSL when that check was skipped,
// because the void function carries no mark for the late report.
//TEST:SIMPLE(filecheck=CLEAN):-target cuda -entry main_void -stage compute -ignore-capabilities
//TEST:SIMPLE(filecheck=CLEAN):-target cpp -entry main_void -stage compute -ignore-capabilities
//TEST:SIMPLE(filecheck=CLEAN):-target hlsl -entry main_void -stage compute -ignore-capabilities
//TEST:SIMPLE(filecheck=PIN_VOID_E41009,exit-code=2):-target spirv -entry main_void_metal_only -stage compute -ignore-capabilities
//TEST:SIMPLE(filecheck=CLEAN):-target glsl -entry main_void_metal_only -stage compute -ignore-capabilities -disable-non-essential-validations

// A helper that only another target's case references is removed by DCE before the report.
//TEST:SIMPLE(filecheck=CLEAN):-target cuda -entry main_helper_user -stage compute -ignore-capabilities
//TEST:SIMPLE(filecheck=CLEAN):-target spirv -entry main_helper_user -stage compute -ignore-capabilities
//TEST:SIMPLE(filecheck=CLEAN):-target hlsl -entry main_helper_user -stage compute -ignore-capabilities

// `sincos` links its metal-only helper on every target, but the helper is only referenced from
// the `metal` case, so DCE must remove it everywhere else and no diagnostic may appear.
//TEST:SIMPLE(filecheck=CLEAN):-target cuda -entry main_sincos -stage compute -ignore-capabilities
//TEST:SIMPLE(filecheck=CLEAN):-target hlsl -entry main_sincos -stage compute -ignore-capabilities
//TEST:SIMPLE(filecheck=CLEAN):-target spirv -entry main_sincos -stage compute -ignore-capabilities

// The target named in E41037 is pinned per target family. PTX is generated from CUDA source and
// reports 'cuda'. `Buffer<T>.Load` has no `cpp` case and is force-inlined, so its name reaches the
// report only through the decoration — pinning it proves the inlined-away-name mechanism.
//TEST:SIMPLE(filecheck=PIN_CUDA,exit-code=2):-target cuda -entry main_generic -stage compute
//TEST:SIMPLE(filecheck=PIN_CUDA,exit-code=2):-target ptx -entry main_generic -stage compute
//TEST:SIMPLE(filecheck=PIN_CPP,exit-code=2):-target cpp -entry main_generic -stage compute
//TEST:SIMPLE(filecheck=PIN_METAL,exit-code=2):-target metal -entry main_generic -stage compute
//TEST:SIMPLE(filecheck=PIN_SPIRV,exit-code=2):-target spirv -entry main_metal_only -stage compute -ignore-capabilities -disable-non-essential-validations
//TEST:SIMPLE(filecheck=PIN_GLSL,exit-code=2):-target glsl -entry main_metal_only -stage compute -ignore-capabilities -disable-non-essential-validations
//TEST:SIMPLE(filecheck=PIN_WGSL,exit-code=2):-target wgsl -entry main_metal_only -stage compute -ignore-capabilities -disable-non-essential-validations
//TEST:SIMPLE(filecheck=PIN_HLSL,exit-code=2):-target hlsl -entry main_metal_only -stage compute -ignore-capabilities
//TEST:SIMPLE(filecheck=PIN_LLVM,exit-code=2):-target llvm-ir -entry main_metal_only -stage compute -ignore-capabilities
//TEST:SIMPLE(filecheck=PIN_LOAD_CPP,exit-code=2):-target cpp -entry main_typed_load -stage compute
// PIN_CUDA: error[E41037]: __target_switch has no case for the current target
// PIN_CUDA: 'positive_only_on_gpu' contains a __target_switch with no case for target 'cuda'
// PIN_CPP: 'positive_only_on_gpu' contains a __target_switch with no case for target 'cpp'
// PIN_METAL: 'positive_only_on_gpu' contains a __target_switch with no case for target 'metal'
// PIN_SPIRV: 'metalOnlyPositive' contains a __target_switch with no case for target 'spirv'
// PIN_GLSL: 'metalOnlyPositive' contains a __target_switch with no case for target 'glsl'
// PIN_WGSL: 'metalOnlyPositive' contains a __target_switch with no case for target 'wgsl'
// PIN_HLSL: 'metalOnlyPositive' contains a __target_switch with no case for target 'hlsl'
// PIN_LLVM: 'metalOnlyPositive' contains a __target_switch with no case for target 'llvm-host-ir'
// One E41037 per source switch, even though this lane runs at -O0 (slang-test's default), where
// the callee is inlined and the inlined copy carries the same mark.
// PIN_LOAD_CPP-COUNT-1: error[E41037]: __target_switch has no case for the current target
// PIN_LOAD_CPP: '_Texture.Load' contains a __target_switch with no case for target 'cpp'
// PIN_E41009_GLSL: non-void function must return in all cases for target 'glsl'
// PIN_E41009_GLSL-NOT: E41037
// PIN_E41009_WGSL: non-void function must return in all cases for target 'wgsl'
// PIN_E41009_WGSL-NOT: E41037
// PIN_SLANGVM: error[E41011]: __target_switch has no compatible target
// PIN_VOID_E41009: non-void function must return in all cases for target 'spirv'
// PIN_VOID_E41009-NOT: E41037
// CLEAN: result code = 0
// CLEAN-NOT: error[

[require(hlsl_spirv)]
bool positive_only_on_gpu(int x)
{
    __target_switch
/*GENERIC:
    ^^^^^^^^^^^^^^^ E41037
    ^^^^^^^^^^^^^^^ 'positive_only_on_gpu' contains a __target_switch with no case for target
*/
/*IGNORE_CAPS:
    ^^^^^^^^^^^^^^^ E41037
    ^^^^^^^^^^^^^^^ 'positive_only_on_gpu' contains a __target_switch with no case for target
*/
/*DIRECT:
    ^^^^^^^^^^^^^^^ see using of 'statement'
*/
    {
    case hlsl:
        __intrinsic_asm "($0 > 0)";
    case spirv:
        return x > 0;
    }
}

interface IQuery
{
    bool query(int x);
}

struct GpuQuery : IQuery
{
    bool query(int x) { return positive_only_on_gpu(x); }
//GENERIC:                                         ^ see call to 'positive_only_on_gpu'
}

bool run_generic<Q : IQuery>(Q q, int x)
{
    return q.query(x);
//GENERIC:        ^ see call to 'GpuQuery.query'
}

RWStructuredBuffer<uint> g_out;

[shader("compute")]
[numthreads(1, 1, 1)]
void main_direct(uint3 tid : SV_DispatchThreadID)
/*DIRECT:
     ^^^^^^^^^^^ unavailable features in entry point
     ^^^^^^^^^^^ entrypoint 'main_direct' uses features that are not available in 'compute' stage for 'cuda' compilation target.
*/
{
    g_out[0] = positive_only_on_gpu(int(tid.x) + 1) ? 1u : 0u;
//IGNORE_CAPS:                     ^ see call to 'positive_only_on_gpu'
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main_generic(uint3 tid : SV_DispatchThreadID)
{
    GpuQuery q;
    g_out[0] = run_generic(q, int(tid.x) + 1) ? 1u : 0u;
//GENERIC:                ^ see call to 'run_generic'
}

bool metalOnlyPositive(int x)
{
    __target_switch
/*METALONLY:
    ^^^^^^^^^^^^^^^ E41037
    ^^^^^^^^^^^^^^^ 'metalOnlyPositive' contains a __target_switch with no case for target
*/
    {
    case metal:
        return x > 0;
    }
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main_metal_only(uint3 tid : SV_DispatchThreadID)
{
    g_out[0] = metalOnlyPositive(int(tid.x)) ? 1u : 0u;
//METALONLY:                    ^ see call to 'metalOnlyPositive'
}

void setToTwo(inout float x)
{
    __target_switch
    {
    case hlsl:
        __intrinsic_asm "($0 = 2)";
    case spirv:
        x = 2;
    }
}

void metalOnlyClear(inout float x)
{
    __target_switch
    {
    case metal:
        x = 0;
    }
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main_void_metal_only(uint3 tid : SV_DispatchThreadID)
{
    float v = 1;
    metalOnlyClear(v);
    g_out[0] = uint(v);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main_void(uint3 tid : SV_DispatchThreadID)
{
    float v = 1;
    setToTwo(v);
    g_out[0] = uint(v);
}

float metalOnlyHelper(float x)
{
    __target_switch
    {
    case metal:
        return x;
    }
}

float usesHelperOnOtherTargets(float x)
{
    __target_switch
    {
    case metal:
        return metalOnlyHelper(x);
    case hlsl:
        return x;
    case spirv:
        return x;
    case cuda:
        return x;
    case cpp:
        return x;
    }
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main_helper_user(uint3 tid : SV_DispatchThreadID)
{
    g_out[0] = uint(usesHelperOnOtherTargets(float(tid.x)));
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main_sincos(uint3 tid : SV_DispatchThreadID)
{
    float s;
    float c;
    sincos(float(tid.x) + 0.5, s, c);
    g_out[0] = uint(s + c);
}

Buffer<float4> typedBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main_typed_load(uint3 tid : SV_DispatchThreadID)
{
    g_out[0] = uint(typedBuf[0].x);
}
