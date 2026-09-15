// `layout(early_fragment_tests) in;` under -allow-glsl marks the fragment entry point to run early
// depth/stencil tests, emitting `OpExecutionMode <entry> EarlyFragmentTests`. Both SPIR-V paths
// (direct and via-glsl) must carry it. The via-glsl operand is wildcarded because glslang renames
// the entry point on that path (see fragment-depth-greater-less.slang), so its SPIR-V id is not
// the Slang entry-point name.

//TEST:SIMPLE(filecheck=CHECK):     -target spirv -stage fragment -entry main -allow-glsl -emit-spirv-directly
//TEST:SIMPLE(filecheck=CHECKGLSL): -target spirv -stage fragment -entry main -allow-glsl -emit-spirv-via-glsl

#version 450

layout(early_fragment_tests) in;
layout(location = 0) out vec4 color;

// CHECK:     OpExecutionMode %main EarlyFragmentTests
// CHECKGLSL: OpExecutionMode {{.*}} EarlyFragmentTests
void main()
{
    color = vec4(1.0);
}
