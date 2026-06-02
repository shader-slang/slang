//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:08+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/types-struct.md#subscript-op
//META: doc_section_digest=624917b929b5db01e3231965412253236cf0ba05012e0fb465fd200622c5ec90
//META: purpose=A `__subscript` operator with `get` and `set` accessors lets the struct be accessed with `[]`; reading invokes `get`, writing invokes `set`.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a `__subscript` operator routes reads to `get` and writes to `set`.
// Three elements are written via x[i] = v and read back via float v = x[i]; CHECK confirms
// each element retains the assigned value, proving both accessors fired.

//TEST:INTERPRET(filecheck=CHECK):

struct Indexed
{
    float arr[4];

    __subscript(int i) -> float
    {
        get { return arr[i]; }
        set { arr[i] = newValue; }
    }
}

void main()
{
    Indexed x = { };
    x[0] = 10.0f;
    x[1] = 20.0f;
    x[2] = 30.0f;

    float v0 = x[0];
    float v1 = x[1];
    float v2 = x[2];
    // CHECK: 10
    printf("%.0f\n", v0);
    // CHECK: 20
    printf("%.0f\n", v1);
    // CHECK: 30
    printf("%.0f\n", v2);
}
