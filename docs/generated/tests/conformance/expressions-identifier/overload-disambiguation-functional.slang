//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T09:36:27+00:00
//META: source_commit=273e29cb6cac600f70da7245a506bae6689e0e23
//META: doc_ref=docs/language-reference/expressions-identifier.md#overloading
//META: doc_section_digest=9484a135a7d65023e550053886ef224a8110c3717ecb4a6f046fb51a7be6e355
//META: purpose=An overloaded identifier resolves to the correct candidate when the call context disambiguates by argument type.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// An identifier may refer to multiple candidates with the same name (overloaded).
// When the call site provides enough type information, the right overload is chosen.
// Uses the overload-probe pattern: each overload returns a distinct integer sentinel.
//TEST:INTERPRET(filecheck=CHECK):

int probe(int x)   { return 1; }
int probe(float x) { return 2; }
int probe(uint x)  { return 3; }

void main()
{
    // Each call passes an argument whose type disambiguates the overload set.
    int ri = probe(1);          // int literal -> probe(int) -> 1
    int rf = probe(1.0f);       // float literal -> probe(float) -> 2
    int ru = probe(uint(99));   // explicit uint -> probe(uint) -> 3
    //CHECK: 1
    //CHECK: 2
    //CHECK: 3
    printf("%d\n", ri);
    printf("%d\n", rf);
    printf("%d\n", ru);
}
