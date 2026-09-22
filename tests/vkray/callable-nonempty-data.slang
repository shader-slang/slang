// Regression test that target legalization leaves a non-empty callable-data struct intact on both
// HLSL/D3D and CUDA. A spurious `_slang_dummy` would mean the physical-carrier policy was applied
// to a type that already has a representation.

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

// Unlike the empty case, a non-empty OptiX callable payload remains a pointer parameter carrying
// its original field.
// CUDA-NOT: _slang_dummy
// CUDA: void __direct_callable__callableMain(NonEmptyData{{.*}}*{{.*}})
