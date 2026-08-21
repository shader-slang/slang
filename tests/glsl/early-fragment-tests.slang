// Regression test for issue #12655: the GLSL fragment input layout qualifier
// `layout(early_fragment_tests) in;` must be accepted under `-allow-glsl` and mark the
// fragment entry point to run early depth/stencil tests, producing the
// `OpExecutionMode %main EarlyFragmentTests` SPIR-V execution mode. Before the fix the
// qualifier was rejected at parse time with error[E31217] (unrecognized GLSL layout
// qualifier), because `early_fragment_tests` was recognized nowhere in the front end.

//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage fragment -entry main -allow-glsl
//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage fragment -entry main -allow-glsl -emit-spirv-directly

#version 450

layout(early_fragment_tests) in;
layout(location = 0) out vec4 color;

// CHECK: OpExecutionMode %main EarlyFragmentTests
void main()
{
    color = vec4(1.0);
}
