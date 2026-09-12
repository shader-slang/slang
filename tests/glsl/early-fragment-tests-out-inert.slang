// `early_fragment_tests` is only meaningful on an input layout. A standalone
// `layout(early_fragment_tests) out;` must NOT activate the early-fragment-tests execution mode.
// This form is isolated in its own file (the checker scans every module-scope EmptyDecl, so a
// second inert form in the same module would not be independently diagnosable). Checked on both
// SPIR-V paths for parity with the positive test.

//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage fragment -entry main -allow-glsl -emit-spirv-directly
//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage fragment -entry main -allow-glsl -emit-spirv-via-glsl

#version 450

layout(early_fragment_tests) out;
layout(location = 0) out vec4 color;

// CHECK-NOT: EarlyFragmentTests
void main()
{
    color = vec4(1.0);
}
