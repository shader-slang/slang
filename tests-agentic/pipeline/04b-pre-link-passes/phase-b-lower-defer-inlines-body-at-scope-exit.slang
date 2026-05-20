//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:50:55+00:00
//META: source_commit=bbd84dc65e58598bfa71fafe72764b4076b0869b
//META: doc_ref=docs/llm-generated/pipeline/04b-pre-link-passes.md#lowerdefer
//META: doc_section_digest=378b9cfa7a738a36382fa87ee64681bba1d424cc7440c22662ab65a447b8a84b
//META: purpose=Phase B's lowerDefer (B3) emits the defer body at scope exit so downstream passes see straight-line code; no defer opcode survives.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The "lowerDefer" notable-passes entry says the defer statement
// is removed by emitting its body at every exit path. Verify by
// writing buf[0] before the defer and observing that buf[1]
// (the deferred write) lands as a normal store and that the
// `defer` opcode is gone.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    defer { buf[1] = 100; }
    buf[0] = 1;
}

// CHECK-LABEL: ### LOWER-TO-IR:
// CHECK-LABEL: func %main
// CHECK: store({{.*}}, 1 : Int)
// CHECK: store({{.*}}, 100 : Int)
// CHECK-NOT: = defer(
