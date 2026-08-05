// gh-10641-groupshared-member-arg.slang

// A field of a group-shared variable names group-shared storage, so it is a valid argument for a
// `groupshared` parameter and must be passed by reference.
//
// `getValidTypeForAddressOf` answers "what address space does this expression name?", and
// `checkGroupSharedArgumentOfParam` uses that answer to accept or reject the argument. `MemberExpr`
// derives from `DeclRefExpr` and a struct field is a `VarDeclBase`, so `gss.v` used to match the
// variable case and be answered for the *field's* declaration. `groupshared` sits on the base
// variable `gss`, never on the field, so that path reported `UserPointer` and the argument was
// rejected as not naming group-shared storage. An array element such as `arr[0]` was unaffected
// because `IndexExpr` is not a `DeclRefExpr`, so it already recursed onto its base -- which is why
// the two spellings disagreed. A non-static member access now defers to that same recursion.

//TEST:SIMPLE(filecheck=CHECK):-target cuda -entry main -stage compute -o /dev/null -dump-ir

struct Inner
{
    uint v;
}

struct Outer
{
    Inner inner;
    uint scratch[4];
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
void readOnlyWholeStruct(const groupshared Inner s)
{
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

    // A whole group-shared struct, passed by reference rather than field by field.
    readOnlyWholeStruct(go.inner);
    readOnlyWholeStruct(arr[1]);
}

// The read-only spellings keep the read-only reference ...
//CHECK-DAG: func %readOnlyField{{.*}}BorrowInParam
//CHECK-DAG: func %readOnlyWholeStruct{{.*}}BorrowInParam

// A field of a *non*-group-shared variable is still rejected: deferring to the base means the
// answer follows the base, not that every member access is accepted. See the companion diagnostic
// test `tests/diagnostics/groupshared-argument-lvalue.slang` for that direction.

// ... and the bare spelling stays the strict read-write reference. Without this half of the
// control, a change that made every group-shared parameter read-only would still pass.
//CHECK-DAG: func %readWriteField{{.*}}RefParam
