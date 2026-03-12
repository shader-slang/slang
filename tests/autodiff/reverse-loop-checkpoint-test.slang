//TEST(compute):COMPARE_COMPUTE_EX:-dx12 -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX:-cuda -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX:-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX:-wgpu -compute -shaderobj -output-using-type
//TEST:SIMPLE(filecheck=CHK):-target glsl -stage compute -entry computeMain -report-checkpoint-intermediates

//TEST_INPUT:ubuffer(data=[0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

typedef DifferentialPair<float> dpfloat;
typedef float.Differential dfloat;


[BackwardDifferentiable]
[PreferRecompute]
float compute(float x, float y)
{
    return x * y;
}

[BackwardDifferentiable]
[ForceInline]
float infinitesimal(float x)
{
    return x - detach(x);
}


// Test that computeLoop's intermediates have no arrays sitting
// around (must not cache the outvar from 'compute()')
// The CHK checks below verify the exact checkpoint contents.

[PreferRecompute]
[BackwardDifferentiable]
[ForceInline]
float3 infinitesimal(float3 x)
{
    return x - detach(x);
}

//CHK-DAG: note: checkpointing context of 16 bytes associated with function: 'computeLoop'
[BackwardDifferentiable]
[PreferRecompute]
float3 computeLoop(float y)
{
    //CHK-DAG: note: 4 bytes (float) used to checkpoint the following item:
    float w = 0;

    //CHK-DAG: note: 12 bytes (Vector<float, 3> ) used to checkpoint the following item:
    float3 w3 = float3(0, 0, 0);

    for (int i = 0; i < 8; i++)
    {
        float k = compute(i, y);
        float g = select(k > 0.0, k, 0.0);
        w += g;
        w3 += float3(g) * y;
    }

    var p = (w3 / float3(w));
    return infinitesimal(p);
}

[BackwardDifferentiable]
float test_simple_loop(float y)
{
    float3 x = computeLoop(y);
    return y + x.x;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    {
        dpfloat dpa = dpfloat(1.0, 0.0);

        __bwd_diff(test_simple_loop)(dpa, 1.0f);
        outputBuffer[0] = dpa.d; // Expect: 1.0
    }

    {
        dpfloat dpa = dpfloat(0.4, 0.0);

        __bwd_diff(test_simple_loop)(dpa, 0.5f);
        outputBuffer[1] = dpa.d; // Expect: 2.0
    }

    outputBuffer[2] = computeLoop(1.0).x;
}

//CHK-NOT: note