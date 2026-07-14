//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=d245d3be9942f0847c45c138b4c2780e7df19ecb
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#error-codes-and-the-name-field
//META: doc_section_digest=58de6e7299bd58eba9a686426fc010a16e2745f7e3231042be33437e83834630
//META: purpose=Negative: dividing by literal zero in a semantic-checked expression renders E30002 with the documented "divide by zero" message.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Pin `divide-by-zero` (30002) to its rendered code. This is the
// semantic-check stage divide-by-zero, distinct from the
// preprocessor-stage one (15202). The lua entry has no per-span
// message, so we FileCheck the SIMPLE compile output for both
// the E30002 code line and the "divide by zero" header line.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int a = int(tid.x);
    int b = a / 0;
}

// CHECK: E30002
// CHECK: divide by zero
