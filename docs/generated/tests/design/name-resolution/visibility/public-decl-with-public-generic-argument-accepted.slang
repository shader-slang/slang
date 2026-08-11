//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/visibility.md#container-level-cap
//META: doc_section_digest=17336538ec29e57920f3b7003dd39a6ad2e2af3018c615ad8a7112f747ce1b92
//META: purpose=When every declaration-reference generic argument is at least as visible as the generic itself, the minimum leaves the specialization fully visible and a `public` signature naming it is accepted.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the accepting side of the same minimum: `Box<int>` specializes a
// `public` generic on a builtin argument, so the minimum stays `public` and
// the `public` function naming `Box<int>` checks cleanly. Pairing this with
// the `Box<Key>` rejection shows the argument, not the generic itself, is
// what lowers the visibility. The CHECK asserts the value round-trips
// through the public signature at run time.

//TEST:INTERPRET(filecheck=CHECK):

public struct Box<T> { T value; __init(T v) { value = v; } }

public int unbox(Box<int> b) { return b.value; }

void main()
{
    // CHECK: b=33
    printf("b=%d\n", unbox(Box<int>(33)));
}
