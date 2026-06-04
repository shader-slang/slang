//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:58:38+00:00
//META: source_commit=a01e79278cf0be9772deb8b2be52ee87687697a3
//META: doc_ref=docs/language-reference/declarations.md#interfaces
//META: doc_section_digest=a18c3296228874543a504555179e0ed00ec448f24436111e6f7990ab61491195
//META: purpose=An interface declaration introduces requirements; a type conforming to the interface must provide matching implementations.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a struct conforming to an interface must implement all interface requirements.
// `ConstRNG : IRandom` provides `next()`; calling it and checking 42 proves the
// conformance was accepted and the implementation dispatched.

//TEST:INTERPRET(filecheck=CHECK):

interface IRandom
{
    uint next();
}

struct ConstRNG : IRandom
{
    uint seed;
    uint next() { return seed; }
}

void main()
{
    ConstRNG rng;
    rng.seed = 42;
    // CHECK: 42
    printf("%u\n", rng.next());
}
