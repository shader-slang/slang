// A fragment entry point whose result struct mixes a color output (SV_Target) with a
// depth output (SV_Depth) must bind the depth to the FragDepth builtin and keep the
// color at Location 0; the depth must not spill into an extra color attachment.
// This holds only while no enclosing result semantic forces the outer-most-wins
// policy over the field semantics; the E39034 diagnostic in
// tests/diagnostics/outer-semantic-overrides-system-value-semantic.slang guards the
// interfering-spelling side of that contract, and this test guards the clean one.
//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage fragment -entry main -emit-spirv-directly

struct FSOut
{
    float4 color : SV_Target;
    float depth : SV_Depth;
};

FSOut main()
{
    FSOut o;
    o.color = float4(1.0f, 0.0f, 0.0f, 1.0f);
    o.depth = 0.5f;
    return o;
}
// CHECK: OpDecorate %entryPointParam_main_color Location 0
// CHECK: OpDecorate %gl_FragDepth BuiltIn FragDepth
// CHECK-NOT: OpDecorate %entryPointParam_main_depth Location 1
