//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ir-reference/values.md#aggregate-constructors
//META: doc_section_digest=6dd59415374c5af65d07340c8540b315850c081483ba8c4243ef7e32cac24212
//META: purpose=The synthesized `__makeCoopVec<T, N>(expand each U)` variadic-pack core-module builtin (invoked by `CoopVec<T, N>(args...)` constructors) lowers to a `makeCoopVectorFromValuePack(...)` IR opcode in the generic-unspecialized form of `CoopVec.$init`.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The Aggregate constructors table lists
// `makeCoopVectorFromValuePack` with AST origin "(synthesized)".
// The user surface is still `CoopVec<T, N>(args...)`: the variadic
// `__init(each U args)` ctor body calls `__makeCoopVec<T, N>(expand
// (each args))`, which is the `__intrinsic_op($(kIROp_MakeCoopVector
// FromValuePack))` builtin in `hlsl.meta.slang`. The opcode appears
// inside the generic (unspecialized) `CoopVec.$init` body in the
// `-dump-ir` output; the specialized form collapses it to plain
// `makeCoopVector` after pack expansion, which is the sibling test.

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

// IR: makeCoopVectorFromValuePack(%{{[0-9a-zA-Z_]+}})
