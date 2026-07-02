// Companion to fragment-depth-greater-less.slang: pins the front-end guarantee
// that the directional depth semantics are output-only. SV_DepthGreaterEqual /
// SV_DepthLessEqual are setter-only system values (core.meta.slang), so using one
// as an `in`/`inout` pixel-shader parameter is rejected with E30702 during
// semantic checking (slang-check-shader.cpp), long before GLSL legalization runs.
//
// This is why the GLSL emitter never needs to guard against a depth input
// re-declaring `gl_FragDepth` a second time: the input shape is foreclosed here.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHK):

[shader("fragment")]
void FS(inout float depth : SV_DepthGreaterEqual)
//CHK:              ^^^^^ E30702
{
    depth += 1.0;
}
