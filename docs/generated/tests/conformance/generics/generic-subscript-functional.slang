//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:43+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/generics.md#description
//META: doc_section_digest=7b3e1c2f4d8a0e9f5b2c6d1e3f7a4b8c0d2e6f9a1b5c3e7f0d4a8b2c6e0f4a
//META: purpose=A generic subscript operator `__subscript<T>(T i) -> float where T : IInteger` accepts any integer type as the index.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// From the doc Examples section (Generic subscript).
// The subscript accepts any IInteger index type and performs integer conversion.

//TEST:INTERPRET(filecheck=CHECK):

struct TestStruct
{
    float arr[5];

    __subscript<T>(T i) -> float
        where T : IInteger
    {
        get { return arr[i.toInt()]; }
        set { arr[i.toInt()] = newValue; }
    }
}

void main()
{
    TestStruct ts;
    ts.arr[0] = 0.0;
    ts.arr[1] = 10.0;
    ts.arr[2] = 20.0;
    ts.arr[3] = 30.0;
    ts.arr[4] = 40.0;

    // Access using int index
    float v0 = ts[int(2)];
    // Access using uint index
    uint idx = 4;
    float v1 = ts[idx];

    // CHECK: 20
    printf("%d\n", (int)v0);
    // CHECK: 40
    printf("%d\n", (int)v1);
}
