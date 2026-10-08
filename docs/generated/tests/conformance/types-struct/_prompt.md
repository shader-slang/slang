# Prompt: docs/generated/tests/conformance/types-struct/

See [`_common.md`](_common.md).

## Target

Bundle at `docs/generated/tests/conformance/types-struct/`,
anchored to
[`docs/language-reference/types-struct.md`](../../../../language-reference/types-struct.md).

## Sub-areas and claim extraction strategy

The doc has the following normative sections:

### Syntax (`#syntax`, `#parameters`)

Grammar productions for struct declarations (no-body, with-members, extern, export) and
member-list productions. These are normative parsing claims. The four forms are claims;
the member-list item-type list is a structural claim (each member kind must be parsable).
The deprecation warning about bracketed attributes after `struct` is normative.

### Description (`#description`)

Dense paragraph text with many normative behavioral claims:

- No-body forward declarations; with-members definitions; link-time extern/export forms.
- Named vs anonymous structs; anonymous struct unique naming; main use in inline type definitions.
- Member kinds enumeration (static data, non-static data, constructors, static/non-static functions, nested type, property, subscript, call-op).
- `static` keyword semantics for data members (global storage) and functions.
- Default initializer semantics (initializer-list case, constructor case, `static const` must have one).
- Non-static data members allocated sequentially within the struct.
- Nested type scoping.
- Interface conformance.
- Extension.
- Access control: `public`, `internal`, `private`; default is `internal`; nested struct access to private of enclosing.
- Deprecation warning about struct inheriting from struct.

### Object (`#object`)

An instance consists of all non-static data members. Data members may be initialized via
initializer list or constructor. This is a definitional claim — testable via actual values.

### Constructors (`#constructor`)

- `__init` syntax.
- User-provided constructor executes on instantiation.
- Constructor has no return type.
- Multiple constructors → overload resolution.
- No-parameter constructor invoked when no initializer list provided.
- Non-static data member not initialized by constructor → undefined state.
- `const` data members cannot be initialized by the constructor.
- Aggregate initialization when no user-provided constructor.
- Partial initializer list → remaining fields default-initialized.
- No initializer and no user constructor → undefined state.

### Static Member Functions (`#static-member-function`)

- A static member function is a regular function in the struct namespace.
- May only access static members.
- Invoked without an object.

### Non-static Member Functions (`#nonstatic-member-function`)

- Hidden `this` parameter refers to the object.
- `this.member` is optional — bare member also works.
- Default: read-only access.
- `[mutating]` attribute required for write access.
- Cannot be invoked without an object.

### Properties (`#property`)

- `property` with `get` and `set` accessors.
- Read calls `get`; write calls `set`.
- Read-only (get only), write-only (set only), or read/write.
- `get` accepts no parameters; parentheses are optional.
- `set` parameter optional; defaults to implicit `newValue` of the property's type.
- Modern and traditional syntax forms (traditional: `property var-decl { ... }`).

### Accessing Members and Nested Types (`#accessing-members-and-nested-types`)

- Dot `.` operator for all member access.
- `::` deprecated.

### Subscript Operator (`#subscript-op`)

- `__subscript` declaration.
- `get` invoked on read; `set` invoked on write.
- Any number of parameters (including zero).
- Multiple `__subscript` overloads with different signatures.
- Overload resolution same as functions.

### Function Call Operator (`#function-call-op`)

- `operator () (...)` declaration.
- Applies parameters to an object as if it were a function.
- Multiple overloads with different signatures.

### Memory Layout (`#natural-layout`, `#c-style-layout`, `#d3d-constant-buffer-layout`)

- Natural layout: alignment = max(1, member alignments); members in declaration order; no tail-padding.
- C-style layout: adds tail-padding to round size up to alignment.
- D3D constant buffer layout: minimum alignment 16; improper straddle rule.

### This Type Keyword (section at end of doc, no explicit anchor)

- `This` in a struct body refers to the enclosing struct type.

## What counts as a claim vs. non-normative

**Claims**: grammar productions; "must" / "shall" / "is rejected" sentences; rules for member
behavior; layout algorithm; deprecation warnings as behavioral claims.

**Non-normative**: remarks (📝), examples in code blocks, cross-references to other docs
(generics, modules, variables, access control) where the claim's detail lives elsewhere.

## What NOT to test here

- Memory layout numeric values (byte offsets, alignment values) — not observable from slangc CLI
  without the reflection API.
- D3D constant buffer layout specifics — requires specific GPU-tier tests.
- `extern struct` / `export struct` forms — require multi-file module linking (`needs-multi-file-test`).
- `where`-clause generics — covered in the generics bundle.
- `ref` accessor on properties/subscripts — doc explicitly warns this is internal and subject to change.
- struct inheriting from another struct — doc says this is deprecated and may not work as expected.
- `private` member access diagnostics — need nested struct in same scope context that is more complex.
- Module-scope `static` data members via `slangi` — known compiler limitation (slangi VM bytecode
  emitter does not support global-inst for module-scope static members, see finding
  `slangi-vm-global-var-bytecode-unsupported.yaml`). Use `-target hlsl` emission instead.
