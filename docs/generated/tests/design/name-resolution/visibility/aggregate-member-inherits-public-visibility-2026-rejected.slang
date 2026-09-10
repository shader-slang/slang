//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/visibility.md#per-keyword-semantics
//META: doc_section_digest=7d96ab9817d6f6251e2148b7d4d9959fb9ec73041354e919db537f46458d416e
//META: purpose=Under Slang 2026 an unmodified field of a `public` aggregate inherits `public`, so a field whose type is only `internal` is rejected with `references less visible type`.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the upward direction of the 2026 aggregate-inheritance rule.
// `Hidden` carries no modifier at module scope, so it is `internal`; `Box`
// is explicitly `public` and its field `h` carries no modifier. Under 2026
// `h` inherits `public` from `Box`, which makes it reference a less visible
// type. The CHECKs pin E30604 on the field and on the synthesized `Box.init`
// that also has to name `Hidden`.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-std 2026

struct Hidden { int v; }

public struct Box { Hidden h; }
//CHECK:                   ^ references less visible type
//CHECK:                   ^ 'h' references less visible type 'Hidden'.
//CHECK:      ^^^ references less visible type
//CHECK:      ^^^ 'Box.init' references less visible type 'Hidden'.
// CHECK: E30604
