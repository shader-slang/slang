//DISABLE_TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -shaderobj

/* Test here is test out operator overloading. This may not be a feature
that works in user code (it is used within the stdlib).

This produces:
.slang(10): error 30019: expected an expression of type 'Vec2d', got 'double'
    return {a.x + b.x, a.y + b.y};

Moreover...

.slang(18): error 30019: expected an expression of type 'Vec2d', got 'int'
    int a = 10 + 11;

It's as if only this operator+ impl can now be seen.
*/

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

struct Vec2d
{   
    double x, y;
};

Vec2d operator+(Vec2d a, Vec2d b)
{
    return {a.x + b.x, a.y + b.y};
}

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{    
    int index = dispatchThreadID.x;
    
    int a = 10 + 11;
    
    outputBuffer[dispatchThreadID.x] = int(v);
}