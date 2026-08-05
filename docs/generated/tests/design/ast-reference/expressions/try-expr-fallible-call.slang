//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#nodes
//META: doc_section_digest=b4aac875016d352a73eaff212c9886fb3c6956320e3406200a947b5679edef57
//META: purpose=Verifies that `try f(x)` on a `throws` function emits a tagged-result check so the caller can branch to its catch handler.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A `try` wraps a fallible call and turns its result into a tagged value that
// the caller tests before using. The emitted code is the observation point:
// the CHECKs assert a result-carrying struct is produced by the callee and
// that the caller branches on its tag field, which is exactly what the wrapper
// has to introduce. Names are matched loosely because locals are suffixed
// during emit.
//
// The tagged-result lowering is target-dependent, so every shader text target
// is exercised. HLSL, GLSL, Metal and WGSL all keep the `ResultType` struct in
// the emitted source and branch on its `tag` member. SPIR-V has no member
// syntax to match, so its CHECKs pin the equivalent instruction sequence
// instead: the `ResultType` struct type carries a member named "tag", the
// fallible callee returns that struct from an `OpFunctionCall`, and the caller
// pulls member 0 out with `OpCompositeExtract %bool` and feeds it to
// `OpBranchConditional` to reach the catch handler.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry computeMain -stage compute

RWStructuredBuffer<int> outputBuffer;

int mayThrow(int x) throws int
{
    if (x < 0) throw 1;
    return x * 2;
}

int caller(int x)
{
    do { return try mayThrow(x); }
    catch (int e) { return -e; }
}

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    outputBuffer[0] = caller(4 + int(tid.x));
    // HLSL: ResultType{{[A-Za-z0-9_]*}} {{[A-Za-z0-9_]+}} = mayThrow{{[A-Za-z0-9_]*}}(
    // HLSL: if({{[A-Za-z0-9_]+}}.tag{{[A-Za-z0-9_]*}})

    // GLSL: ResultType{{[A-Za-z0-9_]*}} {{[A-Za-z0-9_]+}} = mayThrow{{[A-Za-z0-9_]*}}(
    // GLSL: if({{[A-Za-z0-9_]+}}.tag{{[A-Za-z0-9_]*}})

    // METAL: ResultType{{[A-Za-z0-9_]*}} {{[A-Za-z0-9_]+}} = mayThrow{{[A-Za-z0-9_]*}}(
    // METAL: if({{[A-Za-z0-9_]+}}.tag{{[A-Za-z0-9_]*}})

    // WGSL: {{[A-Za-z0-9_]+}} : ResultType{{[A-Za-z0-9_]*}} = mayThrow{{[A-Za-z0-9_]*}}(
    // WGSL: if({{[A-Za-z0-9_]+}}.tag{{[A-Za-z0-9_]*}})

    // SPV: OpMemberName %ResultType{{[A-Za-z0-9_]*}} 0 "tag"
    // SPV: %{{[0-9]+}} = OpFunctionCall %ResultType{{[A-Za-z0-9_]*}} %mayThrow
    // SPV: %{{[0-9]+}} = OpCompositeExtract %bool %{{[0-9]+}} 0
    // SPV: OpBranchConditional
}
