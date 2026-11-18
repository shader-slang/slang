// Transcendentals of a double must be rejected with a source-level error (E41400) on the
// glsl and spirv targets: GLSL has no double-precision transcendental overload under any
// extension, and GLSL.std.450 defines the transcendental instructions only for 16/32-bit
// floats. Without the guard, `sin(d)` for a double either emits GLSL that glslang rejects
// (including a SPIR-V request routed with -emit-spirv-via-glsl) or an invalid SPIR-V module.
// float sin must keep compiling with the native intrinsic, and the cpp/cuda targets, which
// legitimately support double sin, must keep compiling it too.

//DIAGNOSTIC_TEST:SIMPLE(filecheck=VIA_GLSL,exit-code=2): -target spirv -emit-spirv-via-glsl -entry eScalar -stage compute
//DIAGNOSTIC_TEST:SIMPLE(filecheck=GLSL,exit-code=2): -target glsl -entry eScalar -stage compute
//DIAGNOSTIC_TEST:SIMPLE(filecheck=SPIRV,exit-code=2): -target spirv -entry eScalar -stage compute
//DIAGNOSTIC_TEST:SIMPLE(filecheck=VEC,exit-code=2): -target glsl -entry eVector -stage compute
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CONST,exit-code=2): -target spirv -entry eConst -stage compute

// The float path and the targets that support double sin must be unchanged.
//TEST:SIMPLE(filecheck=FLOAT): -target glsl -entry eFloat -stage compute
//TEST:SIMPLE(filecheck=FLOAT_VIA): -target spirv -emit-spirv-via-glsl -entry eFloat -stage compute
//TEST:SIMPLE(filecheck=CPP_DOUBLE): -target cpp -entry eScalar -stage compute
//TEST:SIMPLE(filecheck=CUDA_DOUBLE): -target cuda -entry eScalar -stage compute

RWStructuredBuffer<float> outBuf;

//VIA_GLSL: error[E41400]: static assertion failed
//VIA_GLSL: 'sin' of a 'double' is not supported for the 'glsl' target
//GLSL: error[E41400]: static assertion failed
//GLSL: 'sin' of a 'double' is not supported for the 'glsl' target
//SPIRV: error[E41400]: static assertion failed
//SPIRV: 'sin' of a 'double' is not supported for the 'spirv' target
//CPP_DOUBLE: F64_sin(
//CUDA_DOUBLE: F64_sin(
[shader("compute")][numthreads(1, 1, 1)]
void eScalar(uint3 tid) { double d = outBuf[0]; outBuf[1] = float(sin(d)); }

//VEC: error[E41400]: static assertion failed
//VEC: 'sin' of a 'double' is not supported for the 'glsl' target
[shader("compute")][numthreads(1, 1, 1)]
void eVector(uint3 tid) { double2 d = double2(outBuf[0]); outBuf[2] = float(sin(d).x); }

// The guard fires during specialization of the intrinsic, before constant folding, so a
// literal argument is diagnosed the same way as a runtime value.
//CONST: error[E41400]: static assertion failed
//CONST: 'sin' of a 'double' is not supported for the 'spirv' target
[shader("compute")][numthreads(1, 1, 1)]
void eConst(uint3 tid) { double d = 3.0; outBuf[3] = float(sin(d)); }

//FLOAT: sin(
//FLOAT_VIA: sin(
[shader("compute")][numthreads(1, 1, 1)]
void eFloat(uint3 tid) { float f = outBuf[0]; outBuf[4] = sin(f); }
