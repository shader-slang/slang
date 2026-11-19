//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage fragment -entry main -emit-spirv-directly
//TEST:SIMPLE(filecheck=GLSL): -target glsl -stage fragment -entry main
//TEST:SIMPLE(filecheck=SCALAR): -target spirv -stage fragment -entry mainScalar -emit-spirv-directly
//TEST:SIMPLE(filecheck=ZERO): -target spirv -stage fragment -entry mainZero -emit-spirv-directly

// Regression for shader-slang/slang#12653: a partial write to an `out` array
// parameter must store only the assigned elements, leaving the rest untouched.
// The fix is in the shared GLSL/SPIR-V varying legalizer, so both targets are
// exercised here.

[shader("fragment")]
void main(out float4 result[8] : SV_Target)
{
    result[1] = float4(0.25);
    result[5] = float4(0.75);
}

// On SPIR-V the written elements are stored through per-element access chains
// into %result. The whole array is neither reconstructed (OpCompositeConstruct)
// nor stored back to the array root; the buggy whole-array copy would be emitted
// after these element stores, so the tail-scoped negatives below are sufficient.
// The undef negative is scoped to the vector element type.
//CHECK: OpAccessChain %{{[A-Za-z0-9_]*}} %result %int_1
//CHECK: OpStore
//CHECK: OpAccessChain %{{[A-Za-z0-9_]*}} %result %int_5
//CHECK: OpStore
//CHECK-NOT: OpCompositeConstruct %_arr_v4float_int_8
//CHECK-NOT: OpUndef %v4float
//CHECK-NOT: OpStore %result %

// On the GLSL target the element assignments are emitted directly, with no
// whole-array copy-back of the output variable.
//GLSL: result{{[A-Za-z0-9_]*}}[1] = vec4(0.25
//GLSL: result{{[A-Za-z0-9_]*}}[5] = vec4(0.75
//GLSL-NOT: result{{[A-Za-z0-9_]*}} = {{[^=]}}

// A same-typed scalar/vector output also takes the direct path: it stores
// straight into the output variable rather than round-tripping through a local
// temp. The Function-storage temp negative sits between the output-variable
// declaration and the store so it covers the function body, where a temp (if the
// old path were taken) would be declared.
[shader("fragment")]
void mainScalar(out float4 result : SV_Target)
{
    result = float4(0.5);
}
//SCALAR: %result = OpVariable %_ptr_Output_v4float Output
//SCALAR-NOT: OpVariable %_ptr_Function_v4float Function
//SCALAR: OpStore %result

// A return path that writes no element emits no stores and no whole-array
// reconstruction at all, instead of the old whole-array store of undefined
// values.
[shader("fragment")]
void mainZero(out float4 result[8] : SV_Target)
{
}
//ZERO: OpEntryPoint Fragment %mainZero
//ZERO-NOT: OpStore
//ZERO-NOT: OpCompositeConstruct %_arr_v4float_int_8
