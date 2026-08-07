//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:27:31Z
//META: source_commit=6b490bc8cb3cf11b356ddf70168c5609efa46341
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#macro-expansion
//META: doc_section_digest=cf4f1b505a333d6ccc1a3cb482330c4dde4668bd530a4d0fe4905412ffd9448a
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
