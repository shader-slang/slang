//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-22T00:00:00+00:00
//META: source_commit=6e923e2c0fe3cae4e7cf40e25a96569df5b9f009
//META: doc_ref=docs/generated/design/ir-reference/values.md#aggregate-constructors
//META: doc_section_digest=3e412bb1b8df51b023821b69be767e4f040104ad482e9063949a7dd98d9eda38
//META: purpose=A `CoopVec<T, N>(args...)` cooperative-vector constructor call lowers — via the synthesized `CoopVec.$init` — to a `makeCoopVector(...)` IR opcode with one operand per element.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The Aggregate constructors table lists `makeCoopVector` with AST
// origin "`MakeCoopVectorExpr` (autodiff / coop-vec intrinsic)".
// A `CoopVec<int, 4>(ia, ib, ic, id)` constructor call surfaces a
// `makeCoopVector(...)` line inside the type-specialized
// `CoopVec.$init` function in the IR dump. Requires the
// `cooperative_vector` capability and so is anchored at nightly CI
// runners; the IR opcode is visible at LOWER-TO-IR regardless.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int ia;
uniform int ib;
uniform int ic;
uniform int id;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    let v = CoopVec<int, 4>(ia, ib, ic, id);
    buf[0] = v[0];
}

// IR-LABEL: func %CoopVecx5Fx24init
// IR: makeCoopVector(%{{[0-9a-zA-Z_]+}}, %{{[0-9a-zA-Z_]+}}, %{{[0-9a-zA-Z_]+}}, %{{[0-9a-zA-Z_]+}})
