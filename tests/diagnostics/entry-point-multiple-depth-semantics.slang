// Test that a fragment entry point declaring more than one depth output
// system-value semantic is rejected. HLSL allows at most one depth output per
// fragment shader (any one of SV_Depth / SV_DepthGreaterEqual / SV_DepthLessEqual).
//
// Previously Slang silently accepted multiple depth outputs and produced a
// semantically wrong shader: the GLSL emitter assumes a single directional depth
// qualifier on gl_FragDepth, and the SPIR-V emitter collapses the conflicting
// per-variable depth execution modes to DepthReplacing (dropping the directional
// hint). The conflict is now diagnosed in the front-end (E30705), so it is
// rejected uniformly on every target.
//
// Each function is tested in isolation via `-entry`, which compiles only that one
// entry point, so exactly the single E30705 diagnostic is produced per case.

//DIAGNOSTIC_TEST:SIMPLE(diag=TWOPARAMS):-target spirv -entry twoParams -stage fragment
//DIAGNOSTIC_TEST:SIMPLE(diag=RETSTRUCT):-target spirv -entry retStruct -stage fragment
//DIAGNOSTIC_TEST:SIMPLE(diag=MIXED):-target spirv -entry mixed -stage fragment
//DIAGNOSTIC_TEST:SIMPLE(diag=INDEXED):-target spirv -entry indexed -stage fragment

// Two depth outputs across two `out` parameters.
[shader("fragment")]
void twoParams(float x, out float a : SV_DepthGreaterEqual, out float b : SV_Depth)
//TWOPARAMS: E30705
{
    a = x + 1.0;
    b = x - 1.0;
}

struct FSOut
{
    float a : SV_Depth;
    float b : SV_DepthLessEqual;
};

// Two depth outputs across fields of the return struct.
[shader("fragment")]
FSOut retStruct(float x)
//RETSTRUCT: E30705
{
    FSOut o;
    o.a = x + 1.0;
    o.b = x - 1.0;
    return o;
}

struct DepthOut
{
    float d : SV_DepthGreaterEqual;
};

// One depth output on an `out` parameter and one on a return-struct field.
[shader("fragment")]
DepthOut mixed(float x, out float a : SV_Depth)
//MIXED: E30705
{
    a = x + 1.0;
    DepthOut o;
    o.d = x - 1.0;
    return o;
}

// Indexed spelling of a depth semantic ("SV_Depth0") is treated as the same depth
// output as "SV_Depth", so it must be counted too (the classifier strips the index).
[shader("fragment")]
void indexed(float x, out float a : SV_Depth0, out float b : SV_DepthLessEqual)
//INDEXED: E30705
{
    a = x + 1.0;
    b = x - 1.0;
}
