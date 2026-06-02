//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:08+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/types-struct.md#property
//META: doc_section_digest=2be36ef488309471e001ccf502f4e364705629af21bac8d6137274483b7c4b14
//META: purpose=The `set` accessor may name its parameter explicitly; the named parameter is used in the set body instead of the implicit `newValue`.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a `set` accessor with an explicitly named parameter compiles and functions correctly.
// ExplicitSet.someProp uses `set(float newVal)`; assigning 5.0 then reading back via `get` lets
// CHECK confirm the named parameter received and stored the value.

//TEST:INTERPRET(filecheck=CHECK):

struct ExplicitSet
{
    float m_val;

    property someProp : float
    {
        get() { return m_val; }
        set(float newVal) { m_val = newVal; }
    }
}

void main()
{
    ExplicitSet obj = { };
    obj.someProp = 5.0f;
    float v = obj.someProp;
    // CHECK: 5
    printf("%.0f\n", v);
}
