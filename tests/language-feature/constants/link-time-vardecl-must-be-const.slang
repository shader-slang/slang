//TEST:SIMPLE(filecheck=CHECK): -target spirv -entry computeMain -stage compute -DUSE_EXTERN
//TEST:SIMPLE(filecheck=CHECK): -target spirv -entry computeMain -stage compute
//CHECK: error 31221

//TEST:SIMPLE(filecheck=CHECK_NO_FAIL): -target spirv -entry computeMain -stage compute -DUSE_EXTERN -DCPP
//TEST:SIMPLE(filecheck=CHECK_NO_FAIL): -target spirv -entry computeMain -stage compute -DCPP
//CHECK_NO_FAIL-NOT: error 31221

#ifdef CPP
__extern_cpp 
#endif
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
