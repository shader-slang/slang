// Regression test that the empty-callable-data padding does NOT touch a non-empty callable-data
// struct: the padding passes are guarded by `isEmptyStruct`, so a struct that already has fields
// must be left alone. A spurious `_slang_dummy` here would mean the emptiness guard regressed and
// the passes started padding structs that do not need it.

//TEST:SIMPLE(filecheck=HLSL): -stage callable -entry callableMain -target hlsl -profile sm_6_7

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
