//TEST:SIMPLE(filecheck=CHECK_PASS): -target spirv -entry computeMain -stage compute -DUSE_EXTERN
//TEST:SIMPLE(filecheck=CHECK_PASS): -target spirv -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK_PASS): -target spirv -entry computeMain -stage compute -DUSE_EXTERN -DUSE_CONST -DUSE_STATIC
//TEST:SIMPLE(filecheck=CHECK_PASS): -target spirv -entry computeMain -stage compute -DUSE_CONST -DUSE_STATIC
//CHECK_PASS-NOT: error 31221

//TEST:SIMPLE(filecheck=CHECK_FAIL): -target spirv -entry computeMain -stage compute -DUSE_EXTERN -DUSE_CONST
//TEST:SIMPLE(filecheck=CHECK_FAIL): -target spirv -entry computeMain -stage compute -DUSE_CONST
//TEST:SIMPLE(filecheck=CHECK_FAIL): -target spirv -entry computeMain -stage compute -DUSE_EXTERN -DUSE_STATIC
//TEST:SIMPLE(filecheck=CHECK_FAIL): -target spirv -entry computeMain -stage compute -DUSE_STATIC
//CHECK_FAIL: error 31221


#ifdef USE_CONST
const
#endif
#ifdef USE_STATIC
static
#endif
#ifdef USE_EXTERN
extern int num;
#else
export int num = 10;
#endif

RWStructuredBuffer<float> outputBuffer;

[numthreads(1,1,1)]
void computeMain()
{
    outputBuffer[0] = num;
}
