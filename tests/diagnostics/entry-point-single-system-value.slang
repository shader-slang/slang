// Positive control for the duplicate-system-value diagnostic (E30706, #6319): binding a system
// value once — or binding the same base name at DISTINCT indices — must still compile. This guards
// against the uniqueness check (added in validateEntryPoint) firing on legal code.
//
// Uses the FileCheck runner rather than DIAGNOSTIC_TEST so the negation has real force: under
// DIAGNOSTIC_TEST a `//CHECK-NOT:` line is parsed as an ordinary comment (the annotation parser
// only recognizes exactly `//<prefix>:`), so it would assert nothing. Here `CHECK: OpEntryPoint`
// additionally proves real SPIR-V was produced — impossible if E30706 had aborted compilation.

//TEST:SIMPLE(filecheck=SINGLE):-target spirv-asm -entry singleVertex -stage vertex
//TEST:SIMPLE(filecheck=DISTINCT):-target spirv-asm -entry distinctTargets -stage fragment

// A single input system value is fine.
float4 singleVertex(uint a : SV_VertexID) : SV_Position
{
    return float4(float(a), 0, 0, 1);
}
//SINGLE-NOT: E30706
//SINGLE: OpEntryPoint

// The same base name at distinct indices binds distinct render targets and must stay clean.
void distinctTargets(out float4 a : SV_Target0, out float4 b : SV_Target1)
{
    a = float4(1, 0, 0, 1);
    b = float4(0, 1, 0, 1);
}
//DISTINCT-NOT: E30706
//DISTINCT: OpEntryPoint
