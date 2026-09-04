// An argument to a `groupshared` parameter must name thread-group-shared storage -- a `groupshared`
// variable, a component of one, or a `groupshared` parameter forwarded on. A private local, a copy,
// or an rvalue cannot be aliased as thread-group-shared memory, so it is rejected
// (shader-slang/slang#10641; aligns with HLSL/DXC error 0043).

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -entry computeMain -stage compute

static const uint N = 8;
groupshared uint sharedData[N];
groupshared float4 sharedVec;
groupshared float2x2 sharedMat;

struct Holder
{
    float v;
}

groupshared Holder sharedStruct;
groupshared Holder sharedStructs[2];

void takesGroupShared(uint tid, groupshared uint s[N])
{
    s[tid] = tid;
}

void takesGroupSharedScalar(groupshared float v)
{
}

void takesGroupSharedVector(const groupshared float2 v)
{
}

// Forwarding a `groupshared` parameter into another `groupshared` parameter is valid; the argument
// still names group-shared storage.
void forwards(uint tid, groupshared uint s[N])
{
    takesGroupShared(tid, s);
}

[shader("compute")]
[numthreads(N, 1, 1)]
void computeMain(uint3 tid : SV_GroupThreadID)
{
    // Every argument below names group-shared storage -- a global, a parenthesized form, a single
    // vector/matrix component, and a forwarded `groupshared` parameter -- so none is diagnosed.
    takesGroupShared(tid.x, sharedData);
    takesGroupShared(tid.x, (sharedData));
    takesGroupSharedScalar(sharedVec.x);
    takesGroupSharedScalar((sharedVec).y);
    takesGroupSharedScalar(sharedMat._m00);
    forwards(tid.x, sharedData);

    // A field of a group-shared variable also names group-shared storage: the address space comes
    // from the base object, since `groupshared` is never on the field itself.
    takesGroupSharedScalar(sharedStruct.v);
    takesGroupSharedScalar(sharedStructs[0].v);

    // A private local array is not thread-group-shared memory.
    uint localData[N];
    takesGroupShared(tid.x, localData);
//CHECK:                    ^^^^^^^^^ error E30711

    // A field of private storage is not thread-group-shared memory either.
    Holder localHolder;
    takesGroupSharedScalar(localHolder.v);
//CHECK:                               ^ error E30711

    // A multi-element swizzle may be non-contiguous, so it has no address of its own and the caller
    // has to materialize a private temporary for it -- which is not group-shared storage, even
    // though the vector it reads from is.
    takesGroupSharedVector(sharedVec.xy);
//CHECK:                             ^^ error E30711
    takesGroupSharedVector(sharedMat._m00_m11);
//CHECK:                             ^^^^^^^^ error E30711
}
