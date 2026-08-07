//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:27:31Z
//META: source_commit=6b490bc8cb3cf11b356ddf70168c5609efa46341
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#macro-expansion
//META: doc_section_digest=cf4f1b505a333d6ccc1a3cb482330c4dde4668bd530a4d0fe4905412ffd9448a
//META: purpose=Argument tokens replayed for an expanded-parameter reference are themselves macro-expanded, so a macro name passed as an argument expands in the body.
//META: intent=functional
//META: pipeline_stage=preprocess
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that an argument's token range is replayed through a
// macro-expanding stream: the argument to TWICE is itself an
// object-like macro, and both parameter references in the body must
// see its expansion (21), giving 42. If argument tokens were
// replayed without expansion, the identifier BASE would reach the
// parser unresolved and the test would not compile.

//TEST:INTERPRET(filecheck=CHECK):

#define BASE 21
#define TWICE(x) ((x) + (x))

void main()
{
    // CHECK: twice=42
    printf("twice=%d\n", TWICE(BASE));
}
