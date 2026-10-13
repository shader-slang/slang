//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#name-lookup-and-declref
//META: doc_section_digest=974c295d7822368d7371b23d0dae553135ae7f5870e74a61cb71417386c7531f
//META: purpose=A DeclRef also records how outer-context generic parameters are bound, so a method of a generic struct sees the struct's type argument.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the "outer-context parameters" half of the substitution claim. The
// method `pick` declares no generic parameters of its own; the only way its
// body can name `T` is through the substitution inherited from the enclosing
// `Box<T>` reference. Two instantiations of `Box` are used so a dropped outer
// binding would collapse them; CHECK asserts each kept its own element value.

//TEST:INTERPRET(filecheck=CHECK):

struct Box<T>
{
    T value;
    T pick()
    {
        return value;
    }
}

void main()
{
    Box<int> bi;
    bi.value = 5;
    Box<float> bf;
    bf.value = 2.5f;
    // CHECK: i=5 f=2.5
    printf("i=%d f=%.1f\n", bi.pick(), bf.pick());
}
