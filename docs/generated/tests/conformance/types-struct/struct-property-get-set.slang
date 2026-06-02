//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:08+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/types-struct.md#property
//META: doc_section_digest=2be36ef488309471e001ccf502f4e364705629af21bac8d6137274483b7c4b14
//META: purpose=A `property` with both `get` and `set` accessors: reading the property calls `get`; assigning to it calls `set` with the new value as `newValue`.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that property reads dispatch to `get` and writes dispatch to `set` with `newValue`.
// The set body updates both m_val and m_valIsPositive; CHECK confirms the side effects and
// the get-returned value are consistent across positive and negative assignments.

//TEST:INTERPRET(filecheck=CHECK):

struct TestClass
{
    float m_val;
    bool m_valIsPositive;

    property someProp : float
    {
        get
        {
            return m_val;
        }
        set
        {
            m_val = newValue;
            m_valIsPositive = (newValue > 0.0f);
        }
    }
}

void main()
{
    TestClass obj = { };
    obj.someProp = 3.0f;
    // CHECK: 3
    printf("%.0f\n", obj.m_val);
    // CHECK: 1
    printf("%d\n", (int)obj.m_valIsPositive);

    obj.someProp = -1.0f;
    // CHECK: 0
    printf("%d\n", (int)obj.m_valIsPositive);

    float got = obj.someProp;
    // CHECK: -1
    printf("%.0f\n", got);
}
