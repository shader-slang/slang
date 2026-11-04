//TEST:SIMPLE_EX(filecheck=CHECK): -lang glsl tests/language-feature/constants/static-const-global-spec-const-glsl-mode.slang -target glsl -stage compute -entry main

// In GLSL mode a global `const` without `uniform`, `in`, `out` or `buffer` gets an implicit
// `static`, so a `const` array over a `layout(constant_id)` constant reaches the same E31226
// check as a Slang `static const` array. A specialization constant does not get the implicit
// `static`; it stays a shader parameter, so the check accepts it here too.

#version 450
layout(local_size_x = 2) in;

layout(constant_id = 0) const int N = 4;
const int arr[2] = int[](N, 3);
const int arr2[2] = { N + 1, 64 / N };

layout(std430, binding = 0) buffer Out { int o[]; };

// CHECK-NOT: E31226
// CHECK-DAG: arr_0[2] = int[](N_0, 3);
// CHECK-DAG: arr2_0[2] = int[](N_0 + 1, 64 / N_0);

void main()
{
    uint i = gl_GlobalInvocationID.x & 1u;
    o[gl_GlobalInvocationID.x] = arr[i] + arr2[i];
}
