// An [AutoPyBindCUDA] kernel with a parameter that has no host mapping (String)
// must diagnose E56001 and fail compilation. The mappable parameter is listed
// first so the partial-wrapper cleanup path (discarding already-emitted parameter
// IR) is exercised.

//TEST:SIMPLE(filecheck=CHECK): -target torch -line-directive-mode none

[AutoPyBindCUDA]
[CudaKernel]
void myKernel(float x, String s)
{
}

//CHECK: result code = -1
//CHECK: error{{.*}}E56001{{.*}}CUDA type mapping failed
