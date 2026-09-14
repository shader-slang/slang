//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/target-pipelines/wgsl.md#emitted-spellings
//META: doc_section_digest=a72c747d5e5fa9684a460e1b5d345a74da1aee6a2a789ea6d65d147ba60563d3
//META: purpose=An integer literal is emitted as a WGSL constructor call — i32(N), u32(N), i64(N) — rather than as a bare literal, so the spelling carries the type explicitly.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// This is the row the doc leads with, and the one where expectations carried
// over from Slang syntax stop applying: `emitSimpleValueImpl` writes an integer
// literal as a *constructor call*, not as the bare digits a reader would expect
// from the source text.
//
// One test per literal width the doc enumerates, because each is a separate arm
// of that function rather than one rule with one outcome. Every literal is
// combined with a thread-id-derived value so the constant folder cannot remove
// it before emit, which would leave the CHECK matching nothing.

//TEST:SIMPLE(filecheck=CHECK):-target wgsl -entry main -stage compute

RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int i = 7 + int(tid.x);
    uint u = 9u + tid.y;
    int64_t l = 11L + int64_t(tid.z);
    outBuf[0] = float(i) + float(u) + float(l);
}

// Each width takes its own constructor spelling.
// CHECK-DAG: i32(7)
// CHECK-DAG: u32(9)
// CHECK-DAG: i64(11)
// The subscript index is an int literal too, so it is spelled the same way
// rather than as a bare 0.
// CHECK-DAG: outBuf{{[_0-9]*}}[i32(0)]
