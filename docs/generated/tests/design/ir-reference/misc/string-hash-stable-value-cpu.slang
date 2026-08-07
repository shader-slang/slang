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

// Verifies the hash is a stable function of the literal's bytes: two
// call sites in separate helper functions hashing the same literal
// must agree at run time. The kernel stores 1 only when the two hash
// values compare equal; CHECK pins that 1, which can only happen if
// both calls folded to the same constant.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

RWStructuredBuffer<int> outputBuffer;

uint hashSiteA() { return getStringHash("stable-key-123"); }
uint hashSiteB() { return getStringHash("stable-key-123"); }

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = (hashSiteA() == hashSiteB()) ? 1 : 0;
}

// CHECK: 1
