//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:50:55+00:00
//META: source_commit=bbd84dc65e58598bfa71fafe72764b4076b0869b
//META: doc_ref=docs/generated/design/pipeline/04b-pre-link-passes.md#phase-d-non-essential-validation-stripping-and-finalization
//META: doc_section_digest=ad77551965f16f59589941dbe50f81bd8509b595aa5ee08277212d611c118e05
//META: purpose=Phase D's final eliminateDeadCode (D12) keeps exports and layouts alive so the post-link linkIR has the mangled-name index built by D15 to resolve against.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// D10 strips front-end-only decorations but D12's eliminateDeadCode
// runs with `keepExportsAlive` set. The `[export(...)]` decoration
// on `main` survives into the LOWER-TO-IR dump (it is the index
// key for the post-link buildMangledNameToGlobalInstMap at D15).

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = int(tid.x);
}

// CHECK-LABEL: ### LOWER-TO-IR:
// CHECK: export("{{.*}}main{{.*}}")
// CHECK-LABEL: func %main
