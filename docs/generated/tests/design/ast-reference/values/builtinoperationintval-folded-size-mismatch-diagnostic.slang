//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:00:25Z
//META: source_commit=8ac9e49a5b7020af6a8040544b9f76f0fbf40898
//META: doc_ref=docs/generated/design/ast-reference/values.md#builtinoperationintval-and-the-single-representation-invariant
//META: doc_section_digest=4229b55a01470c7ad99014966f3313e48dd3d9bca875bef59b0f7b4c86cbc182
//META: purpose=After substitution the type checker reasons with the folded constant, not the symbolic division: initializing an int[5] from a Half<9> member typed int[N / 2] is rejected with the folded size 4 in the message.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that once N = 9 is substituted, the member's array bound is
// the folded ConstantIntVal 4 (not a residual symbolic value): the
// mismatch diagnostic names 'int[4]', and the pinned E30019 confirms
// the checker rejected the cross-size initialization.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

struct Half<let N : int>
{
    int data[N / 2];
}

void main()
{
    Half<9> h;
    int five[5] = h.data;
//CHECK: got 'int[4]'
//CHECK: E30019
}
