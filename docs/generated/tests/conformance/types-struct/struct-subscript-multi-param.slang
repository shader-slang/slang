//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:08+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/types-struct.md#subscript-op
//META: doc_section_digest=624917b929b5db01e3231965412253236cf0ba05012e0fb465fd200622c5ec90
//META: purpose=Multiple `__subscript` overloads with different parameter counts are all valid; overload resolution dispatches each call to the right subscript.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that multiple `__subscript` overloads with different parameter counts coexist and
// that overload resolution dispatches each call correctly. m[] exercises the zero-parameter
// overload and m[1] the one-parameter overload; CHECK confirms both read and write paths.

//TEST:INTERPRET(filecheck=CHECK):

struct MultiSubscript
{
    float arr[4];

    // 0-parameter subscript
    __subscript() -> float
    {
        get { return arr[0]; }
        set { arr[0] = newValue; }
    }

    // 1-parameter subscript
    __subscript(int i) -> float
    {
        get { return arr[i]; }
        set { arr[i] = newValue; }
    }
}

void main()
{
    MultiSubscript m = { };

    m[] = 100.0f;
    m[1] = 200.0f;

    float r0 = m[];
    float r1 = m[1];

    // CHECK: 100
    printf("%.0f\n", r0);
    // CHECK: 200
    printf("%.0f\n", r1);
}
