//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:50:55+00:00
//META: source_commit=bbd84dc65e58598bfa71fafe72764b4076b0869b
//META: doc_ref=docs/generated/design/pipeline/04b-pre-link-passes.md#phase-c-mandatory-optimization-passes
//META: doc_section_digest=b2bba5b4923f9f3b4fa76f2dbfcef4a05d4c5bbf61e50737eaabe8565887e46f
//META: purpose=Phase C's simplifyCFG (C3) removes empty/unreachable blocks; an `if (true) ... else ...` collapses to the taken branch with no ifElse opcode.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// SCCP recognises the constant condition and simplifyCFG / peephole
// then prune the dead branch. After Phase C only the taken arm
// (`buf[0] = a;`) remains, with no `ifElse(...)` terminator and
// no reference to the -1 sentinel.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform int a;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    if (true) { buf[0] = a; }
    else { buf[0] = 9999; }
}

// CHECK-LABEL: ### LOWER-TO-IR:
// CHECK-LABEL: func %main
// CHECK: store({{.*}}, %a)
// CHECK-NOT: ifElse(
// CHECK-NOT: 9999 : Int
