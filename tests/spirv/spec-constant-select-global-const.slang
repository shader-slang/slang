//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry main -stage compute

// Test that a ternary/select on specialization constants used as a global
// constant value compiles successfully to SPIR-V using OpSpecConstantOp.
// Regression test for https://github.com/shader-slang/slang/issues/10373

// CHECK-DAG: %[[FOO2:[0-9A-Za-z_]+]] = OpSpecConstant %uint 0
// CHECK-DAG: %[[FOO1:[0-9A-Za-z_]+]] = OpSpecConstant %uint 0
// CHECK-DAG: %[[ZERO:[0-9A-Za-z_]+]] = OpConstant %uint 0
// CHECK-DAG: %[[CMP:[0-9A-Za-z_]+]] = OpSpecConstantOp %bool IEqual %[[FOO2]] %[[ZERO]]
// CHECK-DAG: %[[DIV:[0-9A-Za-z_]+]] = OpSpecConstantOp %uint UDiv %[[FOO1]] %[[FOO2]]
// CHECK-DAG: OpSpecConstantOp %uint Select %[[CMP]] %[[ZERO]] %[[DIV]]

[vk::specialization_constant] const uint FOO1 = 0;
[vk::specialization_constant] const uint FOO2 = 0;

static const uint value = FOO2 == 0 ? 0 : FOO1 / FOO2;

[shader("compute")]
[numthreads(64, 1, 1)]
void main(uint tid : SV_GroupIndex) {
    printf("%d\n", value);
}
