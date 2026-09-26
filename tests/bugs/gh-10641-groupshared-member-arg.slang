// gh-10641-groupshared-member-arg.slang

// A field of a group-shared variable, and a component of such a field, name group-shared storage,
// so each is a valid argument for a `groupshared` parameter and is passed by reference rather than
// copied per invocation (shader-slang/slang#10641).

//TEST:SIMPLE(filecheck=CHECK):-target cuda -entry main -stage compute -o - -dump-ir

struct Inner
{
    uint v;
}

struct Outer
{
    Inner inner;
    uint scratch[4];
    float4 vector;
    float2x2 matrix;
}

groupshared Outer go;
groupshared Inner arr[4];

[noinline]
void readOnlyField(const groupshared uint a)
{
}

[noinline]
void readWriteField(groupshared uint a)
{
    a = 1;
}

[noinline]
void readOnlyComponent(const groupshared float a)
{
}

[noinline]
void readOnlyWholeStruct(const groupshared Inner s)
{
}

[noinline]
void readWriteWholeStruct(groupshared Inner s)
{
    s.v = 2;
}

[numthreads(8, 1, 1)]
void main(uint tid : SV_DispatchThreadID)
{
    go.inner.v = tid;

    // A field of a group-shared variable, reached through one and through two member accesses.
    readOnlyField(go.inner.v);
    readWriteField(go.inner.v);

    // A field of a group-shared array element -- mixes the member and index paths.
    readOnlyField(arr[0].v);

    // An element of an array that is itself a field.
    readOnlyField(go.scratch[0]);

    // A component of a field.
    readOnlyComponent(go.vector.x);
    readOnlyComponent(go.matrix._m00);

    // A whole group-shared struct, passed by reference rather than field by field. The read-write
    // spelling writes through to the caller's storage.
    readOnlyWholeStruct(go.inner);
    readOnlyWholeStruct(arr[1]);
    readWriteWholeStruct(go.inner);
}

// A `const groupshared` parameter is a `RefParam` with `Read` access (`1`) ...
//CHECK-DAG: func %readOnlyField{{.*}}RefParam({{.*}}1 : UInt64, 2147483647
//CHECK-DAG: func %readOnlyComponent{{.*}}RefParam({{.*}}1 : UInt64, 2147483647
//CHECK-DAG: func %readOnlyWholeStruct{{.*}}RefParam({{.*}}1 : UInt64, 2147483647

// ... and a bare `groupshared` parameter is a `RefParam` with `ReadWrite` access (`0`).
//CHECK-DAG: func %readWriteField{{.*}}RefParam({{.*}}0 : UInt64, 2147483647
//CHECK-DAG: func %readWriteWholeStruct{{.*}}RefParam({{.*}}0 : UInt64, 2147483647

// The opposite direction -- a field of a *non*-group-shared variable must still be rejected -- is
// covered by tests/diagnostics/groupshared-argument-lvalue.slang.
