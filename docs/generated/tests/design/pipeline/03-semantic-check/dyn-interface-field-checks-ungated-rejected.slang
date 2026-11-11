//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:36:14Z
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#dyn-interface-restrictions
//META: doc_section_digest=630e7c72909468c5bb7b3968c35bb00b899aedcf09b80f0e1814203509f007d2
//META: purpose=The field checks on a type conforming to a `dyn interface` (E33079, E33080) are not gated on the language version, so they fire under the default `-std`.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the exception the refreshed doc records: unlike the extension and
// generic-conformer checks, the field checks stay active when the gate is
// closed. There is no `-std 2026` here, so the unsized and opaque fields
// being rejected shows the gate does not cover them. CHECK pins one code per
// field.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

dyn interface IFoo
{
    int get();
}

struct Unsized : IFoo
{
    int tail[];
//CHECK: E33079
    int get() { return 0; }
}

struct Opaque : IFoo
{
    Texture2D t;
//CHECK: E33080
    int get() { return 0; }
}

void main()
{
}
