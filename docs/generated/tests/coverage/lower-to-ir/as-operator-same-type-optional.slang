//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Verifies `v as T` where `T` is already `v`'s static type builds an always-present `Optional<T>` and folds away to the value itself.
//META: intent=characterization
//META: covers=source/slang/slang-lower-to-ir.cpp
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Pins the payload-carrying arm of `visitMakeOptionalExpr`. When the checker can
// already prove `v`'s type is a subtype of the `as` right-hand side it rewrites
// `v as T` into a `MakeOptionalExpr` that carries the value (rather than the
// runtime type-test form), and lowering emits `makeOptionalValue`. The
// consequence a user sees is that no type test survives: the emitted HLSL stores
// the struct field straight into the buffer, and the SPIR-V has no OpSelect or
// branch between the add and the store.
//TEST:COMPARE_COMPUTE(filecheck-buffer=CPU):-cpu -output-using-type
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -entry computeMain -stage compute

struct S
{
    int v;
}

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    S s;
    s.v = int(tid.x) + 7;
    Optional<S> o = s as S;
    outputBuffer[0] = o.hasValue ? o.value.v : -1;
}

// The single dispatched thread has tid.x == 0, so the payload is 7 and the
// optional is statically known to hold it.
//CPU: 7

// The whole optional collapses: the field is stored directly, with no
// hasValue test left in the emitted code.
//HLSL: outputBuffer_{{[0-9]+}}[int(0)] = s_{{[0-9]+}}.v_{{[0-9]+}};
//HLSL-NOT: hasValue

//SPV-NOT: OpSelect
