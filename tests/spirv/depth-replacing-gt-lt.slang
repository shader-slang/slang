//TEST:SIMPLE(filecheck=CHECK): -target spirv
// CHECK: OpExecutionMode {{.*}} DepthReplacing
// CHECK: OpExecutionMode %FS_WriteDepthGt DepthGreater
// CHECK: OpExecutionMode %FS_WriteDepthLt DepthLess

[shader("fragment")]
void FS_WriteDepthGt(float a, out float depth: SV_DepthGreaterEqual)
{
    depth = a;
}

[shader("fragment")]
void FS_WriteDepthLt(float a, out float depth: SV_DepthLessEqual)
{
    depth = a;
}
