//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T09:36:27+00:00
//META: source_commit=273e29cb6cac600f70da7245a506bae6689e0e23
//META: doc_ref=docs/language-reference/expressions-identifier.md#implicit-lookup
//META: doc_section_digest=81444a04e3158afb0f979b5eaa4e66672a28128589f884b354ac44b03dbff7ad
//META: purpose=In a method body, a bare identifier resolves to `this.fieldName` via the implicit `this` parameter of the method.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Inside a struct method, a bare name lookup for a member resolves as this.fieldName.
// Both a read access and a mutating (inout) method exercise this claim.
//TEST:INTERPRET(filecheck=CHECK):

struct Counter
{
    int value;

    int get()
    {
        // 'value' resolves to this.value via implicit this.
        return value;
    }

    [mutating]
    void add(int delta)
    {
        // 'value' on the left-hand side also resolves to this.value via implicit this.
        value = value + delta;
    }
}

void main()
{
    Counter c;
    c.value = 5;
    c.add(3);
    int result = c.get();
    //CHECK: 8
    printf("%d\n", result);
}
