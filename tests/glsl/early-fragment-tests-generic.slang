// A *specialized* generic fragment entry point must also pick up
// `layout(early_fragment_tests) in;`. Its immediate parent is the GenericDecl, not the module
// scope that holds the standalone `layout(...) in;` EmptyDecl, so the lift must resolve the scope
// with getParentDecl (skipping the GenericDecl) rather than the raw parentDecl — this test locks
// that branch in. (An unspecialized generic entry point is separately rejected with E38014.)

//TEST:SIMPLE(filecheck=CHECK):     -target spirv -stage fragment -entry fragMain<Red> -allow-glsl -emit-spirv-directly
//TEST:SIMPLE(filecheck=CHECKGLSL): -target spirv -stage fragment -entry fragMain<Red> -allow-glsl -emit-spirv-via-glsl

layout(early_fragment_tests) in;

interface IColor { float4 c(); }
struct Red : IColor { float4 c() { return float4(1.0, 0.0, 0.0, 1.0); } }

// CHECK:     OpExecutionMode {{.*}} EarlyFragmentTests
// CHECKGLSL: OpExecutionMode {{.*}} EarlyFragmentTests
[shader("fragment")]
float4 fragMain<T : IColor>(T t) : SV_Target
{
    return t.c();
}
