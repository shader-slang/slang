//TEST:SIMPLE(filecheck=RET):-target spirv -entry computeMainReturned -stage compute
//TEST:SIMPLE(filecheck=DEREF):-target spirv -entry computeMainDereferenced -stage compute

// Regression test for shader-slang/slang#12563. A pointer local written `&gShared` on one path and
// the result of a call returning `&gShared` on the other holds only Workgroup pointers, so it must
// compile with no conflict diagnostic, whether the local is returned (`pickReturned`) or only
// dereferenced (`writeDereferenced`). The call's result address space is known only once the
// callee settles; until then its declared `Ptr<T>` default is not a storage class of its own.

RWStructuredBuffer<int> outputBuffer;
groupshared int gShared;

[noinline]
int* helper()
{
    return &gShared;
}

[noinline]
int* pickReturned(int n)
{
    int* r;
    if (n <= 0)
        r = &gShared;
    else
        r = helper();
    return r;
}

[noinline]
void writeDereferenced(int n)
{
    int* r;
    if (n <= 0)
        r = &gShared;
    else
        r = helper();
    *r = n;
}

// RET-DAG: OpEntryPoint
// RET-DAG: OpFunctionCall
// RET-NOT: error
[numthreads(1, 1, 1)]
void computeMainReturned(uint3 tid: SV_DispatchThreadID)
{
    *pickReturned(int(tid.x)) = 1;
    outputBuffer[0] = gShared;
}

// DEREF-DAG: OpEntryPoint
// DEREF-DAG: OpFunctionCall
// DEREF-NOT: error
[numthreads(1, 1, 1)]
void computeMainDereferenced(uint3 tid: SV_DispatchThreadID)
{
    writeDereferenced(int(tid.x));
    outputBuffer[0] = gShared;
}
