//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/ir-reference/metadata.md#debugline
//META: doc_section_digest=eb5cc6b339fed6275ffa177c8c0c47a41583954b25feb65903b85fa94849df4a
//META: purpose=Lowering emits separate DebugLine markers for a for statement's condition and its increment, each landing in the block that evaluates it.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a `for` is not marked as one statement but as its parts, which
// is what lets a debugger stop separately on the test and on the step. Both
// sub-expressions are on the same source line, so the markers can only be told
// apart by their column range -- and they land in different blocks, the
// condition ahead of the `ifElse` that leaves the loop and the increment in the
// continue block. The CHECKs anchor at `func %main` so no core-module function
// can satisfy them, then require the two column ranges in that order.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute -g1

uniform RWStructuredBuffer<float> gOut;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    float acc = 0.0;
    for (uint i = 0; i < tid.x; ++i)
    {
        acc += float(i);
    }
    gOut[0] = acc;
}

// CHECK: {{^}}func %main {{.*}}Func(Void,
// The condition `i < tid.x` starts at column 24 of the `for` line; the marker
// precedes the branch that decides whether to leave the loop.
// CHECK: DebugLine(%{{[0-9]+}}, 28 : UInt, 28 : UInt, 24 : UInt, 25 : UInt)
// CHECK: ifElse(
// The increment `++i` starts at column 33 of the same line and is marked in
// the loop's continue block, after the body.
// CHECK: DebugLine(%{{[0-9]+}}, 28 : UInt, 28 : UInt, 33 : UInt, 34 : UInt)
