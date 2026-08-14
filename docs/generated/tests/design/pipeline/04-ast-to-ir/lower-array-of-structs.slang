//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=1195a752790819415908e083ce9656a353d4764f2fab38cc5aa27d92f184c486
//META: purpose=A global fixed-size array of struct elements lowers with IR type `Array(%Struct, N : Int)`; subscripting the array uses `getElement` to recover the struct element value.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expansion: composition axis — array-of-structs. Paired with
// `lower-struct-of-arrays.slang`, this confirms the two
// compositions produce visibly different IR shapes (`Array(%S, N)`
// here vs the struct having `Array(...)` fields there).

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

struct Pair { int x; float y; }

uniform Pair aos[4];
uniform RWStructuredBuffer<float> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = float(aos[0].x) + aos[1].y;
}

// CHECK: [nameHint("aos")]
// CHECK: let {{.*}}aos{{.*}}: Array(%Pair, 4 : Int){{.*}}= global_param
