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
//TEST:INTERPRET(filecheck=CHECK_INTERP): -entry interpretMain

// The fix should ensure `this_type` (the unresolved IR placeholder) does not
// escape into the emitted HLSL.
// CHECK_HLSL-NOT: this_type
// CHECK_HLSL: void main(
// CHECK_HLSL-NOT: this_type
// CHECK_HLSL: return;
// A trailing negative block after the final positive `CHECK_HLSL` extends
// the scope to end-of-input, so a future regression emitted after `main`
// still fails the test.
// CHECK_HLSL-NOT: this_type

// SPIR-V should compile successfully (no ICE) and emit the dispatch chain
// fully constant-folded into observable stores. Pinning the composites +
// stores catches silent miscompilation that an `OpEntryPoint`-only check
// would miss (e.g. a future regression that produces a clean module but
// folds the call away to undef).
// CHECK_SPV: OpEntryPoint
// CHECK_SPV-DAG: %[[F1:[A-Za-z0-9_]+]] = OpConstant %float 1
// CHECK_SPV-DAG: %[[F2:[A-Za-z0-9_]+]] = OpConstant %float 2
// CHECK_SPV-DAG: %[[F3:[A-Za-z0-9_]+]] = OpConstant %float 3
// CHECK_SPV-DAG: %[[V123:[A-Za-z0-9_]+]] = OpConstantComposite %v3float %[[F1]] %[[F2]] %[[F3]]
// CHECK_SPV-DAG: %[[V111:[A-Za-z0-9_]+]] = OpConstantComposite %v3float %[[F1]] %[[F1]] %[[F1]]
// CHECK_SPV: OpStore {{.*}} %[[V123]]
// CHECK_SPV: OpStore {{.*}} %[[V123]]
// CHECK_SPV: OpStore {{.*}} %[[V111]]

// CHECK_INTERP: matched=1
// CHECK_INTERP: cast=1 2 3
// CHECK_INTERP: member=1 2 3

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

// These helpers exercise the same modified-interface return type through
// `is`/`as`, not just through member lookup.
bool is_my_inst(SurfaceInteraction si)
{
    return get_inst(si) is MyInst;
}

float3 cast_my_inst(SurfaceInteraction si)
{
    let castResult = get_inst(si) as MyInst;
    if (castResult.hasValue)
        return castResult.value.collect_properties(si).albedo;
    return float3(0);
}

RWStructuredBuffer<float3> out_buf;

void interpretMain()
{
    SurfaceInteraction si;
    si.pos = float3(1, 2, 3);

    bool matched = is_my_inst(si);
    float3 castResult = cast_my_inst(si);
    float3 memberResult = get_inst(si).collect_properties(si).albedo;

    printf("matched=%d\n", (int)matched);
    printf("cast=%.0f %.0f %.0f\n", castResult.x, castResult.y, castResult.z);
    printf("member=%.0f %.0f %.0f\n", memberResult.x, memberResult.y, memberResult.z);
}

[shader("compute")]
[numthreads(1,1,1)]
void main()
{
    SurfaceInteraction si;
    si.pos = float3(1, 2, 3);
    bool matched = is_my_inst(si);
    float3 castResult = cast_my_inst(si);
    float3 memberResult = get_inst(si).collect_properties(si).albedo;

    out_buf[0] = memberResult;
    out_buf[1] = castResult;
    out_buf[2] = matched ? float3(1, 1, 1) : float3(0);
}
