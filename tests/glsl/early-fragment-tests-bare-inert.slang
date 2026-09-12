// `early_fragment_tests` is only meaningful on an input layout. A bare
// `layout(early_fragment_tests);` (no `in`) must NOT activate the early-fragment-tests execution
// mode. Isolated in its own file so this form is independently diagnosable (the checker scans
// every module-scope EmptyDecl). Checked on both SPIR-V paths for parity with the positive test.

//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage fragment -entry main -allow-glsl -emit-spirv-directly
//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage fragment -entry main -allow-glsl -emit-spirv-via-glsl

#version 450

layout(early_fragment_tests);
layout(location = 0) out vec4 color;

// CHECK-NOT: EarlyFragmentTests
void main()
{
    color = vec4(1.0);
}
