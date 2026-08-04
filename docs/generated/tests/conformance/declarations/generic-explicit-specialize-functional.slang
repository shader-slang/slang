//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:58:38+00:00
//META: source_commit=a01e79278cf0be9772deb8b2be52ee87687697a3
//META: doc_ref=docs/language-reference/declarations.md#generics
//META: doc_section_digest=de256a399516188e43bb96d93ea3c68437fd40807db23a0da48c05221ab85f6d
//META: purpose=A generic is explicitly specialized by supplying generic arguments in angle brackets `<>`; an explicitly specialized function may be used as a non-generic function.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that explicit angle-bracket specialization `sumVec<3>(v)` binds the value parameter.
// The generic uses `N` to bound the loop; passing a float3 with explicit `<3>` and checking
// output 6 proves the specialization resolved the correct loop count.

//TEST:INTERPRET(filecheck=CHECK):

float sumVec<let N : int>(vector<float, N> v)
{
    float s = 0.0;
    for (int i = 0; i < N; i++) s += v[i];
    return s;
}

void main()
{
    float3 v = float3(1.0, 2.0, 3.0);
    // CHECK: 6
    printf("%g\n", sumVec<3>(v));
}
