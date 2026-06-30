//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:03:13Z
//META: source_commit=ed8f508fc647eecd788a4bd2bb63a4a6f5c80246
//META: doc_ref=docs/generated/design/ast-reference/values.md#intval-family
//META: doc_section_digest=76a84a059e7493d5ffee00d494fa33d266abc65cffaa634a39c7c87cf041e48b
//META: purpose=A SizeOfIntVal is a compile-time IntVal produced by sizeof; the resulting integer is usable at compile time and prints as a positive integer.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc lists `SizeOfIntVal` as "compile-time `sizeof`". The
// observable surface is that it yields an integer value that
// participates in compile-time evaluation and is reported as a
// positive integer for a non-empty type.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    // sizeof on a scalar yields a positive integer compile-time
    // value (the precise number is target-dependent; we observe
    // only that it is a positive integer above 0 — int is at
    // least one byte on every reasonable target).
    static const int SZ = int(sizeof(int));
    // CHECK: positive=1
    printf("positive=%d\n", SZ > 0 ? 1 : 0);
}
