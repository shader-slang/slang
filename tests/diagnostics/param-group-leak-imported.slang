// Regression test for shader-slang/slang#11395.
//
// Warnings emitted from IR passes (here E31106/E31107, raised during type
// legalization of a ConstantBuffer) derive their source location from an IR
// instruction's sourceLoc. When the struct is imported from a precompiled
// binary module, the field keys are deserialized and carry no source location
// that resolves in the consuming compile, so the per-member warning (E31107)
// used to be emitted with no file:line at all. Single-file cases already show a
// location, so the loss only reproduces with a precompiled module import.
//
// This test precompiles a module and imports it, then checks that every warning
// is still emitted with a source location.

//TEST:COMPILE: tests/diagnostics/param-group-leak-imported-module.slang -o tests/diagnostics/param-group-leak-imported.slang-module

//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -I tests/diagnostics -target hlsl -entry main -stage fragment

// `import param_group_leak_imported` (underscores) resolves to the precompiled
// `param-group-leak-imported.slang-module` (dashes) written by the //TEST:COMPILE
// step above and found via `-I tests/diagnostics`; loading that binary module is
// what exercises the deserialized-key path. Don't "fix" the underscore/dash name
// difference — it is the mechanism under test.
import param_group_leak_imported;

// All three leak warnings fall back to the parameter group's use site — the
// `ConstantBuffer<Material>` declaration below — so each must carry that exact line
// (pre-fix the E31107 warnings had no location at all).
// CHECK: warning[E31106]
// CHECK: param-group-leak-imported.slang:[[# @LINE+6]]:
// CHECK: warning[E31107]
// CHECK: param-group-leak-imported.slang:[[# @LINE+4]]:
// CHECK: This member will leak into a separate binding slot.
// CHECK: warning[E31107]
// CHECK: param-group-leak-imported.slang:[[# @LINE+1]]:
ConstantBuffer<Material> cb;

float4 main(float2 uv : TEXCOORD) : SV_Target
{
    return cb.albedo.Sample(cb.samp, uv) * cb.tint;
}
