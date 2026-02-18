// Generic interfaces specialized with a user-defined struct should support
// dynamic dispatch. Tests ITransform<T> where T is a Vec2 struct, verifying
// that non-builtin type arguments work through dynamic dispatch.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST:SIMPLE(filecheck=REPORT):-target hlsl -stage compute -entry computeMain -report-dynamic-dispatch-sites

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

struct Vec2
{
    float x;
    float y;
}

interface ITransform<T>
{
    T apply(T v);
    float reduce(T v);
}

struct ScaleTransform : ITransform<Vec2>
{
    Vec2 apply(Vec2 v) { return Vec2(v.x * 2.0, v.y * 2.0); }
    float reduce(Vec2 v) { return v.x + v.y; }
}

struct NegateTransform : ITransform<Vec2>
{
    Vec2 apply(Vec2 v) { return Vec2(-v.x, -v.y); }
    float reduce(Vec2 v) { return v.x * v.y; }
}

ITransform<Vec2> makeTransform(int id)
{
    if (id == 0) return ScaleTransform();
    else return NegateTransform();
}

float test(int id, float x, float y)
{
    let t = makeTransform(id);
    Vec2 v = Vec2(x, y);
    let result = t.apply(v);
    // REPORT-DAG: ([[# @LINE-1]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
    return t.reduce(result);
    // REPORT-DAG: ([[# @LINE-1]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
}

[numthreads(1, 1, 1)]
void computeMain()
{
    // ScaleTransform: apply(3,4) = (6,8), reduce(6,8) = 6+8 = 14
    outputBuffer[0] = test(0, 3.0, 4.0); // CHECK: 14.0
    // NegateTransform: apply(3,4) = (-3,-4), reduce(-3,-4) = (-3)*(-4) = 12
    outputBuffer[1] = test(1, 3.0, 4.0); // CHECK: 12.0
}
