// Positive control for the "multiple depth output" diagnostic (E30705): a fragment entry point
// that declares exactly one depth output must still compile without error. This guards against
// the multiplicity check (added in validateEntryPoint) firing on the legal single-depth case.
//
// Uses the FileCheck runner rather than DIAGNOSTIC_TEST so the negation has real force: under
// DIAGNOSTIC_TEST a `//<prefix>-NOT:` line is parsed as an ordinary comment (the annotation parser
// only recognizes exactly `//<prefix>:`), so it would assert nothing. The positive OpEntryPoint
// match below additionally proves real SPIR-V was produced — impossible if E30705 had aborted
// compilation. (Do not spell the FileCheck prefix + colon literally anywhere in this prose:
// FileCheck scans the whole file, so a stray one becomes a live directive and fails the run —
// which is exactly what this comment previously did.)

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage fragment
//TEST:SIMPLE(filecheck=RETONLY):-target spirv-asm -entry retOnly -stage fragment

struct FSOut
{
    float color : SV_Target;
};

// Single depth output on an `out` parameter, alongside a non-depth return-struct field.
[shader("fragment")]
FSOut main(float x, out float depth : SV_DepthGreaterEqual)
{
    depth = x + 1.0;
    FSOut o;
    o.color = x;
    return o;
}
//CHECK-NOT: E30705
//CHECK: OpEntryPoint

struct DepthOnly
{
    float d : SV_Depth;
    float color : SV_Target;
};

// Single depth output living solely in a return-struct field must also compile clean: the
// collector recurses into the struct but finds exactly one depth output.
[shader("fragment")]
DepthOnly retOnly(float x)
{
    DepthOnly o;
    o.d = x + 1.0;
    o.color = x;
    return o;
}
//RETONLY-NOT: E30705
//RETONLY: OpEntryPoint
