//TEST:SIMPLE(filecheck=CHECK): -target wgsl

RWStructuredBuffer<float> outputBuffer;

// CHECK: fn inner{{.*}}( x{{.*}} : ptr<function, f32>)
// CHECK: (*x{{.*}}) = (*x{{.*}}) + 1.0
void inner(inout float x)
{
    x = x + 1;
}

// CHECK: fn test{{.*}}( x{{.*}} : ptr<function, f32>)
void test(inout float x)
{
    inner(x);
}

[numthreads(1,1,1)]
void computeMain()
{
    float x = 0.0;
    test(x);
    outputBuffer[0] = x;
}
