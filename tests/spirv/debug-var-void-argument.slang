//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -g2 -O0
//
// When function arguments are legalized to Void and removed from the function
// signature, debug variable metadata for the remaining arguments must be
// remapped to the correct argument indices.
// Tests both int[0] (zero-length array) and Conditional<T, false> (struct
// whose only field is specialized away) as sources of void-typed parameters.

struct MyStruct
{
    Conditional<float3, false> m_color;
}

void f(int[0] dropped, MyStruct alsoDropped, int kept)
{
    int x = kept;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    MyStruct s;
    f({}, s, 0);
}

// The removed parameters should not appear as debug variables.
// CHECK-NOT: OpString "dropped"
// CHECK-NOT: OpString "alsoDropped"

// CHECK-DAG: [[KEPT_STR:%[0-9]+]] = OpString "kept"
// kept's DebugLocalVariable ArgNumber (last operand) should be 1 after
// both void parameters are removed and the index is remapped.
// CHECK-DAG: [[KEPT_DBGVAR:%[A-Za-z_0-9]+]] = OpExtInst %void {{%[0-9]+}} DebugLocalVariable [[KEPT_STR]] %{{[0-9]+}} %{{[0-9]+}} %uint_{{[0-9]+}} %uint_{{[0-9]+}} %{{[0-9]+}} %uint_0 %uint_1

// f() should have a single runtime parameter after void cleanup.
// CHECK-LABEL: ; Function f
// CHECK: %{{[A-Za-z_0-9]+}} = OpFunction %void None %{{[0-9]+}}
// CHECK: [[KEPT_PARAM:%[A-Za-z_0-9]+]] = OpFunctionParameter %int
// CHECK: %{{[0-9]+}} = OpLabel
// CHECK: DebugDeclare [[KEPT_DBGVAR]]
// CHECK: OpFunctionEnd
