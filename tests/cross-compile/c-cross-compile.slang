//TEST:CPP_COMPILER_COMPILE: -profile cs_5_0 -entry computeMain -target cpp

enum Color
{
    Red,
    Green = 2,
    Blue,
}

int test(int val)
{
    Color c = Color.Red;

    if(val > 1)
    {
        c = Color.Green;
    }

    if(c == Color.Red)
    {
        if(val & 1)
        {
            c = Color.Blue;
        }
    }

    switch(c)
    {
    case Color.Red:
        val = 1;
        break;

    case Color.Green:
        val = 2;
        break;

    case Color.Blue:
        val = 3;
        break;

    default:
        val = -1;
        break;
    }

    return (val << 4) + int(c);
}

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):dxbinding(0),glbinding(0),out
RWStructuredBuffer<int> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint tid = dispatchThreadID.x;

    matrix<float, 2, 3> mat = { { 0, 1, 2 }, { 3, 4, 5} };
    vector<float, 2> vec = { float(tid + 1), float(tid + 2) };


    vector<float, 3> vec2 = mul(vec, mat);

    vec2 += (-vec2.zyx) * 2;
    
    vector<bool, 3> z = vec2 > 0;

    int val = int(tid) + any(z) ? 1 : 0;
    val = test(val);

    outputBuffer[tid] = val + int(dot(vec2, vec2));
}