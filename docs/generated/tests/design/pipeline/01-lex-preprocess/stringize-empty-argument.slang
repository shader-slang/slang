//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T12:00:00+00:00
//META: source_commit=ef4af96c0996402dfe65ab0fdd347e4ae7e1a742
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#source-location-preservation
//META: doc_section_digest=1f95bdbd2dae47141e9ce007dbaf0bb66550c8aacddbc7708ffa3f8f5b7ad00d
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
