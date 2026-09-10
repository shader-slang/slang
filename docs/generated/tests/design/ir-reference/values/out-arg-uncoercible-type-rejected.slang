//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/ir-reference/values.md#out--inout-argument-casts
//META: doc_section_digest=4e540a522f9a0cc4f325c6d7e9f89f294e159f1a25d5b1bbbfbd0c481519dfe0
//META: purpose=Negative: an `out` argument whose type difference the l-value coercion does not cover (`int` passed to `out float`) is rejected at semantic check with E30047, so no `outImplicitCast` is ever lowered.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the upper boundary of the `outImplicitCast` surface. The
// coercion that lets an l-value argument round-trip through a temporary
// admits only two integer types of the same width, so `int` to `uint`
// works (out-inout-implicit-cast-int-uint.slang) but `int` to `float`
// does not. CHECK pins the l-value rejection at the argument's column
// plus the explanatory note, which together show the failure happens
// before lowering rather than producing a cast opcode.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv-asm -stage compute -entry main

uniform RWStructuredBuffer<int> buf;

[noinline]
void writeOut(out float o) { o = 5.0; }

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int a = int(tid.x);
    writeOut(a);
//CHECK:     ^ argument must be l-value
//CHECK:     ^ argument passed to parameter '0' must be l-value.
//CHECK:     ^ argument was implicitly cast from 'int' to 'float', and Slang does not support using an implicit cast as an l-value with this type
    buf[0] = a;
}
