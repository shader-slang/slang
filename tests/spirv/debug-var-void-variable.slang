//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -g2 -O0
// Zero-length array variables are legalized to void types and removed.
// Check we do not generate debug info for the removed variables.

void f(int[0] droppedArg)
{
    int[0] droppedLocal = {};
    int kept = 1;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    f({});
}

// The droppedArg and droppedLocal variables should not appear in the output.
// CHECK-NOT: OpString "droppedArg"
// CHECK-NOT: OpString "droppedLocal"

// Check the kept variable has debug info
// CHECK-DAG: [[KEPT_STR:%[0-9]+]] = OpString "kept"
// CHECK-DAG: [[KEPT_DBGVAR:%[A-Za-z_0-9]+]] = OpExtInst %void {{%[0-9]+}} DebugLocalVariable [[KEPT_STR]] %{{[0-9]+}} %{{[0-9]+}} %uint_{{[0-9]+}} %uint_{{[0-9]+}} %{{[0-9]+}} %uint_0

// f() should have no parameters after empty-array legalization + void cleanup
// CHECK-LABEL: ; Function f
// CHECK: %{{[A-Za-z_0-9]+}} = OpFunction %void None %{{[0-9]+}}
// CHECK-NOT: OpFunctionParameter
// CHECK: DebugDeclare [[KEPT_DBGVAR]]
