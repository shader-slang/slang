// A callable payload can mix empty aggregate fields with data-bearing fields. The empty array
// disappears during type legalization, but the surviving integer makes this a non-empty payload,
// so ray-tracing legalization must not add a dummy field on HLSL/D3D or CUDA.

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

// The negative check starts before the signature so it also covers the struct declaration.
// HLSL-NOT: _slang_dummy
// HLSL: void callableMain(inout NestedData{{.*}} {{[A-Za-z0-9_]+}})

// The non-empty OptiX callable payload remains a pointer parameter carrying its original value.
// CUDA-NOT: _slang_dummy
// CUDA: void __direct_callable__callableMain(NestedData{{.*}}*{{.*}})
