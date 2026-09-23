//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Verifies -source-embed-name renames the embedded-source array from the default `data` to the requested identifier, and that -source-embed-language rejects a language SourceEmbedUtil does not support with E00051.
//META: intent=characterization
//META: covers=source/slang/slang-options.cpp
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// White-box target: the `OptionKind::SourceEmbedName` and
// `OptionKind::SourceEmbedLanguage` arms of `OptionsParser::_parse` in
// slang-options.cpp. `SourceEmbedName` stores the operand straight into
// `m_requestImpl->m_sourceEmbedName`; `SourceEmbedLanguage` parses the operand
// into `m_sourceEmbedLanguage` and then immediately validates it with
// `SourceEmbedUtil::isSupported`, diagnosing
// `Diagnostics::UnhandledLanguageForSourceEmbedding` and failing the parse when
// the language is not one of the C-family ones.
//
// `-source-embed-style text` turns the compiled HLSL into a C string literal
// array. The array's identifier is what `-source-embed-name` controls, so DEFAULT
// pins `const char data[]` and NAMED pins `const char myBlob[]` from the same
// source — the option is the sole cause of the difference. C is accepted (CLANG),
// while `slang` is not a source-embedding output language and is rejected during
// option parsing before any compilation happens (BADLANG).

//TEST:SIMPLE(filecheck=DEFAULT):-target hlsl -entry main -stage compute -source-embed-style text
//TEST:SIMPLE(filecheck=NAMED):-target hlsl -entry main -stage compute -source-embed-style text -source-embed-name myBlob
//TEST:SIMPLE(filecheck=CLANG):-target hlsl -entry main -stage compute -source-embed-style text -source-embed-language c
//TEST:SIMPLE(filecheck=BADLANG):-target hlsl -entry main -stage compute -source-embed-style text -source-embed-language slang

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    RWStructuredBuffer<int> b;
    b[0] = 1;
}

//DEFAULT: const char data[] =

//NAMED: const char myBlob[] =

//CLANG: const char data[] =

//BADLANG: error[E00051]: unhandled source language for source embedding
