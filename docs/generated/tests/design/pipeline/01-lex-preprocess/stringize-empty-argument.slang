//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#source-location-preservation
//META: doc_section_digest=b376d677453ba50705d8234aa3572303b42f0c428f903a6b2d9d9b73a3df65a7
//META: purpose=Boundary probe of stringize `#x`: an empty argument stringizes to an empty string literal `""`.
//META: intent=boundary
//META: pipeline_stage=preprocess
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `STR()` invokes the macro with an empty argument token list. The
// resulting constructed string-literal token must be the empty
// string `""`. We emit to CPP and FileCheck for the literal in
// the emitted source (slangi printf does not support `%s`).

//TEST:SIMPLE(filecheck=CHECK):-target cpp -entry main -stage compute

#define STR(x) #x

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    printf("%s\n", STR());
    //CHECK: ""
}
