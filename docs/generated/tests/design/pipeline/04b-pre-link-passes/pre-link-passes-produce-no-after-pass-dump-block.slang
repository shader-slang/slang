//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/pipeline/04b-pre-link-passes.md#pre-link-mandatory-passes
//META: doc_section_digest=f3be9705adaba00c166c4c4eb1f2b3db4b7e22926b5ec3e600da34fe99b70618
//META: purpose=The pre-link passes are plain function calls rather than SLANG_PASS-wrapped ones, so no AFTER-pass dump block appears before the LOWER-TO-IR snapshot.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the observable consequence of the page's opening remark that
// this region uses plain `passName(module)` calls instead of the pass-macro
// wrapper the post-link passes use. That wrapper is what gives a pass its
// `### AFTER <pass>:` heading in a `-dump-ir` listing, so if any pre-link
// step were wrapped, its heading would appear ahead of the pre-link
// snapshot. The CHECK-NOT runs from the start of the dump up to the
// `### LOWER-TO-IR:` heading and asserts no such heading exists there;
// the trailing CHECK confirms the post-link headings do start afterwards,
// so the CHECK-NOT is not vacuously passing on an empty dump.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o - -entry main -stage compute

RWStructuredBuffer<int> outBuf;
uniform int a;

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    outBuf[0] = a + 1;
}

// CHECK-NOT: ### AFTER
// CHECK: ### LOWER-TO-IR:
// CHECK: ### AFTER
