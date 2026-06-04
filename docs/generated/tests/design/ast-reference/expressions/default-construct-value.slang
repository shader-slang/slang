//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:30:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#nodes
//META: doc_section_digest=5cae24401ed06bd3567242934951f344da434f10804637bdd679d55ab6b5bb2b
//META: purpose=A `T()` zero-argument invocation default-constructs a value of T, observable as zero-initialized fields of a plain struct.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// DefaultConstructExpr is documented as "synthesized expression for a
// default-constructed value". Its user-spellable surface is `T()`
// with no arguments. For a plain struct of ints, all fields default
// to zero.

//TEST:INTERPRET(filecheck=CHECK):

struct Vec { int x; int y; int z; }

void main()
{
    Vec v = Vec();
    // CHECK: x=0 y=0 z=0
    printf("x=%d y=%d z=%d\n", v.x, v.y, v.z);
}
