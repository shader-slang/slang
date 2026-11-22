// `layout(early_fragment_tests) in;` in a GLSL translation unit marks the fragment entry point to
// run early depth/stencil tests, emitting `OpExecutionMode %main EarlyFragmentTests`. Both SPIR-V
// paths (direct and via-glsl) must carry it; the GLSL `void main()` keeps its name through glslang
// on the via-glsl path, so both check the `%main` operand.

//TEST:SIMPLE_EX(filecheck=CHECK):     -lang glsl tests/glsl/early-fragment-tests.slang -target spirv -stage fragment -entry main -emit-spirv-directly
//TEST:SIMPLE_EX(filecheck=CHECKGLSL): -lang glsl tests/glsl/early-fragment-tests.slang -target spirv -stage fragment -entry main -emit-spirv-via-glsl

#version 450

layout(early_fragment_tests) in;
layout(location = 0) out vec4 color;

// CHECK:     OpExecutionMode %main EarlyFragmentTests
// CHECKGLSL: OpExecutionMode %main EarlyFragmentTests
void main()
{
    color = vec4(1.0);
}
