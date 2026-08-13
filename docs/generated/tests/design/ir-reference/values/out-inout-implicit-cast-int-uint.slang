//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/ir-reference/values.md#out--inout-argument-casts
//META: doc_section_digest=4e540a522f9a0cc4f325c6d7e9f89f294e159f1a25d5b1bbbfbd0c481519dfe0
//META: purpose=Passing an `int` l-value to an `out uint` / `inout uint` parameter materializes `outImplicitCast` / `inOutImplicitCast` on the caller's address, whereas a same-type argument produces no cast at all.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the narrow condition under which the two l-value cast
// opcodes appear. `int` and `uint` are two integer types of the same
// width, which is exactly the pair the checker is willing to round-trip
// through a temporary, so passing an `int` variable to an `out uint` /
// `inout uint` parameter rewrites the argument into a cast whose
// operand is the caller's address. CHECK asserts the two opcodes take
// the `var` pointer `%a` / `%b` and produce a `Ptr(UInt)`, which is the
// pointer-in / pointer-out shape the catalog rows describe; the
// `same()` call with a matching `out int` argument in the same function
// gives the contrast case, and there the `var` pointer reaches the call
// with no cast in between.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform RWStructuredBuffer<int> buf;

[noinline]
void writeOut(out uint o) { o = 5u; }

[noinline]
void bumpInOut(inout uint io) { io = io + 1u; }

[noinline]
void same(out int o) { o = 9; }

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int a = int(tid.x);
    writeOut(a);
    int b = int(tid.y);
    bumpInOut(b);
    int c = int(tid.z);
    same(c);
    buf[0] = a + b + c;
}

// IR-LABEL: func %main
// IR: : Ptr(UInt)	= outImplicitCast(%a)
// IR: : Ptr(UInt)	= inOutImplicitCast(%b)
// IR: call %same(%c)
