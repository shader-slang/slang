//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/misc.md#packbranch
//META: doc_section_digest=64305dc5798875e516872edb40432c593b099dd7219dbb653d2016c5a0eabb98
//META: purpose=`PackBranch` resolves statically, so a zero-length pack yields the `emptyValue` arm and a pack of length one or more yields the `nonEmptyValue` arm.
//META: intent=boundary
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the selection rule at its two edges — the empty pack and the
// shortest non-empty pack — by instantiating the same generic three times and
// printing which arm it resolved to. `IntConst<1>` is the empty arm and
// `IntConst<2>` the non-empty arm, so an "empty=1 / one=2 / many=2" trace
// proves the length-zero case is distinguished from length one, and that the
// choice happened without any run-time pack-length test in the printed value.

//TEST:INTERPRET(filecheck=CHECK):

interface IStaticInt { static int get(); }
struct IntConst<let N : int> : IStaticInt { static int get() { return N; } }

int chooseArm<let each S : int>()
{
    return __packBranch(S, IntConst<1>, IntConst<2>)::get();
}

void main()
{
    // CHECK: empty=1
    printf("empty=%d\n", chooseArm<>());
    // CHECK: one=2
    printf("one=%d\n", chooseArm<7>());
    // CHECK: many=2
    printf("many=%d\n", chooseArm<7, 8, 9>());
}
