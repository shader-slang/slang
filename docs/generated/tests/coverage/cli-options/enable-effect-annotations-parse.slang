//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Verifies -enable-effect-annotations lets the parser treat a `<...;...>` group after a declarator as a legacy effect annotation and skip it, where the default parse rejects the same source with E20001 at the `;` inside the angle brackets.
//META: intent=characterization
//META: covers=source/slang/slang-options.cpp
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// White-box target: the `OptionKind::EnableEffectAnnotations` arm of
// `OptionsParser::_parse` in slang-options.cpp, which calls
// `EndToEndCompileRequest::setEnableEffectAnnotations(true)`. That flows into
// `ParserOptions::enableEffectAnnotations`, read in `parseSourceFile` and
// consumed at the `TokenType::OpLess` case of the declarator parser
// (slang-parser.cpp), which disambiguates `<` after a declarator between a
// generic argument list and a legacy D3D-effect annotation.
//
// Consider `uniform float4 gColor <string uiName = "Color";>;`. Without the flag
// the parser only ever considers the generic-argument reading, walks into the
// angle brackets and hits the `;`, reporting
// `error[E20001]: unexpected token ... unexpected ';', expected '>'`. With the
// flag it looks ahead, sees a `;` before the closing `>`, concludes this is an
// annotation and skips the whole group — so `gColor` becomes an ordinary uniform
// and lands in the generated `cbuffer globalParams_0`.
//
// DEFAULT pins the rejection, ENABLED pins the successful parse via the emitted
// constant buffer and the use of `gColor_0` in the entry point.

//TEST:SIMPLE(filecheck=DEFAULT):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=ENABLED):-target hlsl -entry main -stage compute -enable-effect-annotations

uniform float4 gColor <string uiName = "Color";>;
RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    outBuf[0] = gColor;
}

//DEFAULT: error[E20001]: unexpected token
//DEFAULT: unexpected ';', expected '>'

//ENABLED: cbuffer globalParams_0
//ENABLED: void main()
//ENABLED: globalParams_0.gColor_0;
