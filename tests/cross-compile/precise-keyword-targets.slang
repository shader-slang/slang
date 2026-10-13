// precise-keyword-targets.slang

// `precise` is only valid HLSL/GLSL syntax; on the other C-like targets the
// qualifier is dropped (with a warning) rather than emitted as an invalid token.

//TEST:SIMPLE(filecheck=CUDA):  -target cuda  -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=CPP):   -target cpp   -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=METAL): -target metal -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=WGSL):  -target wgsl  -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=HLSL):  -target hlsl  -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=GLSL):  -target glsl  -stage compute -entry computeMain
//DIAGNOSTIC_TEST:SIMPLE(diag=DIAG): -target cuda  -stage compute -entry computeMain
//DIAGNOSTIC_TEST:SIMPLE(diag=DIAG): -target cpp   -stage compute -entry computeMain
//DIAGNOSTIC_TEST:SIMPLE(diag=DIAG): -target metal -stage compute -entry computeMain
//DIAGNOSTIC_TEST:SIMPLE(diag=DIAG): -target wgsl  -stage compute -entry computeMain

RWStructuredBuffer<float> gOut;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    precise float r = gOut[0] * 2.0f;
    gOut[0] = r;
}

// Targets that do not accept the qualifier: it must be gone.
// CUDA:      2.0f
// CUDA-NOT:  precise float
// CPP:       2.0f
// CPP-NOT:   precise float
// METAL:     2.0f
// METAL-NOT: precise float
// WGSL:      2.0f
// WGSL-NOT:  precise var

// Targets that do accept it: the qualifier stays.
// HLSL:      precise float
// GLSL:      precise float

//DIAG: warning E56005
