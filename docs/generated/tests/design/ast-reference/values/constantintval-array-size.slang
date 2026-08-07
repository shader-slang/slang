//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/values.md#intval-family
//META: doc_section_digest=7dc220c4c4dd5c2b21296a00da8c0b8bb7caf5c69af13f6b032f0ae0e4047ecd
//META: purpose=A literal compile-time integer is the immediately-reducible IntVal form, so a literal array bound gives the array exactly that many independently addressable elements.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the "immediately reducible" end of the IntVal family: a
// literal used as an array bound is a known constant at check time.
// The test writes five distinct values into an `int[5]` and prints
// all five slots; CHECK asserts every slot round-trips, which is only
// possible if the bound resolved to exactly 5 elements.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int a[5];
    for (int i = 0; i < 5; ++i) { a[i] = i + 1; }
    // CHECK: arr=1,2,3,4,5
    printf("arr=%d,%d,%d,%d,%d\n", a[0], a[1], a[2], a[3], a[4]);
}
