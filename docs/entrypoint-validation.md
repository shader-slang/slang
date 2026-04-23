# Entry Point Signature Validation

This document describes the entry-point signature validation system in the
Slang compiler frontend. The system catches invalid types in entry-point
parameters and return types during semantic checking, before IR generation,
preventing crashes and invalid code generation downstream.

## Scope

The validation performed here is **strictly limited to entry-point parameters
and return types**. Types that are valid internally but wrong as a shader
varying are only rejected when they appear at an entry-point boundary; all
other uses are left unaffected.

## Motivation

Without frontend validation, certain types can appear as entry-point varying
parameters or return types and cause the compiler to either crash (segfault
in IR passes like `legalizeEntryPointsForGLSL`) or silently generate invalid
SPIR-V. The validation system rejects these cases with clear error
diagnostics.

## Architecture

Validation lives in `validateEntryPoint()` in
`source/slang/slang-check-shader.cpp`. After the existing return-type checks
(resource types, array types), a validation pass walks each varying parameter
type and the return type, checking them against a **data-driven rule table**.

### Rule Table

Each rule specifies:

- A **type predicate**: a function that returns `true` if the type matches
  (is invalid).
- A **reason string**: human-readable explanation for the diagnostic.
- An optional **target predicate**: when non-null, the rule only applies when
  at least one compilation target matches this predicate (e.g. SPIR-V only).
- An optional **stage predicate**: when non-null, the rule only applies when
  the entry-point stage matches (e.g. only stages that use interface-block
  varyings).

```
Rule                    Type Predicate                          Target      Stage
----                    --------------                          ------      -----
DifferentialPair<T>     as<DifferentialPairType> or             all         all varying
DifferentialPtrPair<T>  as<DifferentialPtrPairType>
Atomic<T>               as<AtomicType>                          all         all varying
CoopVec<T, N>           IntrinsicTypeModifier with              all         interface-block
                        irOp == kIROp_CoopVectorType
                        (or as<CoopVectorExpressionType>)
CoopMat<T,S,M,N,R>      IntrinsicTypeModifier with              all         interface-block
                        irOp == kIROp_CoopMatrixType
vector<bool>            VectorExpressionType with               SPIR-V      interface-block
                        Bool element type
matrix<T, R, C>         MatrixExpressionType with row or        all         interface-block
(out-of-range dims)     column count outside 1..4
```

"Interface-block" means stages that use interface-block-style varyings in
SPIR-V/GLSL: `Vertex`, `Fragment`, `Geometry`, `Hull`, `Domain`, and `Mesh`.
`Amplification` shaders output via `TaskPayloadWorkgroupEXT` rather than
interface blocks, so they are not in this set. Ray-tracing payload/attribute
stages (`Miss`, `AnyHit`, `ClosestHit`, `Callable`, `RayGen`, `Intersection`)
and `Compute` also do not use interface blocks. These non-interface-block
stages are exempt from the rules whose stage predicate is limited to
interface-block varyings; rules with no stage predicate (e.g.
`DifferentialPair`, `Atomic`) still apply to every stage that accepts
varying parameters.

New rules are added by appending entries to the static array. No other code
changes are needed to add a new "type X is not a valid varying" rule.

### Recursive Type Traversal

The type walker recursively descends into:

- **Array element types**: `DifferentialPair<float>[4]` is also caught. This
  check is performed *before* the struct-field recursion because
  `ArrayExpressionType` inherits from `DeclRefType` and its decl is the
  builtin `Array` struct; without ordering array first we would walk the
  internal fields of `Array` instead of the element type.
- **Struct fields**: If a struct is used as a varying parameter, each field's
  type is checked against the rules. This catches cases like
  `struct Foo { DifferentialPair<float> x; }` used as a parameter. Fields are
  iterated through the struct's `DeclRef` so that generic type-parameter
  substitutions are applied; `Wrapper<DifferentialPair<float>>` with
  `struct Wrapper<T> { T x; }` is also caught.
- **ModifiedType wrappers**: Unwrapped transparently.

Cycle detection prevents infinite recursion on self-referential struct types,
and a recursion-depth bound (`kMaxTypeNestingDepth`) handles recursive
generic-struct instantiations whose `Type*` pointer changes at every nesting
level.

### Uniform vs. Varying

Only varying parameters are checked. Parameters that are uniform (explicit
`uniform` modifier, or resource types like textures/samplers/buffers detected
by `isUniformParameterType()`) are skipped. An `Atomic<T>` inside a constant
buffer is valid; it only fails as a varying input/output.

### Target-Specific Rules

For rules with a target predicate, the validation checks all compilation
targets in `linkage->targets`. If any target matches the predicate, the error
is emitted. This handles the case where a user compiles for both SPIR-V and
HLSL -- the SPIR-V restriction still applies.

## Diagnostics

| Code  | Name | Description |
|-------|------|-------------|
| 38050 | `invalid-entry-point-varying-type` | Type cannot be used as entry-point varying parameter or return type |
| 38051 | `invalid-entry-point-varying-type-for-target` | Type cannot be used as entry-point varying for a specific target |

## Extending the System

### Adding a new "never valid as varying" type

Add an entry to `kEntryPointVaryingTypeRules[]` in `slang-check-shader.cpp`:

```cpp
{
    [](Type* type) -> bool { return as<YourType>(type) != nullptr; },
    "YourType is not a valid varying type",
    nullptr,  // target predicate (nullptr = all targets)
    nullptr,  // stage predicate  (nullptr = all varying stages)
},
```

### Adding a target-specific restriction

Same as above, but provide a target predicate:

```cpp
{
    [](Type* type) -> bool { /* match logic */ },
    "reason message",
    [](CodeGenTarget target) -> bool { return isSPIRV(target); },
    nullptr,
},
```

### Adding a stage-specific restriction

Same as above, but provide a stage predicate:

```cpp
{
    [](Type* type) -> bool { /* match logic */ },
    "reason message",
    nullptr,
    [](Stage stage) -> bool { return stage == Stage::Fragment; },
},
```
