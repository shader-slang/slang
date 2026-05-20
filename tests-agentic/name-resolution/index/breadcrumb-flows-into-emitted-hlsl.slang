//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T19:00:00Z
//META: source_commit=2aa9f69f5e2e75f6e2f4231a451a1a022818e18b
//META: doc_ref=docs/llm-generated/name-resolution/index.md#where-this-fits-in-the-pipeline
//META: doc_section_digest=386214978c9ba4cc20d3a513172ffbb1add48a95157ca4f4c9fc93204a1427ba
//META: purpose=The resolved DeclRef flows downstream into AST-to-IR lowering, where the breadcrumb chain becomes a concrete field access in emitted target code.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The index doc states: "the resolved DeclRef flows into AST-to-IR
// lowering where breadcrumb chains are turned into concrete IR
// access patterns." The cross-cutting observable is that a
// reference resolved through a non-trivial breadcrumb chain
// (here: SuperType + Member, walking the struct inheritance
// chain) emerges as an actual field access in emitted HLSL --
// not as an undefined symbol and not as a no-op.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry computeMain -stage compute

struct Base
{
    int baseField;
}

struct Derived : Base
{
    int derivedField;
}

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    Derived d;
    d.baseField = 10;
    d.derivedField = 20;
    // Unqualified `baseField` is resolved on `d` via a SuperType
    // breadcrumb followed by a Member breadcrumb. The breadcrumb
    // chain must lower to an actual field read in emitted HLSL.
    // The mangled field name varies across targets, so match the
    // base-field reference loosely.
    // CHECK: baseField
    outBuf[0] = d.baseField + d.derivedField;
}
