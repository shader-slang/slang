// A `__target_switch` with no case for the current target lowers to `missingReturn`, and a
// value-returning function that falls off its end is undefined behavior at runtime. When such a
// function would still be generated — here because the entry point reaches it through an
// interface-constrained generic, which the frontend capability check does not see through, or
// because `-ignore-capabilities` disabled that check — the compiler reports E41037 instead of
// emitting the empty body. Void functions keep compiling silently, because an empty body is a
// well-defined no-op for them, and a helper that only another target's case references is not
// diagnosed either, because DCE removes it before the report.

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

// Void functions with an unmatched switch keep compiling silently, and a helper that only
// another target's case references is removed by DCE before the report, so both stay clean.
//TEST:SIMPLE(filecheck=CLEAN):-target cuda -entry main_void -stage compute -ignore-capabilities
//TEST:SIMPLE(filecheck=CLEAN):-target cpp -entry main_void -stage compute -ignore-capabilities
//TEST:SIMPLE(filecheck=CLEAN):-target hlsl -entry main_void -stage compute -ignore-capabilities
//TEST:SIMPLE(filecheck=CLEAN):-target cuda -entry main_helper_user -stage compute -ignore-capabilities
//TEST:SIMPLE(filecheck=CLEAN):-target spirv -entry main_helper_user -stage compute -ignore-capabilities
//TEST:SIMPLE(filecheck=CLEAN):-target hlsl -entry main_helper_user -stage compute -ignore-capabilities

// `sincos` links its metal-only helper on every target, but the helper is only referenced from
// the `metal` case, so DCE must remove it everywhere else and no diagnostic may appear.
//TEST:SIMPLE(filecheck=CLEAN):-target cuda -entry main_sincos -stage compute -ignore-capabilities
//TEST:SIMPLE(filecheck=CLEAN):-target hlsl -entry main_sincos -stage compute -ignore-capabilities
//TEST:SIMPLE(filecheck=CLEAN):-target spirv -entry main_sincos -stage compute -ignore-capabilities

// The target named in E41037 is pinned per target family. PTX is generated from CUDA source and
// reports 'cuda'. `Buffer<T>.Load` has no `cpp` case, so using it there is the same E41037
// rather than a silent empty body.
//TEST:SIMPLE(filecheck=PIN_CUDA,exit-code=2):-target cuda -entry main_generic -stage compute
//TEST:SIMPLE(filecheck=PIN_CUDA,exit-code=2):-target ptx -entry main_generic -stage compute
//TEST:SIMPLE(filecheck=PIN_CPP,exit-code=2):-target cpp -entry main_generic -stage compute
//TEST:SIMPLE(filecheck=PIN_METAL,exit-code=2):-target metal -entry main_generic -stage compute
//TEST:SIMPLE(filecheck=PIN_SPIRV,exit-code=2):-target spirv -entry main_metal_only -stage compute -ignore-capabilities -disable-non-essential-validations
//TEST:SIMPLE(filecheck=PIN_LOAD_CPP,exit-code=2):-target cpp -entry main_typed_load -stage compute
// PIN_CUDA: error[E41037]: __target_switch has no case for the current target
// PIN_CUDA: 'positive_only_on_gpu' contains a __target_switch with no case for target 'cuda'
// PIN_CPP: 'positive_only_on_gpu' contains a __target_switch with no case for target 'cpp'
// PIN_METAL: 'positive_only_on_gpu' contains a __target_switch with no case for target 'metal'
// PIN_SPIRV: 'metalOnlyPositive' contains a __target_switch with no case for target 'spirv'
// PIN_LOAD_CPP: error[E41037]: __target_switch has no case for the current target
// PIN_LOAD_CPP: contains a __target_switch with no case for target 'cpp'
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
