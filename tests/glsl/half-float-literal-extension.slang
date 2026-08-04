//TEST:SIMPLE(filecheck=CHECK): -target glsl -profile glsl_450 -capability GL_EXT_shader_explicit_arithmetic_types -stage compute -entry computeMain
// Also round-trip the emitted GLSL through glslang (-emit-spirv-via-glsl) so this guards validity,
// not just the textual form: if the `#extension` directive were missing, glslang would reject the
// `HF` suffix and produce no SPIR-V, failing the OpEntryPoint check below.
//TEST:SIMPLE(filecheck=GLSLSPV): -target spirv-asm -emit-spirv-via-glsl -profile glsl_450 -capability GL_EXT_shader_explicit_arithmetic_types -stage compute -entry computeMain

// Regression test for issue #11836.
//
// A half-float literal that survives to GLSL emission must cause the backend to emit the
// `GL_EXT_shader_explicit_arithmetic_types` extension directive that the `HF` literal suffix
// requires; otherwise glslang rejects the generated GLSL ("'half floating-point suffix' :
// required extension not requested").
//
// The `bit_cast` here is load-bearing: it lowers to a runtime `packHalf2x16` intrinsic, so the
// `61440.hf` value is NOT constant-folded away and genuinely reaches emission as a half literal
// (a bare `half h = 1.5hf; out = float(h);` would fold to a plain float and need no extension).
// Verified that without the fix this emits the `...HF` literal but NO `#extension` line: the
// surrounding `uint16_t(...)` cast comes from the intrinsic expansion and does not itself register
// the extension, so the half literal is the sole construct that must register it.

RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    half h = 61440.hf;
    outputBuffer[0] = uint32_t(bit_cast<uint16_t>(h));
}

// The required extension directive must be present in the front matter...
// CHECK: #extension GL_EXT_shader_explicit_arithmetic_types : require
// ...and the half literal that requires it must actually survive to emission (the exact float
// spelling is left loose, but the `HF` suffix is the point).
// CHECK: {{61440[.0-9]*HF}}

// The glslang round-trip must succeed (it would fail to produce SPIR-V if the extension directive
// were missing, since glslang rejects the `HF` suffix without it).
// GLSLSPV: OpEntryPoint GLCompute
