//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/misc.md#packbranch
//META: doc_section_digest=64305dc5798875e516872edb40432c593b099dd7219dbb653d2016c5a0eabb98
//META: purpose=`__packBranch(pack, emptyValue, nonEmptyValue)` lowers to a three-operand `PackBranch` inst typed as a witness table when the branch selects a conformance.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the operand shape and result type of the static pack conditional.
// The generic picks between two conformers of `IStaticInt` by whether its
// value pack is empty; the CHECK asserts the dump contains a `PackBranch` with
// exactly three operands whose result type is a `witness_table_t`, which is the
// witness-selecting form the pack-branch lowering produces (the type-selecting
// form is typed as a kind instead).

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

interface IStaticInt { static int get(); }
struct IntConst<let N : int> : IStaticInt { static int get() { return N; } }

int chooseArm<let each S : int>()
{
    return __packBranch(S, IntConst<1>, IntConst<2>)::get();
}

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void main()
{
    outputBuffer[0] = chooseArm<>();
    outputBuffer[1] = chooseArm<7, 8>();
}

// IR: : witness_table_t({{.*}}){{[ 	]*}}= PackBranch(%{{[A-Za-z0-9_]+}}, {{.*}}, {{.*}})
