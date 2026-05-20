//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:50:55+00:00
//META: source_commit=bbd84dc65e58598bfa71fafe72764b4076b0869b
//META: doc_ref=docs/llm-generated/pipeline/04b-pre-link-passes.md#lowerdefer
//META: doc_section_digest=378b9cfa7a738a36382fa87ee64681bba1d424cc7440c22662ab65a447b8a84b
//META: purpose=Phase B's lowerDefer emits the defer body at every exit path of the enclosing scope (per the notable-passes section), including the early return.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The notable-passes "lowerDefer" section says the defer body is
// emitted "at every exit path of the enclosing scope". When the
// scope can exit through an early return, the deferred store
// (buf[1] = 200) appears in the IR before the return that takes
// the early path. The `defer` opcode itself is gone.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform int a;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    defer { buf[1] = 200; }
    if (a > 0) { buf[0] = 1; return; }
    buf[0] = 2;
}

// CHECK-LABEL: ### LOWER-TO-IR:
// CHECK-LABEL: func %main
// CHECK: store({{.*}}, 200 : Int)
// CHECK-NOT: = defer(
