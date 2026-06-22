// Regression test for issue #11691: SV_DepthGreaterEqual / SV_DepthLessEqual must
// produce the directional depth execution mode on the GLSL and via-GLSL SPIR-V
// paths, not just the spec-required DepthReplacing.
//
// On the GLSL target the lowered `gl_FragDepth` output must be redeclared with the
// `layout(depth_greater)` / `layout(depth_less)` qualifier; glslang then maps that
// qualifier to the DepthGreater / DepthLess SPIR-V execution mode on -emit-spirv-via-glsl.

// GLSL text output gains the depth-condition layout qualifier.
//TEST:SIMPLE(filecheck=GLSL_GT): -target glsl -entry FS_WriteDepthGt -stage fragment
//TEST:SIMPLE(filecheck=GLSL_LT): -target glsl -entry FS_WriteDepthLt -stage fragment

// via-GLSL SPIR-V carries both DepthReplacing (spec-required) and the directional mode.
//TEST:SIMPLE(filecheck=SPV_GLSL_GT): -target spirv -emit-spirv-via-glsl -entry FS_WriteDepthGt -stage fragment
//TEST:SIMPLE(filecheck=SPV_GLSL_LT): -target spirv -emit-spirv-via-glsl -entry FS_WriteDepthLt -stage fragment

// The default direct SPIR-V path was already correct and must stay correct.
//TEST:SIMPLE(filecheck=SPV_DIRECT): -target spirv

// GLSL_GT: layout(depth_greater) out float gl_FragDepth;
// GLSL_LT: layout(depth_less) out float gl_FragDepth;

// SPV_GLSL_GT-DAG: OpExecutionMode {{.*}} DepthReplacing
// SPV_GLSL_GT-DAG: OpExecutionMode {{.*}} DepthGreater

// SPV_GLSL_LT-DAG: OpExecutionMode {{.*}} DepthReplacing
// SPV_GLSL_LT-DAG: OpExecutionMode {{.*}} DepthLess

// SPV_DIRECT-DAG: OpExecutionMode %FS_WriteDepthGt DepthReplacing
// SPV_DIRECT-DAG: OpExecutionMode %FS_WriteDepthGt DepthGreater
// SPV_DIRECT-DAG: OpExecutionMode %FS_WriteDepthLt DepthReplacing
// SPV_DIRECT-DAG: OpExecutionMode %FS_WriteDepthLt DepthLess

[shader("fragment")]
void FS_WriteDepthGt(float a, out float depth: SV_DepthGreaterEqual)
{
    depth = a;
}

[shader("fragment")]
void FS_WriteDepthLt(float a, out float depth: SV_DepthLessEqual)
{
    depth = a;
}
