//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:34:09Z
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/ast-reference/modifiers.md#nodes
//META: doc_section_digest=29d86296deb73a57a4f207507e16ba645a6bf2e722162db8157e7554d2cf8004
//META: purpose=A `const` written after a function parameter list is the trailing receiver const: the receiver is read-only, so assigning a member is diagnosed.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the `CppStyleTrailingReceiverParamConstModifier` row
// (`int read() const`). The trailing form needs the HLSL-flavored dialect, so
// the directive passes `-lang hlsl`. The method assigns to a member and CHECK
// pins the not-an-l-value code, proving the trailing `const` made `this`
// immutable.

//TEST:SIMPLE(filecheck=CHECK,exit-code=-1): -lang hlsl -target hlsl -stage compute -entry main

// CHECK: E30011
struct S
{
    int v;
    void set() const
    {
        v = 1;
    }
};

[numthreads(1,1,1)]
void main()
{
}
