//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04b-pre-link-passes.md#option-set-toggles
//META: doc_section_digest=7c951a88b67c0d94a6d4504f2d7616fdfe324b58f96f2245c67178d693892a88
//META: purpose=The non-essential-validation toggle gates D1 and D3-D9, so the missing-return warning disappears when validation is disabled while code generation still succeeds.
//META: intent=boundary
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the gate on Phase D's validator block by compiling the same
// module twice. The default run must report the missing-return warning;
// the run with `-disable-non-essential-validations` must not report it at
// all, yet must still emit HLSL -- the positive CHECK on the emitted
// buffer declaration is what distinguishes "validation was skipped" from
// "compilation failed before reaching emit".

//TEST:SIMPLE(filecheck=WARN):-target hlsl -stage compute -entry main
//TEST:SIMPLE(filecheck=NOWARN):-target hlsl -stage compute -entry main -disable-non-essential-validations

uniform int a;
uniform RWStructuredBuffer<int> buf;

int helper(int x) { if (x > 0) return 1; }

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = helper(a);
}

// WARN: warning{{.*}}41010

// NOWARN-NOT: 41010
// NOWARN: RWStructuredBuffer<int
