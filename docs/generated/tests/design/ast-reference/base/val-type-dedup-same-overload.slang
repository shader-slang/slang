//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ast-reference/base.md#val-nodebase
//META: doc_section_digest=17754b4ad55bb3bbeb3b934dd617e4a3120a865a281a81848bf3fc5ab03948d7
//META: purpose=Vals (including Types) are deduplicated by the ASTBuilder; two textually identical type expressions denote the same Val, so an overload set keyed on the type resolves identically at each use.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that Type is a hash-consed Val. The two written uses of
// `vector<int,3>` must denote the same Type Val and select the same
// overload at each site. If the types were distinct values, overload
// resolution at the second call could pick a different overload or
// fail to find one — the test fails to print the expected `3` twice.

//TEST:INTERPRET(filecheck=CHECK):

int kind(int v)             { return 1; }
int kind(vector<int,3> v)   { return 3; }
int kind(vector<float,3> v) { return 4; }

void main()
{
    vector<int,3> a = vector<int,3>(1, 2, 3);
    vector<int,3> b = vector<int,3>(4, 5, 6);
    //CHECK: a=3
    printf("a=%d\n", kind(a));
    //CHECK: b=3
    printf("b=%d\n", kind(b));
}
