//TEST:SIMPLE(filecheck=CHECK): -target glsl -entry main -stage compute

RWStructuredBuffer<float> outputBuffer;

// Check that extensions on generics work
__glsl_extension(GL_generic_struct)
// CHECK-DAG: #extension GL_generic_struct : require
struct Q<T>
{
    T a;
}

// Check that extensions on types work
__glsl_extension(GL_non_generic_struct)
// CHECK-DAG: #extension GL_non_generic_struct : require
struct S
{
    float a;
}

// Check that extensions on functions work
__glsl_extension(GL_non_generic_function)
// CHECK-DAG: #extension GL_non_generic_function : require
float foo(S s)
{
    return s.a;
}

// Check that extensions on functions work
__glsl_extension(GL_generic_function)
// CHECK-DAG: #extension GL_generic_function : require
T bar<T>(Q<T> q)
{
    return q.a;
}

[numthreads(4, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    S s;
    s.a = 2;
    Q<float> q;
    q.a = 1;
    outputBuffer[dispatchThreadID.x] = foo(s) + bar(q);
}
