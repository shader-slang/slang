//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-06-01T17:00:00+00:00
//META: source_commit=d25453d7f0b4867db4cb5eabf34fb6cd088cf596
//META: doc_ref=docs/language-reference/expressions-member-access.md#vector-swizzles
//META: doc_section_digest=ab827430dedad798221ccb6a8d05027ec37ed381ec7b70905cec740e9ed7e864
//META: purpose=Vector swizzle via member: `.r`/`.g`/`.b`/`.a` map to indices 0/1/2/3, parallel to `.x`/`.y`/`.z`/`.w`.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `.r`/`.g`/`.b`/`.a` swizzle characters map to vector elements
// 0/1/2/3 exactly like `.x`/`.y`/`.z`/`.w`. An `int4` initialized with distinct
// values is compared component-by-component; CHECK asserts all four pairs match
// and the equality flag is 1, ruling out any index permutation.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int4 v = int4(11, 22, 33, 44);
    //CHECK: r=11 g=22 b=33 a=44 eq=1
    bool eq = (v.r == v.x) && (v.g == v.y)
           && (v.b == v.z) && (v.a == v.w);
    printf("r=%d g=%d b=%d a=%d eq=%d\n",
           v.r, v.g, v.b, v.a, eq ? 1 : 0);
}
