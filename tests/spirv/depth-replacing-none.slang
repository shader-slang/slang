//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage fragment -entry main -emit-spirv-directly

// Test that a fragment shader that does not modify depth does not
// have a DepthReplacing decoration.

struct PSOut
{
    float4 color : SV_Target;
}
// CHECK-NOT: OpExecutionMode {{.*}} DepthReplacing

PSOut main()
{
    PSOut psout;
    psout.color = float4(1.0f, 0.0f, 0.0f, 1.0f);
    return psout;
}
