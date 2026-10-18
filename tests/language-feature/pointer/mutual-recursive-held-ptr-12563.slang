//TEST:SIMPLE(filecheck=CHECK):-target spirv -stage compute -entry computeMain -disable-non-essential-validations -skip-spirv-validation
//TEST:SIMPLE(filecheck=CHECK):-target spirv -stage compute -entry computeMainPtrArg -disable-non-essential-validations -skip-spirv-validation
//TEST:SIMPLE(filecheck=CHECK):-target spirv -stage compute -entry computeMainDirect -disable-non-essential-validations -skip-spirv-validation
//TEST:SIMPLE(filecheck=CONFLICT):-target spirv -stage compute -entry computeMainConflict -disable-non-essential-validations -skip-spirv-validation

// Regression test for shader-slang/slang#12563. With the recursion check (E55201) disabled, a
// recursive call's result is provisional until the cycle settles, and it must not read as a second
// storage class. Mutual recursion `a -> b -> a` (no pointer argument), `c -> d -> c` (pointer
// argument, so the back-edge reuses a cached clone) and `e -> f -> e` (direct returns, no local)
// yield only Workgroup pointers, so none may report E58005. A settled call to the same recursive function outside the cycle still
// feeds conflict detection: `computeMainConflict` merges it with a StorageBuffer pointer (E58003).
// Once the cycle settles, every call's result type matches its callee's Workgroup return type,
// including the back-edge call. A recursive call graph is invalid SPIR-V regardless, so SPIR-V
// validation is skipped.

groupshared int gShared;
RWStructuredBuffer<int> outBuf;

int* b(int n);

[noinline]
int* a(int n)
{
    int* r;
    if (n <= 0)
        r = &gShared;
    else
        r = b(n - 1);
    return r;
}

[noinline]
int* b(int n)
{
    int* r;
    if (n <= 0)
        r = &gShared;
    else
        r = a(n - 1);
    return r;
}

int* d(int* p, int n);

[noinline]
int* c(int* p, int n)
{
    int* r;
    if (n <= 0)
        r = &gShared;
    else
        r = d(p, n - 1);
    return r;
}

[noinline]
int* d(int* p, int n)
{
    int* r;
    if (n <= 0)
        r = &gShared;
    else
        r = c(p, n - 1);
    return r;
}

int* f(int n);

[noinline]
int* e(int n)
{
    if (n <= 0)
        return &gShared;
    return f(n - 1);
}

[noinline]
int* f(int n)
{
    if (n <= 0)
        return &gShared;
    return e(n - 1);
}

// CHECK-NOT: E58005
// CHECK: OpEntryPoint
// CHECK-NOT: OpFunctionCall %_ptr_PhysicalStorageBuffer_int
[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    *a(int(tid.x)) = 1;
    outBuf[0] = gShared;
}

[numthreads(1, 1, 1)]
void computeMainPtrArg(uint3 tid: SV_DispatchThreadID)
{
    *c(&gShared, int(tid.x)) = 1;
    outBuf[0] = gShared;
}

// CONFLICT: E58003
[numthreads(1, 1, 1)]
void computeMainDirect(uint3 tid: SV_DispatchThreadID)
{
    *e(int(tid.x)) = 1;
    outBuf[0] = gShared;
}

[numthreads(1, 1, 1)]
void computeMainConflict(uint3 tid: SV_DispatchThreadID)
{
    int* r;
    if (tid.x == 0)
        r = c(&gShared, int(tid.x));
    else
        r = &outBuf[tid.x];
    outBuf[0] = *r;
}
