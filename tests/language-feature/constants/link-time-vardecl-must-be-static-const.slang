//TEST:SIMPLE(filecheck=CHECK): -target spirv -entry computeMain -stage compute -DUSE_EXTERN
//TEST:SIMPLE(filecheck=CHECK): -target spirv -entry computeMain -stage compute
//CHECK: error 31221

#ifdef USE_EXTERN
extern static int num;
#else
export static int num = 10;
#endif

RWStructuredBuffer<float> outputBuffer;

[numthreads(1,1,1)]
void computeMain()
{
    outputBuffer[0] = num;
}
