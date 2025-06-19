//TEST:SIMPLE(filecheck=CHECK): -target spirv
// CHECK: OpExecutionMode {{.*}} DepthReplacing
// CHECK: OpExecutionMode %FS_WriteDepthGt DepthGreater
// CHECK: OpExecutionMode %FS_WriteDepthLt DepthLess

[shader("fragment")]
void FS_WriteDepthGt(out float depth: SV_DepthGreaterEqual)
{
    depth = 0.5;
}

[shader("fragment")]
void FS_WriteDepthLt(out float depth: SV_DepthLessEqual)
{
    depth = 0.5;
}
