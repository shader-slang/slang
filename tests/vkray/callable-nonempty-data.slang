// Regression test that the empty-callable-data padding does NOT touch a non-empty callable-data
// struct on either target path (HLSL/D3D and CUDA): the shared padding pass is guarded by
// `isEmptyStruct`, so a struct that already has fields is left alone. A spurious `_slang_dummy`
// here would mean the emptiness guard regressed and the pass started padding structs that do not
// need it.

//TEST:SIMPLE(filecheck=HLSL): -stage callable -entry callableMain -target hlsl -profile sm_6_7
//TEST:SIMPLE(filecheck=CUDA): -stage callable -entry callableMain -target cuda

struct NonEmptyData
{
    int value;
}

[shader("callable")]
void callableMain(inout NonEmptyData data)
{
    data.value = 1;
}

// No dummy field is added anywhere (the struct is non-empty, so the guard skips it): the negative
// check precedes the signature match so it covers the struct declaration, which is emitted first.
// HLSL-NOT: _slang_dummy
// HLSL: void callableMain(inout NonEmptyData{{.*}} {{[A-Za-z0-9_]+}})

// The CUDA padding pass shares the same `isEmptyStruct` guard, so the non-empty struct keeps its
// original field and gains no dummy: the negative check runs ahead of the signature match, covering
// the struct declaration too. The callable entry point survives as a pointer parameter carrying its
// original field — no dummy is added.
// CUDA-NOT: _slang_dummy
// CUDA: void __direct_callable__callableMain(NonEmptyData{{.*}}*{{.*}})
