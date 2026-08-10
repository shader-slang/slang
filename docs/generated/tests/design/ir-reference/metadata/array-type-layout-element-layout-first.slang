//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/metadata.md#layout-family
//META: doc_section_digest=8133b77a147a20aa12fcc20641167e9fc2f42707469a60ef2b36207749f54a95
//META: purpose=An arrayTypeLayout stores the element type layout in operand 0 and derives the element stride rather than storing it.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies both halves of the array-layout claim. `float arr[4]` in a constant
// buffer produces an `arrayTypeLayout`; the CHECKs capture its three operands
// and show operand 0 is the element type layout (a `typeLayout` reporting the
// 4-byte element) while the remaining two are the array's own `size` and
// `TypeAlignment` attributes. The whole array is 64 bytes for a 4-byte
// element, so the 16-byte stride is recoverable from size/alignment and is
// deliberately not an operand of its own.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

cbuffer CB
{
    float arr[4];
}

uniform RWStructuredBuffer<float> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = arr[tid.x % 4];
}

// CHECK: = arrayTypeLayout(%[[ELEM:[0-9]+]], %[[SZ:[0-9]+]], %[[AL:[0-9]+]])
// CHECK: %[[AL]]{{.*}}= TypeAlignment(16 : Int)
// The array occupies 4 * 16 bytes even though its element type layout is 4.
// CHECK: %[[SZ]]{{.*}}= size(8 : Int, 64 : Int)
// CHECK: %[[ELEM]]{{.*}}= typeLayout(%[[ESZ:[0-9]+]], %{{[0-9]+}})
// CHECK: %[[ESZ]]{{.*}}= size(8 : Int, 4 : Int)
