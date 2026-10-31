// gh-10641-const-groupshared-param.slang

// A `groupshared` parameter is always a reference to one thread-group-shared location. The
// read-only spelling `const groupshared` therefore uses the same strict `ref` passing mode as the
// read-write spelling and differs from it only in access. A borrow
// (`BorrowInParam`) is not enough: lowering may copy a borrowed argument into a temporary when it
// cannot take its address, and a callee that reads after `GroupMemoryBarrierWithGroupSync()` would
// then see a stale snapshot instead of the other invocations' writes.
//
// `[noinline]` keeps the call boundary alive so the parameter survives to the IR dump.

//TEST:SIMPLE(filecheck=CHECK):-target cuda -entry main -stage compute -o - -dump-ir

groupshared uint sh[8];

[noinline]
void readOnlyConst(const groupshared uint a[8])
{
}

[noinline]
void readWrite(groupshared uint a[8])
{
    a[0] = 1;
}

[numthreads(8, 1, 1)]
void main(uint tid : SV_DispatchThreadID)
{
    sh[tid] = tid;
    readOnlyConst(sh);
    readWrite(sh);
}

// The read-only spelling lowers to a `RefParam` with `Read` access (`1`) ...
//CHECK-DAG: func %readOnlyConst{{.*}}RefParam(Array(UInt, 8 : Int), 1 : UInt64

// ... the bare spelling to a `RefParam` with `ReadWrite` access (`0`) ...
//CHECK-DAG: func %readWrite{{.*}}RefParam(Array(UInt, 8 : Int), 0 : UInt64

// ... and neither is a borrow, whose lowering has a copy-in fallback.
//CHECK-NOT: RateQualified(GroupShared, BorrowInParam
