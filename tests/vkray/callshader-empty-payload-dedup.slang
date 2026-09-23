// Regression test for the dedup path of the empty-callable-data padding: one empty struct used
// BOTH as a callable entry point's payload and as a `CallShader` caller's payload, in a single
// module, must be padded exactly once. The ray-tracing legalization pass collects both use sites
// into one `HashSet<IRStructType*>`, so the struct is padded a single time; a
// double-pad would add an extra dummy field, so the field count below verifies that the shared
// carrier is padded only once.

//TEST:SIMPLE(filecheck=HLSL): -target hlsl -profile sm_6_7

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
