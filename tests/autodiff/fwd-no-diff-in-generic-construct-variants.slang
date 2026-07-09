//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type

// Test that forward-mode autodiff correctly handles all-void-operand constructs
// in generic functions: MakeVectorFromScalar, MakeMatrixFromScalar,
// MakeMatrix (from vectors), and MakeArrayFromElement.

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

interface IModel : IDifferentiable {
    [Differentiable]
    float eval(float x);
}

struct SimpleModel : IModel {
    float scale;
    [Differentiable]
    float eval(float x) { return scale * x; }
}

// --- MakeVectorFromScalar: lerp(float3, float3, no_diff float) ---
[Differentiable]
float3 test_vector_from_scalar<M : IModel>(M m, float3 a, float3 b, no_diff float t) {
    return lerp(a, b, t) * m.eval(1.0);
}

// --- MakeMatrixFromScalar: float3x3(no_diff float) ---
[Differentiable]
float test_matrix_from_scalar<M : IModel>(M m, float3 v, no_diff float s) {
    float3x3 mat = float3x3(s);
    float3 r = mul(mat, v);
    return m.eval(r.x);
}

// --- MakeMatrix from no_diff row vectors ---
[Differentiable]
float test_matrix_from_vector<M : IModel>(M m, float3 v, no_diff float3 row) {
    float3x3 mat = float3x3(row, row, row);
    float3 r = mul(mat, v);
    return m.eval(r.x);
}

// --- MakeArrayFromElement: array from no_diff element ---
[Differentiable]
float test_array_from_element<M : IModel>(M m, float x, no_diff float y) {
    float arr[3] = { y, y, y };
    return m.eval(x + arr[0]);
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    SimpleModel m;
    m.scale = 2.0;

    // test_vector_from_scalar: lerp((1,1,1), (3,3,3), 0.5) = (2,2,2), *2 = (4,4,4)
    // d/da with da=(1,1,1): d_lerp = 0.5*(1,1,1), *2 = (1,1,1)
    {
        DifferentialPair<SimpleModel> dpm = diffPair(m, SimpleModel.dzero());
        DifferentialPair<float3> dpa = diffPair(float3(1,1,1), float3(1,1,1));
        DifferentialPair<float3> dpb = diffPair(float3(3,3,3), float3(0,0,0));
        var r = __fwd_diff(test_vector_from_scalar)(dpm, dpa, dpb, 0.5);
        outputBuffer[0] = r.p.x; // CHECK: 4.0
        outputBuffer[1] = r.d.x; // CHECK: 1.0
    }

    // test_matrix_from_scalar: float3x3(0.5) fills all 9 elements with 0.5
    // mul(mat, (1,1,1)) = (1.5,1.5,1.5), r.x = 1.5, eval = 2*1.5 = 3.0
    // d/dv with dv=(1,0,0): mul(mat,dv) = (0.5,0.5,0.5), d(r.x) = 0.5, d(eval) = 2*0.5 = 1.0
    {
        DifferentialPair<SimpleModel> dpm = diffPair(m, SimpleModel.dzero());
        DifferentialPair<float3> dpv = diffPair(float3(1,1,1), float3(1,0,0));
        var r = __fwd_diff(test_matrix_from_scalar)(dpm, dpv, 0.5);
        outputBuffer[2] = r.p;   // CHECK: 3.0
        outputBuffer[3] = r.d;   // CHECK: 1.0
    }

    // test_matrix_from_vector: mat with all rows = (1,0,0), mul(mat, (1,2,3)) = (1,1,1)
    // eval = 2*1 = 2.0, d/dv with dv=(1,0,0): d(mul) = (1,1,1), d(eval) = 2*1 = 2.0
    {
        DifferentialPair<SimpleModel> dpm = diffPair(m, SimpleModel.dzero());
        DifferentialPair<float3> dpv = diffPair(float3(1,2,3), float3(1,0,0));
        var r = __fwd_diff(test_matrix_from_vector)(dpm, dpv, float3(1,0,0));
        outputBuffer[4] = r.p;   // CHECK: 2.0
        outputBuffer[5] = r.d;   // CHECK: 2.0
    }

    // test_array_from_element: arr = {1, 1, 1}, eval(x + arr[0]) = eval(2+1) = 2*3 = 6.0
    // d/dx with dx=1: d(eval) = 2*1 = 2.0
    {
        DifferentialPair<SimpleModel> dpm = diffPair(m, SimpleModel.dzero());
        DifferentialPair<float> dpx = diffPair(2.0, 1.0);
        var r = __fwd_diff(test_array_from_element)(dpm, dpx, 1.0);
        outputBuffer[6] = r.p;   // CHECK: 6.0
        outputBuffer[7] = r.d;   // CHECK: 2.0
    }
}
