//TEST:SIMPLE: -target spirv -entry computeMain -stage compute -validate-ir -o 13074-static-append-buffer.spv
//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry computeMain -stage compute -validate-ir

//CHECK-NOT: cachedOutputBuffer
//CHECK: OpAtomicIIncrement

AppendStructuredBuffer<float> outputBuffer;
static AppendStructuredBuffer<float> cachedOutputBuffer = outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    cachedOutputBuffer.Append(1.0);
}
