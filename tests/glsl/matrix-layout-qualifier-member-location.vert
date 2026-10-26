//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK, non-exhaustive):-target spirv -entry main

// Matrix layout is the only block-member `layout(...)` qualifier we accept, so a member
// `location` (which we do not honor there) stays an error instead of being dropped.

#version 450
out VsOut { layout(row_major) mat2x3 m; layout(location = 3) vec4 a; } vo;
//CHECK:                                ^^^^^^ modifier not allowed
void main() { vo.a = vec4(1); gl_Position = vec4(0); }
