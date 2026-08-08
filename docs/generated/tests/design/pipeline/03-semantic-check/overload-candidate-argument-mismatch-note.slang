//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#failure-modes
//META: doc_section_digest=4c9f71a256907b7a79759ca00a313ad651ac24d85015af4f3fe1673fd66d9dde
//META: purpose=When a call matches no overload, each candidate note is followed by a note naming the argument index that rejected it (E40018).
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the per-candidate argument-mismatch reporting. Two overloads both
// fail on their first parameter for different reasons; a diagnostic that only
// listed signatures would leave the user to diff them by eye. CHECK pins the
// top-level failure, both E40011 candidate notes, and both E40018 notes that
// name argument 0 and the expected type for each candidate.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

struct A
{
    int a;
}

struct B
{
    int b;
}

void g(A x, int y)
{
}

void g(B x, float y)
{
}

void main()
{
    float z = 1.0;
    g(z, z);
//CHECK: E39999
//CHECK: E40011
//CHECK: argument 0 does not match: expected 'A', got 'float'
//CHECK: E40011
//CHECK: argument 0 does not match: expected 'B', got 'float'
}
