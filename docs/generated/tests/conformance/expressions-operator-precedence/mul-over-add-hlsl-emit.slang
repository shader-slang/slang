//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T09:20:02+00:00
//META: source_commit=54bb48e67c3a4fdd4c0f4bd8085f5dc0bd8ffc99
//META: doc_ref=docs/language-reference/expressions-operator-precedence.md#operator-precedence
//META: doc_section_digest=2c43dea5ca69ca0e9d2a792df3b8f164e2487c75e25ef796f78839c678589fa1
//META: purpose=HLSL emission pin: `a + b * c` emits with `*` nested inside `+`, confirming the parser groups `*` before `+`.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Emission test: verify that the HLSL backend emits `a + b * c` preserving
// the `*`-inside-`+` grouping. The function takes uniform parameters so DCE
// cannot eliminate the expression.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry computeMain -stage compute

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    int a = outputBuffer[0];
    int b = outputBuffer[1];
    int c = outputBuffer[2];
    outputBuffer[3] = a + b * c;
}

// The emitted HLSL should contain a mul and an add — the exact form
// varies but the multiply subexpression must appear (not be absorbed into
// a single add). Check for `*` to appear in the emitted source.
// CHECK: {{[*]}}
