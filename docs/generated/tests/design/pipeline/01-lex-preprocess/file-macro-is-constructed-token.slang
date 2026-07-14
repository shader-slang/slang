//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=ef4af96c0996402dfe65ab0fdd347e4ae7e1a742
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#source-location-preservation
//META: doc_section_digest=377365e730e5a4dfaf556ef64127fd8adbee9dab99b45e23293376d8f95d3072
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
