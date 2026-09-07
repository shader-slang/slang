//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#the-major-node-families
//META: doc_section_digest=90f68d89058e95e927dbc5ffd876802120cff1e5fe6639465168476f05517709
//META: purpose=The user-level surface of the `Val` node family is a generic value argument, so the `3` in `vector<float, 3>` and the `4` in `int a[4]` are recoverable as compile-time values.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a vector's element count and an array's size are values in the
// same sense a type argument is a type — the surface the `Val` family names.
// Two generics take an integer value parameter and recover it by inference from
// the argument's type alone; the body then returns `N` as an ordinary integer,
// which is only possible if the count really was bound as a generic value
// argument rather than being baked into the type opaquely. `float3` is passed
// alongside the spelled-out `vector<float, 2>` so the interned typedef spelling
// is shown to carry the same value.

//TEST:INTERPRET(filecheck=CHECK):

int countOfVec<let N : int>(vector<float, N> v) { return N; }
int countOfArr<let N : int>(int a[N]) { return N; }

void main()
{
    float3 v3 = float3(1, 2, 3);
    vector<float, 2> v2 = float2(4, 5);
    int a4[4] = { 1, 2, 3, 4 };

    // The `3` inside the `float3` typedef's expansion.
    //CHECK: v3=3
    printf("v3=%d\n", countOfVec(v3));

    //CHECK: v2=2
    printf("v2=%d\n", countOfVec(v2));

    // The `4` in the array declarator.
    //CHECK: a4=4
    printf("a4=%d\n", countOfArr(a4));
}
