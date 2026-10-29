//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/targets.md#capability-system
//META: doc_section_digest=42bb2b8e4a42e0ceb11d49a4ca220fc9b2fff51da3e1dfe9c94764da839027ca
//META: purpose=`WaveActiveSum` carries a wave-intrinsic capability available on the HLSL target but not on the C++ shader target; calling it from a C++ entry point trips the capability join and surfaces a target-mismatch diagnostic.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Cross-target capability-availability difference. The doc's Vocabulary
// section says target atoms populate the `target` keyhole; a feature
// whose capability disjunction does not include `cpp` is unavailable
// under `-target cpp`. `WaveActiveSum` is the smallest such probe —
// HLSL accepts it (see the smoke test on per-target lowering) while
// C++ rejects with E36107.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target cpp -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(32, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
/*CHECK:
     ^^^^ entrypoint 'main' uses features that are not available in 'compute' stage for 'cpp' compilation target.
*/
//CHECK: E36107
{
    outBuf[tid.x] = WaveActiveSum(int(tid.x));
}
