// Regression test for the dedup path of the empty-callable-data padding: one empty struct used
// BOTH as a callable entry point's payload and as a `CallShader` caller's payload, in a single
// module, must be padded exactly once. `legalizeEmptyCallableDataPayloadsForHLSLAndCUDA` collects
// both use sites into one `HashSet<IRStructType*>`, so the struct is padded a single time; a
// double-pad would re-enter `padEmptyStructWithDummyField` on a now-non-empty struct and trip its
// `SLANG_RELEASE_ASSERT`, aborting the compile (so a successful compile is itself the dedup signal).
// The same pass runs on D3D and CUDA, so both are exercised here.

//TEST:SIMPLE(filecheck=HLSL): -target hlsl -profile sm_6_7
//TEST:SIMPLE(filecheck=CUDA): -target cuda

struct EmptyData
{
}

[shader("callable")]
void callableMain(inout EmptyData data)
{
}

[shader("raygeneration")]
void rgen()
{
    EmptyData d;
    CallShader(0, d);
}

// Exactly one dummy field is added for the single shared EmptyData struct (padded once, not once
// per use site).
// HLSL-COUNT-1: _slang_dummy
// CUDA-COUNT-1: _slang_dummy
