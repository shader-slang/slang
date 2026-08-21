// Verifies that coalescing stops at a call that may abandon the
// invocation. Statements in one basic block normally execute the same
// number of times, which is what lets them share a counter — but a
// callee that discards breaks that: the statements before the call run
// and the ones after it do not. Merging them onto one counter would
// misreport one side or the other, so the region must split at the
// call.
//
// The analysis is interprocedural: `maybeKill` discards only on one
// path, and the split still happens at its call site in `fragMain`.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -stage fragment -entry fragMain -trace-coverage

void maybeKill(float v)
{
    if (v < 0.5f)
        discard;
}

float4 fragMain(float2 uv : TEXCOORD) : SV_Target
{
    // One region up to and including the call...
    float a = uv.x + 1.0f;
    float b = a * 2.0f;
    maybeKill(b);
    // ...and a second region after it, because `maybeKill` may not
    // return.
    float c = b + 3.0f;
    return float4(c, c, c, 1.0f);
}

// Four regions total: `maybeKill`'s `if` and its `discard` arm, then
// `fragMain` split either side of the call. Were the split missing,
// `fragMain` would contribute one region instead of two and slot 3
// would not exist.
//CHECK-DAG: OpAccessChain {{.*}} %__slang_coverage {{.*}} %int_0
//CHECK-DAG: OpAccessChain {{.*}} %__slang_coverage {{.*}} %int_1
//CHECK-DAG: OpAccessChain {{.*}} %__slang_coverage {{.*}} %int_2
//CHECK-DAG: OpAccessChain {{.*}} %__slang_coverage {{.*}} %int_3

// And no fifth region: the split is at the call, not at every
// statement.
//CHECK-NOT: %__slang_coverage {{.*}} %int_4
