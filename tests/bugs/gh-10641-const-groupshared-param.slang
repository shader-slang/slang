// gh-10641-const-groupshared-param.slang

// A `const groupshared` parameter must lower to a read-only reference, distinct from the
// read-write reference a bare `groupshared` parameter gets.
//
// `getExplicitlyDeclaredParamPassingMode` picks the passing mode from `RefModifier` /
// `BorrowModifier` and the direction modifiers; it never inspects `ConstModifier`. So before this
// fix, `const groupshared T p[N]` -- which the checker accepts, and whose writes it rejects --
// fell through to the read-write groupshared default and lowered to the same `BorrowInOutParam` as
// a bare `groupshared` parameter. The read-only-ness existed only in the AST and was invisible to
// every IR pass and emitter: the two spellings produced byte-identical CUDA signatures and the same
// mangled name. The checker now gives a `const groupshared` parameter the same `BorrowModifier`
// that the explicit `__constref` spelling carries, so both agree on `BorrowInParam`.
//
// `[noinline]` keeps the call boundary alive so the parameter survives to the IR dump.

//TEST:SIMPLE(filecheck=CHECK):-target cuda -entry main -stage compute -o /dev/null -dump-ir

groupshared uint sh[8];

[noinline]
void readOnlyConst(const groupshared uint a[8])
{
}

[noinline]
void readOnlyConstRef(__constref groupshared uint a[8])
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
    readOnlyConstRef(sh);
    readWrite(sh);
}

// Both read-only spellings lower to `BorrowInParam` ...
//CHECK-DAG: func %readOnlyConst{{.*}}BorrowInParam
//CHECK-DAG: func %readOnlyConstRef{{.*}}BorrowInParam

// ... while the bare spelling keeps the read-write `BorrowInOutParam`. This is the other half of
// the control: injecting the borrow modifier unconditionally would silently make every bare
// `groupshared` parameter read-only, which this check would catch.
//CHECK-DAG: func %readWrite{{.*}}BorrowInOutParam
