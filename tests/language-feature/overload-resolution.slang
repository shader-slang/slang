//TEST:SIMPLE(filecheck=CHECK): -target hlsl -stage compute -entry main
RWStructuredBuffer<float> result;

[ForceInline]
float myF(inout int a, int b)
{
    return a + b;
}

[ForceInline]
float myF(inout uint a, uint b)
{
    return a - b;
}

[ForceInline]
T myGenF<T : __BuiltinIntegerType>(inout T a, T b)
{
    if (__isSignedInt<T>())
    {
        return a + b;
    }
    else
    {
        return a - b;
    }
}
// CHECK: result{{.*}}[0{{U?}}] = 1
// CHECK: result{{.*}}[1{{U?}}] = 4
// CHECK: result{{.*}}[2{{U?}}] = 1
// CHECK: result{{.*}}[3{{U?}}] = 4
[numthreads(1,1,1)]
void main()
{
    int ic = 1;
    uint a = 2;
    result[0] = myF(a, ic);

    int b = 3;
    uint uc = 1;
    result[1] = myF(b, uc);

    result[2] = myGenF(a, ic);
    result[3] = myGenF(b, uc);
}