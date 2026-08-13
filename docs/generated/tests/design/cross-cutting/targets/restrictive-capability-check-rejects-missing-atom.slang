//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/targets.md#runtime-representation
//META: doc_section_digest=e64e6f3a2c5831d0619d23756c968fa54849d3d1bc131e48679d1bfbe8e8e9b7
//META: purpose=Capability sets are joined against the active profile. `-restrictive-capability-check` upgrades a missing-atom mismatch to an error; without the flag the same input emits an implicit-upgrade warning.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `__requireCapability(cooperative_vector)` declares that this entry
// point needs an atom that is NOT in `sm_5_0`'s capability set.
// Under `-restrictive-capability-check` the compiler raises an
// error at the entry-point signature. Without the flag the same
// input emits an "implicitly upgraded" warning instead. Both are
// the documented user-visible consequence of the doc's "Computing
// the join (intersection) of two capability sets" bullet.
//
// The two scenarios live behind `#if ERROR / #elif WARN` so each
// DIAGNOSTIC_TEST's checks can hang off a distinct line of source.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK-ERR):-stage compute -entry main -target hlsl -profile sm_5_0 -restrictive-capability-check -DERROR=1 -DWARN=0
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK-WARN):-stage compute -entry main -target hlsl -profile sm_5_0 -DERROR=0 -DWARN=1

#if ERROR

[numthreads(1, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
/*CHECK-ERR:
     ^^^^ entry point uses capabilities not in specified profile
     ^^^^ entry point 'main' uses capabilities that are not part of the specified profile 'sm_5_0'. Missing capabilities are: 'sm_6_9'
*/
{
    __requireCapability(cooperative_vector);
/*CHECK-ERR:
    ^^^^^^^^^^^^^^^^^^^ see call to '__requireCapability'
*/
}

#elif WARN

[numthreads(1, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
/*CHECK-WARN:
     ^^^^ profile implicitly upgraded
     ^^^^ entry point 'main' uses additional capabilities that are not part of the specified profile 'sm_5_0'. The profile setting is automatically updated to include these capabilities: 'sm_6_9'
*/
{
    __requireCapability(cooperative_vector);
/*CHECK-WARN:
    ^^^^^^^^^^^^^^^^^^^ see call to '__requireCapability'
*/
}

#endif
