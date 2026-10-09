# Prompt: docs/generated/tests/conformance/types-vector-and-matrix/

See [`_common.md`](_common.md) for universal rules, the source-of-
truth hierarchy, and the parallel-trees policy.

## Target

Bundle at
`docs/generated/tests/conformance/types-vector-and-matrix/`,
anchored to
[`docs/language-reference/types-vector-and-matrix.md`](../../../../language-reference/types-vector-and-matrix.md).

## High-value claims

- **Vector element access** via `[]` and `.x`/`.y`/`.z`/`.w`.
- **Swizzle extract**: `.xy` returns a smaller vector; the same
  element may be repeated (`.xww`).
- **Swizzle assign**: target columns must be unique.
- **Unary op is element-wise**: `-v` negates every component.
- **Binary op with scalar broadcasts**: `v - 1` subtracts from each
  component; `4 - v` subtracts each component from 4.
- **Binary op with same-length vector is element-wise**.
- **Matrix element access**: `m[r][c]` indexes row then column;
  `m[r]` returns a row vector; `m[r].yx` swizzles a row.
- **`mul()` for matrix multiplication** — NOT `*`. The doc has an
  explicit remark distinguishing this from GLSL.

## What NOT to test here

- The full Slang matrix `mul()` numerical semantics — focus on the
  doc's claim "`*` is element-wise, `mul()` is matrix multiply" via
  a small example that distinguishes the two.
- Memory layout / alignment claims — not observable from the slangc
  CLI (record as untested-claim if relevant).
- Standard type aliases like `int4` / `float4x3` — exercising the
  alias is a no-op behavioural test. Note them in `## Untested
claims` instead.
