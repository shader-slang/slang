// Regression test for https://github.com/shader-slang/slang/issues/11272.
//
// A `[Differentiable]` function that returns an interface (existential) type
// wraps the return type with a `no_diff` modifier. The semantic checker's
// `maybeOpenExistential` must look through that modifier so the existential
// is opened and the lowered IR contains the standard
// `extractExistential*` + `lookupWitness` dispatch idiom. Without that,
// `this_type(...)` leaks all the way to codegen — invalid HLSL on
// `-target hlsl` and an ICE on `-target spirv`.

//TEST:SIMPLE(filecheck=CHECK_HLSL): -target hlsl -stage compute -entry main
//TEST:SIMPLE(filecheck=CHECK_SPV): -target spirv-asm -stage compute -entry main

// The fix should ensure neither `this_type` (the unresolved IR placeholder)
// nor the interface dispatcher key escapes into the emitted HLSL.
// CHECK_HLSL-NOT: this_type
// CHECK_HLSL-NOT: IMaterialInstance_collect_properties
// CHECK_HLSL: void main(
// CHECK_HLSL-NOT: this_type
// CHECK_HLSL-NOT: IMaterialInstance_collect_properties
// CHECK_HLSL: return;
// A trailing negative block after the final positive `CHECK_HLSL` extends
// the scope to end-of-input, so an out-of-line dispatcher emitted after
// `main` (e.g. via a future regression) still fails the test.
// CHECK_HLSL-NOT: this_type
// CHECK_HLSL-NOT: IMaterialInstance_collect_properties

// SPIR-V should compile successfully (no ICE) and emit the dispatch chain
// fully constant-folded into a store of `(1, 2, 3)` into `out_buf`. Pinning
// the composite + store catches silent miscompilation that an
// `OpEntryPoint`-only check would miss (e.g. a future regression that
// produces a clean module but folds the call away to undef).
// CHECK_SPV: OpEntryPoint
// CHECK_SPV-DAG: %[[F1:[A-Za-z0-9_]+]] = OpConstant %float 1
// CHECK_SPV-DAG: %[[F2:[A-Za-z0-9_]+]] = OpConstant %float 2
// CHECK_SPV-DAG: %[[F3:[A-Za-z0-9_]+]] = OpConstant %float 3
// CHECK_SPV-DAG: %[[V:[A-Za-z0-9_]+]] = OpConstantComposite %v3float %[[F1]] %[[F2]] %[[F3]]
// CHECK_SPV: OpStore {{.*}} %[[V]]

public struct SurfaceInteraction { public float3 pos; }
public struct MaterialProperties { public float3 albedo; }

public interface IMaterialInstance
{
    MaterialProperties collect_properties(SurfaceInteraction si);
}

struct MyInst : IMaterialInstance
{
    MaterialProperties collect_properties(SurfaceInteraction si)
    {
        MaterialProperties p;
        p.albedo = si.pos;
        return p;
    }
}

[Differentiable]
IMaterialInstance get_inst(SurfaceInteraction si)
{
    return MyInst();
}

RWStructuredBuffer<float3> out_buf;

[shader("compute")]
[numthreads(1,1,1)]
void main()
{
    SurfaceInteraction si;
    si.pos = float3(1, 2, 3);
    out_buf[0] = get_inst(si).collect_properties(si).albedo;
}
