//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage fragment -entry main -emit-spirv-directly

// Regression for shader-slang/slang#12653: a partial write to an `out` array
// parameter must store only the assigned elements, leaving the rest untouched.

[shader("fragment")]
void main(out float4 result[8] : SV_Target)
{
    result[1] = float4(0.25);
    result[5] = float4(0.75);
}

//CHECK: OpAccessChain %{{[A-Za-z0-9_]*}} %result %int_1
//CHECK: OpStore
//CHECK: OpAccessChain %{{[A-Za-z0-9_]*}} %result %int_5
//CHECK: OpStore

// The unwritten elements must not be touched: the whole array is neither
// reconstructed nor stored back to the output variable's root.
//CHECK-NOT: OpCompositeConstruct %_arr_v4float_int_8
//CHECK-NOT: OpUndef
//CHECK-NOT: OpStore %result %
