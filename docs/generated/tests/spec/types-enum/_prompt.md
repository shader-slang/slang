# Prompt: docs/generated/tests/spec/types-enum/

See [`_common.md`](_common.md).

## Target

[`docs/language-reference/types-enum.md`](../../../../language-reference/types-enum.md).

## High-value claims

- **Default underlying type is `int`** when none is specified.
- **Implicit numbering**: first enumerator defaults to 0; each
  subsequent unspecified enumerator is previous+1.
- **Explicit-then-resume**: an explicit value resumes implicit
  numbering from that value (e.g. `A=10, B, C` → 10, 11, 12).
- **`[Flags]` attribute** changes defaults: first is 1, each
  subsequent unspecified is previous-shifted-left-by-1.
- **Shared values allowed**: `A=5, B=5` is legal.
- **Scoped access**: `EnumType.NAME` (Slang's default for `enum`).

## What NOT to test

- Unscoped enum behaviour — Remark 1 in the doc says this is being
  deprecated; not worth pinning.
- `enum class` vs `enum` distinction — Remark 2 says they'll
  collapse; not worth pinning.
