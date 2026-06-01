//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-06-01T15:30:00+00:00
//META: source_commit=d25453d7f0b4867db4cb5eabf34fb6cd088cf596
//META: doc_ref=docs/language-reference/types-vector-and-matrix.md#operators
//META: doc_section_digest=b0ab4edd7e624713af836700667d410c7afa9b2f65ed30df6db9b64ee04184ba
//META: purpose=Binary arithmetic between a vector and a scalar broadcasts the scalar across all vector elements; both `v - 1` and `4 - v` are documented.
//META: intent=boundary
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//TEST:INTERPRET(filecheck=CHECK):

int4 subOne(int4 v) { return v - 1; }
int4 fourMinus(int4 v) { return 4 - v; }

void main()
{
    int4 a = subOne(int4(1, 2, 3, 4));    // { 0, 1, 2, 3 }
    int4 b = fourMinus(int4(1, 2, 3, 4)); // { 3, 2, 1, 0 }
    //CHECK: a=0 1 2 3
    //CHECK: b=3 2 1 0
    printf("a=%d %d %d %d\n", a.x, a.y, a.z, a.w);
    printf("b=%d %d %d %d\n", b.x, b.y, b.z, b.w);
}
