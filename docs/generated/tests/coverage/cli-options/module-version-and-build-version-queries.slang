//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Verifies -get-supported-module-versions prints the serialized-module version window and -v prints the build tag, both as notes emitted during option parsing while the compile still proceeds.
//META: intent=characterization
//META: covers=source/slang/slang-options.cpp
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// White-box target: the `OptionKind::GetSupportedModuleVersions` and
// `OptionKind::Version` arms of `OptionsParser::_parse` in slang-options.cpp.
// The first `snprintf`s `IRModule::k_minSupportedModuleVersion` and
// `k_maxSupportedModuleVersion` into a two-line note; the second writes
// `m_session->getBuildTagString()`. Both go out through
// `m_sink->diagnoseRaw(Severity::Note, ...)` and then `break`, so option parsing
// keeps going and the shader is still compiled — which is why each run below can
// also check that HLSL came out.
//
// The concrete numbers move with every module-format bump and the build tag is
// per-build, so the CHECKs pin the surrounding literal text and wildcard the
// values. Observed on this build: "Minimum supported version: 4" /
// "Maximum supported version: 28", and a build tag of the form
// "2026.12.2-85-gaf6b35b975" (a build with no tag reports "unknown").
//
// Both notes are written to standard error, ahead of the generated HLSL on
// standard output; BUILDVER anchors on slang-test's `standard error = {` banner
// so the wildcarded tag line cannot accidentally match a line of shader text.

//TEST:SIMPLE(filecheck=MODVER):-target hlsl -entry main -stage compute -get-supported-module-versions
//TEST:SIMPLE(filecheck=BUILDVER):-target hlsl -entry main -stage compute -v

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    RWStructuredBuffer<int> b;
    b[0] = 1;
}

//MODVER: Minimum supported version: {{[0-9]+}}
//MODVER-NEXT: Maximum supported version: {{[0-9]+}}
//MODVER: void main(

//BUILDVER: standard error = {
//BUILDVER-NEXT: {{[A-Za-z0-9._+-]+$}}
//BUILDVER: void main(
