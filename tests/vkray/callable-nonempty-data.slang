// Regression test that target legalization preserves the data-bearing fields of callable data
// on both HLSL/D3D and CUDA. A spurious `_slang_dummy` would mean the physical-carrier policy was
// applied to a payload that already has a representation.

//TEST:SIMPLE(filecheck=HLSL): -stage callable -entry callableMain -target hlsl -profile sm_6_7 -validate-ir
//TEST:SIMPLE(filecheck=CUDA): -stage callable -entry callableMain -target cuda -validate-ir

struct NonEmptyData
{
    int value;
}

struct Empty
{
}

struct NestedData
{
    Empty empty[2];
    NonEmptyData inner;
}

[shader("callable")]
void callableMain(inout NestedData data)
{
    data.inner.value = 1;
}

// No dummy field is added anywhere (the struct is non-empty, so the guard skips it): the negative
// check precedes the signature match so it covers the struct declaration, which is emitted first.
// HLSL-NOT: _slang_dummy
// HLSL: void callableMain(inout NestedData{{.*}} {{[A-Za-z0-9_]+}})

// Unlike the empty case, a non-empty OptiX callable payload remains a pointer parameter carrying
// its original field.
// CUDA-NOT: _slang_dummy
// CUDA: void __direct_callable__callableMain(NestedData{{.*}}*{{.*}})
