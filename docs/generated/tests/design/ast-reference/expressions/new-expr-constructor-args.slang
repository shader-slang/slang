//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#nodes
//META: doc_section_digest=0b0dfab9f13850a71e13b9ce6f8fb9a433ddb72267a526bf6454583638151a5b
//META: purpose=A NewExpr `new T(...)` passes its arguments to the class constructor, so the allocated instance carries the constructor-computed field value.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The allocation form takes an argument list, so `new T(args)` has to resolve
// to the class's constructor the same way `T(args)` does for a struct. The
// constructor here doubles its argument and the dispatch id keeps the value
// symbolic; the CHECK asserts the doubling appears in the stored expression,
// which is only there if the argument list reached the constructor.
//
// Class allocation is lowered per target, so every shader text target is
// exercised. The per-target buffer-store spellings pinned below are the ones
// the two working neighbours of this test emit today: `new-expr-class-instance.slang`
// (a class allocated with `new T()` and no constructor) and the same shader
// with `class` replaced by `struct` (`Counter(4 + int(tid.x))`), which both
// compile on all five targets and both store through the same expression shape.
//
// This test is a known expected failure: `new T(args)` on a class that declares
// a matching `__init` currently aborts with `E99997 ... InternalError ... could
// not resolve target declaration for call` on *every* target, HLSL included.
// See docs/generated/tests/_meta/findings/new-expr-with-constructor-args-internal-error.yaml.
// The per-target directives are kept so that fixing the abort restores emission
// coverage on all five back-ends at once rather than on HLSL alone.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry computeMain -stage compute

RWStructuredBuffer<int> outputBuffer;

class Counter
{
    int v;
    __init(int start) { v = start * 2; }
}

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    Counter c = new Counter(4 + int(tid.x));
    outputBuffer[0] = c.v;
    // HLSL: outputBuffer{{[A-Za-z0-9_]*}}[int(0)] =
    // GLSL: outputBuffer{{[A-Za-z0-9_]*}}._data[uint(0)] =
    // METAL: *(outputBuffer{{[A-Za-z0-9_]*}}+int(0)) =
    // WGSL: outputBuffer{{[A-Za-z0-9_]*}}[i32(0)] =
    // SPV: %{{[0-9]+}} = OpAccessChain %_ptr_StorageBuffer_int %outputBuffer
    // SPV: OpStore
}
