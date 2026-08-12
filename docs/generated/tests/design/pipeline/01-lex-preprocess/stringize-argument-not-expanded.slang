//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#macro-expansion
//META: doc_section_digest=74e43f68dacbee88ce5a4d003aeba806114c7db9ff5ee4ebefea236b92eca467
//META: purpose=A stringized parameter takes the argument's raw tokens, not their macro expansion, so `#x` of a macro name yields the name's spelling.
//META: intent=functional
//META: pipeline_stage=preprocess
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a stringized parameter reference bypasses argument
// expansion: only expanded-parameter references get the
// macro-expanding replay, so STR(NAME) must produce the spelling
// "NAME" even though NAME is itself a macro for 42. We emit to CPP
// and FileCheck the quoted spelling (slangi printf does not support
// %s). A result of "42" would mean the argument was expanded before
// stringization.

//TEST:SIMPLE(filecheck=CHECK):-target cpp -entry main -stage compute

#define NAME 42
#define STR(x) #x

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    printf("%s\n", STR(NAME));
    //CHECK: "NAME"
}
