// A multi-component swizzle assignment that reaches emit as a `swizzleSet` (here through a
// property setter) must be spelled one component at a time on targets whose vector types have no
// swizzle l-value: C++, CUDA and WGSL.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-cpu -compute -output-using-type -shaderobj
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-vk -compute -output-using-type -shaderobj
//TEST:SIMPLE(filecheck=CPP): -target cpp -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSL): -target wgsl -entry computeMain -stage compute

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

struct S
{
    float4 data;

    property float4 v
    {
        get { return data; }
        set { data = newValue; }
    }
}

[numthreads(1, 1, 1)]
void computeMain()
{
    S s;
    s.data = float4(1.0f, 2.0f, 3.0f, 4.0f);
    s.v.zx = float2(5.0f, 6.0f);

    outputBuffer[0] = s.data.x;
    outputBuffer[1] = s.data.y;
    outputBuffer[2] = s.data.z;
    outputBuffer[3] = s.data.w;
}

// BUF: 6
// BUF-NEXT: 2
// BUF-NEXT: 5
// BUF-NEXT: 4

// CPP: [[V:_S[0-9]+]].z = [[SRC:_S[0-9]+]].x;
// CPP-NEXT: [[V]].x = [[SRC]].y;
// CPP-NOT: .zx =

// CUDA: [[V:_S[0-9]+]].z = [[SRC:_S[0-9]+]].x;
// CUDA-NEXT: [[V]].x = [[SRC]].y;
// CUDA-NOT: .zx =

// WGSL: [[V:_S[0-9]+]].z = [[SRC:_S[0-9]+]].x;
// WGSL-NEXT: [[V]].x = [[SRC]].y;
// WGSL-NOT: .zx =
