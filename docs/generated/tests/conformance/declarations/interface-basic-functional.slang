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

// The doc says: "The body of an interface declaration may contain function,
// initializer, subscript, and associated type declarations. Each declaration
// in the body of an interface introduces a _requirement_ of the interface.
// Types that declare conformance to the interface must provide matching
// implementations of the requirements."

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
