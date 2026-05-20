//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:48:41Z
//META: source_commit=bbd84dc65e58598bfa71fafe72764b4076b0869b
//META: doc_ref=docs/llm-generated/name-resolution/overload-resolution.md#tie-breaking-comparator
//META: doc_section_digest=cf4ad761e64c7c27fa923b073fa264eed8e3a5152ac4e063d5e36eba8cb6323b
//META: purpose=CompareLookupResultItems (comparator step 3) prefers a closer-scope same-name candidate over an outer one when both are Applicable; a struct-local method is preferred over a free function of the same name accessed unqualified on a struct receiver.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// "CompareLookupResultItems ... lexical 'how was the candidate
// found' comparison. Prefers shadowing members of a closer scope
// over an outer one". A struct method of name `act` is preferred
// to a same-name free function when called via the struct receiver.

//TEST:INTERPRET(filecheck=CHECK):

int act(int x) { return 1; }    // free function

struct S
{
    int act(int x) { return 2; } // member of S
}

void main()
{
    S s;
    int tag = s.act(7);
    // CHECK: tag=2
    printf("tag=%d\n", tag);
}
