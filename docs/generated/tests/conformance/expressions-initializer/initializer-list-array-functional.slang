//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T09:36:11+00:00
//META: source_commit=273e29cb6cac600f70da7245a506bae6689e0e23
//META: doc_ref=docs/language-reference/expressions-initializer.md#initializer-list-expression
//META: doc_section_digest=68be737ab4b8adb6374f77514e63e2dd70ca264c5b4d8f7ff4cffa68f75e41dc
//META: purpose=An initializer list expression used as the initial-value of a fixed-size array variable initializes each element in index order.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// C4 applied to arrays: the doc's example shows heterogeneous types
// ({ 1, "hello", 2.0f }) so the init list is not struct-specific.
// Test with a fixed-size array of int to verify element-order semantics.

//TEST:INTERPRET(filecheck=CHECK):

int passInt(int v) { return v; }

void main()
{
    int arr[3] = { passInt(10), passInt(20), passInt(30) };
    //CHECK: 10 20 30
    printf("%d %d %d\n", arr[0], arr[1], arr[2]);
}
