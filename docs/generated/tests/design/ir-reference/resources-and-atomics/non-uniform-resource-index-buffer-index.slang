//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#nonuniformresourceindex
//META: doc_section_digest=859fd0b1dec68bc497f01fa8ca7af8a2359da36472f25c5cff70bfe0da629789
//META: purpose=`NonUniformResourceIndex(i)` is a no-op at the value level: the operand and result share the same type, so the surrounding indexing expression sees an integer of the same width.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the divergence marker is a value-level identity: it
// wraps an index and hands back a value of the same type, which is why
// the wrapped expression can be used directly as a buffer subscript.
// This file uses the signed-index shape so it differs from the unsigned
// form in `non-uniform-resource-index.slang`.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform RWStructuredBuffer<int> rwbuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int i = (int)tid.x;
    int nu = NonUniformResourceIndex(i);
    rwbuf[nu] = nu;
}

// IR-LABEL: func %main
// IR: nonUniformResourceIndex(%{{.*}})
