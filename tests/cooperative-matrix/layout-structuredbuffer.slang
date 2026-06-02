//TEST:SIMPLE(filecheck=CHECK):-target spirv -emit-spirv-directly -entry computeMain -stage compute -capability cooperative_matrix

// Check that cooperative matrix StructuredBuffer overloads preserve the
// StructuredBuffer data layout on the element pointer used by load/store.

// CHECK-DAG: OpDecorate %[[PTR_140:[A-Za-z0-9_]+]] ArrayStride 16
// CHECK-DAG: OpDecorate %[[PTR_430:[A-Za-z0-9_]+]] ArrayStride 4
// CHECK-DAG: %[[COOP_MAT:[A-Za-z0-9_]+]] = OpTypeCooperativeMatrixKHR %float
// CHECK-DAG: %[[PTR_140]] = OpTypePointer StorageBuffer %float
// CHECK-DAG: %[[PTR_430]] = OpTypePointer StorageBuffer %float
// The trailing access-chain index and store operand are temporarily wildcarded:
// disabling the MergeReturn/InlineExhaustive pair for #11146 can spell those
// values differently while preserving the layout-sensitive pointer and load/store
// coverage. Restore strict checks when the optimizer workaround is removed.
// CHECK: %[[INPUT_140:[A-Za-z0-9_]+]] = OpAccessChain %[[PTR_140]] %input140 %int_0 %{{[A-Za-z0-9_]+}}
// CHECK: %[[MAT_140:[A-Za-z0-9_]+]] = OpCooperativeMatrixLoadKHR %[[COOP_MAT]] %[[INPUT_140]]
// CHECK: %[[OUTPUT_140:[A-Za-z0-9_]+]] = OpAccessChain %[[PTR_140]] %output140 %int_0 %{{[A-Za-z0-9_]+}}
// CHECK: OpCooperativeMatrixStoreKHR %[[OUTPUT_140]] %{{[A-Za-z0-9_]+}}
// CHECK: %[[INPUT_430:[A-Za-z0-9_]+]] = OpAccessChain %[[PTR_430]] %input430 %int_0 %{{[A-Za-z0-9_]+}}
// CHECK: %[[MAT_430:[A-Za-z0-9_]+]] = OpCooperativeMatrixLoadKHR %[[COOP_MAT]] %[[INPUT_430]]
// CHECK: %[[OUTPUT_430:[A-Za-z0-9_]+]] = OpAccessChain %[[PTR_430]] %output430 %int_0 %{{[A-Za-z0-9_]+}}
// CHECK: OpCooperativeMatrixStoreKHR %[[OUTPUT_430]] %{{[A-Za-z0-9_]+}}

// The scalar float element type has the same size in both layouts, but the
// StructuredBuffer element stride differs: std140 rounds up to 16, std430 stays at 4.

using namespace linalg;

StructuredBuffer<float, Std140DataLayout> input140;
RWStructuredBuffer<float, Std140DataLayout> output140;

StructuredBuffer<float, Std430DataLayout> input430;
RWStructuredBuffer<float, Std430DataLayout> output430;

typealias CoopMatType = CoopMat<float, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse.MatrixAccumulator>;

[numthreads(32, 1, 1)]
void computeMain()
{
    let stride = 16;

    let a = CoopMatType.Load<CoopMatMatrixLayout.RowMajor>(input140, 0, stride);
    a.Store<CoopMatMatrixLayout.RowMajor>(output140, 0, stride);

    let b = CoopMatType.Load<CoopMatMatrixLayout.RowMajor>(input430, 0, stride);
    b.Store<CoopMatMatrixLayout.RowMajor>(output430, 0, stride);
}
