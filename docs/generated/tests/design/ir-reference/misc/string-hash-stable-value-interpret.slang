//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:12:20Z
//META: source_commit=5f0e613fdd3c2890bb087d9ff76154e2605a14d2
//META: doc_ref=docs/generated/design/ir-reference/misc.md#getstringhash
//META: doc_section_digest=b534354f891756509322eeb68d274d9dc67208cbe58e97b555830eba07b9e425
//META: purpose=`getStringHash` of the same literal yields the same runtime hash value at every call site (compile-time-stable hash of the literal's bytes).
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the same stable-hash claim on the slangi byte-code VM:
// two call sites hashing an identical literal must produce equal
// values when the folded constants are actually evaluated. printf
// reports the comparison result and CHECK pins the equal case.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    uint a = getStringHash("stable-key-123");
    uint b = getStringHash("stable-key-123");
    int eq = (a == b) ? 1 : 0;
    // CHECK: eq=1
    printf("eq=%d\n", eq);
}
