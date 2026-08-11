//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#source-location-preservation
//META: doc_section_digest=efc11981208d28591f6638940ff7418ff4c200e6019f91cc90cd83be58370ef5
//META: purpose=`__FILE__` is a constructed token synthesized fresh at the use site; it expands to a string literal naming the current source file.
//META: intent=functional
//META: pipeline_stage=preprocess
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `__FILE__` expands to a string literal that contains
// the source-file name. We emit to CPP and FileCheck for the file
// name fragment in the emitted source (slangi printf does not
// support `%s`, so the interpreter path is unavailable for
// string-typed observations).

//TEST:SIMPLE(filecheck=CHECK):-target cpp -entry main -stage compute

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    printf("%s\n", __FILE__);
    //CHECK: file-macro-is-constructed-token.slang
}
