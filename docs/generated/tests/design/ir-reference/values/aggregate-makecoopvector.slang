//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ir-reference/values.md#aggregate-constructors
//META: doc_section_digest=6dd59415374c5af65d07340c8540b315850c081483ba8c4243ef7e32cac24212
//META: purpose=A specialized `CoopVec<T, N>(args...)` constructor call collapses, after value-pack expansion, to a `makeCoopVector(...)` IR opcode with one operand per element.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The Aggregate constructors table lists `makeCoopVector` with AST
// origin "`MakeCoopVectorExpr` (autodiff / coop-vec intrinsic)". The
// variadic ctor body calls the `makeCoopVectorFromValuePack` builtin
// (see the sibling `aggregate-makecoopvectorfromvaluepack.slang`), but
// once `CoopVec<int, 4>(ia, ib, ic, id)` is type-specialized the pack
// expands and the opcode collapses to a per-element `makeCoopVector`
// with one operand per element. We assert that collapsed form directly
// (the surrounding init function is generically named `$init`, so the
// opcode — not a function label — is the stable anchor). Requires the
// `cooperative_vector` capability; the IR opcode is visible at
// LOWER-TO-IR regardless.

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

// IR: makeCoopVector(%{{[0-9a-zA-Z_]+}}, %{{[0-9a-zA-Z_]+}}, %{{[0-9a-zA-Z_]+}}, %{{[0-9a-zA-Z_]+}})
