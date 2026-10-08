//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -stage compute -entry computeMain

// Regression test for shader-slang/slang#11518.
//
// A `Ptr<T, GroupShared>` (Workgroup address space) that survives as a
// parameter of a NON-INLINED SPIR-V function must cause
// `SPV_KHR_variable_pointers` / `VariablePointers` to be declared.
//
// `readScalar` is `[noinline]`, so its parameter survives as a
// `%_ptr_Workgroup_float` operand of the emitted `OpTypeFunction`. It
// dereferences a scalar (`*p`), so no access-chain value site is emitted, and
// the caller passes the address of a whole groupshared variable, so no value
// site declares the capability either. Before the fix the capability was
// silently omitted and the module was invalid SPIR-V; the fix declares the
// capability from the surviving function signature.
//
// This covers the (GroupShared, parameter) combination, which is the only one
// that can be isolated as a regression: the new helper also walks the result
// type and forwards the StorageBuffer address space, but a pointer in those
// positions is always produced/consumed by a value site that already declares
// the capability (a returned groupshared pointer declares it at the call's
// result type in `emitCall`; a StorageBuffer pointer comes from an access
// chain), so a test of those arms would pass with or without the fix. They are
// exercised by the same `requireVariableBufferCapabilityIfNeeded` forwarding.

groupshared float gs;

// `outputBuffer` and the write below exist only to keep `gs` and the
// `readScalar` call (and thus its non-inlined signature) from being
// dead-code-eliminated.
RWStructuredBuffer<float> outputBuffer;

[noinline]
float readScalar(Ptr<float, Access::ReadWrite, AddressSpace::GroupShared> p)
{
    return *p;
}

[numthreads(1, 1, 1)]
void computeMain(uint i : SV_GroupIndex)
{
    gs = float(i);
    outputBuffer[0] = readScalar(__getAddress(gs));
}

// The extension and capability must be declared even though the only Workgroup
// pointer appears in the (non-inlined) function signature.
// CHECK: OpCapability VariablePointers
// CHECK: OpExtension "SPV_KHR_variable_pointers"
// The Workgroup pointer must actually appear in the function signature, proving
// we exercise the surviving-signature path rather than an inlined one.
// CHECK: %[[WGPTR:[0-9A-Za-z_]+]] = OpTypePointer Workgroup %float
// CHECK: OpTypeFunction %float %[[WGPTR]]
