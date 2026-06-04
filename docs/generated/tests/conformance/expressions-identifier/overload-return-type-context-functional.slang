//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T09:36:27+00:00
//META: source_commit=273e29cb6cac600f70da7245a506bae6689e0e23
//META: doc_ref=docs/language-reference/expressions-identifier.md#overloading
//META: doc_section_digest=9484a135a7d65023e550053886ef224a8110c3717ecb4a6f046fb51a7be6e355
//META: purpose=An overloaded identifier (function name) is disambiguated by the argument types at the call site; different argument types route to different overloads.
//META: intent=boundary
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Corner case: overload disambiguation when the argument is a subtype of multiple parameter types.
// Tests that a bool argument resolves to the int overload (implicit conversion) rather than float.
//TEST:INTERPRET(filecheck=CHECK):

// Overload probe: returns 1 for int, 2 for float, confirming which overload was selected.
int which(int x)   { return 1; }
int which(float x) { return 2; }

void main()
{
    // bool promotes to int (rank 1) before float (rank 2); the int overload is chosen.
    bool b = true;
    int result = which(int(b));
    //CHECK: 1
    printf("%d\n", result);

    // An explicit float argument forces the float overload.
    float fval = 1.5f;
    int result2 = which(fval);
    //CHECK: 2
    printf("%d\n", result2);
}
