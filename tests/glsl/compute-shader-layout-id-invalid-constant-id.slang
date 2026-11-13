// A layout argument that is not a compile-time constant gets one error and nothing else:
// - a `constant_id` id, before or after `layout(local_size_x_id = ...) in;`, which the id
//   lookup skips;
// - a size that follows the id statement, which the implicit constant's default skips;
// - a `binding`, which stays attached for the `atomic_uint` offset check.

//DIAGNOSTIC_TEST:SIMPLE_EX(diag=AFTER): -lang glsl tests/glsl/compute-shader-layout-id-invalid-constant-id.slang -stage compute -entry main -target spirv -DAFTER
//DIAGNOSTIC_TEST:SIMPLE_EX(diag=BEFORE): -lang glsl tests/glsl/compute-shader-layout-id-invalid-constant-id.slang -stage compute -entry main -target spirv -DBEFORE
//DIAGNOSTIC_TEST:SIMPLE_EX(diag=SIZE): -lang glsl tests/glsl/compute-shader-layout-id-invalid-constant-id.slang -stage compute -entry main -target spirv -DSIZE
//DIAGNOSTIC_TEST:SIMPLE_EX(diag=ATOMIC): -lang glsl tests/glsl/compute-shader-layout-id-invalid-constant-id.slang -stage compute -entry main -target spirv -DATOMIC

int notConstant() { return 5; }

#if defined(AFTER)
layout(local_size_x_id = 5) in;
layout(constant_id = notConstant()) const int c = 4;
//AFTER:                        ^ expression does not evaluate to a compile-time constant
#elif defined(BEFORE)
layout(constant_id = notConstant()) const int c = 4;
//BEFORE:                       ^ expression does not evaluate to a compile-time constant
layout(local_size_x_id = 5) in;
#elif defined(SIZE)
int notConstantSize = 4;
layout(local_size_x_id = 18) in;
layout(local_size_x = notConstantSize) in;
//SIZE:               ^^^^^^^^^^^^^^^ expression does not evaluate to a compile-time constant
#elif defined(ATOMIC)
layout(binding = notConstant()) uniform atomic_uint counter;
//ATOMIC:                   ^ expression does not evaluate to a compile-time constant
layout(local_size_x = 1) in;
#endif

void main() {}
