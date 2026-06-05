# Prompt: docs/generated/tests/conformance/declarations/

See [`_common.md`](_common.md).

## Target

Bundle at `docs/generated/tests/conformance/declarations/`,
anchored to
[`docs/language-reference/declarations.md`](../../../../language-reference/declarations.md).

## Sub-areas and claim extraction strategy

The doc covers 16 major declaration forms with normative sub-sections.

### Modules (`#modules`)

A module is one or more source units compiled together. Declarations may freely
refer to others anywhere in the module. The order-independence rule and
cross-source-unit visibility are normative structural claims.

### Imports (`#imports`)

`import` declarations bring a module's declarations into the current source unit's
scope, but not into modules that later import the current module. Compound
import names are supported. The non-transitive scoping rule is normative.
`__exported import` is experimental (non-normative for this bundle).

### Variables (`#variables`)

`let` / `var` / traditional `const` syntax. `let` is immutable; `var` is mutable.
Missing type + missing initializer is an error. `let` without an initializer is an
error. `var` with an explicit type but no initializer is allowed. Every
sub-category (global constant, static global, global shader parameter,
function-scope constant, function-scope static, local variable) is a separate
normative claim.

### Functions (`#functions`, `#parameters`, `#body`)

`func` keyword syntax with `name: type` parameters and `-> result-type` clause.
Parameter directions: `in` (default), `out`, `in out` / `inout`. Default
parameter values. Forward declarations (no body, terminated with `;`). Elided
`void` result type. Traditional C-style syntax is also normative.

The doc claims `in out` direction works with the `func` keyword syntax; the
compiler currently crashes with an internal error when this is used — a finding
is recorded and the test for that specific combination is omitted as a
compiler-bug-pending claim.

### Structure Types (`#structure-types`)

Covered in sibling bundle `conformance/types-struct`. Only claims unique to this
doc (struct as `struct … { } varDecl;` inline pattern, no-semicolon rule) are
tested here; remainder are `out-of-bundle`.

### Enumeration Types (`#enumeration-types`)

Enum declaration, case tag values (first=0, subsequent=prev+1, explicit),
implicit conversion to tag type, explicit conversion back, tag type specification,
default tag type is `int`.

### Type Aliases (`#type-aliases`)

`typealias` and traditional `typedef` syntax both introduce a name equivalent to
a type. Both are normative.

### Constant Buffers and Texture Buffers (`#constant-buffers-and-texture-buffers`)

`cbuffer` expands to a `ConstantBuffer<AnonType>` with a `__transparent`
modifier. `tbuffer` expands similarly to `TextureBuffer<T>`. Both are
normative compatibility features observable in emitted HLSL.

### Interfaces (`#interfaces`)

Interface declarations with `interface` keyword. Requirements (functions,
initializers, subscripts, associated types). Interface inheritance. The claim
that requirement bodies are not allowed. Error when overlapping conformances.

### Associated Types (`#associated-types`)

`associatedtype` in interface bodies. Error when declared outside an interface.
Inheritance clause on an associated type constrains the satisfying concrete type.

### Initializers (`#initializers`)

`__init` syntax in struct or interface bodies. Invoked by calling the type as
a function. Access to implicit `this`. Cannot be `static`. Cannot have result
type clause.

### Subscripts (`#subscripts`)

`__subscript` syntax. `get` accessor. Accessed via `[]`. Result type clause is
mandatory.

### Extensions (`#extensions`)

`extension` keyword adds behavior to existing type. Cannot include variable
declarations. Members accessed through the extended type. Inheritance clause for
new conformances. Must not declare overlapping conformances. Currently only
applies to structure types.

### Generics (`#generics`)

Generic parameter list `<T>`, type parameter constraints `T : IFoo`, value
parameter `let N : int`. Default generic parameter values. Explicit
specialization `<int, 3>`. Implicit specialization via inference.

## What counts as a claim vs. non-normative

**Claims**: grammar productions; "must" / "shall" / "is rejected" / "is an error"
sentences; table rows; rule statements ("the first case … uses value 0").

**Non-normative**: `> Note:` callouts (implementation notes, provisional syntax,
experimental features, future-language notes), code examples that just illustrate
already-stated rules.

## What NOT to test here

- Module cross-file compilation (`import`) — requires multi-file tests.
- `__exported import` — marked experimental in the doc.
- Entry-point semantics — separate doc.
- `struct` member-level claims (mutating, property, subscript, layout) — in
  `conformance/types-struct`.
- Mixing `#include` and `import` — implementation-defined behavior in the Note.
- `func` + `in out` / `inout` parameters — internal compiler error filed as
  finding `declarations-func-inout-ice`.
- `tbuffer` semantic on SPIR-V / other non-HLSL targets — not mentioned by doc.
- Interface method default bodies — doc says "must not have bodies" but the
  compiler does not currently reject them; recorded as doc gap.
- Generic extensions — doc states "no user-exposed syntax" (non-normative absence).
- struct-to-struct inheritance — deprecated, tested in types-struct bundle.
