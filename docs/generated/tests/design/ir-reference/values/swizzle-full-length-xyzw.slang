//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/values.md#swizzle-swizzleset-swizzledstore
//META: doc_section_digest=b3a4828968ba320e73bd253425b6f464bf34ceea7d61ab6e1933e1f91bcb19a9
//META: purpose=Boundary: full-length swizzle `.xyzw` on a `float4` lowers to `swizzle(%v, 0, 1, 2, 3)` — the M=N edge of the swizzle width.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The upper-equal edge: M = N = 4. The IR carries the four literal
// indices verbatim on the operand list of the `swizzle` opcode.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform float4 v;
uniform RWStructuredBuffer<float4> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = v.xyzw;
}

// IR-LABEL: func %main
// IR: swizzle(%v, 0 : Int, 1 : Int, 2 : Int, 3 : Int)
