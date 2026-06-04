# Prompt: docs/generated/tests/conformance/types-pointer/

See [`_common.md`](_common.md).

## Target

Bundle at `docs/generated/tests/conformance/types-pointer/`,
anchored to
[`docs/language-reference/types-pointer.md`](../../../../language-reference/types-pointer.md).

## High-value claims

The doc covers several distinct surfaces, each requiring its own
test axis:

### Declaration syntax (`#syntax`)

- `T*` is the pointer-declaration syntax.
- `T*` is equivalent to `Ptr<T, Access.ReadWrite, AddressSpace.Device>`.
- These two forms can be used interchangeably (assignments compile).

### Generic pointer types (`#generic-pointer`)

- `Ptr<T, AccessMode, AddressSpace>` is the generic form.
- `ImmutablePtr<T, AddressSpace>` is the immutable-data alias.

### Description (`## Description`)

- Address-of operator `&` obtains a pointer to an object.
- `__getAddress(obj)` is an alternative to `&`.
- Dereference `*` accesses the pointed-to object.
- Member access via `->` (and `.` after `*`) work on struct/class pointers.
- Pointer arithmetic: `p + n` / `p - n` / `p++` / `p--` offset by n elements.
- Null pointer (`nullptr`) is a valid pointer value.
- The past-end pointer (`&arr[N-1] + 1`) is valid to hold but not dereference.

### Pointer traits (`#traits`)

- A pointer type carries: element type, access mode, address space.
- Default address space = `AddressSpace.Device`; default access mode = `Access.ReadWrite`.
- No implicit conversion from read-write (`Ptr<T>`) to read-only (`ImmutablePtr<T>`).

### Documented limitations (negative / `DIAGNOSTIC_TEST`)

- `const int*` is rejected (E20017 — `const` not allowed on C-style pointer).
- Assigning `Ptr<T>` (RW) to `ImmutablePtr<T>` (immutable) is rejected (E30019).

## Claim-extraction strategy

Every statement that uses "is", "may", "can", "must", or the pattern
"X is rejected" in the doc body is treated as a claim. Remarks (📝)
that describe concrete observable behavior count as claims; warnings
(⚠️) that describe current limitations count as negative claims
(DIAGNOSTIC_TEST or `## Untested claims` with `compiler-bug-pending`
/ `gpu-other`).

The "TODO" notes (e.g. "pointer expressions (TODO)") are not claims.

## What NOT to test here

- `loadAligned()` / `storeAligned()` — separate library surface,
  not defined in this doc.
- Pointer to `DescriptorHandle<T>` — mentioned as a workaround for
  unsupported opaque-handle pointers; the DescriptorHandle type is
  not defined in this doc.
- Pointer-to-interface casting — the doc states this is unsupported
  ("cannot be cast to a pointer to `I`"), but the compiler actually
  accepts the cast (the limitation only appears at code-gen time with
  an interface dispatch error). Record as a doc gap.
- Pointer traits with non-Device address spaces — the doc names the
  surface (`Ptr<T, AccessMode, AddressSpace>`) but does not commit to
  specific runtime behaviors for each space; test only the default.
- Memory layout / alignment — no doc claim about layout observable
  from slang-test.
- `AddressSpace` values beyond `Device` — the doc does not enumerate
  all address spaces in this file.
