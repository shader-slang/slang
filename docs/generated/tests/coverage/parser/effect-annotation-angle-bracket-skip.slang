//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Verifies that with `-enable-effect-annotations` a `< ... ; ... ; >` clause after a declarator is skipped as a legacy effect annotation instead of being parsed as a generic argument list.
//META: intent=characterization
//META: covers=source/slang/slang-parser.cpp
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterization: `parseDirectAbstractDeclarator` normally treats a `<`
// following a declarator as the start of a generic argument list. Under
// `-enable-effect-annotations` it first has to disambiguate: `<let` and
// `< X :` are still generic argument lists, but otherwise the parser scans
// ahead with a scratch `TokenReader` looking for a `;` before the matching
// `>`. Finding one proves this is a legacy D3D effect annotation, so the
// parser commits the scratch reader and swallows the whole clause.
//
// `int slider = 3;` inside the angle brackets is what supplies that `;`.
// Without the flag the same source is a syntax error, because the generic
// argument parser reaches the `;` and reports an unexpected token.
//
// CHECK confirms the declaration survived: `g` is emitted as an ordinary
// global with its annotation dropped.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute -enable-effect-annotations

struct S { int a; };

S g < string ui = "hello"; int slider = 3; > ;

//CHECK: globalParams
//CHECK: g_0.a_0

[numthreads(1, 1, 1)]
void main(uniform RWStructuredBuffer<int> o)
{
    o[0] = g.a;
}
