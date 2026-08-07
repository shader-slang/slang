//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/visibility.md#per-keyword-semantics
//META: doc_section_digest=7d96ab9817d6f6251e2148b7d4d9959fb9ec73041354e919db537f46458d416e
//META: purpose=Under Slang 2026 an unmodified member of an `internal` aggregate inherits the aggregate's visibility instead of the module default, so a public-default module accepts the declaration.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the 2026 rule that an aggregate's unmodified members take the
// aggregate's effective visibility. The module is marked `public`, so the
// module-wide default is `public`; `base` and `doubled` carry no modifier.
// If they took the module default they would be more visible than the
// `internal` struct that owns them and the declaration would be rejected.
// Compiling clean and emitting the member function is the observation that
// they inherited `internal` instead.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -std 2026 -entry computeMain -stage compute

public module aggregateMemberInherits2026;

internal struct Counter
{
    int base;
    int doubled() { return base * 2; }
}

internal int run(Counter c) { return c.doubled(); }

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = run(Counter(21));
}

// CHECK-DAG: int Counter_doubled_{{[0-9]+}}(
// CHECK-DAG: outputBuffer_{{[0-9]+}}[int(0)] = run_
