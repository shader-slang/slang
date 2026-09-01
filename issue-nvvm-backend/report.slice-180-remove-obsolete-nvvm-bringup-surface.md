# Slice 180: Remove obsolete direct-NVVM bring-up surface

## Motivation

The shielded LLVM provider already has generic typed value operations and generic construction for
calls, phis, and returns. The C++ facade nevertheless retained earlier integer- and Float32-specific
methods, making every scalar operation appear to have two contracts and requiring production code
to choose between equivalent paths.

## Proposed solution

Delete the redundant facade and use the current generic ABI directly. Keep scalar fixture setup
readable with test-local descriptor helpers, but give those helpers no production visibility and no
independent semantic policy.

## Change summary

- Removed 23 convenience declarations and their duplicate implementations.
- Routed all production calls, phis, incoming values, and returns through generic construction.
- Consolidated scalar fixture construction on typed value-operation descriptors.
- Removed assertions that tested the obsolete adapter's narrower operation range.
- Replayed frozen corpus v1 and the separate discovery corpus without regression.

## Concepts and vocabulary

**Facade** is the in-process C++ `NVVMIRBuilder` wrapper around provider ABI revision 34.

**Generic construction** is the provider interface for structural LLVM operations such as calls,
phis, and returns. **Typed value operation** is a semantic operation descriptor containing the
operation, result type, and operand types.

## Process report

Consider an ordinary signed-i32 helper call. Before this slice, `_usesGenericNVVMFunctions`
selected either `emitCall` or `emitIntegerCall`, even though both methods dispatched the exact same
`m_construction.emitCall` callback. Phi creation/incoming values and returns repeated the pattern.
The lowered provider type already carries the complete contract, so the integer spelling was not a
valid alternate IR representation; it was bring-up history. Production now calls the generic
method once and retains the same Half boundary conversion and diagnostic ownership.

Arithmetic conveniences similarly constructed fixed signed-i32 or Float32 descriptors and then
called `emitValueOperation`. Tests now state those descriptors through local helpers. One old test
expected `emitIntegerBinary` to reject `SUBTRACT + 1`, which is `MULTIPLY`; the generic semantic
catalog intentionally accepts it. That assertion proved only the removed adapter's arbitrary
range. Generic unknown-operation rejection and exact descriptor validation remain, including the
no-mutation serialization check.

The self-review inventory found no surviving compatibility method or new production special case.
The only new helpers live in the test translation-unit support header, translate explicit scalar
types into the current descriptor, and fail if removed from the fixtures that call them. No
provider callback, ABI metadata, source syntax reconstruction, fallback, or accepted IR shape was
added.

Measured counted lines changed as follows: facade header 470 to 332, facade implementation 1,230
to 897, and emitter 15,663 to 15,620. The two test files changed from 23,032 to 23,289 because the
single descriptor contract states types explicitly; total production reduction is 514 lines and
provider ABI revision 34 is unchanged.

Frozen corpus v1 remains exactly 452 workloads/427 healthy references at 417/417/417 O0/O3/both.
All-row direct classifications remain 431 correct, three runtime mismatches, and 18 preflight
failures per mode. Discovery remains exactly 82 workloads/72 healthy references at 72/72/72, with
no newly unlocked workload and no old-correct regression. Focused real/fake provider tests pass;
the selected prefix and permanent category are unchanged behaviorally.
