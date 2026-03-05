// Implementation with multiple pointer fields of different types. Tests that
// AnyValue packing correctly handles mixed pointer/value fields: each pointer
// contributes its platform size (8 bytes on 64-bit) and is individually
// bitcast to uint2 during pack and back during unpack.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//DISABLE_TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type -emit-spirv-directly
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

struct Vec2
{
    int x;
    int y;
};

interface ICompute
{
    int compute();
}

// Three pointer fields: int*, float*, Vec2*
struct MultiPtrImpl : ICompute
{
    int* pInt;
    float* pFloat;
    Vec2* pVec;

    int compute()
    {
        return *pInt + (int)*pFloat + pVec->x + pVec->y;
    }
}

// No pointer fields for comparison
struct ValueImpl : ICompute
{
    int a;
    int b;

    int compute()
    {
        return a - b;
    }
}

ICompute makeCompute(int id, int* pi, float* pf, Vec2* pv)
{
    if (id == 0)
    {
        MultiPtrImpl m;
        m.pInt = pi;
        m.pFloat = pf;
        m.pVec = pv;
        return m;
    }
    else
    {
        ValueImpl v;
        v.a = 100;
        v.b = 1;
        return v;
    }
}

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    int ival = 10;
    float fval = 5.0;
    Vec2 vec;
    vec.x = 20;
    vec.y = 7;

    // MultiPtrImpl: 10 + 5 + 20 + 7 = 42
    var comp = makeCompute(0, &ival, &fval, &vec);
    // CHECK: 42
    outputBuffer[0] = comp.compute();

    // ValueImpl: 100 - 1 = 99
    var comp2 = makeCompute(1, nullptr, nullptr, nullptr);
    // CHECK: 99
    outputBuffer[1] = comp2.compute();

    // Modify pointed-to values and verify
    ival = 100;
    fval = 50.0;
    vec.x = 0;
    vec.y = 0;
    var comp3 = makeCompute(0, &ival, &fval, &vec);
    // CHECK: 150
    outputBuffer[2] = comp3.compute();
}
