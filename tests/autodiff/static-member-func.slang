//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-output-using-type -cpu

// Test calling bwd_diff on a static member function using the
// StructName::methodName syntax, e.g. bwd_diff(MyStruct::load)(...).

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

struct MyMaterial : IDifferentiable
{
    float roughness;
    float3 albedo;
}

struct MaterialData
{
    float roughnessValue;
    float3 albedoValue;
}

struct MyLoader
{
    // A backward-differentiable static method that loads
    // a differentiable struct from non-differentiable inputs.
    [BackwardDifferentiable]
    static MyMaterial load(no_diff MaterialData data, no_diff float2 uv)
    {
        MyMaterial m;
        m.roughness = data.roughnessValue;
        m.albedo = data.albedoValue;
        return m;
    }
}

// A backward-differentiable function that uses the loaded material.
[BackwardDifferentiable]
float computeValue(MyMaterial mat)
{
    // f(roughness, albedo) = roughness * (albedo.x + albedo.y + albedo.z)
    return mat.roughness * (mat.albedo.x + mat.albedo.y + mat.albedo.z);
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    MaterialData data;
    data.roughnessValue = 2.0;
    data.albedoValue = float3(1.0, 2.0, 3.0);
    float2 uv = float2(0.0, 0.0);

    // Forward: load material and compute value
    MyMaterial mat = MyLoader::load(data, uv);
    float value = computeValue(mat);

    // Backward: compute derivative of computeValue w.r.t. material
    var matPair = diffPair(mat);
    bwd_diff(computeValue)(matPair, 1.0);

    // matPair.d now holds the gradient:
    //   d/d(roughness) = albedo.x + albedo.y + albedo.z = 1+2+3 = 6
    //   d/d(albedo.x)  = roughness = 2
    //   d/d(albedo.y)  = roughness = 2
    //   d/d(albedo.z)  = roughness = 2

    // Now propagate that gradient through the static load function
    // using the StructName::methodName pattern.
    MyMaterial.Differential dMaterial = matPair.d;
    bwd_diff(MyLoader::load)(data, uv, dMaterial);

    // Since the load function simply copies from data fields,
    // the backward pass is essentially a no-op on the gradients
    // (all inputs are no_diff), but the call itself must compile
    // and run correctly.

    // CHECK: 6.0
    outputBuffer[0] = matPair.d.roughness;
    // CHECK: 2.0
    outputBuffer[1] = matPair.d.albedo.x;
    // CHECK: 2.0
    outputBuffer[2] = matPair.d.albedo.y;
    // CHECK: 2.0
    outputBuffer[3] = matPair.d.albedo.z;
}
