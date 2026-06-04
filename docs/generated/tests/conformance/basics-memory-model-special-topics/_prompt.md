# Prompt: docs/generated/tests/conformance/basics-memory-model-special-topics/

See [`_common.md`](_common.md).

## Target

Bundle at `docs/generated/tests/conformance/basics-memory-model-special-topics/`,
anchored to
[`docs/language-reference/basics-memory-model-special-topics.md`](../../../../language-reference/basics-memory-model-special-topics.md).

## Doc summary

The doc (≈72 lines, 2 sections) covers two special-topic corners of the Slang memory model:

1. **Function Parameters with in/out/inout Modifiers** (lines 3–61): Normative rules about
   how the `in`, `out`, and `inout` directional qualifiers govern how arguments may be
   accessed during a function call. The section includes:
   - Access rules: `in` = any number of reads; `out` = any number of writes; `inout` = any
     number of reads and writes.
   - A "default direction" claim: `in` is the default.
   - A cross-reference to data race rules for concurrent calls with the same variable.
   - A normative UB statement: passing the same variable to two parameters where at least one
     is `out`/`inout` is undefined behavior.
   - A non-normative remark about two implementation approaches (copy-in/copy-out vs pointer).
   - Example 1 demonstrating the UB aliasing case (`a` passed to both `i1` and `o`).

2. **Memory Aliasing via Binding** (lines 63–72): Normative rules about resource-handle aliasing.
   If two different resource handles (`ConstantBuffer<T>`, `RWStructuredBuffer<T>`,
   `Texture2D<T>`) are bound to the same underlying memory, accesses via either handle are
   considered overlapping for data race analysis. A data race can occur even with different
   handles if one modifies overlapping memory non-atomically.

## Claim extraction strategy

- **Access rules (C1–C4)**: Normative — "may read N times", "may write N times". Observable
  in emission: SPIRV passes `out`/`inout` arguments as `_ptr_Function_int` pointers (pointer
  indirection approach); HLSL preserves the `inout`/`out` keyword directly. Test both backends.
- **Default direction (C1)**: Observable in HLSL emission: a plain parameter and an explicit
  `in` parameter emit identically (no `inout` keyword on either).
- **"even if function does nothing" (C5)**: Non-normative freedom for the implementation;
  no observable surface. Mark untested with reason `non-normative`.
- **Data race with concurrent threads (C6)**: Purely runtime concern; no emission or
  diagnostic surface. Mark untested with reason `gpu-other`.
- **Aliasing UB / W30051 (C7)**: Normative and compiler-observable — the compiler emits
  `W30051` when the same variable appears as two `out`/`inout` arguments. Test with
  `DIAGNOSTIC_TEST` pinning `E30051`. Cover two sub-cases: inout+inout (from the doc's
  own example) and in+out (the "at least one is out/inout" minimum case).
- **Aliasing via binding (C8–C10)**: The fact that `ConstantBuffer<T>` and `RWStructuredBuffer<T>`
  can alias is a runtime concern, but the compilability and emission shape of the two resource
  types is observable. Emit tests pin that both resources compile to distinct binding declarations.
  The runtime-race aspect of C9/C10 is untested (reason: `gpu-other`).
- **Client API restrictions (C11)**: Non-normative from Slang's perspective; mark untested.

## What NOT to test here

- Actual runtime data races between threads (C6, C9, C10) — require GPU compute execution.
- The specific implementation approach (copy-in/copy-out vs pointer) — the remark explicitly
  says it is unspecified; asserting either approach would be testing an implementation detail.
- Multi-file or multi-dispatch tests — `in`/`out`/`inout` semantics are single-call.
- `ref` parameters — the doc does not name `ref` in this section (though W30051 mentions it
  in its message text).
