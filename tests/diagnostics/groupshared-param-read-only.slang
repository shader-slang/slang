//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target cuda -entry main -stage compute

// A read-only `groupshared` parameter (`const groupshared`, or the equivalent
// `__constref groupshared`) is a reference its callee cannot write through (shader-slang/slang#10641).
// It can be passed on to another read-only `groupshared` parameter, which is still the same
// group-shared storage, but not to a read-write one.

groupshared uint sh[8];

uint readOnly(const groupshared uint a[8])
{
    return a[0];
}

void readWrite(groupshared uint a[8])
{
    a[0] = 1;
}

// --- Accepted: a read-only parameter passed on to another read-only parameter ---

uint forwardConst(const groupshared uint a[8])
{
    return readOnly(a);
}

uint forwardConstRef(__constref groupshared uint a[8])
{
    return readOnly(a);
}

// --- Rejected: a read-only parameter passed on to a read-write parameter ---

void forwardToReadWrite(const groupshared uint a[8])
{
    readWrite(a);
//CHECK:      ^ argument passed to parameter '0' must be l-value.
//CHECK:      ^ error E30047
}

// --- Rejected: writes through either read-only spelling ---

void writeConst(const groupshared uint a[8])
{
    a[0] = 1;
//CHECK: ^ left of '=' is not an l-value.
//CHECK: ^ error E30011
}

void writeConstRef(__constref groupshared uint a[8])
{
    a[0] = 1;
//CHECK: ^ left of '=' is not an l-value.
//CHECK: ^ error E30011
}

// --- Rejected: an atomic, whose destination is taken by reference ---

void atomicConst(const groupshared uint a[8])
{
    InterlockedAdd(a[0], 1);
//CHECK:            ^ argument passed to parameter '0' must be l-value.
//CHECK:            ^ error E30047
}

[numthreads(8, 1, 1)]
void main(uint tid : SV_DispatchThreadID)
{
    sh[tid] = tid;
    sh[tid] = forwardConst(sh) + forwardConstRef(sh);
}
