//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#failure-modes
//META: doc_section_digest=045c5379570a64224b0f0d784c24bc57410eef152e105148ecb4e662cb3f9c88
//META: purpose=A decimal `z` or `ll` literal with no `u` is signed up to INT64_MAX, and exactly INT64_MAX + 1 is typed unsigned unless a surrounding unary `-` rewrites the type back to the signed pointer-width or 64-bit type.
//META: intent=boundary
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the signed/unsigned boundary of decimal integer-literal typing and
// the exception that sits exactly on it. INT64_MAX itself is signed; one past
// it flips to unsigned, because that magnitude no longer fits a signed 64-bit
// type. The exception is what makes `-9223372036854775808` — the signed minimum,
// which can only be spelled as a negation of an unrepresentable positive — come
// out signed after all: the literal is marked when it lands on that exact value
// and the prefix `-` rewrites the type. An overload probe reports the resulting
// static type, and the test runs the same three points on both `ll` and `z` so
// the rule is shown to be about the magnitude rather than the width letter.

//TEST:INTERPRET(filecheck=CHECK):

int probe(intptr_t) { return 1; }
int probe(uintptr_t) { return 2; }
int probe(int64_t) { return 5; }
int probe(uint64_t) { return 6; }

void main()
{
    // INT64_MAX — still signed.
    //CHECK: max_ll=5
    printf("max_ll=%d\n", probe(9223372036854775807ll));

    // INT64_MAX + 1 — no longer fits signed, so the literal is unsigned.
    //CHECK: maxp1_ll=6
    printf("maxp1_ll=%d\n", probe(9223372036854775808ll));

    // The same literal under a unary minus is rewritten back to signed.
    //CHECK: neg_maxp1_ll=5
    printf("neg_maxp1_ll=%d\n", probe(-9223372036854775808ll));

    // The pointer-width suffix follows the same schedule.
    //CHECK: max_z=1
    printf("max_z=%d\n", probe(9223372036854775807z));

    //CHECK: neg_maxp1_z=1
    printf("neg_maxp1_z=%d\n", probe(-9223372036854775808z));
}
