//TEST:SIMPLE(filecheck=CHECK): -target spirv
// CHECK: OpEntryPoint 
public interface IBSDF
{
    public float3 get_value();
}

public struct AnisotropicGGX // : IDifferentiable
{
    float2 alpha = {};
    public __init() { }
};

public struct TestDiffVisibility : IBSDF, IDifferentiable
{
    public AnisotropicGGX D = {};
    public __init() { }
    public float3 get_value() { return { 0.3 }; }
};

[numthreads(1,1,1)]
void main()
{
    TestDiffVisibility test = {};
    test.get_value();
}
