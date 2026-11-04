// A `constant_id` whose id is not a compile-time constant gets one error, and the
// `local_size_x_id` lookup skips it, whether it comes before or after `layout(...) in;`.

//DIAGNOSTIC_TEST:SIMPLE_EX(diag=AFTER): -lang glsl tests/glsl/compute-shader-layout-id-invalid-constant-id.slang -stage compute -entry main -target spirv -DAFTER
//DIAGNOSTIC_TEST:SIMPLE_EX(diag=BEFORE): -lang glsl tests/glsl/compute-shader-layout-id-invalid-constant-id.slang -stage compute -entry main -target spirv -DBEFORE

int notConstant() { return 5; }

#if defined(AFTER)
layout(local_size_x_id = 5) in;
layout(constant_id = notConstant()) const int c = 4;
//AFTER:                        ^ expression does not evaluate to a compile-time constant
#else
layout(constant_id = notConstant()) const int c = 4;
//BEFORE:                       ^ expression does not evaluate to a compile-time constant
layout(local_size_x_id = 5) in;
#endif

void main() {}
