//TEST:SIMPLE(filecheck=CHECK): -target metal
//TEST:SIMPLE(filecheck=CHECK-ASM): -target metallib

uniform RWStructuredBuffer<float> outputBuffer;

// CHECK: {{.*}}kernel{{.*}} void main_kernel(float device* {{.*}})

// CHECK-ASM: define void @main_kernel

void func(float v)
{
    outputBuffer[0] = v;
    outputBuffer[1] = outputBuffer.Load(0);
}

[numthreads(1,1,1)]
void main_kernel()
{
    func(3.0f);
}