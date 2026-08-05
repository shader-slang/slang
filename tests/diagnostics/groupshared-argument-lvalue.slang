// An argument to a `groupshared` parameter must name thread-group-shared storage -- an addressable
// expression in `AddressSpace::GroupShared` (a `groupshared` variable, a component of one, or a
// dereference of a group-shared pointer). A private local, a copy, or an rvalue cannot be aliased
// as thread-group-shared memory, so it is rejected (shader-slang/slang#10641; aligns with HLSL/DXC
// error 0043).

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

    // Deferring to the base does not make every member access acceptable: a field of a private
    // local is still rejected.
    Holder localHolder;
    takesGroupSharedScalar(localHolder.v);
}

// CHECK: 30711