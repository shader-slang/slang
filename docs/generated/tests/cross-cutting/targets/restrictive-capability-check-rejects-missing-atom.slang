//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/generated/design/cross-cutting/targets.md#runtime-representation
//META: doc_section_digest=bd3a8a9e5737485c9a7ddbe47c464799f12940b6a68f5d5db5a85b41bbef6560
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
//CHECK-ERR: E41013
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
//CHECK-WARN: W41012
}

#endif
