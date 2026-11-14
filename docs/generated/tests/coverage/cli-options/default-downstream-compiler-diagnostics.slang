//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Verifies -default-downstream-compiler validates both of its operands, rejecting an unrecognized source language with E00019 and an unrecognized pass-through compiler with E00016, and accepting a valid pair.
//META: intent=characterization
//META: covers=source/slang/slang-options.cpp
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// White-box target: the `OptionKind::DefaultDownstreamCompiler` arm of
// `OptionsParser::_parse` in slang-options.cpp. The arm reads two operands and
// resolves them in order: `TypeTextUtil::findSourceLanguage` on the first (a
// `SLANG_SOURCE_LANGUAGE_UNKNOWN` result diagnoses
// `Diagnostics::UnknownSourceLanguage` and fails), then
// `TypeTextUtil::findPassThrough` on the second (a failure diagnoses
// `Diagnostics::UnknownPassThroughTarget` and fails), and finally
// `IGlobalSession::setDefaultDownstreamCompiler`.
//
// The two failure arms are what make the operand order observable: BADLANG names
// a bogus language with a real compiler and gets E00019 naming the language;
// BADCOMPILER names a real language with a bogus compiler and gets E00016 naming
// the compiler. OK pairs `c` with `gcc`, which resolves and lets the HLSL compile
// proceed, showing that a successful call does not disturb the rest of the run.
//
// All three messages were copied verbatim from real slangc runs. The diagnostics
// attach to the synthetic command-line source, so only code and message text are
// pinned, not a caret column.

//TEST:SIMPLE(filecheck=BADLANG):-target hlsl -entry main -stage compute -default-downstream-compiler bogus-lang dxc
//TEST:SIMPLE(filecheck=BADCOMPILER):-target hlsl -entry main -stage compute -default-downstream-compiler hlsl bogus-compiler
//TEST:SIMPLE(filecheck=OK):-target hlsl -entry main -stage compute -default-downstream-compiler c gcc

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    RWStructuredBuffer<int> b;
    b[0] = 1;
}

//BADLANG: error[E00019]: unknown source language 'bogus-lang'

//BADCOMPILER: error[E00016]: unknown pass-through target 'bogus-compiler'

//OK-NOT: error[E
//OK: void main(
