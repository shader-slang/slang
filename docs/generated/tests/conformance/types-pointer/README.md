---
generated: true
model: claude-sonnet-4-6
generated_at: 2026-06-02T07:02:32+00:00
source_commit: 9c1a6a00ef413932805da5b813465a7a9d517fb9
watched_paths_digest: 2b0e253fb65db603f9df6fa19ba622f223bf86e9067a43af8b300c966481c5fb
source_doc: docs/language-reference/types-pointer.md
source_doc_digest: 91cfb04551a773c3d6aaf4f84bb90acc966ba4a3469377cb572cccf312389004
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Tests for conformance/types-pointer

## Intent

Tests verify pointer-type claims in the **language reference** at
[`docs/language-reference/types-pointer.md`](../../../../language-reference/types-pointer.md).
The doc covers five areas: declaration syntax (`T*`), generic pointer
types (`Ptr<T>` / `ImmutablePtr<T>`), pointer operations (address-of,
dereference, member access, arithmetic), pointer traits (access mode,
address space, no implicit RW-to-immutable conversion), and documented
limitations (`const` pointer rejection). Pointers to local memory are
supported on CPU and CUDA targets; the test suite uses `INTERPRET` and
CPP/CUDA/SPIRV-asm emission to exercise each claim. Negative claims
are tested as `DIAGNOSTIC_TEST` with pinned E-codes.

## Claims

### §syntax — Declaration Syntax

- **C1.** The `T*` declaration syntax creates a pointer-to-T type.
- **C2.** `T*` is equivalent to `Ptr<T, Access.ReadWrite, AddressSpace.Device>` (Remark 1).
- **C3.** Slang does not currently support `const` pointer declarations (`const T*`); the compiler rejects them with E20017.

### §generic-pointer — Generic Pointer Types

- **C4.** `Ptr<T, AccessMode, AddressSpace>` is a usable standard-library generic pointer type.
- **C5.** `ImmutablePtr<T, AddressSpace>` is a usable standard-library type alias for pointer-to-immutable-data.

### §description — Description

- **C6.** The address-of operator `&` obtains a pointer to an object.
- **C7.** `__getAddress(obj)` is an alternative to `&` for obtaining the address of an object.
- **C8.** The dereference operator `*` accesses (reads) the pointed-to object.
- **C9.** A dereference `*p` is an l-value; assigning through `*p = v` mutates the pointed-to object.
- **C10.** For a pointer to a struct or class, `p->member` accesses the member (equivalent to `(*p).member`).
- **C11.** Adding an integer `n` to a pointer-to-array-element yields a pointer `n` elements forward.
- **C12.** Subtracting an integer `n` from a pointer-to-array-element yields a pointer `n` elements back.
- **C13.** The `++` / `--` operators on a pointer advance or retreat by one element.
- **C14.** A null pointer (`nullptr`) is a valid pointer value that compares equal to `nullptr`.
- **C15.** A pointer one past the last element of an array (`&arr[N-1] + 1`) is a valid past-end sentinel (may not be dereferenced).

### §traits — Pointer Traits

- **C16.** The default pointer address space is `AddressSpace.Device` and the default access mode is `Access.ReadWrite`; `T*` and `Ptr<T, Access.ReadWrite, AddressSpace.Device>` are the same type.
- **C17.** There is no implicit conversion from a read-write pointer (`Ptr<T>`) to a read-only pointer (`ImmutablePtr<T>`); the compiler rejects the implicit assignment with E30019.

### §examples — Examples

- **C18.** Pointer range iteration `[start, end)` using `p != end; ++p` traverses all array elements; the result is correct.

## Functional coverage

| Claim                                                                                                          | Intent     | Anchor                                                                              | Tests                                                                                                                                                                                          |
| -------------------------------------------------------------------------------------------------------------- | ---------- | ----------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| **C1.** `T*` declaration syntax creates a pointer-to-T; address-of and dereference round-trip correctly.       | functional | [#syntax](../../../../language-reference/types-pointer.md#syntax)                   | [`declare-star-syntax-functional.slang`](declare-star-syntax-functional.slang), [`declare-star-syntax-emission.slang`](declare-star-syntax-emission.slang)                                     |
| **C2.** `T*` is the same type as `Ptr<T, Access.ReadWrite, AddressSpace.Device>`; values are interchangeable.  | functional | [#syntax](../../../../language-reference/types-pointer.md#syntax)                   | [`ptr-star-equals-ptr-generic-functional.slang`](ptr-star-equals-ptr-generic-functional.slang), [`pointer-default-access-mode-functional.slang`](pointer-default-access-mode-functional.slang) |
| **C3.** `const T*` is rejected with E20017.                                                                    | negative   | [#syntax](../../../../language-reference/types-pointer.md#syntax)                   | [`const-pointer-rejected-negative.slang`](const-pointer-rejected-negative.slang)                                                                                                               |
| **C4.** `Ptr<T>` generic type is declared and dereferenced correctly.                                          | functional | [#generic-pointer](../../../../language-reference/types-pointer.md#generic-pointer) | [`ptr-generic-type-functional.slang`](ptr-generic-type-functional.slang)                                                                                                                       |
| **C5.** `ImmutablePtr<T>` type alias compiles and dereferences correctly via explicit cast.                    | functional | [#generic-pointer](../../../../language-reference/types-pointer.md#generic-pointer) | [`immutableptr-type-functional.slang`](immutableptr-type-functional.slang)                                                                                                                     |
| **C6.** Address-of operator `&` obtains a live pointer; `*(&obj) == obj`.                                      | functional | [#syntax](../../../../language-reference/types-pointer.md#syntax)                   | [`address-of-operator-functional.slang`](address-of-operator-functional.slang)                                                                                                                 |
| **C7.** `__getAddress(obj)` is an alternative to `&`; yields the same live pointer.                            | functional | [#syntax](../../../../language-reference/types-pointer.md#syntax)                   | [`getaddress-builtin-functional.slang`](getaddress-builtin-functional.slang)                                                                                                                   |
| **C8/C9.** Dereference `*p` reads the value; `*p = v` mutates the pointed-to object.                           | functional | [#syntax](../../../../language-reference/types-pointer.md#syntax)                   | [`dereference-write-functional.slang`](dereference-write-functional.slang)                                                                                                                     |
| **C10.** `p->member` and `(*p).member` both access the same struct member; mutation through `->` is visible.   | functional | [#syntax](../../../../language-reference/types-pointer.md#syntax)                   | [`struct-arrow-member-access-functional.slang`](struct-arrow-member-access-functional.slang)                                                                                                   |
| **C11.** `p + n` advances a pointer `n` elements; the result dereferences to the correct element.              | functional | [#syntax](../../../../language-reference/types-pointer.md#syntax)                   | [`pointer-arithmetic-add-functional.slang`](pointer-arithmetic-add-functional.slang), [`pointer-arithmetic-spirv-emission.slang`](pointer-arithmetic-spirv-emission.slang)                     |
| **C12.** `p - n` retreats a pointer `n` elements; the result dereferences to the correct element.              | boundary   | [#syntax](../../../../language-reference/types-pointer.md#syntax)                   | [`pointer-arithmetic-sub-functional.slang`](pointer-arithmetic-sub-functional.slang)                                                                                                           |
| **C13.** `++p` / `--p` advance and retreat by one element; the result dereferences to the correct element.     | boundary   | [#syntax](../../../../language-reference/types-pointer.md#syntax)                   | [`pointer-increment-functional.slang`](pointer-increment-functional.slang)                                                                                                                     |
| **C14.** `nullptr` is a valid null pointer value; `p == nullptr` is true for null and false for non-null.      | boundary   | [#syntax](../../../../language-reference/types-pointer.md#syntax)                   | [`null-pointer-functional.slang`](null-pointer-functional.slang)                                                                                                                               |
| **C15/C18.** Past-end pointer `&arr[N-1] + 1` is a valid sentinel; range iteration produces the correct sum.   | functional | [#examples](../../../../language-reference/types-pointer.md#examples)               | [`pointer-range-iteration-functional.slang`](pointer-range-iteration-functional.slang)                                                                                                         |
| **C16.** Default address space is `AddressSpace.Device` and access mode is `Access.ReadWrite`.                 | functional | [#traits](../../../../language-reference/types-pointer.md#traits)                   | [`pointer-default-access-mode-functional.slang`](pointer-default-access-mode-functional.slang)                                                                                                 |
| **C17.** No implicit conversion from read-write `Ptr<T>` to read-only `ImmutablePtr<T>`; rejected with E30019. | negative   | [#traits](../../../../language-reference/types-pointer.md#traits)                   | [`no-implicit-rw-to-immutable-negative.slang`](no-implicit-rw-to-immutable-negative.slang)                                                                                                     |
| A pointer passed as a function argument lets the callee mutate the caller's variable through dereference.      | functional | [#examples](../../../../language-reference/types-pointer.md#examples)               | [`pointer-pass-to-function-functional.slang`](pointer-pass-to-function-functional.slang)                                                                                                       |

## Untested claims

| Claim                                                                                                                               | Reason         | Anchor                                                            | Why untested                                                                                                                                                                                                                                                                      |
| ----------------------------------------------------------------------------------------------------------------------------------- | -------------- | ----------------------------------------------------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Pointers to local memory are supported only on CUDA and CPU targets (not other targets).                                            | gpu-other      | [#syntax](../../../../language-reference/types-pointer.md#syntax) | The doc states "Pointers to local memory are supported only on CUDA and CPU targets." Verifying that a GPU target rejects local-memory pointers requires a running GPU driver (Vulkan/D3D12). CI validates this.                                                                  |
| Slang does not support pointers to opaque handle types such as `Texture2D`; `DescriptorHandle<T>` should be used instead.           | gpu-other      | [#syntax](../../../../language-reference/types-pointer.md#syntax) | Taking `&myTex` of a `Texture2D` at global scope fails with E30079 ("cannot take address of immutable object") rather than a dedicated "opaque handle pointer" rejection. The actual error is about mutability, not opacity; the doc wording is imprecise. Recorded as a doc gap. |
| Slang does not support custom alignment specification; `loadAligned()` / `storeAligned()` may be used for aligned loads/stores.     | out-of-bundle  | [#syntax](../../../../language-reference/types-pointer.md#syntax) | `loadAligned()` and `storeAligned()` are separate library functions defined outside this doc. Alignment is not directly testable from the declaration syntax surface.                                                                                                             |
| Slang does not support pointer-to-interface inheritance casting (pointer to struct conforming to I cannot be cast to pointer to I). | gpu-other      | [#syntax](../../../../language-reference/types-pointer.md#syntax) | The doc says this is unsupported, but the compiler accepts the explicit cast at parse/check time. The failure surfaces only as E50100 at code-gen time when an interface dispatch is attempted. See doc gap row below.                                                            |
| A pointer belongs to one of: valid-object, past-end, null, or invalid; dereferencing non-object pointers is undefined behavior.     | (unclassified) | [#syntax](../../../../language-reference/types-pointer.md#syntax) | The "undefined behavior" classification is normative but not testable: the compiler is not required to reject or flag UB, and the behavior is by definition unspecified.                                                                                                          |
| Pointer arithmetic in a multi-dimensional array must stay within the same innermost array; any other result is undefined behavior.  | (unclassified) | [#syntax](../../../../language-reference/types-pointer.md#syntax) | UB — not testable for the same reason as the general dereference-of-invalid claim above.                                                                                                                                                                                          |
| `Ptr<T, AccessMode, AddressSpace>` with non-Device address spaces (e.g. `AddressSpace.GroupShared`).                                | gpu-other      | [#traits](../../../../language-reference/types-pointer.md#traits) | The doc names `Ptr<T, AccessMode, AddressSpace>` as the general form but does not enumerate all address spaces in this file. Testing non-Device spaces requires a GPU runner and specific capability sets. CI validates.                                                          |

## Doc gaps observed

| Anchor                                                            | Kind            | Gap                                                                                                                                                                                                                                                                                                                                                                   | Suggested addition                                                                                                                                                                                                                                   |
| ----------------------------------------------------------------- | --------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| [#syntax](../../../../language-reference/types-pointer.md#syntax) | ambiguous-claim | The doc states "Slang does not support pointers to opaque handle types such as Texture2D." In practice, `&myTex` at global scope is rejected with E30079 ("cannot take address of immutable object"), not a dedicated "no pointer to opaque type" diagnostic. A reader cannot distinguish whether the error is about opacity or global immutability.                  | Clarify the mechanism: note that opaque resource types are implicitly immutable objects, so `&myTex` fails with E30079. Alternatively, add a dedicated diagnostic for attempting to take the address of an opaque handle type and document its code. |
| [#syntax](../../../../language-reference/types-pointer.md#syntax) | ambiguous-claim | The doc states that "a pointer to a structure conforming to interface I cannot be cast to a pointer to I." The compiler accepts the explicit cast `(IFoo*)bp` at parse/check time without error; the failure only appears as E50100 at code-gen time when an interface dispatch is attempted. The doc implies type-system rejection, but this only fails at code-gen. | Clarify that the cast is accepted syntactically but the resulting pointer-to-interface cannot be used for dynamic dispatch (code-gen rejects the dispatch, not the cast).                                                                            |
