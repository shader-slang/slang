//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -stage compute -entry computeMain

// A `[noinline]` function that *returns* a pointer with an explicit address
// space (here `Ptr<float, GroupShared>` / Workgroup) is legal SPIR-V under
// `SPV_KHR_variable_pointers`, so it must survive as a real function with its
// `DontInline` marker rather than being inlined away.

groupshared float gs[4];

// `outputBuffer` and the write keep `gs` and the `getGs` calls from being
// dead-code-eliminated.
RWStructuredBuffer<float> outputBuffer;

[noinline]
Ptr<float, Access::ReadWrite, AddressSpace::GroupShared> getGs(int i)
{
    return __getAddress(gs[i]);
}

[numthreads(1, 1, 1)]
void computeMain(uint i: SV_GroupIndex)
{
    *getGs(0) = float(i);
    outputBuffer[0] = *getGs(0);
}

// The module declares the variable-pointers capability, and the function
// survives (was not inlined): it keeps its DontInline marker and returns a
// Workgroup pointer, proving the pointer-return boundary was preserved.
// CHECK-DAG: OpCapability VariablePointers
// CHECK-DAG: OpTypePointer Workgroup %float
// CHECK: OpFunction %_ptr_Workgroup_float DontInline
