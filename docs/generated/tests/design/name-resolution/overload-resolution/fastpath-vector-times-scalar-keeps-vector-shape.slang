//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#the-builtin-operator-fast-path
//META: doc_section_digest=1c3af19385b1b45e53fc2911de4a5495ab14067ddf0d2925bb4569db9c7887aa
//META: purpose=coerceOperandsOfBuiltinBinaryExpr coerces each operand to its own shape, so float3 * float stays a vector-times-scalar operation whose result is float3.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the fast path does not broaden a vector-times-scalar operand
// pair into vector-times-vector: each operand keeps its own shape and the
// result keeps the vector shape. An overload probe on `float` versus `float3`
// observes the type of `v * s`; CHECK asserts the float3 overload fired.

//TEST:INTERPRET(filecheck=CHECK):

int probe(float x)  { return 1; }
int probe(float3 x) { return 3; }

void main()
{
    float3 v = float3(1.0f, 2.0f, 3.0f);
    float s = 2.0f;
    int tag = probe(v * s);
    // CHECK: tag=3
    printf("tag=%d\n", tag);
}
