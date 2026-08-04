//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#tie-breaking-comparator
//META: doc_section_digest=dfab71819770e1c3116c8ab890c4c05608d82d83fe8f00436fcbffa88f52900d
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
