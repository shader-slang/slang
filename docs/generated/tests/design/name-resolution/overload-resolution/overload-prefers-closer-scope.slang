//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#tie-breaking-comparator
//META: doc_section_digest=dfab71819770e1c3116c8ab890c4c05608d82d83fe8f00436fcbffa88f52900d
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
