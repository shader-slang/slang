//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:50:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/llm-generated/ir-reference/types.md#array-vs-unsizedarray
//META: doc_section_digest=3a34a70cd8512ec6be2ec7cc1f7561654dfdb9f02b11880c6bcb79480abf474b
//META: purpose=An unsized array (runtime extent) is typed UnsizedArray(elementType) in the IR dump.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's "`Array` vs `UnsizedArray`" note says `UnsizedArray`
// "omits the `elementCount` operand and represents an array of
// runtime extent — the common form on the GPU side of variable-
// size buffer access." A module-scope `uniform float tail[];`
// surfaces `UnsizedArray(Float)` in the dump on the `global_param`
// that holds it.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform float tail[];
uniform RWStructuredBuffer<float> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = tail[0];
}

// IR: let {{.*}}%tail{{.*}}: UnsizedArray(Float){{.*}}= global_param
