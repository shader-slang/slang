//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/misc.md#pack-and-expansion
//META: doc_section_digest=23248f45180c2ed6ca7d90a9cde90609e58c45a8a68d6fc6c5245f5bc4c2e1df
//META: purpose=Trimming a pack removes exactly one slot, so a one-element pack trims to length zero and an already-empty pack stays at length zero.
//META: intent=boundary
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the trim helpers at the length edges the pack grammar admits:
// zero, one, and a longer pack. `countof` of the trimmed pack is printed for
// each instantiation, so "n0=0 / n1=0 / n3=2" shows that trimming never goes
// below zero and always removes exactly one slot when there is one to remove.

//TEST:INTERPRET(filecheck=CHECK):

int trimmedFirstCount<let each D : int>()
{
    return countof(__trimFirst(D));
}

int trimmedLastCount<let each D : int>()
{
    return countof(__trimLast(D));
}

void main()
{
    // CHECK: n0=0
    printf("n0=%d\n", trimmedFirstCount<>());
    // CHECK: n1=0
    printf("n1=%d\n", trimmedFirstCount<5>());
    // CHECK: n3=2
    printf("n3=%d\n", trimmedFirstCount<1, 2, 3>());
    // CHECK: l1=0
    printf("l1=%d\n", trimmedLastCount<5>());
    // CHECK: l3=2
    printf("l3=%d\n", trimmedLastCount<1, 2, 3>());
}
