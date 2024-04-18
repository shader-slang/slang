//TEST:SIMPLE(filecheck=CHECK): -target metal

uniform RWStructuredBuffer<float> outputBuffer;

// CHECK: {{.*}}kernel{{.*}} void main_kernel(float device* {{.*}})

void func()
{
    outputBuffer[0] = 1.0f;
    outputBuffer[1] = outputBuffer.Load(0);
}

[numthreads(1,1,1)]
void main_kernel()
{
    func();
}