//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T22:00:02Z
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#function-style-declarations
//META: doc_section_digest=d2933f044671826cdda47d2bf6584eb3ef9b48c4c8f5e77a17b011c61c8729d4
//META: purpose=The C++-style trailing `const` after a parameter list is accepted in HLSL source and the member function emits.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the optional trailing `const` receiver qualifier parses in an HLSL
// source file; CHECK asserts the member result reaches the output buffer.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -stage compute -entry main

struct S
{
    int v;
    int get() const { return v; }
};

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void main()
{
    S s;
    s.v = 3;
    outputBuffer[0] = s.get();
}

// CHECK: {{S_get_[0-9]+}}(
