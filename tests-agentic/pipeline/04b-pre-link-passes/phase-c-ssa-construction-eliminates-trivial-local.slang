//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:50:55+00:00
//META: source_commit=bbd84dc65e58598bfa71fafe72764b4076b0869b
//META: doc_ref=docs/llm-generated/pipeline/04b-pre-link-passes.md#phase-c-mandatory-optimization-passes
//META: doc_section_digest=b2bba5b4923f9f3b4fa76f2dbfcef4a05d4c5bbf61e50737eaabe8565887e46f
//META: purpose=Phase C's constructSSA (C1) promotes addressable locals to SSA temporaries; trivial `int x = a; x = x + 1;` shows no var/load/store of a local Int.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A plain local `int x` does not survive as a `var Int` after
// Phase C; reads/writes appear as SSA temporaries fed directly
// into the buffer store. The local's address is never taken.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform int a;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int x = a;
    x = x + 1;
    x = x + 1;
    buf[0] = x;
}

// CHECK-LABEL: ### LOWER-TO-IR:
// CHECK-LABEL: func %main
// CHECK-NOT: : Ptr(Int){{[ \t]+}}= var
