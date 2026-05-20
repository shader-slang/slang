//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:48:41Z
//META: source_commit=bbd84dc65e58598bfa71fafe72764b4076b0869b
//META: doc_ref=docs/llm-generated/name-resolution/overload-resolution.md#tie-breaking-comparator
//META: doc_section_digest=cf4ad761e64c7c27fa923b073fa264eed8e3a5152ac4e063d5e36eba8cb6323b
//META: purpose=Scope-distance step of the comparator (step 7) picks the closer-scope candidate when other ranking signals tie; a same-named function in the enclosing namespace at the use site beats a sibling-namespace one of equal cost.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// "Scope distance. For non-generic flavors, the comparator computes
// the lexical distance from the call site to each declaration and
// prefers the closer one." Here the namespace-scoped `Near::pick`
// is lexically closer to the call site (one parent walk) than the
// file-scope `pick` (two parent walks).

//TEST:INTERPRET(filecheck=CHECK):

int pick(int x) { return 1; }

namespace Near
{
    int pick(int x) { return 2; }

    int call()
    {
        return pick(7);
    }
}

void main()
{
    int tag = Near::call();
    // CHECK: tag=2
    printf("tag=%d\n", tag);
}
