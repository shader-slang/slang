// A `groupshared` parameter lowers to a `RefParam`: bare `groupshared` with `ReadWrite` access
// (`0`) and `const groupshared` with `Read` access (`1`). It is never a `BorrowInParam`, because
// lowering may copy a borrowed argument into a temporary, and a callee that reads after a barrier
// would then miss the other invocations' writes. The arguments are a whole group-shared variable
// and parts of one: an array element, a field, and a vector or matrix component. `[noinline]`
// keeps each callee in the dump (shader-slang/slang#10641).

//TEST:SIMPLE(filecheck=CHECK):-target cuda -entry main -stage compute -o - -dump-ir
//TEST:SIMPLE(filecheck=NOBORROW):-target cuda -entry main -stage compute -o - -dump-ir

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

groupshared uint sh[8];
groupshared Outer go;
groupshared Inner arr[4];

[noinline]
void readOnlyArray(const groupshared uint a[8])
{
}

[noinline]
void readWriteArray(groupshared uint a[8])
{
    a[0] = 1;
}

[noinline]
void readOnlyScalar(const groupshared uint a)
{
}

[noinline]
void readWriteScalar(groupshared uint a)
{
    a = 1;
}

[noinline]
void readOnlyComponent(const groupshared float a)
{
}

[noinline]
void readOnlyStruct(const groupshared Inner s)
{
}

[noinline]
void readWriteStruct(groupshared Inner s)
{
    s.v = 2;
}

[numthreads(8, 1, 1)]
void main(uint tid : SV_DispatchThreadID)
{
    sh[tid] = tid;
    go.inner.v = tid;

    readOnlyArray(sh);
    readWriteArray(sh);

    readOnlyScalar(go.inner.v);
    readWriteScalar(go.inner.v);
    readOnlyScalar(arr[0].v);
    readOnlyScalar(go.scratch[0]);

    readOnlyComponent(go.vector.x);
    readOnlyComponent(go.matrix._m00);

    readOnlyStruct(go.inner);
    readOnlyStruct(arr[1]);
    readWriteStruct(go.inner);
}

//CHECK-DAG: func %readOnlyArray{{.*}}RefParam(Array(UInt, 8 : Int), 1 : UInt64, 2147483647
//CHECK-DAG: func %readOnlyScalar{{.*}}RefParam({{.*}}1 : UInt64, 2147483647
//CHECK-DAG: func %readOnlyComponent{{.*}}RefParam({{.*}}1 : UInt64, 2147483647
//CHECK-DAG: func %readOnlyStruct{{.*}}RefParam({{.*}}1 : UInt64, 2147483647
//CHECK-DAG: func %readWriteArray{{.*}}RefParam(Array(UInt, 8 : Int), 0 : UInt64, 2147483647
//CHECK-DAG: func %readWriteScalar{{.*}}RefParam({{.*}}0 : UInt64, 2147483647
//CHECK-DAG: func %readWriteStruct{{.*}}RefParam({{.*}}0 : UInt64, 2147483647

//NOBORROW-NOT: RateQualified(GroupShared, BorrowInParam
