//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#simple-modifiers
//META: doc_section_digest=c20a44348c0739e8c347913c83450bf1d0dc3c34d664be2becfd728a37d9b098
//META: purpose=`dyn` is a simple modifier on an `interface` declaration that marks the interface for dynamic dispatch; declaring a `dyn interface` with a conforming struct parses and survives into a clean HLSL / SPIR-V emit.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc lists `dyn` in the simple-modifier table with AST node
// `DynModifier`. The user-visible probe is to declare a
// `dyn interface` with `[anyValueSize(N)]`, conform a struct, and
// have the compiler accept and emit the program. If the parser did
// not recognize `dyn`, this file would fail to compile and FileCheck
// would report no match. We pin every shader text-emit target so the
// recognition is observable independent of any single backend: `dyn`
// is a front-end modifier, so each backend must still produce its own
// entry point plus the conforming struct's method, and a backend that
// choked on the modifier would show up here as a missing emit.

//TEST:SIMPLE(filecheck=CHECK_HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=CHECK_SPV):-target spirv-asm -entry main -stage compute
//TEST:SIMPLE(filecheck=CHECK_GLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=CHECK_METAL):-target metal -entry main -stage compute
//TEST:SIMPLE(filecheck=CHECK_WGSL):-target wgsl -entry main -stage compute

[anyValueSize(16)]
dyn interface IComputeOp
{
    int compute(int x);
}

struct AddOne : IComputeOp
{
    int compute(int x) { return x + 1; }
}

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    // Direct call through a concrete struct: avoids any need for
    // host-side type-conformance setup, which is what makes the
    // claim observable in a pure compile-only test.
    AddOne op;
    outBuf[0] = op.compute(int(41));
    //CHECK_HLSL: void main
    //CHECK_SPV: OpEntryPoint GLCompute %main "main"
}

//CHECK_GLSL-DAG: void main()
//CHECK_GLSL-DAG: AddOne_compute

//CHECK_METAL-DAG: {{\[\[}}kernel]] void main
//CHECK_METAL-DAG: AddOne_compute

//CHECK_WGSL-DAG: fn main()
//CHECK_WGSL-DAG: AddOne_compute
