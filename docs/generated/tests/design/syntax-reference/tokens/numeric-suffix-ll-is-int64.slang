//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/tokens.md#content-tokens
//META: doc_section_digest=6d196fa35abfeab93be9899c03adbb528a464f03a35cc28a6454968f1c88eb2a
//META: purpose=Boundary - the `ll` suffix on an IntegerLiteral types it as `int64_t` (signed 64-bit).
//META: intent=boundary
//META: pipeline_stage=lex
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary probe: `42ll` should resolve to `int64_t`. Distinguished
// from `int` and `uint64_t` via overload set so the suffix decoding
// is unambiguously checked.

//TEST:INTERPRET(filecheck=CHECK):

int kind(int v) { return 1; }
int kind(uint v) { return 2; }
int kind(int64_t v) { return 3; }
int kind(uint64_t v) { return 4; }

void main()
{
    // CHECK: kind=3
    printf("kind=%d\n", kind(42ll));
}
