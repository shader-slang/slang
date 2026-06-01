# Prompt: docs/generated/tests/language-reference/types-fundamental-integer/

See [`_common.md`](_common.md) for universal rules. **Read the
`### Source-of-truth hierarchy` section in `_common.md` first.**

## Target

Produce the test bundle at
`docs/generated/tests/language-reference/types-fundamental-integer/`,
anchored to
[`docs/language-reference/types-fundamental.md`](../../../../language-reference/types-fundamental.md)
(the `#integer-types` section).

The bundle scope is **the integer-type claims only** —
sibling float / bool / void claims live in their own bundles when
written. Specifically:

1. **Type list.** The doc enumerates `int8_t`, `int16_t`,
   `int`/`int32_t`, `int64_t`, `uint8_t`, `uint16_t`,
   `uint`/`uint32_t`, `uint64_t`. The pairs `int`/`int32_t` and
   `uint`/`uint32_t` are spec-level aliases.

2. **Wrap on overflow.** _"All arithmetic operations on signed and
   unsigned integers wrap on overflow."_ This is the single most
   important behavioural claim in the section and must be exercised
   across the documented integer types with boundary inputs (MAX,
   MAX+1, MIN, MIN-1).

3. **Natural size and alignment.** _"All integer types are stored
   in memory with their natural size and alignment on all targets
   that support them."_

4. **Target dependence.** Only `int`/`int32_t` and `uint`/`uint32_t`
   are universally supported; the others depend on target +
   capabilities.

## Boundary axes

Per [`_common.md § Mandatory axes`](_common.md): every integer type
the doc mentions deserves at least the `0`, `MIN`, `MAX`, `MAX+1`
(wrap), `MIN-1` (wrap for signed) tests. Many of these are
slangi-observable via `INTERPRET` + `printf`.

For 8-bit and 16-bit types, target portability is uneven. Limit
boundary tests to `int8_t`/`uint8_t`/`int16_t`/`uint16_t` claims
that the language reference itself makes; if the doc is silent on
8/16-bit wraparound, write the test on the 32-/64-bit types where
the claim is unambiguous and record a doc-gap.

## Observation tooling

- **Wrap-on-overflow:** `//TEST:INTERPRET(filecheck=CHECK):` and a
  `printf("%d", expr)` (or `%u`/`%lld`/`%llu` as appropriate). Use
  `uniform` inputs to defeat the constant folder when the operands
  would otherwise be folded. See
  [`_common.md § Defeat the optimizer`](_common.md).
- **Storage-layout claims** (natural size + alignment) are hard to
  observe directly without `sizeof`/`alignof` exposure. Where the
  doc makes the claim but the slang surface cannot observe it, write
  it into `## Untested claims` with reason `internal-source-fact`.
- **Aliasing** (`int` ↔ `int32_t`): test that a variable declared as
  one and assigned from the other parses cleanly and behaves
  identically across a few operations.

## What NOT to test

- Behaviours the language reference does not describe (e.g. specific
  emit forms for `int8_t` on Metal). Those belong in target-pipeline
  bundles, not here.
- Behaviours that are target-conditional and not part of the
  universally-supported set — write them only if you also add a
  capability gate.

## Bundle size guidance

Target ~12-20 tests. The wrap-on-overflow claim is the centerpiece;
spend most of the budget proving it holds at the documented edges
on the 32- and 64-bit types.
