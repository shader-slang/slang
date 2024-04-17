//TEST:SIMPLE(filecheck=CHECK): -target metal

RWStructuredBuffer<float> outputBuffer;

// CHECK: [[kernel]] void main()
[numthreads(1,1,1)]
void main()
{
    outputBuffer[0] = 1.0f;
}